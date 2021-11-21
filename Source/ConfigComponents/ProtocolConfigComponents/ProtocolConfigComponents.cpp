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

#include "ProtocolConfigComponents.h"

#include "../../ProtocolComponent.h"
#include <RemoteProtocolBridgeCommon.h>

//==============================================================================
// Class ProtocolConfigComponent_Abstract
//==============================================================================
/**
 * @fn bool ProtocolConfigComponent_Abstract::DumpActiveHandlingUsed()
 * @return True on success, false on failure.
 * Pure virtual function to be implemented to return if protocol specific config elements are set to active handling or not.
 */

/**
 * @fn Array<RemoteObject> ProtocolConfigComponent_Abstract::DumpActiveRemoteObjects()
 * @return True on success, false on failure.
 * Pure virtual function to be implemented by derived config components to dump ui contents regarding remote object active setting.
 */

/**
 * @fn void ProtocolConfigComponent_Abstract::FillActiveRemoteObjects(const Array<RemoteObject>& Objs)
 * @return True on success, false on failure.
 * Pure virtual function to be implemented by derived config components to fill ui contents regarding remote object active setting.
 */

/**
 * @fn const std::pair<int, int> ProtocolConfigComponent_Abstract::GetSuggestedSize()
 * @return True on success, false on failure.
 * Pure virtual function to be implemented by protocol config component objects to suggest a favoured size.
 */

/**
 * Class constructor.
 */
ProtocolConfigComponent_Abstract::ProtocolConfigComponent_Abstract(ProtocolRole role)
{
	m_ProtocolRole = role;
	m_parentListener	= 0;

	m_Headline = std::make_unique<Label>();
	m_Headline->setText("Generic Protocol Configuration:", dontSendNotification);
	addAndMakeVisible(m_Headline.get());

	m_HostPortLabel = std::make_unique<Label>();
	addAndMakeVisible(m_HostPortLabel.get());
	m_HostPortLabel->setText("Listening port", dontSendNotification);
	m_HostPortEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_HostPortEdit.get());
	m_ClientPortLabel = std::make_unique<Label>();
	addAndMakeVisible(m_ClientPortLabel.get());
	m_ClientPortLabel->setText("Remote port", dontSendNotification);
	m_ClientPortEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_ClientPortEdit.get());

	m_applyConfigButton = std::make_unique<TextButton>("Ok");
	addAndMakeVisible(m_applyConfigButton.get());
	m_applyConfigButton->addListener(this);
}

/**
 * Class destructor.
 */
ProtocolConfigComponent_Abstract::~ProtocolConfigComponent_Abstract()
{
}

/**
 * Reimplemented paint method that fills background with solid color
 *
 * @param g	Graphics painting object to use for filling background
 */
void ProtocolConfigComponent_Abstract::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void ProtocolConfigComponent_Abstract::buttonClicked(Button* button)
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
void ProtocolConfigComponent_Abstract::AddListener(ProtocolConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to pair of ports to return to the app to initialize from
 *
 * @return	The pair of ports (host/client) set in UI to be dumped into config.
 */
std::pair<int, int> ProtocolConfigComponent_Abstract::DumpProtocolPorts()
{
	int clientPort = 0;
	int hostPort = 0;

	StringArray portStrings;
	portStrings.addTokens(m_HostPortEdit->getText(), ";, ", "");
	if (portStrings.size() == 1)
	{
		hostPort = portStrings[0].getIntValue();
	}
	portStrings.clear();
	portStrings.addTokens(m_ClientPortEdit->getText(), ";, ", "");
	if (portStrings.size() == 1)
	{
		clientPort = portStrings[0].getIntValue();
	}

	return std::pair<int, int>(clientPort, hostPort);
}

/**
 * Method to trigger filling contents of
 * configcomponent member with protocol ports
 *
 * @param ports		The client+host port from config.
 */
void ProtocolConfigComponent_Abstract::FillProtocolPorts(const std::pair<int, int>& ports)
{
	if (m_HostPortEdit)
		m_HostPortEdit->setText(String(ports.second));
	if (m_ClientPortEdit)
		m_ClientPortEdit->setText(String(ports.first));

	return;
}

/**
 * Setter of state of button for if active object handling shall be used
 *
 * @param active	True if active object handling shall be activated.
 */
void ProtocolConfigComponent_Abstract::SetActiveHandlingUsed(bool active)
{
	ignoreUnused(active);
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to global config object
 *
 * @param NId The id of the node to dump the config for
 * @param NId The id of the protocol to dump the config for
 * @param config	The global configuration object to dump data to
 * @return	True on success
 */
std::unique_ptr<XmlElement> ProtocolConfigComponent_Abstract::createStateXml()
{
	auto ports = DumpProtocolPorts();

	auto clientPortXmlElement = m_protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (!clientPortXmlElement)
		clientPortXmlElement = m_protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), ports.first);

	auto hostPortXmlElement = m_protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (!hostPortXmlElement)
		hostPortXmlElement = m_protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), ports.second);

	return std::make_unique<XmlElement>(*m_protocolXmlElement);
}

/**
 * Setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param NId The id of the node to set the config for
 * @param NId The id of the protocol to set the config for
 * @param config	The global configuration object.
 */
bool ProtocolConfigComponent_Abstract::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ((m_ProtocolRole == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB)))
		return false;

	m_protocolXmlElement = std::make_unique<XmlElement>(*stateXml);

	std::pair<int, int> ports{ 0, 0 };
	auto clientPortXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		ports.first = clientPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
	auto hostPortXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		ports.second = hostPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
	FillProtocolPorts(ports);

	return true;
}


//==============================================================================
// Class BasicProtocolConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
BasicProtocolConfigComponent::BasicProtocolConfigComponent(ProtocolRole role)
	: ProtocolConfigComponent_Abstract(role)
{
	m_UseActiveHandlingCheck = std::make_unique<ToggleButton>();
	addAndMakeVisible(m_UseActiveHandlingCheck.get());

	m_UseActiveHandlingLabel = std::make_unique<Label>();
	addAndMakeVisible(m_UseActiveHandlingLabel.get());
	m_UseActiveHandlingLabel->setText("Enable active object handling", dontSendNotification);

	m_Headline->setText("Objects to activly handle (OSC polling, OCA subscriptions)", dontSendNotification);

	m_EnableHeadlineLabel = std::make_unique<Label>();
	m_EnableHeadlineLabel->setText("active", dontSendNotification);
	addAndMakeVisible(m_EnableHeadlineLabel.get());
	m_ChannelHeadlineLabel = std::make_unique<Label>();
	m_ChannelHeadlineLabel->setText("channel", dontSendNotification);
	addAndMakeVisible(m_ChannelHeadlineLabel.get());
	m_RecordHeadlineLabel = std::make_unique<Label>();
	m_RecordHeadlineLabel->setText("record", dontSendNotification);
	addAndMakeVisible(m_RecordHeadlineLabel.get());

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		m_RemObjNameLabels[i] = std::make_unique<Label>();
		m_RemObjNameLabels.at(i)->setText(ProcessingEngineConfig::GetObjectDescription((RemoteObjectIdentifier)i), dontSendNotification);
		addAndMakeVisible(m_RemObjNameLabels.at(i).get());

		m_RemObjEnableChecks[i] = std::make_unique<ToggleButton>();
		addAndMakeVisible(m_RemObjEnableChecks.at(i).get());
		
		m_RemObjActiveChannelEdits[i] = std::make_unique<TextEditor>();
		addAndMakeVisible(m_RemObjActiveChannelEdits.at(i).get());
		
		m_RemObjActiveRecordEdits[i] = std::make_unique<TextEditor>();
		addAndMakeVisible(m_RemObjActiveRecordEdits.at(i).get());
	}

}

