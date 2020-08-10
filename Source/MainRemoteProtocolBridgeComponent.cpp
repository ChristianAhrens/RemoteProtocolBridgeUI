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

#include "MainRemoteProtocolBridgeComponent.h"

#include "RemoteProtocolBridgeCommon.h"
#include "NodeComponent.h"
#include "LoggingWindow.h"
#include "ConfigComponents/GlobalConfigComponents.h"
#include "ProcessingEngineConfig.h"


// **************************************************************************************
//    class MainRemoteProtocolBridgeComponent
// **************************************************************************************
/**
 * Constructor
 */
MainRemoteProtocolBridgeComponent::MainRemoteProtocolBridgeComponent()
{
	m_config = std::make_unique<ProcessingEngineConfig>(ProcessingEngineConfig::getDefaultConfigFilePath());
	m_config->addDumper(this);

	m_config->addWatcher(this);
	m_config->addWatcher(&m_engine);

    
	m_ConfigDialog = 0;
	m_LoggingDialog = 0;

	/******************************************************/
	m_AddNodeButton = std::make_unique<ImageButton>();
	m_AddNodeButton->addListener(this);
	addAndMakeVisible(m_AddNodeButton.get());
	Image AddNormalImage = ImageCache::getFromMemory(BinaryData::AddNormalImage_png, BinaryData::AddNormalImage_pngSize);
	Image AddOverImage = ImageCache::getFromMemory(BinaryData::AddOverImage_png, BinaryData::AddOverImage_pngSize);
	Image AddDownImage = ImageCache::getFromMemory(BinaryData::AddDownImage_png, BinaryData::AddDownImage_pngSize);
	m_AddNodeButton->setImages(false, true, true, 
		AddNormalImage, 1.0, Colours::transparentBlack,
		AddOverImage, 1.0, Colours::transparentBlack,
		AddDownImage, 1.0, Colours::transparentBlack);

	m_RemoveNodeButton = std::make_unique<ImageButton>();
	m_RemoveNodeButton->addListener(this);
	addAndMakeVisible(m_RemoveNodeButton.get());
	Image RemoveNormalImage = ImageCache::getFromMemory(BinaryData::RemoveNormalImage_png, BinaryData::RemoveNormalImage_pngSize);
	Image RemoveOverImage = ImageCache::getFromMemory(BinaryData::RemoveOverImage_png, BinaryData::RemoveOverImage_pngSize);
	Image RemoveDownImage = ImageCache::getFromMemory(BinaryData::RemoveDownImage_png, BinaryData::RemoveDownImage_pngSize);
	m_RemoveNodeButton->setImages(false, true, true,
		RemoveNormalImage, 1.0, Colours::transparentBlack,
		RemoveOverImage, 1.0, Colours::transparentBlack,
		RemoveDownImage, 1.0, Colours::transparentBlack);

	/******************************************************/
	m_TriggerOpenConfigButton = std::make_unique<TextButton>();
	m_TriggerOpenConfigButton->addListener(this);
	addAndMakeVisible(m_TriggerOpenConfigButton.get());
	m_TriggerOpenConfigButton->setButtonText("Global Configuration");
	m_TriggerOpenConfigButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
	m_TriggerOpenConfigButton->setColour(Label::textColourId, Colours::white);

	m_TriggerOpenLoggingButton = std::make_unique<TextButton>();
	m_TriggerOpenLoggingButton->addListener(this);
	addAndMakeVisible(m_TriggerOpenLoggingButton.get());
	m_TriggerOpenLoggingButton->setButtonText("Show Traffic Logging");
	m_TriggerOpenLoggingButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
	m_TriggerOpenLoggingButton->setColour(Label::textColourId, Colours::white);

	m_EngineStartStopButton = std::make_unique<TextButton>();
	m_EngineStartStopButton->addListener(this);
	addAndMakeVisible(m_EngineStartStopButton.get());
	m_EngineStartStopButton->setButtonText("Start Engine");
	m_EngineStartStopButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
	m_EngineStartStopButton->setColour(Label::textColourId, Colours::white);

	if (!m_config->isValid())
	{
		m_config->triggerConfigurationDump();
	}
	else
	{
		m_config->triggerWatcherUpdate();
	}

	auto currentConfig = m_config->getConfigState();
	auto globalConfigXmlElement = currentConfig->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::GLOBALCONFIG));
	if (globalConfigXmlElement)
	{
		auto engineXmlElement = globalConfigXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ENGINE));
		if (engineXmlElement && engineXmlElement->getBoolAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::AUTOSTART)))
		{
			if (!m_engine.IsRunning() && m_engine.Start())
			{
				m_EngineStartStopButton->setColour(TextButton::buttonColourId, Colours::lightgreen);
				m_EngineStartStopButton->setColour(Label::textColourId, Colours::dimgrey);
				m_EngineStartStopButton->setButtonText("Stop Engine");
			}
		}
	}
}

