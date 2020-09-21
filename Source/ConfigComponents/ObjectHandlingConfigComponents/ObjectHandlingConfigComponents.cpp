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

#include "ObjectHandlingConfigComponents.h"

#include "../../NodeComponent.h"
#include "../../RemoteProtocolBridgeCommon.h"

//==============================================================================
// Class ObjectHandlingConfigComponent_Abstract
//==============================================================================

/**
 * @fn void ProcessingEngineConfig::ObjectHandlingData ObjectHandlingConfigComponent_Abstract::DumpObjectHandlingData()
 * @return The dumped set of object handling configuration data.
 * Pure virtual function to be implemented by data handling configuration components to dump the active configuration.
 */

/**
 * @fn void ObjectHandlingConfigComponent_Abstract::FillObjectHandlingData(const ProcessingEngineConfig::ObjectHandlingData& ohData)
 * @param ohData	
 * Pure virtual function to be implemented by data handling config component objects to insert a set of object handling configuration data.
 */

 /**
  * @fn const std::pair<int, int>  ObjectHandlingConfigComponent_Abstract::GetSuggestedSize()
  * @return The components favoured xy size
  * Pure virtual function to be implemented by data handling config component objects to suggest a favoured size.
  */

/**
 * Class constructor.
 */
ObjectHandlingConfigComponent_Abstract::ObjectHandlingConfigComponent_Abstract(ObjectHandlingMode mode)
{
	m_parentListener = 0;
	m_mode = mode;
}

/**
 * Class destructor.
 */
ObjectHandlingConfigComponent_Abstract::~ObjectHandlingConfigComponent_Abstract()
{
}

/**
 * Reimplemented paint method that fills background with solid color
 *
 * @param g	Graphics painting object to use for filling background
 */
void ObjectHandlingConfigComponent_Abstract::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void ObjectHandlingConfigComponent_Abstract::buttonClicked(Button* button)
{
	if (button == m_applyConfigButton.get())
	{
		if (m_parentListener)
			m_parentListener->OnEditingFinished();
	}
}

/**
 * Method to add parent object as 'listener'.
 * This is done in a way JUCE uses to connect child-parent relations for handling 'signal' calls
 */
void ObjectHandlingConfigComponent_Abstract::AddListener(ObjectHandlingConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The object handling config data to use when running the engine.
 */
std::unique_ptr<XmlElement> ObjectHandlingConfigComponent_Abstract::createStateXml()
{
	auto ohXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));

	ohXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode::OHM_Bypass));
	ohXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DATAPRECISION), 0.001);

	auto aChCntXmlElement = ohXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
	if (aChCntXmlElement)
		aChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 0);

	auto bChCntXmlElement = ohXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
	if (bChCntXmlElement)
		bChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 0);

	return ohXmlElement;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with ObjectHandling data
 *
 * @param ohData	The data to set into UI elms.
 */
bool ObjectHandlingConfigComponent_Abstract::setStateXml(XmlElement* stateXml)
{
	ignoreUnused(stateXml);
	
	return true;
}


//==============================================================================
// Class OHNoConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
OHNoConfigComponent::OHNoConfigComponent(ObjectHandlingMode mode)
	: ObjectHandlingConfigComponent_Abstract(mode)
{
	m_Headline = std::make_unique<Label>();
	m_Headline->setText("This object handling mode does not require configuration.", dontSendNotification);
	addAndMakeVisible(m_Headline.get());

	m_applyConfigButton = std::make_unique<TextButton>("Ok");
	addAndMakeVisible(m_applyConfigButton.get());
	m_applyConfigButton->addListener(this);
}

/**
 * Class destructor.
 */
