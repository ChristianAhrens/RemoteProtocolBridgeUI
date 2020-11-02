/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of RemoteProtocolBridge.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/

#include "ProtocolComponent.h"

#include "NodeComponent.h"
#include "ConfigComponents/ProtocolConfigComponents/ProtocolConfigComponents.h"
#include "ProcessingEngine/ProcessingEngineConfig.h"


// **************************************************************************************
//    class ProtocolGroupComponent
// **************************************************************************************
/**
 * Constructor
 */
ProtocolGroupComponent::ProtocolGroupComponent(const ProtocolRole& role)
	: GroupComponent()
{
	m_NodeId = 0;
	m_ProtocolRole = role;

	/******************************************************/
	m_AddProtocolButton = std::make_unique<ImageButton>();
	m_AddProtocolButton->addListener(this);
	addAndMakeVisible(m_AddProtocolButton.get());
	Image AddNormalImage = ImageCache::getFromMemory(BinaryData::AddNormalImage_png, BinaryData::AddNormalImage_pngSize);
	Image AddOverImage = ImageCache::getFromMemory(BinaryData::AddOverImage_png, BinaryData::AddOverImage_pngSize);
	Image AddDownImage = ImageCache::getFromMemory(BinaryData::AddDownImage_png, BinaryData::AddDownImage_pngSize);
	m_AddProtocolButton->setImages(false, true, true,
		AddNormalImage, 1.0, Colours::transparentBlack,
		AddOverImage, 1.0, Colours::transparentBlack,
		AddDownImage, 1.0, Colours::transparentBlack);

	m_RemoveProtocolButton = std::make_unique<ImageButton>();
	m_RemoveProtocolButton->addListener(this);
	addAndMakeVisible(m_RemoveProtocolButton.get());
	Image RemoveNormalImage = ImageCache::getFromMemory(BinaryData::RemoveNormalImage_png, BinaryData::RemoveNormalImage_pngSize);
	Image RemoveOverImage = ImageCache::getFromMemory(BinaryData::RemoveOverImage_png, BinaryData::RemoveOverImage_pngSize);
	Image RemoveDownImage = ImageCache::getFromMemory(BinaryData::RemoveDownImage_png, BinaryData::RemoveDownImage_pngSize);
	m_RemoveProtocolButton->setImages(false, true, true,
		RemoveNormalImage, 1.0, Colours::transparentBlack,
		RemoveOverImage, 1.0, Colours::transparentBlack,
		RemoveDownImage, 1.0, Colours::transparentBlack);

	/******************************************************/
}

/**
 * Destructor
 */
ProtocolGroupComponent::~ProtocolGroupComponent()
{
	jassert(m_ProtocolComponents.size() == m_ProtocolIds.size());

	for (const ProtocolId& PId : m_ProtocolIds)
	{
		removeChildComponent(m_ProtocolComponents[PId].get());
		m_ProtocolComponents[PId].reset();
		m_ProtocolComponents.erase(PId);
		m_ProtocolIds.removeAllInstancesOf(PId);
	}

	m_ProtocolComponents.clear();
}

/**
 * Overloaded method to resize contents
 */
void ProtocolGroupComponent::resized()
{
	GroupComponent::resized();

	/*Add/Remove Buttons*/
	int yPositionAddRemButtons = getHeight() - UIS_ElmSize - UIS_Margin_s;
	int xPositionAddRemButtons = getWidth() - UIS_ElmSize - UIS_Margin_s;
	if (m_AddProtocolButton)
		m_AddProtocolButton->setBounds(xPositionAddRemButtons, yPositionAddRemButtons, UIS_ElmSize - UIS_Margin_s, UIS_ElmSize - UIS_Margin_s);

	xPositionAddRemButtons -= UIS_ElmSize;
	if (m_RemoveProtocolButton)
		m_RemoveProtocolButton->setBounds(xPositionAddRemButtons, yPositionAddRemButtons, UIS_ElmSize - UIS_Margin_s, UIS_ElmSize - UIS_Margin_s);

	/*Dynamically sized elements*/
	int yOffset = UIS_Margin_m;
	for (std::map<ProtocolId, std::unique_ptr<ProtocolComponent>>::iterator piter = m_ProtocolComponents.begin(); piter != m_ProtocolComponents.end(); ++piter)
	{
		if(piter->second)
			piter->second->setBounds(UIS_Margin_s, UIS_Margin_s + yOffset, getWidth() - 2 * UIS_Margin_s, UIS_ElmSize);

		yOffset += UIS_ElmSize + UIS_Margin_s;
	}
}

