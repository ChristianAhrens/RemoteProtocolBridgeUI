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

#include "NodeComponent.h"

#include "MainRemoteProtocolBridgeComponent.h"
#include "ProtocolComponent.h"
#include "ConfigComponents/ObjectHandlingConfigComponents/ObjectHandlingConfigComponents.h"

#include "ProcessingEngine/ProcessingEngine.h"
#include "ProcessingEngine/ProcessingEngineConfig.h"


// **************************************************************************************
//    class NodeComponent
// **************************************************************************************
/**
 * Constructor
 */
NodeComponent::NodeComponent(NodeId NId)
	: GroupComponent()
{
	GroupComponent::setColour(outlineColourId, Colours::white);

    m_NodeId = NId;

	m_protocolsAComponent = std::make_unique<ProtocolGroupComponent>(ProtocolRole::PR_A);
	addAndMakeVisible(m_protocolsAComponent.get());
	m_protocolsAComponent->setText("Role A");

	m_protocolsBComponent = std::make_unique<ProtocolGroupComponent>(ProtocolRole::PR_B);
	addAndMakeVisible(m_protocolsBComponent.get());
	m_protocolsBComponent->setText("Role B");

	m_NodeModeDrop = std::make_unique<ComboBox>();
	m_NodeModeDrop->addListener(this);
	addAndMakeVisible(m_NodeModeDrop.get());
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Bypass), OHM_Bypass);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Remap_A_X_Y_to_B_XY), OHM_Remap_A_X_Y_to_B_XY);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mux_nA_to_mB), OHM_Mux_nA_to_mB);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mux_nA_to_mB_withValFilter), OHM_Mux_nA_to_mB_withValFilter);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mirror_dualA_withValFilter), OHM_Mirror_dualA_withValFilter);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges), OHM_Forward_only_valueChanges);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_A1active_withValFilter), OHM_A1active_withValFilter);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_A2active_withValFilter), OHM_A2active_withValFilter);
	m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_DS100_DeviceSimulation), OHM_DS100_DeviceSimulation);
    m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_A_to_B_only), OHM_Forward_A_to_B_only);
    m_NodeModeDrop->addItem(ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Reverse_B_to_A_only), OHM_Reverse_B_to_A_only);
	m_NodeModeDrop->setColour(Label::textColourId, Colours::white);
	m_NodeModeDrop->setJustificationType(Justification::right);

	m_NodeModeLabel = std::make_unique<Label>();
	addAndMakeVisible(m_NodeModeLabel.get());
	m_NodeModeLabel->setText("Data handling", dontSendNotification);
	m_NodeModeLabel->setColour(Label::textColourId, Colours::white);
	m_NodeModeLabel->setJustificationType(Justification::right);
	m_NodeModeLabel->attachToComponent(m_NodeModeDrop.get(), true);
	
	m_OHMConfigEditButton = std::make_unique<TextButton>();
	m_OHMConfigEditButton->addListener(this);
	addAndMakeVisible(m_OHMConfigEditButton.get());
	m_OHMConfigEditButton->setButtonText("Configuration");
	m_OHMConfigEditButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
	m_OHMConfigEditButton->setColour(Label::textColourId, Colours::white);

	m_OHMConfigDialog = 0;
}

/**
 * Destructor
 */
NodeComponent::~NodeComponent()
{
}

/**
 * Overloaded method to resize contents
 */
void NodeComponent::resized()
{
	GroupComponent::resized();

	int windowWidth = getWidth();
	int windowHeight = getHeight();

	/*Config, TrafficLogging, Start/Stop Buttons*/
	int yPositionModeDrop = windowHeight - UIS_ElmSize - UIS_Margin_m;
	if (m_NodeModeDrop)
		m_NodeModeDrop->setBounds(UIS_AttachedLabelWidth, yPositionModeDrop, windowWidth - UIS_NodeModeDropWidthOffset - UIS_ConfigButtonWidth - UIS_Margin_m, UIS_ElmSize);
	m_OHMConfigEditButton->setBounds(windowWidth - UIS_ConfigButtonWidth - UIS_Margin_m - UIS_Margin_s, yPositionModeDrop, UIS_ConfigButtonWidth, UIS_ElmSize);

	/*gather data for dynamic sizing of protocols*/
	Array<ProtocolId> PAIds = m_protocolsAComponent->GetProtocolIds();
	int protocolsACount = PAIds.size()+1;
	Array<ProtocolId> PBIds = m_protocolsBComponent->GetProtocolIds();
	int protocolsBCount = PBIds.size()+1;
	int absProtocolCount = protocolsACount + protocolsBCount;
	int protocolsAreaHeight = yPositionModeDrop - 2*UIS_Margin_m;

	/*Dynamically sized protocol components*/
	int protocolsAHeight = absProtocolCount > 0 ? (protocolsAreaHeight / absProtocolCount)*protocolsACount : 0;
	if (m_protocolsAComponent)
		m_protocolsAComponent->setBounds(UIS_Margin_m, UIS_Margin_m + UIS_Margin_s, windowWidth - 2 * UIS_Margin_m, protocolsAHeight);

	int protocolsBHeight = absProtocolCount > 0 ? (protocolsAreaHeight / absProtocolCount)*protocolsBCount : 0;
	if (m_protocolsBComponent)
		m_protocolsBComponent->setBounds(UIS_Margin_m, protocolsAHeight + UIS_Margin_m + UIS_Margin_s, windowWidth - 2 * UIS_Margin_m, protocolsBHeight);
}