OHNoConfigComponent::~OHNoConfigComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OHNoConfigComponent::resized()
{
	double usableWidth = (double)(getWidth() - 2 * UIS_Margin_s);

	// headline
	int yOffset = UIS_Margin_s;
	m_Headline->setBounds(UIS_Margin_m, yOffset, (int)usableWidth, UIS_ElmSize);

	// ok button
	yOffset += UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> OHNoConfigComponent::GetSuggestedSize()
{
	int width = UIS_BasicConfigWidth;
	int height = 100;

	return std::pair<int, int>(width, height);
}


//==============================================================================
// Class OHMultiplexAtoBConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
OHMultiplexAtoBConfigComponent::OHMultiplexAtoBConfigComponent(ObjectHandlingMode mode)
	: ObjectHandlingConfigComponent_Abstract(mode)
{
	m_Headline = std::make_unique <Label>();
	m_Headline->setText("Multiplexing parameters:", dontSendNotification);
	addAndMakeVisible(m_Headline.get());

	m_CountAEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_CountAEdit.get());
	m_CountAEdit->addListener(this);

	m_CountALabel = std::make_unique<Label>();
	m_CountALabel->setText("Ch. count per ProtocolA (n)", dontSendNotification);
	addAndMakeVisible(m_CountALabel.get());
	m_CountALabel->attachToComponent(m_CountAEdit.get(), true);

	m_CountBEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_CountBEdit.get());
	m_CountBEdit->addListener(this);

	m_CountBLabel = std::make_unique<Label>();
	m_CountBLabel->setText("Ch. count per ProtocolB (m)", dontSendNotification);
	addAndMakeVisible(m_CountBLabel.get());
	m_CountBLabel->attachToComponent(m_CountBEdit.get(), true);

	m_applyConfigButton = std::make_unique<TextButton>("Ok");
	addAndMakeVisible(m_applyConfigButton.get());
	m_applyConfigButton->addListener(this);
}

/**
 * Class destructor.
 */
OHMultiplexAtoBConfigComponent::~OHMultiplexAtoBConfigComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OHMultiplexAtoBConfigComponent::resized()
{
	double usableWidth = (double)(getWidth() - 2 * UIS_Margin_s);
	
	// active objects headline
	int yOffset = UIS_Margin_s;
	m_Headline->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, (int)usableWidth, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_CountAEdit->setBounds(Rectangle<int>(UIS_WideAttachedLabelWidth + UIS_Margin_s, yOffset, (int)usableWidth - UIS_WideAttachedLabelWidth - UIS_Margin_s, UIS_ElmSize));
	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_CountBEdit->setBounds(Rectangle<int>(UIS_WideAttachedLabelWidth + UIS_Margin_s, yOffset, (int)usableWidth - UIS_WideAttachedLabelWidth - UIS_Margin_s, UIS_ElmSize));

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void OHMultiplexAtoBConfigComponent::textEditorFocusLost(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void OHMultiplexAtoBConfigComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> OHMultiplexAtoBConfigComponent::GetSuggestedSize()
{
	int width = UIS_OSCConfigWidth;
	int height =	UIS_Margin_s +
					2 * UIS_Margin_m + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
					UIS_Margin_s + UIS_ElmSize;

	return std::pair<int, int>(width, height);
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The object handling config data to use when running the engine.
 */
std::unique_ptr<XmlElement> OHMultiplexAtoBConfigComponent::createStateXml()
{
	auto ohXmlElement = ObjectHandlingConfigComponent_Abstract::createStateXml();

	auto aChCnt = 0;
	auto bChCnt = 0;
	if (m_CountAEdit)
	{
		aChCnt = m_CountAEdit->getText().getIntValue();
	}
	if (m_CountBEdit)
	{
		bChCnt = m_CountBEdit->getText().getIntValue();
	}

	auto aChCntXmlElement = ohXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
	if (aChCntXmlElement)
		aChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), aChCnt);

	auto bChCntXmlElement = ohXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
	if (bChCntXmlElement)
		bChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), bChCnt);

	return ohXmlElement;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with ObjectHandling data
 *
 * @param ohData	The data to set into UI elms.
 */
bool OHMultiplexAtoBConfigComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING))
		return false;

	if (stateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)) != ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode::OHM_Mux_nA_to_mB))
		return false;

	auto aChCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
	if (aChCntXmlElement && m_CountAEdit)
		m_CountAEdit->setText(String(aChCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT))), dontSendNotification);
	else
		return false;

	auto bChCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
	if (bChCntXmlElement && m_CountBEdit)
		m_CountBEdit->setText(String(bChCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT))), dontSendNotification);
	else
		return false;

	return true;
}


//==============================================================================
// Class OHForwardOnlyValueChangesConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
OHForwardOnlyValueChangesConfigComponent::OHForwardOnlyValueChangesConfigComponent(ObjectHandlingMode mode)
	: ObjectHandlingConfigComponent_Abstract(mode)
{
	m_Headline = std::make_unique <Label>();
	m_Headline->setText("Filtering parameters:", dontSendNotification);
	addAndMakeVisible(m_Headline.get());

	m_PrecisionSelect = std::make_unique<ComboBox>();
	addAndMakeVisible(m_PrecisionSelect.get());
	m_PrecisionSelect->addListener(this);

	m_PrecisionSelect->addItem(PrecisionValToString(PV_EVEN), PV_EVEN);
	m_PrecisionSelect->addItem(PrecisionValToString(PV_CENTI), PV_CENTI);
	m_PrecisionSelect->addItem(PrecisionValToString(PV_MILLI), PV_MILLI);
	m_PrecisionSelect->addItem(PrecisionValToString(PV_MICRO), PV_MICRO);

	m_PrecisionLabel = std::make_unique<Label>();
	m_PrecisionLabel->setText("Value change precision sensibility", dontSendNotification);
	addAndMakeVisible(m_PrecisionLabel.get());
	m_PrecisionLabel->attachToComponent(m_PrecisionSelect.get(), true);

	m_applyConfigButton = std::make_unique<TextButton>("Ok");
	addAndMakeVisible(m_applyConfigButton.get());
	m_applyConfigButton->addListener(this);
}