/**
 * Class destructor.
 */
BasicProtocolConfigComponent::~BasicProtocolConfigComponent()
{
	m_RemObjNameLabels.clear();
	m_RemObjEnableChecks.clear();
	m_RemObjActiveChannelEdits.clear();
	m_RemObjActiveRecordEdits.clear();
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void BasicProtocolConfigComponent::resized()
{
	double usableWidth = double(getWidth()) - 2 * UIS_Margin_s;
	int remObjNameWidth = (int)(usableWidth*0.5);
	int remObjEnableWidth = (int)(usableWidth*0.1);
	int remObjChRngeWidth = (int)(usableWidth*0.2);
	int remObjRecRngeWidth = (int)(usableWidth*0.2);

	int yOffset = UIS_Margin_s;
	m_HostPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
	m_HostPortEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + remObjNameWidth, yOffset, remObjEnableWidth + remObjChRngeWidth - UIS_Margin_m, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_ClientPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
	m_ClientPortEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + remObjNameWidth, yOffset, remObjEnableWidth + remObjChRngeWidth - UIS_Margin_m, UIS_ElmSize));

	// active handling checkbox
	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_UseActiveHandlingLabel->setBounds(Rectangle<int>(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize)));
	m_UseActiveHandlingCheck->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s, yOffset, remObjEnableWidth, UIS_ElmSize));

	// table headline labels
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_EnableHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s, yOffset, remObjEnableWidth, UIS_ElmSize));
	m_ChannelHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth, yOffset, remObjChRngeWidth, UIS_ElmSize));
	m_RecordHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth, yOffset, remObjRecRngeWidth, UIS_ElmSize));

	// table items
	yOffset += UIS_Margin_s;
	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		yOffset += UIS_Margin_s + UIS_ElmSize;
		if (m_RemObjNameLabels.count(i) && m_RemObjNameLabels.at(i))
			m_RemObjNameLabels.at(i)->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(i))
			m_RemObjEnableChecks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s, yOffset, remObjEnableWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjActiveChannelEdits.count(i) && m_RemObjActiveChannelEdits.at(i))
			m_RemObjActiveChannelEdits.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth, yOffset, remObjChRngeWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjActiveRecordEdits.count(i) && m_RemObjActiveRecordEdits.at(i))
			m_RemObjActiveRecordEdits.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth, yOffset, remObjRecRngeWidth - UIS_Margin_s, UIS_ElmSize));
	}

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void BasicProtocolConfigComponent::buttonClicked(Button* button)
{
	if(button == m_applyConfigButton.get())
	{
		if (m_parentListener)
			m_parentListener->OnEditingFinished();
	}
}

/**
 * Method to add parent object as 'listener'.
 * This is done in a way JUCE uses to connect child-parent relations for handling 'signal' calls
 */
void BasicProtocolConfigComponent::AddListener(ProtocolConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The list of objects to actively handle when running the engine.
 */
std::vector<RemoteObject> BasicProtocolConfigComponent::DumpActiveRemoteObjects()
{
	std::vector<RemoteObject> activeObjects;

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		RemoteObject obj;

		//ToggleButton* c = m_RemObjEnableChecks[i];
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(i) && m_RemObjEnableChecks.at(i)->getToggleState())
		{
			obj._Id = static_cast<RemoteObjectIdentifier>(i);

			// Since we expect the user to enter something like "1,4,6,8-12" to select
			// channels 1 4 6 8 9 10 11 12, we need to do some parsing. First we split
			// our input string for channels and records based on separators ',' or ';'
			// and second we inspect the results if they are singles or ranges.
			// The result is supposted to be two array with all channels and records included
			// as separate entries.
			Array<int> channels;
			Array<int> records;
			if (m_RemObjActiveChannelEdits.count(i) && m_RemObjActiveChannelEdits.at(i))
			{
				StringArray channelNumberSections;
				String chSelToSplit = m_RemObjActiveChannelEdits.at(i)->getText();
				channelNumberSections.addTokens(chSelToSplit, ",; ", "");
				for (int j = 0; j < channelNumberSections.size(); ++j)
				{
					StringArray channelNumbers;
					channelNumbers.addTokens(channelNumberSections[j], "-", "");
					if (channelNumbers.size() == 1)
					{
						channels.add(channelNumbers[0].getIntValue());
					}
					else if (channelNumbers.size() == 2)
					{
						int startVal = channelNumbers[0].getIntValue();
						int stopVal = channelNumbers[1].getIntValue();
						for (int k = startVal; k <= stopVal; ++k)
							channels.add(k);
					}
				}
			}
			if (m_RemObjActiveRecordEdits.count(i) && m_RemObjActiveRecordEdits.at(i))
			{
				StringArray recordNumberSections;
				String recSelToSplit = m_RemObjActiveRecordEdits.at(i)->getText();
				recordNumberSections.addTokens(recSelToSplit, ",; ", "");
				for (int j = 0; j < recordNumberSections.size(); ++j)
				{
					StringArray recordNumbers;
					recordNumbers.addTokens(recordNumberSections[j], "-", "");
					if (recordNumbers.size() == 1)
					{
						records.add(recordNumbers[0].getIntValue());
					}
					else if (recordNumbers.size() == 2)
					{
						int startVal = recordNumbers[0].getIntValue();
						int stopVal = recordNumbers[1].getIntValue();
						for (int k = startVal; k <= stopVal; ++k)
							records.add(k);
					}
				}
			}

			// now that we have all channels and records, recursively iterate through both arrays
			// to add an entry to our active objects list for every resulting ch/rec combi object
			for (int j = 0; j < channels.size(); ++j)
			{
				if (records.size() > 0)
				{
					for (int k = 0; k < records.size(); ++k)
					{
						obj._Addr._first = static_cast<ChannelId>(channels[j]);
						obj._Addr._second = static_cast<RecordId>(records[k]);
						activeObjects.push_back(obj);
					}
				}
				else
				{
					obj._Addr._first = int16(channels[j]);
					obj._Addr._second = -1;
					activeObjects.push_back(obj);
				}
			}
		}
	}

	return activeObjects;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with list of objects
 *
 * @param Objs	The list of objects to set as default.
 */