/**
 * Destructor
 */
MainRemoteProtocolBridgeComponent::~MainRemoteProtocolBridgeComponent()
{
	if (m_engine.IsRunning())
		m_engine.Stop();
}

/**
 * Method to set up ui elements according to configuration contents.
 */
void MainRemoteProtocolBridgeComponent::onConfigUpdated()
{
	auto currentConfigState = m_config->getConfigState();
    auto nodeIds = m_config->GetNodeIds();

	// go through current ui node boxes to find out which ones need to be removed/destroyed
	Array<NodeId> nodeIdsToRemove;
	for (auto const & nodeBox : m_NodeBoxes)
		if (!nodeIds.contains(nodeBox.first))
			nodeIdsToRemove.add(nodeBox.first);

	for(auto nodeId : nodeIdsToRemove)
	{
		if (m_NodeBoxes.count(nodeId) && m_NodeBoxes.at(nodeId))
		{
			removeChildComponent(m_NodeBoxes.at(nodeId).get());
			m_NodeBoxes.erase(nodeId);
		}
	}

	int requiredNodeAreaHeight = 0;

	// now go through all node ids currently in config and create those nodes
	// that do not already exist or simply update those that are present
    for (auto nodeId : nodeIds)
    {
		if (m_NodeBoxes.count(nodeId) == 0)
		{
			auto node = std::make_unique<NodeComponent>(nodeId);
			node->AddListener(this);
			node->setText("Protocol Bridging Node Id" + String(nodeId));
			addAndMakeVisible(node.get());

			m_NodeBoxes[nodeId] = std::move(node);
		}
    }

	auto nodeXmlElement = currentConfigState->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	while (nodeXmlElement != nullptr)
	{
		auto nodeId = nodeXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
		if (m_NodeBoxes.count(nodeId) > 0)
		{
			m_NodeBoxes.at(nodeId)->setStateXml(nodeXmlElement);
			requiredNodeAreaHeight += m_NodeBoxes[nodeId]->GetCurrentRequiredHeight();
		}

		nodeXmlElement = nodeXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	}

	auto globalConfigXmlElement = currentConfigState->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::GLOBALCONFIG));
	if (globalConfigXmlElement)
	{
		m_GlobalConfigXml = std::make_unique<XmlElement>(*globalConfigXmlElement);

		auto trafficLoggingXmlElement = globalConfigXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::TRAFFICLOGGING));
		if (trafficLoggingXmlElement && trafficLoggingXmlElement->getBoolAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ALLOWED)))
			addAndMakeVisible(m_TriggerOpenLoggingButton.get());
		else
			removeChildComponent(m_TriggerOpenLoggingButton.get());
	}

#if defined JUCE_IOS ||  defined JUCE_ANDROID
    if(getScreenBounds().getWidth()<1 || getScreenBounds().getHeight()<1)
        setSize(1,1);
    else
        setSize(getScreenBounds().getWidth(), getScreenBounds().getHeight());
#else
	// For +- and logging/config/start buttons, add some additional height
	int requiredGlobalControlsHeight = 2 * (UIS_ElmSize + UIS_Margin_m);

	setSize(UIS_MainComponentWidth, requiredNodeAreaHeight + requiredGlobalControlsHeight);
#endif
}

/**
 * Method to allow access to internal engine.
 *
 * @return	Reference to the internal engine object
 */
ProcessingEngine* MainRemoteProtocolBridgeComponent::GetEngine()
{
	return &m_engine;
}

/**
 * Reimplemented from JUCEAppBasics::AppConfigurationBase::Dumper to be called from configuration
 * to start a complete dump of the app configuration state into the config object contents
 */