/**
 * Class destructor.
 */
OHForwardOnlyValueChangesConfigComponent::~OHForwardOnlyValueChangesConfigComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OHForwardOnlyValueChangesConfigComponent::resized()
{
	double usableWidth = (double)(getWidth() - 2 * UIS_Margin_s);

	// active objects headline
	int yOffset = UIS_Margin_s;
	m_Headline->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, (int)usableWidth, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_PrecisionSelect->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth - UIS_Margin_s, yOffset, UIS_ButtonWidth, UIS_ElmSize));
	
	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for changes to our comboBox.
 * @param comboBox	The ComboBox object that has changed.
 */
void OHForwardOnlyValueChangesConfigComponent::comboBoxChanged(ComboBox* comboBox)
{
	ignoreUnused(comboBox);
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> OHForwardOnlyValueChangesConfigComponent::GetSuggestedSize()
{
	int width = UIS_OSCConfigWidth;
	int height = UIS_Margin_s +
		2 * UIS_Margin_m + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
		UIS_Margin_s + UIS_ElmSize;

	return std::pair<int, int>(width, height);
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The object handling config data to use when running the engine.
 */
std::unique_ptr<XmlElement> OHForwardOnlyValueChangesConfigComponent::createStateXml()
{
	auto ohXmlElement = ObjectHandlingConfigComponent_Abstract::createStateXml();

	auto precision = 0.001;
	if (m_PrecisionSelect)
	{
		PrecVal pv = static_cast<PrecVal>(m_PrecisionSelect->getSelectedId());
		switch (pv)
		{
		case PV_EVEN:
			precision = 1;
			break;
		case PV_CENTI:
			precision = 0.1;
			break;
		case PV_MILLI:
			precision = 0.01;
			break;
		case PV_MICRO:
		default:
			precision = 0.001;
			break;
		}
	}

	ohXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DATAPRECISION), precision);

	return ohXmlElement;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with ObjectHandling data
 *
 * @param ohData	The data to set into UI elms.
 */
bool OHForwardOnlyValueChangesConfigComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING))
		return false;

	if (stateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)) != ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode::OHM_Forward_only_valueChanges))
		return false;

	auto precision = stateXml->getDoubleAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DATAPRECISION), 0.001);

	if (m_PrecisionSelect)
	{
		if (precision == 1)
			m_PrecisionSelect->setSelectedId(PV_EVEN);
		else if (precision == 0.1)
			m_PrecisionSelect->setSelectedId(PV_CENTI);
		else if (precision == 0.01)
			m_PrecisionSelect->setSelectedId(PV_MILLI);
		else if (precision == 0.001)
			m_PrecisionSelect->setSelectedId(PV_MICRO);
		else
			m_PrecisionSelect->setSelectedId(PV_MICRO);
	}
	else
		return false;

	return true;
}

/**
 * Helper method to convert a given enum value to its string representation
 *
 * @param pv	The precision value enum to get the string for.
 */
String OHForwardOnlyValueChangesConfigComponent::PrecisionValToString(PrecVal pv)
{
	switch (pv)
	{
	case PV_EVEN:
		return "1";
	case PV_CENTI:
		return "0.1";
	case PV_MILLI:
		return "0.01";
	case PV_MICRO:
	default:
		return "0.001";
	}
}