/**
 * Method to gather data from ui input elements and dump them to given configuration object.
 *
 * @param config	The configuration object to be filled with contents from ui elements of this node
 */
std::unique_ptr<XmlElement> ProtocolGroupComponent::createStateXml()
{
	auto protocolsXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	protocolsXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), static_cast<int>(m_NodeId));

	for (auto PId : m_ProtocolIds)
	{
		if (m_ProtocolComponents.count(PId) && m_ProtocolComponents[PId])
		{
			protocolsXmlElement->addChildElement(m_ProtocolComponents[PId]->createStateXml().release());
		}
	}

	return protocolsXmlElement;
}

/**
 * Method to update ui input elements with data to show from config
 *
 * @param parentNodeId	The node id of the parent node object
 * @param protocolIds	The array of ids of protocols to handle in this component
 * @param config		The configuration object to extract the data to show on ui from
 * @return	The calculated theoretically required size for this component
 */
bool ProtocolGroupComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE))
		return false;

	m_NodeId = stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));

	auto protocolRoles = std::map<ProtocolId, ProtocolRole>{};
	auto protocolXmls = std::map<ProtocolId, XmlElement*>{};
	XmlElement* protocolXmlElement = stateXml->getChildByName((m_ProtocolRole == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	while (protocolXmlElement != nullptr)
	{
		protocolRoles.insert(std::make_pair(static_cast<ProtocolId>(protocolXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID))), m_ProtocolRole));
		protocolXmls.insert(std::make_pair(static_cast<ProtocolId>(protocolXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID))), protocolXmlElement));

		protocolXmlElement = protocolXmlElement->getNextElementWithTagName((m_ProtocolRole == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	}

	// go through current ui node boxes to find out which ones need to be removed/destroyed
	Array<ProtocolId> PIdsToRemove;
	for(auto const & protocolComponentKV : m_ProtocolComponents)
		if (protocolRoles.count(protocolComponentKV.first) == 0)
			PIdsToRemove.add(protocolComponentKV.first);

	for (const ProtocolId& PId : PIdsToRemove)
	{
		if (m_ProtocolComponents.count(PId))
		{
			removeChildComponent(m_ProtocolComponents[PId].get());
			m_ProtocolComponents[PId].reset();
			m_ProtocolComponents.erase(PId);
		}
		m_ProtocolIds.removeAllInstancesOf(PId);
	}

	// now go through all protocol ids currently in config and create those protocols
	// that do not already exist or simply update those that are present
	for (auto const& PId : protocolRoles)
	{
		if (!m_ProtocolIds.contains(PId.first))
		{
			m_ProtocolIds.add(PId.first);
	
			ProtocolComponent* Protocol = new ProtocolComponent(m_NodeId, PId.first, PId.second);
			Protocol->setStateXml(protocolXmls.at(PId.first));
			addAndMakeVisible(Protocol);
	
			m_ProtocolComponents[PId.first] = std::unique_ptr<ProtocolComponent>(Protocol);
		}
	}

	return true;
}