/**
 * Method to be called by child windows when closed, to enshure
 * button states and the internal
 * window object is invalidated to avoid accessviolation
 *
 * @param childWindow	The DialogWindow object that has been triggered to close
 */
void NodeComponent::childWindowCloseTriggered(DialogWindow* childWindow)
{
	if (m_OHMConfigDialog && childWindow == m_OHMConfigDialog.get())
	{
		m_ohmXmlElement = m_OHMConfigDialog->createStateXml();
		triggerConfigurationUpdate(true);

		if (m_OHMConfigEditButton)
		{
			m_OHMConfigEditButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
			m_OHMConfigEditButton->setColour(Label::textColourId, Colours::white);
		}

		m_OHMConfigDialog.reset();
	}
}

/**
 * Method to gather data from ui input elements and dump them to given configuration object.
 *
 * @return	The configuration xml object of this node
 */
std::unique_ptr<XmlElement> NodeComponent::createStateXml()
{
	auto nodeXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	nodeXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), static_cast<int>(m_NodeId));

	auto protocolsAXmlElement = m_protocolsAComponent->createStateXml();
	if (protocolsAXmlElement)
	{
		auto protocolAXmlElement = protocolsAXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
		while (protocolAXmlElement != nullptr)
		{
			nodeXmlElement->addChildElement(std::make_unique<XmlElement>(*protocolAXmlElement).release());
			protocolAXmlElement = protocolAXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
		}
	}

	auto protocolsBXmlElement = m_protocolsBComponent->createStateXml();
	if (protocolsBXmlElement)
	{
		auto protocolBXmlElement = protocolsBXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
		while (protocolBXmlElement != nullptr)
		{
			nodeXmlElement->addChildElement(std::make_unique<XmlElement>(*protocolBXmlElement).release());
			protocolBXmlElement = protocolBXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
		}
	}

	if (m_NodeModeDrop)
	{
		auto selectedOHMode = static_cast<ObjectHandlingMode>(m_NodeModeDrop->getSelectedId());
		m_ohmXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(selectedOHMode));
	}
	nodeXmlElement->addChildElement(std::make_unique<XmlElement>(*m_ohmXmlElement).release());

	return nodeXmlElement;
}

/**
 * Method to update ui input elements with data to show from config
 *
 * @param config	The configuration object to extract the data to show on ui from
 * @return	The calculated theoretically required size for this component
 */
bool NodeComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE))
		return false;

	m_NodeId = static_cast<NodeId>(stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID)));

	auto objectHandlingStateXml = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (objectHandlingStateXml)
		m_ohmXmlElement = std::make_unique<XmlElement>(*objectHandlingStateXml);
	else
		return false;

	ObjectHandlingMode selectedOHM = static_cast<ObjectHandlingMode>(ProcessingEngineConfig::ObjectHandlingModeFromString(m_ohmXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE))));
	m_NodeModeDrop->setSelectedId(selectedOHM, dontSendNotification);

	m_protocolsAComponent->setStateXml(stateXml);
	m_protocolsBComponent->setStateXml(stateXml);

	return true;
}

/**
 *
 */
int NodeComponent::GetCurrentRequiredHeight()
{
	int requiredHeight = 0;
    
	requiredHeight += m_protocolsAComponent->GetCurrentRequiredHeight();
	requiredHeight += m_protocolsBComponent->GetCurrentRequiredHeight();
	requiredHeight += UIS_Margin_s;

	requiredHeight += UIS_ElmSize + UIS_Margin_m;

	return requiredHeight;
}

/**
 * Getter for this nodes' NodeId.
 *
 * @return	This nodes' NodeID
 */
NodeId NodeComponent::GetNodeId()
{
	return m_NodeId;
}

/**
 * Method to add the parent listener to this instance of NodeComponent.
 * This can afterwards be used for e.g. callbacks, etc.
 *
 * @param listener	The parent listener object to be used to invoke public methods from ('callback')
 */
void NodeComponent::AddListener(MainRemoteProtocolBridgeComponent* listener)
{
    m_parentComponent = listener;
}

/**
 * Overloaded method called by button objects on click events.
 * All internal button objects are registered to trigger this by calling
 * their ::addListener method with this object as argument
 *
 * @param button	The button object that has been clicked
 */
void NodeComponent::buttonClicked(Button* button)
{
	if (button == m_OHMConfigEditButton.get())
	{
		ToggleOpenCloseObjectHandlingConfig(m_OHMConfigEditButton.get());
	}
	else
		triggerConfigurationUpdate(true);
}