void MainRemoteProtocolBridgeComponent::performConfigurationDump()
{
	auto running = GetEngine()->IsRunning();
	GetEngine()->Stop();

	if (m_NodeBoxes.empty() || !m_GlobalConfigXml)
	{
		// Add a default node
		auto defaultNodeXmlElement = ProcessingEngineConfig::GetDefaultNode();
		m_config->setConfigState(std::move(defaultNodeXmlElement));
		// Add default global config
		auto defaultGlobalXmlElement = ProcessingEngineConfig::GetDefaultGlobalConfig();
		m_config->setConfigState(std::move(defaultGlobalXmlElement));
	}
	else
	{
		for (auto const & nodeKV : m_NodeBoxes)
		{
			if (nodeKV.second)
			{
				m_config->setConfigState(nodeKV.second->createStateXml(), ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
			}
		}

		m_config->setConfigState(std::make_unique<XmlElement>(*m_GlobalConfigXml));
	}

	if (running)
		GetEngine()->Start();
}

/**
 * Overloaded paint method that fills background with solid color
 *
 * @param g	Graphics painting object to use for filling background
 */
void MainRemoteProtocolBridgeComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

/**
 * Overloaded method to resize contents
 */
void MainRemoteProtocolBridgeComponent::resized()
{
	Component::resized();

	int windowWidth = getWidth();
	int windowHeight = getHeight();

	/*Config, TrafficLogging, Start/Stop Buttons*/
	int yPositionConfTrafButtons = windowHeight - UIS_ElmSize - UIS_Margin_m;
	if (m_TriggerOpenConfigButton)
		m_TriggerOpenConfigButton->setBounds(UIS_Margin_m, yPositionConfTrafButtons, UIS_OpenConfigWidth, UIS_ElmSize);
	if (m_TriggerOpenLoggingButton)
		m_TriggerOpenLoggingButton->setBounds(windowWidth - 160, yPositionConfTrafButtons, UIS_ButtonWidth, UIS_ElmSize);
	if (m_EngineStartStopButton)
		m_EngineStartStopButton->setBounds(windowWidth - 80, yPositionConfTrafButtons, UIS_ButtonWidth, UIS_ElmSize);

	/*Add/Remove Buttons*/
	int yPositionAddRemButts = yPositionConfTrafButtons - UIS_ElmSize;
	if (m_AddNodeButton)
		m_AddNodeButton->setBounds(windowWidth - 25, yPositionAddRemButts, UIS_ElmSize - UIS_Margin_s, UIS_ElmSize - UIS_Margin_s);
	if (m_RemoveNodeButton)
		m_RemoveNodeButton->setBounds(windowWidth - 45, yPositionAddRemButts, UIS_ElmSize - UIS_Margin_s, UIS_ElmSize - UIS_Margin_s);

	/*Dynamically sized nodes*/
	int nodeAreaWidth = windowWidth - 2 * UIS_Margin_s;
	int nodeAreaHeight = yPositionAddRemButts - UIS_Margin_m;
    Array<NodeId> NIds = m_config->GetNodeIds();
	int nodeCount = NIds.size();
	int nodeHeight = nodeCount > 0 ? nodeAreaHeight / nodeCount : 0;
    for (int i = 0; i < nodeCount; ++i)
    {
        NodeId NId = NIds[i];
		if (m_NodeBoxes.count(NId) && m_NodeBoxes.at(NId))
			m_NodeBoxes.at(NId)->setBounds(UIS_Margin_s, UIS_Margin_s + (i*nodeHeight), nodeAreaWidth, nodeHeight);
    }

}

/**
 * Overloaded method called by button objects on click events.
 * All internal button objects are registered to trigger this by calling
 * their ::addListener method with this object as argument
 *
 * @param button	The button object that has been clicked
 */
void MainRemoteProtocolBridgeComponent::buttonClicked(Button* button)
{
	if (button == m_AddNodeButton.get())
	{
		m_config->setConfigState(ProcessingEngineConfig::GetDefaultNode(), ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
		m_config->triggerWatcherUpdate();
		m_config->triggerConfigurationDump();
	}
	else if (button == m_RemoveNodeButton.get())
	{
		if (m_config->GetNodeIds().size() > 0)
		{
			auto configStateXml = m_config->getConfigState();
			configStateXml->removeChildElement(configStateXml->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(static_cast<int>(m_config->GetNodeIds().getLast()))), true);
			m_config->resetConfigState(std::move(configStateXml));
			m_config->triggerConfigurationDump();
		}
	}
	else if (button == m_TriggerOpenConfigButton.get())
	{
		// if the config dialog exists, this is a uncheck (close) click,
		// which means we have to process edited data
		if (m_ConfigDialog != 0)
		{
			m_GlobalConfigXml = m_ConfigDialog->createStateXml();
			m_config->triggerConfigurationDump();
			m_ConfigDialog.reset();

			button->setColour(TextButton::buttonColourId, Colours::dimgrey);
			button->setColour(Label::textColourId, Colours::white);
		}
		// otherwise we have to create the dialog and show it
		else
		{
			m_ConfigDialog = std::make_unique<GlobalConfigWindow>("General configuration", Colours::dimgrey, false);
			m_ConfigDialog->AddListener(this);
			m_ConfigDialog->setResizable(true, true);
			m_ConfigDialog->setUsingNativeTitleBar(true);
			m_ConfigDialog->setVisible(true);
#if defined JUCE_IOS ||  defined JUCE_ANDROID
            m_ConfigDialog->setFullScreen(true);
#else
            std::pair<int, int> size = m_ConfigDialog->GetSuggestedSize();
            m_ConfigDialog->setResizeLimits(size.first, size.second, size.first, size.second);
			m_ConfigDialog->setBounds(Rectangle<int>(getScreenBounds().getX() + getWidth(), getScreenBounds().getY(), size.first, size.second));
#endif
			m_ConfigDialog->setStateXml(m_GlobalConfigXml.get());

			button->setColour(TextButton::buttonColourId, Colours::lightblue);
			button->setColour(Label::textColourId, Colours::dimgrey);
		}
	}
	else if (button == m_TriggerOpenLoggingButton.get())
	{
		if (m_engine.IsLoggingEnabled())
		{
			m_engine.SetLoggingEnabled(false);
			m_engine.SetLoggingTarget(0);
			button->setColour(TextButton::buttonColourId, Colours::dimgrey);
			button->setColour(Label::textColourId, Colours::white);

			m_LoggingDialog.reset();
		}
		else
		{
			m_LoggingDialog = std::make_unique<LoggingWindow>("Protocol Traffic Logging", Colours::dimgrey, false);
			m_LoggingDialog->AddListener(this);
			m_LoggingDialog->setResizeLimits(480, 320, 1920, 1080);
			m_LoggingDialog->setResizable(true, true);
			m_LoggingDialog->setUsingNativeTitleBar(true);
			m_LoggingDialog->setVisible(true);
#if defined JUCE_IOS ||  defined JUCE_ANDROID
			m_LoggingDialog->setFullScreen(true);
#else
			m_LoggingDialog->setBounds(Rectangle<int>(getScreenBounds().getX() + getWidth(), getScreenBounds().getY(), 800, 500));
#endif

			m_engine.SetLoggingEnabled(true);
			m_engine.SetLoggingTarget(m_LoggingDialog.get());

			button->setColour(TextButton::buttonColourId, Colours::orange);
			button->setColour(Label::textColourId, Colours::dimgrey);
		}
	}
	else if (button == m_EngineStartStopButton.get())
	{
		if (m_engine.IsRunning())
		{
			m_engine.Stop();
			button->setColour(TextButton::buttonColourId, Colours::dimgrey);
			button->setColour(Label::textColourId, Colours::white);
			//button->setButtonText("Start Engine");
		}
		else
		{
			// Get data from ui together to start the engine correctly.
			m_config->triggerConfigurationDump();

			if (m_engine.Start())
			{
				button->setColour(TextButton::buttonColourId, Colours::lightgreen);
				button->setColour(Label::textColourId, Colours::dimgrey);
				//button->setButtonText("Stop Engine");
			}
		}
	}
}

/**
 * Method to be called by child windows when closed, to enshure
 * button states, logging activity are resetted and the internal
 * window object is invalidated to avoid accessviolation
 *
 * @param childWindow	The DialogWindow object that has been triggered to close
 */
void MainRemoteProtocolBridgeComponent::childWindowCloseTriggered(DialogWindow* childWindow)
{
	if (childWindow == m_ConfigDialog.get())
	{
		if (m_ConfigDialog != 0)
		{
			m_GlobalConfigXml = m_ConfigDialog->createStateXml();
			m_config->triggerConfigurationDump();

			if (m_TriggerOpenConfigButton)
			{
				m_TriggerOpenConfigButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
				m_TriggerOpenConfigButton->setColour(Label::textColourId, Colours::white);
			}
		}

		m_ConfigDialog.reset();
	}
	else if (childWindow == m_LoggingDialog.get())
	{
		if (m_engine.IsLoggingEnabled())
		{
			m_engine.SetLoggingEnabled(false);
			m_engine.SetLoggingTarget(0);

			if (m_TriggerOpenLoggingButton)
			{
				m_TriggerOpenLoggingButton->setColour(TextButton::buttonColourId, Colours::dimgrey);
				m_TriggerOpenLoggingButton->setColour(Label::textColourId, Colours::white);
			}
		}

		m_LoggingDialog.reset();
	}
}