int ProtocolGroupComponent::GetCurrentRequiredHeight()
{
	// top margin v space
	int requiredHeight = UIS_Margin_m;

	for(auto const & protocolComponent : m_ProtocolComponents)
		requiredHeight += protocolComponent.second->GetCurrentRequiredHeight();
	
	// margin v space
	requiredHeight += UIS_Margin_s;
	
	// reserve some v space for +- buttons
	requiredHeight += (UIS_ElmSize + UIS_Margin_m);
	
	// margin v space
	requiredHeight += UIS_Margin_m;
	
	return requiredHeight;
}

/**
 * Getter for this protocol components' parent NodeId.
 *
 * @return	The NodeID
 */
NodeId ProtocolGroupComponent::GetNodeId()
{
	return m_NodeId;
}

/**
 * Getter for the ids of the protocols in this group component
 * 
 * @return	The list of protocol ids
 */
const Array<ProtocolId>& ProtocolGroupComponent::GetProtocolIds()
{
	return m_ProtocolIds;
}

/**
 * 
 */
void ProtocolGroupComponent::RemoveProtocol(const ProtocolId& PId)
{
	if (m_ProtocolComponents.count(PId) && m_ProtocolComponents[PId])
	{
		m_ProtocolComponents.erase(PId);
		m_ProtocolIds.remove(m_ProtocolIds.indexOf(PId));
		triggerConfigurationUpdate(true);
	}
}

/**
 * Overloaded method called by button objects on click events.
 * All internal button objects are registered to trigger this by calling
 * their ::addListener method with this object as argument
 *
 * @param button	The button object that has been clicked
 */
void ProtocolGroupComponent::buttonClicked(Button* button)
{
	if (button == m_AddProtocolButton.get())
	{

		auto config = ProcessingEngineConfig::getInstance();
		if (config)
		{
			auto currentState = config->getConfigState();
			if (currentState)
			{
				auto nodeXmlElement = currentState->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(m_NodeId));
				if (nodeXmlElement)
				{
					nodeXmlElement->addChildElement(ProcessingEngineConfig::GetDefaultProtocol(m_ProtocolRole).release());
					setStateXml(nodeXmlElement);
					triggerConfigurationUpdate(true);
				}
			}
		}
	}
	else if (button == m_RemoveProtocolButton.get())
	{
		RemoveProtocol(m_ProtocolIds.getLast());
	}
}


// **************************************************************************************
//    class ProtocolComponent
// **************************************************************************************
/**
 * Constructor
 */
ProtocolComponent::ProtocolComponent(const NodeId& NId, const ProtocolId& PId, const ProtocolRole& role)
	: Component()
{
	m_NodeId = NId;
	m_ProtocolId = PId;
	m_Role = role;

	m_protocolXmlElement = std::make_unique<XmlElement>(m_Role == ProtocolRole::PR_A ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

	/******************************************************/
	m_ProtocolDrop = std::make_unique<ComboBox>();
	m_ProtocolDrop->addListener(this);
	addAndMakeVisible(m_ProtocolDrop.get());
	m_ProtocolDrop->addItem(ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol), PT_OSCProtocol);
	//m_ProtocolDrop->addItem(ProcessingEngineConfig::ProtocolTypeToString(PT_OCAProtocol), PT_OCAProtocol); // not yet implemented, feel free to step in
	m_ProtocolDrop->addItem(ProcessingEngineConfig::ProtocolTypeToString(PT_RTTrPMProtocol), PT_RTTrPMProtocol);
	//m_ProtocolDrop->addItem(ProcessingEngineConfig::ProtocolTypeToString(PT_DummyMidiProtocol), PT_DummyMidiProtocol); // not yet implemented, feel free to step in
	m_ProtocolDrop->setColour(Label::textColourId, Colours::white);
	m_ProtocolDrop->setJustificationType(Justification::right);

	m_ProtocolLabel = std::make_unique<Label>();
	addAndMakeVisible(m_ProtocolLabel.get());
	m_ProtocolLabel->setText("Protocol " + String(PId), dontSendNotification);
	m_ProtocolLabel->setColour(Label::textColourId, Colours::white);
	m_ProtocolLabel->setJustificationType(Justification::right);
	m_ProtocolLabel->attachToComponent(m_ProtocolDrop.get(), true);

	m_IpEdit = std::make_unique<TextEditor>();
	m_IpEdit->addListener(this);
	addAndMakeVisible(m_IpEdit.get());
	m_IpEdit->setColour(Label::textColourId, Colours::white);
    
    m_ZeroconfIpDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
    addAndMakeVisible(m_ZeroconfIpDiscovery.get());
    m_ZeroconfIpDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info) { handleOnServiceSelected(type, info); };

	m_ProtocolConfigEditButton = std::make_unique<TextButton>();
	m_ProtocolConfigEditButton->addListener(this);
	addAndMakeVisible(m_ProtocolConfigEditButton.get());
	m_ProtocolConfigEditButton->setButtonText("Configuration");
	m_ProtocolConfigEditButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
	m_ProtocolConfigEditButton->setColour(Label::textColourId, Colours::white);

	m_ProtocolConfigDialog = 0;

}