void BasicProtocolConfigComponent::FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs)
{
	Array<int> activeObjects;
	HashMap<int, Array<int>> channelsPerObj;
	HashMap<int, Array<int>> recordsPerObj;

	for (int i = 0; i < Objs.size(); ++i)
	{
		Array<int> selChs = channelsPerObj[Objs[i]._Id];
		if (!selChs.contains(Objs[i]._Addr._first))
		{
			selChs.add(Objs[i]._Addr._first);
			channelsPerObj.set(Objs[i]._Id, selChs);
		}

		Array<int> selRecs = recordsPerObj[Objs[i]._Id];
		if (!selRecs.contains(Objs[i]._Addr._second))
		{
			selRecs.add(Objs[i]._Addr._second);
			recordsPerObj.set(Objs[i]._Id, selRecs);
		}

		if (!activeObjects.contains(Objs[i]._Id))
			activeObjects.add(Objs[i]._Id);
	}

	for (int i = 0; i < activeObjects.size(); ++i)
	{
		int Id = activeObjects[i];
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(Id) && !m_RemObjEnableChecks.at(Id)->getToggleState())
			m_RemObjEnableChecks.at(Id)->setToggleState(true, dontSendNotification);

		if (m_RemObjActiveChannelEdits.count(i) && m_RemObjActiveChannelEdits.at(Id))
		{
			String selChanTxt;
			for (int j = 0; j < channelsPerObj[Id].size(); ++j)
			{
				if (channelsPerObj[Id][j] > 0)
				{
					if (!selChanTxt.isEmpty())
						selChanTxt << ", ";
					selChanTxt << channelsPerObj[Id][j];
				}
			}

			m_RemObjActiveChannelEdits.at(Id)->setText(selChanTxt);
		}

		if (m_RemObjActiveRecordEdits.count(i) && m_RemObjActiveRecordEdits.at(Id))
		{
			String selRecTxt;
			for (int j = 0; j < recordsPerObj[Id].size(); ++j)
			{
				if (recordsPerObj[Id][j] > 0)
				{
					if (!selRecTxt.isEmpty())
						selRecTxt << ", ";
					selRecTxt << recordsPerObj[Id][j];
				}
			}

			m_RemObjActiveRecordEdits.at(Id)->setText(selRecTxt);
		}
	}
}

/**
 * Method to trigger dumping of state of button for if active object handling shall be used
 *
 * @return	True if active object handling shall be used.
 */
bool BasicProtocolConfigComponent::DumpActiveHandlingUsed()
{
	if (m_UseActiveHandlingCheck)
		return m_UseActiveHandlingCheck->getToggleState();
	else
		return false;
}

/**
 * Setter of state of button for if active object handling shall be used
 *
 * @param active	True if active object handling shall be activated.
 */
void BasicProtocolConfigComponent::SetActiveHandlingUsed(bool active)
{
	if (m_UseActiveHandlingCheck)
		m_UseActiveHandlingCheck->setToggleState(active, dontSendNotification);
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> BasicProtocolConfigComponent::GetSuggestedSize()
{
	int width = UIS_BasicConfigWidth;
	int height = UIS_Margin_s +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
		UIS_Margin_s +
		((ROI_BridgingMAX - ROI_Invalid)*(UIS_Margin_s + UIS_ElmSize + UIS_Margin_s)) +
		UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
		UIS_Margin_s;

	return std::pair<int, int>(width, height);
}


//==============================================================================
// Class ActiveObjectScrollContentsComponent
//==============================================================================
/**
 * Class constructor.
 */
ActiveObjectScrollContentsComponent::ActiveObjectScrollContentsComponent()
{
	int predictedHeight = UIS_Margin_s;

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		m_RemObjNameLabels[i] = std::make_unique<Label>();
		m_RemObjNameLabels.at(i)->setText(ProcessingEngineConfig::GetObjectDescription((RemoteObjectIdentifier)i), dontSendNotification);
		addAndMakeVisible(m_RemObjNameLabels.at(i).get());

		m_RemObjEnableChecks[i] = std::make_unique<ToggleButton>();
		addAndMakeVisible(m_RemObjEnableChecks.at(i).get());

		// the object# textedits are only relevant for some objects
		if (ProcessingEngineConfig::IsChannelAddressingObject(static_cast<RemoteObjectIdentifier>(i)))
		{
			m_RemObjActiveChannelEdits[i] = std::make_unique<TextEditor>();
			addAndMakeVisible(m_RemObjActiveChannelEdits.at(i).get());
		}
		// the mapping checks are only relevant for select objects
		if (ProcessingEngineConfig::IsRecordAddressingObject(static_cast<RemoteObjectIdentifier>(i)))
		{
			m_RemObjMappingArea1Checks[i] = std::make_unique<ToggleButton>();
			addAndMakeVisible(m_RemObjMappingArea1Checks.at(i).get());

			m_RemObjMappingArea2Checks[i] = std::make_unique<ToggleButton>();
			addAndMakeVisible(m_RemObjMappingArea2Checks.at(i).get());

			m_RemObjMappingArea3Checks[i] = std::make_unique<ToggleButton>();
			addAndMakeVisible(m_RemObjMappingArea3Checks.at(i).get());

			m_RemObjMappingArea4Checks[i] = std::make_unique<ToggleButton>();
			addAndMakeVisible(m_RemObjMappingArea4Checks.at(i).get());
		}

		predictedHeight += UIS_Margin_s + UIS_ElmSize;
	}

	setSize(getWidth(), predictedHeight);
}

/**
 * Class destructor.
 */
ActiveObjectScrollContentsComponent::~ActiveObjectScrollContentsComponent()
{
}

/**
 * Helper method to query if any of the ui elements indicate that active handling is enabled.
 * @return True if any element indicates active handling, false if not.
 */
bool ActiveObjectScrollContentsComponent::IsActiveHandlingEnabled()
{
	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(i) && m_RemObjEnableChecks.at(i)->getToggleState())
			return true;
	}

	return false;
}

/**
 * Getter for the remote object listing of currently enabled remote objects.
 * @return The requested remote object listing.
 */