//==============================================================================
// Class OHDS100SimConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
OHDS100SimConfigComponent::OHDS100SimConfigComponent(ObjectHandlingMode mode)
	: ObjectHandlingConfigComponent_Abstract(mode)
{
	m_Headline = std::make_unique <Label>();
	m_Headline->setText("Simulation parameters:", dontSendNotification);
	addAndMakeVisible(m_Headline.get());


	//==============================================================================
	m_CountChannelsEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_CountChannelsEdit.get());
	m_CountChannelsEdit->addListener(this);

	m_CountChannelsLabel = std::make_unique<Label>();
	m_CountChannelsLabel->setText("Simulated Soundsource Count", dontSendNotification);
	addAndMakeVisible(m_CountChannelsLabel.get());
	m_CountChannelsLabel->attachToComponent(m_CountChannelsEdit.get(), true);


	//==============================================================================
	m_CountMappingsEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_CountMappingsEdit.get());
	m_CountMappingsEdit->addListener(this);

	m_CountMappingsLabel = std::make_unique<Label>();
	m_CountMappingsLabel->setText("Simulated Mappings Count", dontSendNotification);
	addAndMakeVisible(m_CountMappingsLabel.get());
	m_CountMappingsLabel->attachToComponent(m_CountMappingsEdit.get(), true);


	//==============================================================================
	m_RefreshIntervalEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_RefreshIntervalEdit.get());
	m_RefreshIntervalEdit->addListener(this);

	m_RefreshIntervalLabel = std::make_unique<Label>();
	m_RefreshIntervalLabel->setText("Simulation update interval", dontSendNotification);
	addAndMakeVisible(m_RefreshIntervalLabel.get());
	m_RefreshIntervalLabel->attachToComponent(m_RefreshIntervalEdit.get(), true);


	//==============================================================================
	m_applyConfigButton = std::make_unique<TextButton>("Ok");
	addAndMakeVisible(m_applyConfigButton.get());
	m_applyConfigButton->addListener(this);
}

/**
 * Class destructor.
 */
OHDS100SimConfigComponent::~OHDS100SimConfigComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OHDS100SimConfigComponent::resized()
{
	double usableWidth = (double)(getWidth() - 2 * UIS_Margin_s);

	// active objects headline
	int yOffset = UIS_Margin_s;
	m_Headline->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, (int)usableWidth, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_CountChannelsEdit->setBounds(Rectangle<int>(UIS_WideAttachedLabelWidth + UIS_Margin_s, yOffset, (int)usableWidth - UIS_WideAttachedLabelWidth - UIS_Margin_s, UIS_ElmSize));
	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_CountMappingsEdit->setBounds(Rectangle<int>(UIS_WideAttachedLabelWidth + UIS_Margin_s, yOffset, (int)usableWidth - UIS_WideAttachedLabelWidth - UIS_Margin_s, UIS_ElmSize));
	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_RefreshIntervalEdit->setBounds(Rectangle<int>(UIS_WideAttachedLabelWidth + UIS_Margin_s, yOffset, (int)usableWidth - UIS_WideAttachedLabelWidth - UIS_Margin_s, UIS_ElmSize));

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void OHDS100SimConfigComponent::textEditorFocusLost(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void OHDS100SimConfigComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> OHDS100SimConfigComponent::GetSuggestedSize()
{
	int width = UIS_OSCConfigWidth;
	int height = UIS_Margin_s +
		2 * UIS_Margin_m + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
		UIS_Margin_s + UIS_ElmSize;

	return std::pair<int, int>(width, height);
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The object handling config data to use when running the engine.
 */
std::unique_ptr<XmlElement> OHDS100SimConfigComponent::createStateXml()
{
	auto ohXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));

	ohXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode::OHM_DS100_DeviceSimulation));

	auto channelsCount = 0;
	auto mappingsCount = 0;
	auto updateInterval = 0;

	if (m_CountChannelsEdit)
	{
		channelsCount = m_CountChannelsEdit->getText().getIntValue();
	}
	if (m_CountMappingsEdit)
	{
		mappingsCount = m_CountMappingsEdit->getText().getIntValue();
	}
	if (m_RefreshIntervalEdit)
	{
		updateInterval = m_RefreshIntervalEdit->getText().getIntValue();
	}

	auto simChCntXmlElement = ohXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMCHCNT));
	if (simChCntXmlElement)
		simChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), channelsCount);

	auto simMapingsCntXmlElement = ohXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMMAPCNT));
	if (simMapingsCntXmlElement)
		simMapingsCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), mappingsCount);

	auto refreshIntervalXmlElement = ohXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REFRESHINTERVAL));
	if (refreshIntervalXmlElement)
		refreshIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), updateInterval);

	return ohXmlElement;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with ObjectHandling data
 *
 * @param ohData	The data to set into UI elms.
 */