/**
 * Destructor
 */
ProtocolComponent::~ProtocolComponent()
{
}

/**
 * Helper method to set the current protocol type to use for zeroconf
 * @param type	The new protocol type
 * @return True on success, false on failure
 */
bool ProtocolComponent::setZeroConfProtocolType(ProtocolType type)
{
	auto hostPortXmlElement = m_protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
	{
		auto protocolHostPort = hostPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
		if (m_ZeroconfIpDiscovery)
		{
			switch (type)
			{
			case PT_OSCProtocol:
				m_ZeroconfIpDiscovery->clearServices();
				m_ZeroconfIpDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZST_OSC, static_cast<unsigned short>(protocolHostPort));
				m_ZeroconfIpDiscovery->setVisible(true);
				break;
			case PT_OCAProtocol:
				m_ZeroconfIpDiscovery->clearServices();
				m_ZeroconfIpDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZST_OCA, static_cast<unsigned short>(protocolHostPort));
				m_ZeroconfIpDiscovery->setVisible(true);
				break;
			case PT_RTTrPMProtocol:
			case PT_DummyMidiProtocol:
				m_ZeroconfIpDiscovery->clearServices();
				m_ZeroconfIpDiscovery->setVisible(false);
				break;
			case PT_UserMAX:
			case PT_Invalid:
				return false;
			}
		}
		else
			return false;
	}
	else
		return false;

	resized();

	return true;
}

/**
 * Overloaded method to resize contents
 */
void ProtocolComponent::resized()
{
	Component::resized();

	auto useZeroconf = false;
	if (m_ProtocolDrop)
	{
		auto selectedType = static_cast<ProtocolType>(m_ProtocolDrop->getSelectedId());
		switch (selectedType)
		{
		case PT_OSCProtocol:
		case PT_OCAProtocol:
			useZeroconf = true;
			break;
		case PT_RTTrPMProtocol:
		case PT_DummyMidiProtocol:
		case PT_UserMAX:
		case PT_Invalid:
			useZeroconf = false;
			break;
		}
	}

	int xPos = getWidth() - UIS_ConfigButtonWidth - UIS_Margin_s;
	if (m_ProtocolConfigEditButton)
		m_ProtocolConfigEditButton->setBounds(xPos, 0, UIS_ConfigButtonWidth, UIS_ElmSize);

	int IpEditWidth = getWidth() - UIS_ProtocolLabelWidth - UIS_Margin_s - UIS_ProtocolDropWidth - UIS_Margin_s - (useZeroconf ? (UIS_ElmSize + UIS_Margin_s) : 0) - UIS_ConfigButtonWidth - UIS_Margin_s;
	xPos = (UIS_ProtocolLabelWidth + UIS_Margin_s + UIS_ProtocolDropWidth);
	if (m_IpEdit)
		m_IpEdit->setBounds(xPos, 0, IpEditWidth, UIS_ElmSize);

	xPos += IpEditWidth + UIS_Margin_s;
	if (m_ZeroconfIpDiscovery && useZeroconf)
	{
		m_ZeroconfIpDiscovery->setBounds(xPos, 0, UIS_ElmSize, UIS_ElmSize);
		m_ZeroconfIpDiscovery->resized();
	}
	
	xPos = UIS_ProtocolLabelWidth;
	if (m_ProtocolDrop)
		m_ProtocolDrop->setBounds(UIS_ProtocolLabelWidth, 0, UIS_ProtocolDropWidth, UIS_ElmSize);
}