std::vector<RemoteObject> ActiveObjectScrollContentsComponent::GetActiveRemoteObjects()
{
	std::vector<RemoteObject> activeObjects;

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		RemoteObject obj;

		//ToggleButton* c = m_RemObjEnableChecks.at(i).get();
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(i) && m_RemObjEnableChecks.at(i)->getToggleState())
		{
			obj._Id = static_cast<RemoteObjectIdentifier>(i);

			// Since we expect the user to enter something like "1,4,6,8-12" to select
			// channels 1 4 6 8 9 10 11 12, we need to do some parsing. First we split
			// our input string for channels and records based on separators ',' or ';'
			// and second we inspect the results if they are singles or ranges.
			// The result is supposted to be two array with all channels and records included
			// as separate entries.
			Array<int> channels;
			Array<int> records;
			if (m_RemObjActiveChannelEdits.count(i) && m_RemObjActiveChannelEdits.at(i))
			{
				StringArray channelNumberSections;
				String chSelToSplit = m_RemObjActiveChannelEdits.at(i)->getText();
				channelNumberSections.addTokens(chSelToSplit, ",; ", "");
				for (int j = 0; j < channelNumberSections.size(); ++j)
				{
					StringArray channelNumbers;
					channelNumbers.addTokens(channelNumberSections[j], "-", "");
					if (channelNumbers.size() == 1)
					{
						channels.add(channelNumbers[0].getIntValue());
					}
					else if (channelNumbers.size() == 2)
					{
						int startVal = channelNumbers[0].getIntValue();
						int stopVal = channelNumbers[1].getIntValue();
						for (int k = startVal; k <= stopVal; ++k)
							channels.add(k);
					}
				}
			}
			if (m_RemObjMappingArea1Checks.count(i) && m_RemObjMappingArea1Checks.at(i) && m_RemObjMappingArea1Checks.at(i)->getToggleState())
				records.add(1);
			if (m_RemObjMappingArea2Checks.count(i) && m_RemObjMappingArea2Checks.at(i) && m_RemObjMappingArea2Checks.at(i)->getToggleState())
				records.add(2);
			if (m_RemObjMappingArea3Checks.count(i) && m_RemObjMappingArea3Checks.at(i) && m_RemObjMappingArea3Checks.at(i)->getToggleState())
				records.add(3);
			if (m_RemObjMappingArea4Checks.count(i) && m_RemObjMappingArea4Checks.at(i) && m_RemObjMappingArea4Checks.at(i)->getToggleState())
				records.add(4);

			// now that we have all channels and records, recursively iterate through both arrays
			// to add an entry to our active objects list for every resulting ch/rec combi object
			for (int j = 0; j < channels.size(); ++j)
			{
				if (records.size() > 0)
				{
					for (int k = 0; k < records.size(); ++k)
					{
						obj._Addr._first = static_cast<ChannelId>(channels[j]);
						obj._Addr._second = static_cast<RecordId>(records[k]);
						activeObjects.push_back(obj);
					}
				}
				else
				{
					obj._Addr._first = static_cast<ChannelId>(channels[j]);
					obj._Addr._second = -1;
					activeObjects.push_back(obj);
				}
			}
		}
	}

	return activeObjects;
}

/**
 * Setter for the remote object listing of currently enabled remote objects.
 * @param Objs	The remote objects to set as to be shown enabled on ui.
 */
void ActiveObjectScrollContentsComponent::SetActiveRemoteObjects(const std::vector<RemoteObject>& Objs)
{
	Array<int> activeObjects;
	HashMap<int, Array<int>> channelsPerObj;
	HashMap<int, Array<int>> recordsPerObj;

	for (int i = 0; i < Objs.size(); ++i)
	{
		Array<int> selChs = channelsPerObj[Objs[i]._Id];
		if (!selChs.contains(Objs[i]._Addr._first))
		{
			selChs.add(Objs[i]._Addr._first);
			channelsPerObj.set(Objs[i]._Id, selChs);
		}

		Array<int> selRecs = recordsPerObj[Objs[i]._Id];
		if (!selRecs.contains(Objs[i]._Addr._second))
		{
			selRecs.add(Objs[i]._Addr._second);
			recordsPerObj.set(Objs[i]._Id, selRecs);
		}

		if (!activeObjects.contains(Objs[i]._Id))
			activeObjects.add(Objs[i]._Id);
	}

	for (int i = 0; i < activeObjects.size(); ++i)
	{
		int Id = activeObjects[i];
		if (m_RemObjEnableChecks.count(Id) && m_RemObjEnableChecks.at(Id) && !m_RemObjEnableChecks.at(Id)->getToggleState())
			m_RemObjEnableChecks.at(Id)->setToggleState(true, dontSendNotification);

		if (m_RemObjActiveChannelEdits.count(Id) && m_RemObjActiveChannelEdits.at(Id))
		{
			String selChanTxt;
			for (int j = 0; j < channelsPerObj[Id].size(); ++j)
			{
				if (channelsPerObj[Id][j] > 0)
				{
					if (!selChanTxt.isEmpty())
						selChanTxt << ", ";
					selChanTxt << channelsPerObj[Id][j];
				}
			}

			m_RemObjActiveChannelEdits.at(Id)->setText(selChanTxt);
		}

		if (m_RemObjMappingArea1Checks.count(Id) && m_RemObjMappingArea1Checks.at(Id))
			m_RemObjMappingArea1Checks.at(Id)->setToggleState(recordsPerObj[Id].contains(1), dontSendNotification);
		if (m_RemObjMappingArea2Checks.count(Id) && m_RemObjMappingArea2Checks.at(Id))
			m_RemObjMappingArea2Checks.at(Id)->setToggleState(recordsPerObj[Id].contains(2), dontSendNotification);
		if (m_RemObjMappingArea3Checks.count(Id) && m_RemObjMappingArea3Checks.at(Id))
			m_RemObjMappingArea3Checks.at(Id)->setToggleState(recordsPerObj[Id].contains(3), dontSendNotification);
		if (m_RemObjMappingArea4Checks.count(Id) && m_RemObjMappingArea4Checks.at(Id))
			m_RemObjMappingArea4Checks.at(Id)->setToggleState(recordsPerObj[Id].contains(4), dontSendNotification);
	}
}

/**
 * Reimplemented to handle sizing of elements.
 */