/**
 * Overloaded method called by ComboBox objects on change events.
 * This is similar to ::buttonClicked but originates from inherited ComboBox::Listener
 *
 * @param comboBox	The comboBox object that has been changed
 */
void NodeComponent::comboBoxChanged(ComboBox* comboBox)
{
	ignoreUnused(comboBox);

	triggerConfigurationUpdate(true);
}

/**
 * Overloaded method called by TextEditor objects on textchange events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has been changed
 */
void NodeComponent::textEditorTextChanged(TextEditor& textEdit)
{
	ignoreUnused(textEdit);

	triggerConfigurationUpdate(true);
}

/**
 * Overloaded method called by TextEditor objects on keypress events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has received keypress
 */
void NodeComponent::textEditorReturnKeyPressed(TextEditor& textEdit)
{
	ignoreUnused(textEdit);

	triggerConfigurationUpdate(true);
}

/**
 * Overloaded method called by TextEditor objects on keypress events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has received keypress
 */
void NodeComponent::textEditorEscapeKeyPressed(TextEditor& textEdit)
{
	ignoreUnused(textEdit);
}

/**
 * Overloaded method called by TextEditor objects on focuslost events.
 * This is similar to ::buttonClicked but originates from inherited TextEditor::Listener
 *
 * @param textEdit	The textEdit object that has lost focus
 */
void NodeComponent::textEditorFocusLost(TextEditor& textEdit)
{
	ignoreUnused(textEdit);
}

/**
 * Helper method to be called by child protocolgroupcomponent for adding a new default protocol.
 * Detecting if new protocol shall be added as A or B type is done through comparing the sender
 * object pointer to internally kept member objects.
 *
 * @param targetPGC	The sending protocol group component object
 * @return	True on success, false on failure
 */
bool NodeComponent::AddDefaultProtocol(const ProtocolGroupComponent* targetPGC)
{
	auto config = ProcessingEngineConfig::getInstance();
	if (!config)
		return false;

	if (targetPGC == m_protocolsAComponent.get())
	{
		auto nodeState = createStateXml();
		nodeState->addChildElement(ProcessingEngineConfig::GetDefaultProtocol(ProtocolRole::PR_A).release());
		return config->setConfigState(std::move(nodeState), ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
	}
	else if (targetPGC == m_protocolsBComponent.get())
	{
		auto nodeState = createStateXml();
		nodeState->addChildElement(ProcessingEngineConfig::GetDefaultProtocol(ProtocolRole::PR_B).release());
		return config->setConfigState(std::move(nodeState), ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
	}
	else
		return false;
}



/**
 * Method to remove a given nodes' protocol with specified is
 *
 * @param PId	The protocol to remove
 * @return	True on success false on failure
 */
bool NodeComponent::RemoveProtocol(const ProtocolId& PId)
{
	if (m_protocolsAComponent->GetProtocolIds().contains(PId))
		m_protocolsAComponent->RemoveProtocol(PId);
	else if (m_protocolsBComponent->GetProtocolIds().contains(PId))
		m_protocolsBComponent->RemoveProtocol(PId);
	else
		return false;

	return true;
}

/**
 * Helper method to do the toggling of config dialog on respective buttonClicked
 *
 * @param button	The button object that has been clicked
 */
void NodeComponent::ToggleOpenCloseObjectHandlingConfig(Button* button)
{
	jassert(button);
	if (!m_parentComponent || !button)
		return;

	// if the config dialog exists, this is a uncheck (close) click,
	// which means we have to process edited data
	if (m_OHMConfigDialog)
	{
		childWindowCloseTriggered(m_OHMConfigDialog.get());
	}
	// otherwise we have to create the dialog and show it
	else
	{
		ObjectHandlingMode ohMode = static_cast<ObjectHandlingMode>(ProcessingEngineConfig::ObjectHandlingModeFromString(m_ohmXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE))));

		String dialogTitle = ProcessingEngineConfig::ObjectHandlingModeToString(ohMode) + " obj. handling configuration (Node Id" + String(m_NodeId) + ")";

		m_OHMConfigDialog = std::make_unique<ObjectHandlingConfigWindow>(dialogTitle, Colours::dimgrey, false, m_NodeId, ohMode);
		m_OHMConfigDialog->AddListener(this);
		m_OHMConfigDialog->setResizable(true, true);
		m_OHMConfigDialog->setUsingNativeTitleBar(true);
		m_OHMConfigDialog->setVisible(true);
		m_OHMConfigDialog->setStateXml(m_ohmXmlElement.get());
#if defined JUCE_IOS ||  defined JUCE_ANDROID
        m_OHMConfigDialog->setFullScreen(true);
#else
		const std::pair<int, int> size = m_OHMConfigDialog->GetSuggestedSize();
		m_OHMConfigDialog->setResizeLimits(size.first, size.second, size.first, size.second);
		m_OHMConfigDialog->setBounds(juce::Rectangle<int>(getScreenBounds().getX(), getScreenBounds().getY(), size.first, size.second));
#endif
		button->setColour(TextButton::buttonColourId, Colours::lightblue);
		button->setColour(Label::textColourId, Colours::dimgrey);
	}
}