/**
 * Method to be called by child windows when closed, to enshure
 * button states and the internal
 * window object is invalidated to avoid accessviolation
 *
 * @param childWindow	The DialogWindow object that has been triggered to close
 */
void ProtocolComponent::childWindowCloseTriggered(DialogWindow* childWindow)
{
	ignoreUnused(childWindow);

	ToggleOpenCloseProtocolConfig(m_ProtocolConfigEditButton.get());
}

/**
 * Method to dump the ui elemtents' current data to given data struct
 *
 * @param protocolData	The data struct to dump the ui values into
 */
std::unique_ptr<XmlElement> ProtocolComponent::createStateXml()
{
	m_protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), static_cast<int>(m_ProtocolId));
	m_protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(static_cast<ProtocolType>(m_ProtocolDrop->getSelectedId())));

	auto ipAdressEdit = m_protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressEdit)
		ipAdressEdit->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), m_IpEdit->getText());

	return std::make_unique<XmlElement>(*m_protocolXmlElement);
}

/**
 * Method to update ui input elements with data to show from config
 *
 * @param protocolData	The config data structure to use to refresh what the ui displays
 * @return	The calculated theoretically required size for this component
 */
bool ProtocolComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ((m_Role == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB)))
		return false;

	m_ProtocolId = stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));

	m_protocolXmlElement = std::make_unique<XmlElement>(*stateXml);

	auto protocolType = ProcessingEngineConfig::ProtocolTypeFromString(m_protocolXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE)));
	if (m_ProtocolDrop)
		m_ProtocolDrop->setSelectedId(protocolType, dontSendNotification);
	else
		return false;

	auto ipXmlElement = m_protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipXmlElement)
	{
		auto protocolIpAdress = ipXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS));
		if (m_IpEdit)
			m_IpEdit->setText(protocolIpAdress);
		else
			return false;
	}
	else
		return false;

	return setZeroConfProtocolType(protocolType);
}

/**
 *
 */
int ProtocolComponent::GetCurrentRequiredHeight()
{
	return UIS_ElmSize;
}

/**
 * Getter for this protocol components protocolId
 *
 * @return	This protocols' protocolId
 */
ProtocolId ProtocolComponent::GetProtocolId()
{
	return m_ProtocolId;
}

/**
 * Overloaded method called by button objects on click events.
 * All internal button objects are registered to trigger this by calling
 * their ::addListener method with this object as argument
 *
 * @param button	The button object that has been clicked
 */
void ProtocolComponent::buttonClicked(Button* button)
{
	if (button == m_ProtocolConfigEditButton.get())
	{
		ToggleOpenCloseProtocolConfig(m_ProtocolConfigEditButton.get());
	}
}

/**
 * Overloaded method called by ComboBox objects on change events.
 * This is similar to ::buttonClicked but originates from inherited ComboBox::Listener
 *
 * @param comboBox	The comboBox object that has been changed
 */
void ProtocolComponent::comboBoxChanged(ComboBox* comboBox)
{
	if (comboBox == m_ProtocolDrop.get())
	{
		auto protocolType = static_cast<ProtocolType>(m_ProtocolDrop->getSelectedId());
		setZeroConfProtocolType(protocolType);
	}

	auto config = ProcessingEngineConfig::getInstance();
	if (config)
		config->triggerConfigurationDump();
}