void ActiveObjectScrollContentsComponent::resized()
{
	double usableWidth = double(getWidth()) - 2 * UIS_Margin_s;
	int remObjNameWidth = (int)(usableWidth * 0.45);
	int remObjEnableWidth = (int)(usableWidth * 0.1);
	int remObjChRngeWidth = (int)(usableWidth * 0.2);
	int yOffset = UIS_Margin_s;
	
	// table items
	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
	{
		if (m_RemObjNameLabels.count(i) && m_RemObjNameLabels.at(i))
			m_RemObjNameLabels.at(i)->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjEnableChecks.count(i) && m_RemObjEnableChecks.at(i))
			m_RemObjEnableChecks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s, yOffset, remObjEnableWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjActiveChannelEdits.count(i) && m_RemObjActiveChannelEdits.at(i))
			m_RemObjActiveChannelEdits.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth, yOffset, remObjChRngeWidth - UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjMappingArea1Checks.count(i) && m_RemObjMappingArea1Checks.at(i))
			m_RemObjMappingArea1Checks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth, yOffset, UIS_ElmSize + UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjMappingArea2Checks.count(i) && m_RemObjMappingArea2Checks.at(i))
			m_RemObjMappingArea2Checks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 1 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize + UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjMappingArea3Checks.count(i) && m_RemObjMappingArea3Checks.at(i))
			m_RemObjMappingArea3Checks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 2 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize + UIS_Margin_s, UIS_ElmSize));
		if (m_RemObjMappingArea4Checks.count(i) && m_RemObjMappingArea4Checks.at(i))
			m_RemObjMappingArea4Checks.at(i)->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 3 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize + UIS_Margin_s, UIS_ElmSize));
		yOffset += UIS_Margin_s + UIS_ElmSize;
	}
}


//==============================================================================
// Class OSCProtocolConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
OSCProtocolConfigComponent::OSCProtocolConfigComponent(ProtocolRole role)
	: ProtocolConfigComponent_Abstract(role)
{
	m_Headline->setText("Objects enabled for polling:", dontSendNotification);

	m_EnableHeadlineLabel = std::make_unique<Label>();
	m_EnableHeadlineLabel->setText("enable", dontSendNotification);
	addAndMakeVisible(m_EnableHeadlineLabel.get());

	m_ChannelHeadlineLabel = std::make_unique<Label>();
	m_ChannelHeadlineLabel->setText("Object Nr.", dontSendNotification);
	addAndMakeVisible(m_ChannelHeadlineLabel.get());

	m_MappingsHeadlineLabel = std::make_unique<Label>();
	m_MappingsHeadlineLabel->setText("Mapping", dontSendNotification);
	addAndMakeVisible(m_MappingsHeadlineLabel.get());
	m_Mapping1HeadlineLabel = std::make_unique<Label>();
	m_Mapping1HeadlineLabel->setText("1", dontSendNotification);
	addAndMakeVisible(m_Mapping1HeadlineLabel.get());
	m_Mapping2HeadlineLabel = std::make_unique<Label>();
	m_Mapping2HeadlineLabel->setText("2", dontSendNotification);
	addAndMakeVisible(m_Mapping2HeadlineLabel.get());
	m_Mapping3HeadlineLabel = std::make_unique<Label>();
	m_Mapping3HeadlineLabel->setText("3", dontSendNotification);
	addAndMakeVisible(m_Mapping3HeadlineLabel.get());
	m_Mapping4HeadlineLabel = std::make_unique<Label>();
	m_Mapping4HeadlineLabel->setText("4", dontSendNotification);
	addAndMakeVisible(m_Mapping4HeadlineLabel.get());

	m_activeObjectsListComponent = std::make_unique<ActiveObjectScrollContentsComponent>();
	addAndMakeVisible(m_activeObjectsListComponent.get());
	m_activeObjectsListScrollView = std::make_unique<Viewport>();
	m_activeObjectsListScrollView->setViewedComponent(m_activeObjectsListComponent.get(), false);
	addAndMakeVisible(m_activeObjectsListScrollView.get());

	m_PollingIntervalLabel = std::make_unique<Label>();
	addAndMakeVisible(m_PollingIntervalLabel.get());
	m_PollingIntervalLabel->setText("Polling interval", dontSendNotification);
	m_PollingIntervalEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_PollingIntervalEdit.get());
}

/**
 * Class destructor.
 */
OSCProtocolConfigComponent::~OSCProtocolConfigComponent()
{
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OSCProtocolConfigComponent::resized()
{
	double usableWidth = double(getWidth()) - 2 * UIS_Margin_s - m_activeObjectsListScrollView->getScrollBarThickness();
	int remObjNameWidth = (int)(usableWidth*0.45);
	int remObjEnableWidth = (int)(usableWidth*0.1);
	int remObjChRngeWidth = (int)(usableWidth*0.2);
	int remObjRecRngeWidth = (int)(usableWidth*0.2);

	// port edits with labels
	int yOffset = UIS_Margin_s;
	m_HostPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
	m_HostPortEdit->setBounds(Rectangle<int>(2*UIS_Margin_s + remObjNameWidth, yOffset, remObjEnableWidth + remObjChRngeWidth - UIS_Margin_m, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_ClientPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
	m_ClientPortEdit->setBounds(Rectangle<int>(2*UIS_Margin_s + remObjNameWidth, yOffset, remObjEnableWidth + remObjChRngeWidth - UIS_Margin_m, UIS_ElmSize));
	
	// active objects headline
	yOffset += 2*UIS_Margin_m + UIS_ElmSize;
	m_Headline->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, (int)usableWidth, UIS_ElmSize));

	// table headline labels
	//yOffset += UIS_ElmSize + UIS_Margin_s;
	m_MappingsHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth, yOffset, remObjRecRngeWidth, UIS_ElmSize));

	yOffset += UIS_ElmSize;
	m_EnableHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s, yOffset, remObjEnableWidth, UIS_ElmSize));
	m_ChannelHeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth, yOffset, remObjChRngeWidth, UIS_ElmSize));

	m_Mapping1HeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth, yOffset, UIS_ElmSize, UIS_ElmSize));
	m_Mapping2HeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 1 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize, UIS_ElmSize));
	m_Mapping3HeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 2 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize, UIS_ElmSize));
	m_Mapping4HeadlineLabel->setBounds(Rectangle<int>(remObjNameWidth + UIS_Margin_s + remObjEnableWidth + remObjChRngeWidth + 3 * (UIS_ElmSize + UIS_Margin_s), yOffset, UIS_ElmSize, UIS_ElmSize));

	// component holding table items and corresp. scrollview
	yOffset += UIS_Margin_s + UIS_ElmSize;
	auto objectsListBounds = Rectangle<int>(getWidth() - m_activeObjectsListScrollView->getScrollBarThickness(), m_activeObjectsListComponent->getHeight());
	m_activeObjectsListComponent->setBounds(objectsListBounds);
	m_activeObjectsListScrollView->setBounds(Rectangle<int>(0 , yOffset, getWidth(), 8 * UIS_ElmSize));
	yOffset += 8 * UIS_ElmSize;

	// polling interval edit/label
	yOffset += UIS_Margin_s + UIS_Margin_s;
	m_PollingIntervalLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, remObjNameWidth - UIS_Margin_s, UIS_ElmSize));
	m_PollingIntervalEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + remObjNameWidth, yOffset, remObjEnableWidth + remObjChRngeWidth - UIS_Margin_m, UIS_ElmSize));

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void OSCProtocolConfigComponent::buttonClicked(Button* button)
{
	if(button == m_applyConfigButton.get())
	{
		if (m_parentListener)
			m_parentListener->OnEditingFinished();
	}
}