bool OHDS100SimConfigComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING))
		return false;

	if (stateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)) != ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode::OHM_DS100_DeviceSimulation))
		return false;

	auto simChCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMCHCNT));
	if (simChCntXmlElement && m_CountChannelsEdit)
		m_CountChannelsEdit->setText(String(simChCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT))), dontSendNotification);
	else
		return false;

	auto simMapingsCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMMAPCNT));
	if (simMapingsCntXmlElement && m_CountMappingsEdit)
		m_CountMappingsEdit->setText(String(simMapingsCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT))), dontSendNotification);
	else
		return false;

	auto refreshIntervalXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REFRESHINTERVAL));
	if (refreshIntervalXmlElement && m_RefreshIntervalEdit)
		m_RefreshIntervalEdit->setText(String(refreshIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL))), dontSendNotification);
	else
		return false;

	return true;
}


// **************************************************************************************
//    class ObjectHandlingConfigWindow
// **************************************************************************************
/**
 * Constructor for the ObjectHandlingConfigWindow class. Internal contents are created here.
 *
 * @param name							The name of the window
 * @param backgroundColour				The background color of the window
 * @param escapeKeyTriggersCloseButton	Flag value to set behaviour of hitting escape key on window
 * @param NId							Node Id of the node this object handling config window implements editing for
 * @param mode							Object handling mode this config window shall provide editing for
 * @param addToDesktop					Flag value to define if window is to be standalone or embedded in other content
 */
ObjectHandlingConfigWindow::ObjectHandlingConfigWindow(const String &name, Colour backgroundColour, bool escapeKeyTriggersCloseButton, NodeId NId,
													   ObjectHandlingMode mode, bool addToDesktop)
	: DialogWindow(name, backgroundColour, escapeKeyTriggersCloseButton, addToDesktop)
{
	m_parentListener = 0;

	m_NId = NId;

	switch (mode)
	{
	case ObjectHandlingMode::OHM_Mux_nA_to_mB:
		m_configComponent = std::make_unique<OHMultiplexAtoBConfigComponent>(mode);
		break;
	case ObjectHandlingMode::OHM_Forward_only_valueChanges:
		m_configComponent = std::make_unique<OHForwardOnlyValueChangesConfigComponent>(mode);
		break;
	case ObjectHandlingMode::OHM_DS100_DeviceSimulation:
		m_configComponent = std::make_unique<OHDS100SimConfigComponent>(mode);
		break;
	case ObjectHandlingMode::OHM_Bypass:
		// intentionally no break to run into default
	case ObjectHandlingMode::OHM_Remap_A_X_Y_to_B_XY:
		// intentionally no break to run into default
	case ObjectHandlingMode::OHM_Invalid:
		// intentionally no break to run into default
	case ObjectHandlingMode::OHM_Forward_A_to_B_only:
		// intentionally no break to run into default
	case ObjectHandlingMode::OHM_Reverse_B_to_A_only:
		// intentionally no break to run into default
	default:
		m_configComponent = std::make_unique<OHNoConfigComponent>(mode);
		break;
	}

	// Component resizes automatically anyway, but need size > 0;
	m_configComponent->setBounds(Rectangle<int>(1, 1));
	m_configComponent->AddListener(this);

	setContentOwned(m_configComponent.get(), true);
}

/**
 * Destructor
 */
ObjectHandlingConfigWindow::~ObjectHandlingConfigWindow()
{

}

/**
 * Overloaded method that is called when window close button is pressed.
 * We enshure the window self-destroys here, but first notify the parent of it.
 */
void ObjectHandlingConfigWindow::closeButtonPressed()
{
	if (m_parentListener)
		m_parentListener->childWindowCloseTriggered(this);
}

/**
 * Method to add parent object as 'listener'.
 * This is done in a way JUCE uses to connect child-parent relations for handling 'signal' calls
 */
void ObjectHandlingConfigWindow::AddListener(NodeComponent* listener)
{
	m_parentListener = listener;
}

/**
 * Proxy method to trigger dumping contents of configcomponent member
 * to global config object
 *
 * @param config	The global configuration object to dump data to
 * @return	True on success
 */
std::unique_ptr<XmlElement> ObjectHandlingConfigWindow::createStateXml()
{
	return m_configComponent->createStateXml();
}

/**
 * Proxy setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param config	The global configuration object.
 */
bool ObjectHandlingConfigWindow::setStateXml(XmlElement* stateXml)
{
	return m_configComponent->setStateXml(stateXml);
}

/**
 * Method to be called by child component to trigger closing down and applying
 * edited contents.
 */
void ObjectHandlingConfigWindow::OnEditingFinished()
{
	closeButtonPressed();
}

/**
 * Proxy method to get the windows' components' suggested size.
 *
 * @return	The pair of int representing the suggested size for the window
 */
const std::pair<int, int> ObjectHandlingConfigWindow::GetSuggestedSize()
{
	return m_configComponent->GetSuggestedSize();
}