/**
 * Overloaded method called by TextEditor objects on textchange events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has been changed
 */
void ProtocolComponent::textEditorTextChanged(TextEditor& textEdit)
{
	ignoreUnused(textEdit);
}

/**
 * Overloaded method called by TextEditor objects on keypress events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has received keypress
 */
void ProtocolComponent::textEditorReturnKeyPressed(TextEditor& textEdit)
{
	ignoreUnused(textEdit);

	auto config = ProcessingEngineConfig::getInstance();
	if (config)
		config->triggerConfigurationDump();
}

/**
 * Overloaded method called by TextEditor objects on keypress events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has received keypress
 */
void ProtocolComponent::textEditorEscapeKeyPressed(TextEditor& textEdit)
{
	ignoreUnused(textEdit);
}

/**
 * Overloaded method called by TextEditor objects on focuslost events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has lost focus
 */
void ProtocolComponent::textEditorFocusLost(TextEditor& textEdit)
{
	ignoreUnused(textEdit);
}

/**
 * Helper method to do the toggling of config dialog on respective buttonClicked
 *
 * @param button	The button object that has been clicked
 */
void ProtocolComponent::ToggleOpenCloseProtocolConfig(Button* button)
{
	jassert(button);
	if (!button)
		return;

	// if the config dialog exists, this is a uncheck (close) click,
	// which means we have to process edited data
	if (m_ProtocolConfigDialog)
	{
		if (m_protocolXmlElement)
		{
			m_protocolXmlElement = m_ProtocolConfigDialog->createStateXml();
			triggerConfigurationUpdate(true);
		}

		button->setColour(TextButton::buttonColourId, Colours::dimgrey);
		button->setColour(Label::textColourId, Colours::white);

		m_ProtocolConfigDialog.reset();
	}
	// otherwise we have to create the dialog and show it
	else
	{
		if (m_protocolXmlElement)
		{
			ProtocolType protocolType = ProcessingEngineConfig::ProtocolTypeFromString(m_protocolXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE)));

			String dialogTitle = ProcessingEngineConfig::ProtocolTypeToString(protocolType) + " protocol configuration (Node Id" + String(m_NodeId) + ", Protocol Id" + String(m_ProtocolId) + ")";

			m_ProtocolConfigDialog = std::make_unique<ProtocolConfigWindow>(dialogTitle, Colours::dimgrey, false, m_NodeId, m_ProtocolId, m_Role, protocolType);
			m_ProtocolConfigDialog->AddListener(this);
			m_ProtocolConfigDialog->setResizable(true, true);
			m_ProtocolConfigDialog->setUsingNativeTitleBar(true);
			m_ProtocolConfigDialog->setVisible(true);
			m_ProtocolConfigDialog->setStateXml(m_protocolXmlElement.get());
#if defined JUCE_IOS ||  defined JUCE_ANDROID
            m_ProtocolConfigDialog->setFullScreen(true);
#else
			const std::pair<int, int> size = m_ProtocolConfigDialog->GetSuggestedSize();
			m_ProtocolConfigDialog->setResizeLimits(size.first, size.second, size.first, size.second);
			m_ProtocolConfigDialog->setBounds(Rectangle<int>(getScreenBounds().getX(), getScreenBounds().getY(), size.first, size.second));
#endif
			button->setColour(TextButton::buttonColourId, Colours::lightblue);
			button->setColour(Label::textColourId, Colours::dimgrey);
		}
	}
}

/**
 * Helper method to do the handling of selected zeroconf services and fill the service's ip address in to our ip edit
 *
 * @param serviceType    The service that was selected
 * @param info     The detailled info on the selected service
 */
void ProtocolComponent::handleOnServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType serviceType, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info)
{
	ignoreUnused(serviceType);

    if (info && m_IpEdit)
        m_IpEdit->setText(info->ip);
}