/**
 * Method to add parent object as 'listener'.
 * This is done in a way JUCE uses to connect child-parent relations for handling 'signal' calls
 */
void OSCProtocolConfigComponent::AddListener(ProtocolConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The list of objects to actively handle when running the engine.
 */
std::vector<RemoteObject> OSCProtocolConfigComponent::DumpActiveRemoteObjects()
{
	return m_activeObjectsListComponent->GetActiveRemoteObjects();
}

/**
 * Method to trigger filling contents of
 * configcomponent member with list of objects
 *
 * @param Objs	The list of objects to set as default.
 */
void OSCProtocolConfigComponent::FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs)
{
	m_activeObjectsListComponent->SetActiveRemoteObjects(Objs);
}

/**
 * Method to trigger dumping of state of button for if active object handling shall be used
 *
 * @return	True if active object handling shall be used.
 */
bool OSCProtocolConfigComponent::DumpActiveHandlingUsed()
{
	return m_activeObjectsListComponent->IsActiveHandlingEnabled();
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to integer interval return value
 *
 * @return	Polling interval time value.
 */
int OSCProtocolConfigComponent::DumpPollingInterval()
{
	int PollingInterval = 0;

	StringArray intervalStrings;
	intervalStrings.addTokens(m_PollingIntervalEdit->getText(), ";, ", "");
	if (intervalStrings.size() == 1)
	{
		PollingInterval = intervalStrings[0].getIntValue();
	}
	else if (intervalStrings.size() == 2 && intervalStrings[1] == "ms")
	{
		PollingInterval = intervalStrings[0].getIntValue();
	}

	return PollingInterval;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with polling interval value
 *
 * @param PollingInterval The interval value
 */
void OSCProtocolConfigComponent::FillPollingInterval(int PollingInterval)
{
	if (m_PollingIntervalEdit)
		m_PollingIntervalEdit->setText(String(PollingInterval) + String(" ms"));
	
	return;
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> OSCProtocolConfigComponent::GetSuggestedSize()
{
	int width	=	UIS_OSCConfigWidth;
	int height	=	UIS_Margin_s + 
					UIS_Margin_s + UIS_ElmSize + 
					2 * UIS_Margin_m + UIS_ElmSize + 
					UIS_ElmSize + 
					(8 * (UIS_Margin_s + UIS_ElmSize + UIS_Margin_s)) +
					UIS_Margin_s + UIS_Margin_s + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize + UIS_Margin_s +
					UIS_Margin_s;

	return std::pair<int, int>(width, height);
}

/**
 * Reimplemented method to trigger dumping contents of configcomponent member
 * to global config object
 *
 * @param NId The id of the node to dump the config for
 * @param NId The id of the protocol to dump the config for
 * @param config	The global configuration object to dump data to
 * @return	True on success
 */
std::unique_ptr<XmlElement> OSCProtocolConfigComponent::createStateXml()
{
	auto protocolStateXml = ProtocolConfigComponent_Abstract::createStateXml();

	auto activeHandlingUsed = DumpActiveHandlingUsed();
	auto activeObjects = DumpActiveRemoteObjects();

	protocolStateXml->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), static_cast<int>(activeHandlingUsed ? 1 : 0));
	auto activeObjsXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
	if (activeObjsXmlElement)
	{
		ProcessingEngineConfig::WriteActiveObjects(activeObjsXmlElement.get(), activeObjects);
		auto existingActiveObjsXmlElement = protocolStateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
		if (existingActiveObjsXmlElement)
			protocolStateXml->replaceChildElement(existingActiveObjsXmlElement, activeObjsXmlElement.release());
		else
			protocolStateXml->addChildElement(activeObjsXmlElement.release());
	}

	auto pollingIntervalXmlElement = protocolStateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
	if (!pollingIntervalXmlElement)
		pollingIntervalXmlElement = protocolStateXml->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
	pollingIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), DumpPollingInterval());

	return protocolStateXml;
}

/**
 * Reimplemented setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param NId The id of the node to set the config for
 * @param NId The id of the protocol to set the config for
 * @param config	The global configuration object.
 */
bool OSCProtocolConfigComponent::setStateXml(XmlElement* stateXml)
{
	SetActiveHandlingUsed(stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ)) == 1);
	auto activeObjsXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
	if (activeObjsXmlElement)
	{
		std::vector<RemoteObject> activeObjects;
		ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, activeObjects);
		FillActiveRemoteObjects(activeObjects);
	}

	auto pollingIntervalXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
	if(pollingIntervalXmlElement)
		FillPollingInterval(pollingIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL)));

	return ProtocolConfigComponent_Abstract::setStateXml(stateXml);
}


//==============================================================================
// Class MappingAreaProtocolConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
MappingAreaProtocolConfigComponent::MappingAreaProtocolConfigComponent(ProtocolRole role)
	: ProtocolConfigComponent_Abstract(role)
{
	m_MappingAreaIdLabel = std::make_unique<Label>();
	addAndMakeVisible(m_MappingAreaIdLabel.get());
	m_MappingAreaIdLabel->setText("MappingArea Id", dontSendNotification);
	m_MappingAreaIdEdit = std::make_unique<TextEditor>();
	addAndMakeVisible(m_MappingAreaIdEdit.get());
}

/**
 * Class destructor.
 */
MappingAreaProtocolConfigComponent::~MappingAreaProtocolConfigComponent()
{

}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MappingAreaProtocolConfigComponent::resized()
{
	double usableWidth = double(getWidth()) - 2 * UIS_Margin_s;
	int labelWidth = (int)(usableWidth * 0.45);
	int editWidth = (int)(usableWidth * 0.3);

	// port edits with labels
	int yOffset = UIS_Margin_s;
	m_HostPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, labelWidth - UIS_Margin_s, UIS_ElmSize));
	m_HostPortEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + labelWidth, yOffset, editWidth - UIS_Margin_m, UIS_ElmSize));

	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_ClientPortLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, labelWidth - UIS_Margin_s, UIS_ElmSize));
	m_ClientPortEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + labelWidth, yOffset, editWidth - UIS_Margin_m, UIS_ElmSize));

	// MappingArea Id edit/label
	yOffset += UIS_Margin_s + UIS_ElmSize;
	m_MappingAreaIdLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, labelWidth - UIS_Margin_s, UIS_ElmSize));
	m_MappingAreaIdEdit->setBounds(Rectangle<int>(2 * UIS_Margin_s + labelWidth, yOffset, editWidth - UIS_Margin_m, UIS_ElmSize));

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void MappingAreaProtocolConfigComponent::textEditorFocusLost(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void MappingAreaProtocolConfigComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void MappingAreaProtocolConfigComponent::buttonClicked(Button* button)
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
void MappingAreaProtocolConfigComponent::AddListener(ProtocolConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The list of objects to actively handle when running the engine.
 */
std::vector<RemoteObject> MappingAreaProtocolConfigComponent::DumpActiveRemoteObjects()
{
	return std::vector<RemoteObject>();
}

/**
 * Method to trigger filling contents of
 * configcomponent member with list of objects
 *
 * @param Objs	The list of objects to set as default.
 */
void MappingAreaProtocolConfigComponent::FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs)
{
	ignoreUnused(Objs);
}

/**
 * Method to trigger dumping of state of button for if active object handling shall be used
 *
 * @return	True if active object handling shall be used.
 */
bool MappingAreaProtocolConfigComponent::DumpActiveHandlingUsed()
{
	return false;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to integer MappingArea Id return value
 *
 * @return	MappingArea Id value.
 */
int MappingAreaProtocolConfigComponent::DumpMappingAreaId()
{
	int MappingAreaId = -1;

	StringArray intervalStrings;
	intervalStrings.addTokens(m_MappingAreaIdEdit->getText(), ";, ", "");
	if (intervalStrings.size() == 1)
	{
		MappingAreaId = intervalStrings[0].getIntValue();
	}

	return MappingAreaId;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with MappingArea Id value
 *
 * @param MappingAreaId The id value
 */
void MappingAreaProtocolConfigComponent::FillMappingAreaId(int MappingAreaId)
{
	if (m_MappingAreaIdEdit)
		m_MappingAreaIdEdit->setText(String(MappingAreaId));

	return;
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> MappingAreaProtocolConfigComponent::GetSuggestedSize()
{
	int width = UIS_BasicConfigWidth;
	int height =	UIS_Margin_m + UIS_ElmSize +
					UIS_Margin_m + UIS_ElmSize +
					UIS_Margin_m + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize +
					UIS_Margin_s + UIS_ElmSize +
					UIS_Margin_s;

	return std::pair<int, int>(width, height);
}

/**
 * Reimplemented method to trigger dumping contents of configcomponent member
 * to global config object
 *
 * @param NId The id of the node to dump the config for
 * @param NId The id of the protocol to dump the config for
 * @param config	The global configuration object to dump data to
 * @return	True on success
 */
std::unique_ptr<XmlElement> MappingAreaProtocolConfigComponent::createStateXml()
{
	auto protocolStateXml = ProtocolConfigComponent_Abstract::createStateXml();

	auto mappingAreaXmlElement = protocolStateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	if (!mappingAreaXmlElement)
		mappingAreaXmlElement = protocolStateXml->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	mappingAreaXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DumpMappingAreaId());

	return protocolStateXml;
}

/**
 * Reimplemented setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param NId The id of the node to set the config for
 * @param NId The id of the protocol to set the config for
 * @param config	The global configuration object.
 */
bool MappingAreaProtocolConfigComponent::setStateXml(XmlElement* stateXml)
{
	auto mappingAreaXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	if (mappingAreaXmlElement)
		FillMappingAreaId(mappingAreaXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID)));

	return ProtocolConfigComponent_Abstract::setStateXml(stateXml);
}


//==============================================================================
// Class MIDIProtocolConfigComponent
//==============================================================================
/**
 * Class constructor.
 */
MIDIProtocolConfigComponent::MIDIProtocolConfigComponent(ProtocolRole role)
	: ProtocolConfigComponent_Abstract(role)
{
	m_deviceManager = std::make_unique<AudioDeviceManager>();
	
	// collect available devices to populate our dropdown
	auto midiInputs = juce::MidiInput::getAvailableDevices();
	juce::StringArray midiInputNames;
	for (auto input : midiInputs)
		midiInputNames.add(input.name);

	m_midiInputList = std::make_unique<ComboBox>();
	addAndMakeVisible(m_midiInputList.get());
	m_midiInputList->setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");

	m_midiInputListLabel = std::make_unique<Label>();
	addAndMakeVisible(m_midiInputListLabel.get());
	m_midiInputListLabel->setText("MIDI Input:", juce::dontSendNotification);

	m_midiInputList->addItemList(midiInputNames, 1);
	m_midiInputList->onChange = [this] { setMidiInput(m_midiInputList->getSelectedItemIndex()); };

	// find the first enabled device and use that by default
	for (auto input : midiInputs)
	{
		if (m_deviceManager->isMidiInputDeviceEnabled(input.identifier))
		{
			setMidiInput(midiInputs.indexOf(input));
			break;
		}
	}

	// if no enabled devices were found just use the first one in the list
	if (m_midiInputList->getSelectedId() == 0)
		setMidiInput(0);
}

/**
 * Class destructor.
 */
MIDIProtocolConfigComponent::~MIDIProtocolConfigComponent()
{

}

/**
 * Starts listening to a MIDI input device, enabling it if necessary.
 * @param index	The new device index to set as selected.
 */
void MIDIProtocolConfigComponent::setMidiInput(int index)
{
	auto list = juce::MidiInput::getAvailableDevices();

	if (list.size() <= index)
		return;

	auto newInput = list[index];

	if (!m_deviceManager->isMidiInputDeviceEnabled(newInput.identifier))
		m_deviceManager->setMidiInputDeviceEnabled(newInput.identifier, true);

	m_midiInputList->setSelectedId(index + 1, juce::dontSendNotification);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MIDIProtocolConfigComponent::resized()
{
	double usableWidth = double(getWidth()) - 2 * UIS_Margin_s;
	int labelWidth = (int)(usableWidth * 0.45);
	int editWidth = (int)(usableWidth * 0.3);

	// MidiInput Index select/label
	int yOffset = UIS_Margin_s;
	m_midiInputListLabel->setBounds(Rectangle<int>(UIS_Margin_s, yOffset, labelWidth - UIS_Margin_s, UIS_ElmSize));
	m_midiInputList->setBounds(Rectangle<int>(2 * UIS_Margin_s + labelWidth, yOffset, editWidth - UIS_Margin_m, UIS_ElmSize));

	// ok button
	yOffset += UIS_Margin_s + UIS_ElmSize + UIS_Margin_s;
	m_applyConfigButton->setBounds(Rectangle<int>((int)usableWidth - UIS_ButtonWidth, yOffset, UIS_ButtonWidth, UIS_ElmSize));
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void MIDIProtocolConfigComponent::textEditorFocusLost(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void MIDIProtocolConfigComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The button object that was pressed.
 */
void MIDIProtocolConfigComponent::buttonClicked(Button* button)
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
void MIDIProtocolConfigComponent::AddListener(ProtocolConfigWindow* listener)
{
	m_parentListener = listener;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to list of objects to return to the app to initialize from
 *
 * @return	The list of objects to actively handle when running the engine.
 */
std::vector<RemoteObject> MIDIProtocolConfigComponent::DumpActiveRemoteObjects()
{
	return std::vector<RemoteObject>();
}

/**
 * Method to trigger filling contents of
 * configcomponent member with list of objects
 *
 * @param Objs	The list of objects to set as default.
 */
void MIDIProtocolConfigComponent::FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs)
{
	ignoreUnused(Objs);
}

/**
 * Method to trigger dumping of state of button for if active object handling shall be used
 *
 * @return	True if active object handling shall be used.
 */
bool MIDIProtocolConfigComponent::DumpActiveHandlingUsed()
{
	return false;
}

/**
 * Method to trigger dumping contents of configcomponent member
 * to integer MidiInput Index return value
 *
 * @return	MappingArea Id value.
 */
int MIDIProtocolConfigComponent::DumpSelectedMidiInputIndex()
{
	int MidiInputIndex = -1;

	auto list = juce::MidiInput::getAvailableDevices();

	if (m_midiInputList)
		MidiInputIndex = m_midiInputList->getSelectedId() - 1;

	if (list.size() > MidiInputIndex && MidiInputIndex >= 0)
		return MidiInputIndex;
	else
		return 0;
}

/**
 * Method to trigger filling contents of
 * configcomponent member with MidiInput Index value
 *
 * @param MidiInputIndex The index value
 */
void MIDIProtocolConfigComponent::FillSelectedMidiInputIndex(int MidiInputIndex)
{
	setMidiInput(MidiInputIndex);
}

/**
 * Method to get the components' suggested size. This will be deprecated as soon as
 * the primitive UI is refactored and uses dynamic / proper layouting
 *
 * @return	The pair of int representing the suggested size for this component
 */
const std::pair<int, int> MIDIProtocolConfigComponent::GetSuggestedSize()
{
	int width = UIS_BasicConfigWidth;
	int height = UIS_Margin_m + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize +
		UIS_Margin_s + UIS_ElmSize;

	return std::pair<int, int>(width, height);
}

/**
 * Reimplemented method to trigger dumping contents of configcomponent member
 * to global config object
 *
 * @param NId The id of the node to dump the config for
 * @param NId The id of the protocol to dump the config for
 * @param config	The global configuration object to dump data to
 * @return	True on success
 */
std::unique_ptr<XmlElement> MIDIProtocolConfigComponent::createStateXml()
{
	auto protocolStateXml = std::make_unique<XmlElement>(*m_protocolXmlElement);

	auto midiInputIndexXmlElement = protocolStateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
	if (!midiInputIndexXmlElement)
		midiInputIndexXmlElement = protocolStateXml->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
	midiInputIndexXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER), DumpSelectedMidiInputIndex());

	return protocolStateXml;
}

/**
 * Reimplemented setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param NId The id of the node to set the config for
 * @param NId The id of the protocol to set the config for
 * @param config	The global configuration object.
 */
bool MIDIProtocolConfigComponent::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || stateXml->getTagName() != ((m_ProtocolRole == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB)))
		return false;

	m_protocolXmlElement = std::make_unique<XmlElement>(*stateXml);

	auto midiInputIndexXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
	if (midiInputIndexXmlElement)
		FillSelectedMidiInputIndex(midiInputIndexXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER)));

	return true;
}


// **************************************************************************************
//    class ProtocolConfigWindow
// **************************************************************************************
/**
 * Constructor for the ProtocolConfigWindow class. Internal contents are created here.
 *
 * @param name							The name of the window
 * @param backgroundColour				The background color of the window
 * @param escapeKeyTriggersCloseButton	Flag value to set behaviour of hitting escape key on window
 * @param NId							Node id of the node the protocol this object is used to configure belongs to.
 * @param PId							Protocol id of the protocol this object is used to configure.
 * @param type							Type of the protocol this object is used to configure.
 * @param addToDesktop					Flag value to define if window is to be standalone or embedded in other content
 */
ProtocolConfigWindow::ProtocolConfigWindow(const String& name, Colour backgroundColour, bool escapeKeyTriggersCloseButton, NodeId NId, ProtocolId PId, ProtocolRole role, ProtocolType type, bool addToDesktop)
	: DialogWindow(name, backgroundColour, escapeKeyTriggersCloseButton, addToDesktop)
{
	m_parentListener = 0;

	m_NId = NId;
	m_PId = PId;

	switch (type)
	{
	case ProtocolType::PT_OSCProtocol:
		m_configComponent = std::make_unique<OSCProtocolConfigComponent>(role);
		break;
	case ProtocolType::PT_RTTrPMProtocol:
		// intentionally no break to run into MappingAreaCfg
	case ProtocolType::PT_YamahaOSCProtocol:
		// intentionally no break to run into MappingAreaCfg
	case ProtocolType::PT_ADMOSCProtocol:
		m_configComponent = std::make_unique<MappingAreaProtocolConfigComponent>(role);
		break;
	case ProtocolType::PT_MidiProtocol:
		m_configComponent = std::make_unique<MIDIProtocolConfigComponent>(role);
		break;
	case ProtocolType::PT_OCAProtocol:
		// intentionally no break to run into default
	case ProtocolType::PT_Invalid:
		// intentionally no break to run into default
	default:
		m_configComponent = std::make_unique<BasicProtocolConfigComponent>(role);
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
ProtocolConfigWindow::~ProtocolConfigWindow()
{

}

/**
 * Overloaded method that is called when window close button is pressed.
 * We enshure the window self-destroys here, but first notify the parent of it.
 */
void ProtocolConfigWindow::closeButtonPressed()
{
	if (m_parentListener)
		m_parentListener->childWindowCloseTriggered(this);
}

/**
 * Method to add parent object as 'listener'.
 * This is done in a way JUCE uses to connect child-parent relations for handling 'signal' calls
 */
void ProtocolConfigWindow::AddListener(ProtocolComponent* listener)
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
std::unique_ptr<XmlElement> ProtocolConfigWindow::createStateXml()
{
	return m_configComponent->createStateXml();
}

/**
 * Proxy setter method to trigger filling contents of
 * configcomponent member with configuration contents
 *
 * @param config	The global configuration object.
 */
bool ProtocolConfigWindow::setStateXml(XmlElement* stateXml)
{
	return m_configComponent->setStateXml(stateXml);
}

/**
 * Method to be called by child component to trigger closing down and applying
 * edited contents.
 */
void ProtocolConfigWindow::OnEditingFinished()
{
	closeButtonPressed();
}

/**
 * Proxy method to get the windows' components' suggested size.
 *
 * @return	The pair of int representing the suggested size for the window
 */
const std::pair<int, int> ProtocolConfigWindow::GetSuggestedSize()
{
	return m_configComponent->GetSuggestedSize();
}
