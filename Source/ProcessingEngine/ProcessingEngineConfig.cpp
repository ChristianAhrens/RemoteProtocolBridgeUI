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

#include "ProcessingEngineConfig.h"



// **************************************************************************************
//    class ProcessingEngineConfig
// **************************************************************************************
/**
 * Constructs an object
 * and calls the InitConfiguration method
 */
ProcessingEngineConfig::ProcessingEngineConfig(const File& file)
	: JUCEAppBasics::AppConfigurationBase(file)
{

}

/**
 * Destructor for the object
 */
ProcessingEngineConfig::~ProcessingEngineConfig()
{
}

/**
 * Overridden from AppConfigurationBase to custom validate this xml configuration
 * @return True if the validation succeeded, false if the xml tree is not present or not corrupt
 */
bool ProcessingEngineConfig::isValid()
{
	if (!JUCEAppBasics::AppConfigurationBase::isValid())
		return false;

	XmlElement* rootChild = m_xml->getFirstChildElement();
	if (rootChild == nullptr)
		return false;

	while (rootChild != nullptr)
	{

		if (rootChild->getTagName() == getTagName(TagID::NODE))
		{
			XmlElement* nodeSectionElement = rootChild;
			ValidateUniqueId(nodeSectionElement->getIntAttribute(getAttributeName(AttributeID::ID)));

			XmlElement* objHandleSectionElement = nodeSectionElement->getChildByName(getTagName(TagID::OBJECTHANDLING));
			if (!objHandleSectionElement)
				return false;

			XmlElement* protocolASectionElement = nodeSectionElement->getChildByName(getTagName(TagID::PROTOCOLA));
			if (protocolASectionElement)
			{
                ValidateUniqueId(protocolASectionElement->getIntAttribute(getAttributeName(AttributeID::ID)));
                
				XmlElement* ipAddrSectionElement = protocolASectionElement->getChildByName(getTagName(TagID::IPADDRESS));
				if (!ipAddrSectionElement)
					return false;
				XmlElement* clientPortSectionElement = protocolASectionElement->getChildByName(getTagName(TagID::CLIENTPORT));
				if (!clientPortSectionElement)
					return false;
				XmlElement* hostPortSectionElement = protocolASectionElement->getChildByName(getTagName(TagID::HOSTPORT));
				if (!hostPortSectionElement)
					return false;
				XmlElement* pollingSectionElement = protocolASectionElement->getChildByName(getTagName(TagID::POLLINGINTERVAL));
				if (!pollingSectionElement)
					return false;
				XmlElement* activeObjSectionElement = protocolASectionElement->getChildByName(getTagName(TagID::ACTIVEOBJECTS));
				if (!activeObjSectionElement)
					return false;
			}
			else
				return false;

			XmlElement* protocolBSectionElement = nodeSectionElement->getChildByName(getTagName(TagID::PROTOCOLB));
			if (protocolBSectionElement)
			{
                ValidateUniqueId(protocolBSectionElement->getIntAttribute(getAttributeName(AttributeID::ID)));
                
				XmlElement* ipAddrSectionElement = protocolBSectionElement->getChildByName(getTagName(TagID::IPADDRESS));
				if (!ipAddrSectionElement)
					return false;
				XmlElement* clientPortSectionElement = protocolBSectionElement->getChildByName(getTagName(TagID::CLIENTPORT));
				if (!clientPortSectionElement)
					return false;
				XmlElement* hostPortSectionElement = protocolBSectionElement->getChildByName(getTagName(TagID::HOSTPORT));
				if (!hostPortSectionElement)
					return false;
				XmlElement* pollingSectionElement = protocolBSectionElement->getChildByName(getTagName(TagID::POLLINGINTERVAL));
				if (!pollingSectionElement)
					return false;
				XmlElement* activeObjSectionElement = protocolBSectionElement->getChildByName(getTagName(TagID::ACTIVEOBJECTS));
				if (!activeObjSectionElement)
					return false;
			}
			// no else 'return false' here, since RoleB protocol is not mandatory!

		}
		else if (rootChild->getTagName() == getTagName(TagID::GLOBALCONFIG))
		{
			XmlElement* globalConfigSectionElement = rootChild;

			XmlElement* trafficLoggingSectionElement = globalConfigSectionElement->getChildByName(getTagName(TagID::TRAFFICLOGGING));
			if (!trafficLoggingSectionElement)
				return false;
			XmlElement* engineSectionElement = globalConfigSectionElement->getChildByName(getTagName(TagID::ENGINE));
			if (!engineSectionElement)
				return false;
		}
		else
			return false;

		rootChild = m_xml->getNextElement();
	}

	return true;
}

/**
 * Getter for the list of node ids of current configuration
 *
 * @return	The list of node ids of current configuration
 */
Array<NodeId> ProcessingEngineConfig::GetNodeIds()
{
	auto currentConfig = getConfigState();
	if (!currentConfig)
		return Array<NodeId>{};

	Array<NodeId> NIds;
	XmlElement* nodeXmlElement = currentConfig->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	while (nodeXmlElement != nullptr)
	{
		NIds.add(nodeXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID)));
		nodeXmlElement = nodeXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	}

	return NIds;
}

/**
 * Method to read the node configuration part regarding active objects per protocol
 *
 * @param ActiveObjectsElement	The xml element for the nodes' protocols' active objects in the DOM
 * @param RemoteObjects			The remote objects list to fill according config contents
 * @return	True if remote objects were inserted, false if empty list is returned
 */
bool ProcessingEngineConfig::ReadActiveObjects(XmlElement* activeObjectsElement, Array<RemoteObject>& RemoteObjects)
{
	RemoteObjects.clear();

	if (!activeObjectsElement || activeObjectsElement->getTagName() != getTagName(TagID::ACTIVEOBJECTS))
		return false;

	XmlElement* objectChild = activeObjectsElement->getFirstChildElement();
	RemoteObject obj;
	
	while (objectChild != nullptr)
	{
		Array<int> channels;
		String chStrToSplit = objectChild->getAttributeValue(0);
		StringArray chNumbers;
		chNumbers.addTokens(chStrToSplit, ", ", "");
		for (int j = 0; j < chNumbers.size(); ++j)
		{
			int chNum = chNumbers[j].getIntValue();
			if (chNum > 0)
				channels.add(chNum);
		}
	
		Array<int> records;
		String recStrToSplit = objectChild->getAttributeValue(1);
		StringArray recNumbers;
		recNumbers.addTokens(recStrToSplit, ", ", "");
		for (int j = 0; j < recNumbers.size(); ++j)
		{
			int recNum = recNumbers[j].getIntValue();
			if (recNum > 0)
				records.add(recNum);
		}
	
		for (int i = ROI_Invalid + 1; i < ROI_UserMAX; ++i)
		{
			RemoteObjectIdentifier ROId = (RemoteObjectIdentifier)i;
			if (objectChild->getTagName() == GetObjectDescription(ROId).removeCharacters(" "))
			{
				obj.Id = ROId;
	
				// now that we have all channels and records, recursively iterate through both arrays
				// to add an entry to our active objects list for every resulting ch/rec combi object
				for (int j = 0; j < channels.size(); ++j)
				{
					if (records.size() > 0)
					{
						for (int k = 0; k < records.size(); ++k)
						{
							obj.Addr.first = (int16)channels[j];
							obj.Addr.second = (int16)records[k];
							RemoteObjects.add(obj);
						}
					}
					else
					{
						obj.Addr.first = (int16)channels[j];
						obj.Addr.second = -1;
						RemoteObjects.add(obj);
					}
				}
			}
		}
	
		objectChild = objectChild->getNextElement();
	}

	return !RemoteObjects.isEmpty();
}

/**
 * Method to read the node configuration part regarding muted objects per protocol
 *
 * @param mutedObjectChannelsElement	The xml element for the nodes' protocols' muted objects in the DOM
 * @param channel						The remote object channels list to fill according config contents
 * @return	True if remote object channels were inserted, false if empty list is returned
 */
bool ProcessingEngineConfig::ReadMutedObjectChannels(XmlElement* mutedObjectChannelsElement, Array<int>& channels)
{
	channels.clear();

	if (!mutedObjectChannelsElement || mutedObjectChannelsElement->getTagName() != getTagName(TagID::MUTEDCHANNELS))
		return false;

	XmlElement* objectsTextXmlElement = mutedObjectChannelsElement->getFirstChildElement();
	if (objectsTextXmlElement && objectsTextXmlElement->isTextElement())
	{
		String chStrToSplit = objectsTextXmlElement->getText();
		StringArray chNumbers;
		chNumbers.addTokens(chStrToSplit, ", ", "");
		for (int j = 0; j < chNumbers.size(); ++j)
		{
			int chNum = chNumbers[j].getIntValue();
			if (chNum > 0)
				channels.add(chNum);
		}
	}

	return !channels.isEmpty();
}

/**
 * Method to read the node configuration part regarding polling interval per protocol.
 * Includes fixup to default if not found in xml.
 *
 * @param PollingIntervalElement	The xml element for the nodes' protocols' polling interval in the DOM
 * @param PollingInterval			The polling interval var to fill according config contents
 * @return	True if value was read from xml, false if default was used.
 */
bool ProcessingEngineConfig::ReadPollingInterval(XmlElement* PollingIntervalElement, int& PollingInterval)
{
	XmlElement* objectChild = PollingIntervalElement;
	PollingInterval = ET_DefaultPollingRate;

	if (objectChild != nullptr && objectChild->getAttributeName(0) == getTagName(TagID::POLLINGINTERVAL))
	{
		PollingInterval = objectChild->getAttributeValue(0).getIntValue();
		return true;
	}
	else
		return false;
}

/**
 * Method to write the node configuration part regarding active objects per protocol
 *
 * @param ActiveObjectsElement	The xml element for the nodes' protocols' active objects in the DOM
 * @param RemoteObjects			The remote objects to set active in config
 * @return	True on success, false on failure
 */
bool ProcessingEngineConfig::WriteActiveObjects(XmlElement* ActiveObjectsElement, Array<RemoteObject> const& RemoteObjects)
{
	if (!ActiveObjectsElement)
		return false;

	int RemoteObjectCount = RemoteObjects.size();

	HashMap<int, Array<int>> channelsPerObj;
	HashMap<int, Array<int>> recordsPerObj;
	for (int j = 0; j < RemoteObjectCount; ++j)
	{
		Array<int> selChs = channelsPerObj[RemoteObjects[j].Id];
		if (!selChs.contains(RemoteObjects[j].Addr.first))
		{
			selChs.add(RemoteObjects[j].Addr.first);
			channelsPerObj.set(RemoteObjects[j].Id, selChs);
		}

		Array<int> selRecs = recordsPerObj[RemoteObjects[j].Id];
		if (!selRecs.contains(RemoteObjects[j].Addr.second))
		{
			selRecs.add(RemoteObjects[j].Addr.second);
			recordsPerObj.set(RemoteObjects[j].Id, selRecs);
		}
	}

	for (int k = ROI_Invalid + 1; k < ROI_UserMAX; ++k)
	{
		if (XmlElement* ObjectElement = ActiveObjectsElement->createNewChildElement(GetObjectDescription((RemoteObjectIdentifier)k).removeCharacters(" ")))
		{
			String selChanTxt;
			for (int j = 0; j < channelsPerObj[k].size(); ++j)
			{
				if (channelsPerObj[k][j] > 0)
				{
					if (!selChanTxt.isEmpty())
						selChanTxt << ", ";
					selChanTxt << channelsPerObj[k][j];
				}
			}
			ObjectElement->setAttribute("channels", selChanTxt);

			String selRecTxt;
			for (int j = 0; j < recordsPerObj[k].size(); ++j)
			{
				if (recordsPerObj[k][j] > 0)
				{
					if (!selRecTxt.isEmpty())
						selRecTxt << ", ";
					selRecTxt << recordsPerObj[k][j];
				}
			}
			ObjectElement->setAttribute("records", selRecTxt);
		}
	}

	return true;
}

/**
 * Method to write the node configuration part regarding muted channels per protocol
 *
 * @param mutedObjectChannelsElement	The xml element for the nodes' protocols' active objects in the DOM
 * @param channels			The remote objects to set active in config
 * @return	True on success, false on failure
 */
bool ProcessingEngineConfig::WriteMutedObjectChannels(XmlElement* mutedObjectChannelsElement, Array<int> const& channels)
{
	if (!mutedObjectChannelsElement || mutedObjectChannelsElement->getTagName() != getTagName(TagID::MUTEDCHANNELS))
		return false;

	String mutedChannels;
	for (auto channelNr : channels)
	{
		if (!mutedChannels.isEmpty())
			mutedChannels << ", ";

		mutedChannels << channelNr;
	}

	auto mutedChannelsTextXmlElement = mutedObjectChannelsElement->getFirstChildElement();
	if (!mutedChannelsTextXmlElement)
	{
		mutedChannelsTextXmlElement = mutedObjectChannelsElement->createTextElement(mutedChannels);
		mutedObjectChannelsElement->addChildElement(mutedChannelsTextXmlElement);
	}
	else if (mutedChannelsTextXmlElement->isTextElement())
	{
		mutedChannelsTextXmlElement->setText(mutedChannels);
	}
	else
		jassertfalse;

	return true;
}

/**
 * Method to replace the node configuration part regarding active objects per protocol
 *
 * @param ActiveObjectsElement	The xml element for the nodes' protocols' active objects in the DOM
 * @param RemoteObjects			The remote objects to set active in config
 * @return	True on success, false on failure
 */
bool ProcessingEngineConfig::ReplaceActiveObjects(XmlElement* ActiveObjectsElement, Array<RemoteObject> const& RemoteObjects)
{
	if (!ActiveObjectsElement)
		return false;

	int RemoteObjectCount = RemoteObjects.size();

	HashMap<int, Array<int>> channelsPerObj;
	HashMap<int, Array<int>> recordsPerObj;
	for (int j = 0; j < RemoteObjectCount; ++j)
	{
		Array<int> selChs = channelsPerObj[RemoteObjects[j].Id];
		if (!selChs.contains(RemoteObjects[j].Addr.first))
		{
			selChs.add(RemoteObjects[j].Addr.first);
			channelsPerObj.set(RemoteObjects[j].Id, selChs);
		}

		Array<int> selRecs = recordsPerObj[RemoteObjects[j].Id];
		if (!selRecs.contains(RemoteObjects[j].Addr.second))
		{
			selRecs.add(RemoteObjects[j].Addr.second);
			recordsPerObj.set(RemoteObjects[j].Id, selRecs);
		}
	}

	for (int k = ROI_Invalid + 1; k < ROI_UserMAX; ++k)
	{
		if (XmlElement* ObjectElement = ActiveObjectsElement->getChildByName(GetObjectDescription((RemoteObjectIdentifier)k).removeCharacters(" ")))
		{
			String selChanTxt;
			for (int j = 0; j < channelsPerObj[k].size(); ++j)
			{
				if (channelsPerObj[k][j] > 0)
				{
					if (!selChanTxt.isEmpty())
						selChanTxt << ", ";
					selChanTxt << channelsPerObj[k][j];
				}
			}
			ObjectElement->setAttribute("channels", selChanTxt);

			String selRecTxt;
			for (int j = 0; j < recordsPerObj[k].size(); ++j)
			{
				if (recordsPerObj[k][j] > 0)
				{
					if (!selRecTxt.isEmpty())
						selRecTxt << ", ";
					selRecTxt << recordsPerObj[k][j];
				}
			}
			ObjectElement->setAttribute("records", selRecTxt);
		}
	}

	return true;
}

/**
 * Method to generate next available unique id.
 * There is no cleanup / recycling of old ids available yet,
 * we only perform a ++ on a static counter
 */
int ProcessingEngineConfig::GetNextUniqueId()
{
    return ++uniqueIdCounter;
}

/**
 * Method to validate new external unique id to not be in conflict with internal id counter.
 * Internal counter is simply increased to not be in conflict with given new id.
 */
int ProcessingEngineConfig::ValidateUniqueId(int uniqueId)
{
	if (uniqueIdCounter < uniqueId)
		uniqueIdCounter = uniqueId;

	return uniqueId;
}

/**
 * Returns a default global config xml section
 */
std::unique_ptr<XmlElement>	ProcessingEngineConfig::GetDefaultGlobalConfig()
{
	auto globalConfigXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::GLOBALCONFIG));

	auto trafficLoggingXmlElement = globalConfigXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::TRAFFICLOGGING));
	if (trafficLoggingXmlElement)
		trafficLoggingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ALLOWED), 1);

	auto engineXmlElement = globalConfigXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ENGINE));
	if (engineXmlElement)
		engineXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::AUTOSTART), 0);

    return globalConfigXmlElement;
}

/**
 * Adds a bridging node with default values to configuration object
 */
std::unique_ptr<XmlElement>	ProcessingEngineConfig::GetDefaultNode()
{
	auto nodeXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));

	auto thisConfig = dynamic_cast<ProcessingEngineConfig*>(ProcessingEngineConfig::getInstance());
	if (thisConfig)
		nodeXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), thisConfig->GetNextUniqueId());
	
	auto objectHandlingXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (objectHandlingXmlElement)
	{
		objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Bypass));
		objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DATAPRECISION), 0.001);

		auto aChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
		if (aChCntXmlElement)
			aChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 0);

		auto bChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
		if (bChCntXmlElement)
			bChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 0);
	}

	nodeXmlElement->addChildElement(GetDefaultProtocol(ProtocolRole::PR_A).release());
	nodeXmlElement->addChildElement(GetDefaultProtocol(ProtocolRole::PR_B).release());

    return nodeXmlElement;
}

/**
 * Creates a bridging node protocol xml element of specified role with default values
 *
 * @param role	The protocol role to use
 * @return The created xml element or nullptr
 */
std::unique_ptr<XmlElement> ProcessingEngineConfig::GetDefaultProtocol(ProtocolRole role)
{
	auto protocolXmlElement = std::make_unique<XmlElement>((role == ProtocolRole::PR_A) ? ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) : ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

	auto thisConfig = dynamic_cast<ProcessingEngineConfig*>(ProcessingEngineConfig::getInstance());
	if (thisConfig)
		protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), thisConfig->GetNextUniqueId());

	protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
	protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 1);

	auto clientPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), 50010);

	auto hostPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), 50011);

	// Active objects preparation
	Array<RemoteObject> activeObjects;
	RemoteObject objectX, objectY;

	objectX.Id = ROI_SoundObject_Position_X;
	objectY.Id = ROI_SoundObject_Position_Y;
	for (int16 i = 1; i <= 16; ++i)
	{
		RemoteObjectAddressing addr;
		addr.first = i; //channel = source
		addr.second = 1; //record = mapping

		objectX.Addr = addr;
		objectY.Addr = addr;

		activeObjects.add(objectX);
		activeObjects.add(objectY);
	}
	auto activeObjsXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
	if (activeObjsXmlElement)
		ProcessingEngineConfig::WriteActiveObjects(activeObjsXmlElement, activeObjects);

	auto ipAdressXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressXmlElement)
		ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), "10.255.0.100");

	auto pollIntervalXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
	if (pollIntervalXmlElement)
		pollIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), ET_DefaultPollingRate);

    return protocolXmlElement;
}

/**
 * Removes the bridging node with given NodeId from configuration object
 *
 * @param NId	The id of the node to remove
 */
bool ProcessingEngineConfig::RemoveNodeOrProtocol(int Id)
{
	auto nodeXmlElement = m_xml->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(Id));
	if (nodeXmlElement)
	{
		m_xml->removeChildElement(nodeXmlElement, true);
		triggerWatcherUpdate();
		return true;
	}
	else
		return false;
}

/**
* Helper to resolve ROI to human readable string.
*
* @param Id	The remote object id to be resolved to a string.
*/
String ProcessingEngineConfig::GetObjectDescription(RemoteObjectIdentifier Id)
{
	switch (Id)
	{
	case ROI_HeartbeatPing:
		return "PING";
	case ROI_HeartbeatPong:
		return "PONG";
	case ROI_SoundObject_Position_X:
		return "Sound Object Position X";
	case ROI_SoundObject_Position_Y:
		return "Sound Object Position Y";
	case ROI_SoundObject_Position_XY:
		return "Sound Object Position XY";
	case ROI_SoundObject_Spread:
		return "Sound Object Spread";
	case ROI_SoundObject_DelayMode:
		return "Sound Object Delay Mode";
	case ROI_MatrixInput_ReverbSendGain:
		return "En-Space Send Gain";
	default:
		return "-";
	}
}

/**
* Convenience function to resolve enum to sth. human readable (e.g. in config file)
*/
String  ProcessingEngineConfig::ProtocolTypeToString(ProtocolType pt)
{
	switch (pt)
	{
	case PT_OCAProtocol:
		return "OCA";
	case PT_OSCProtocol:
		return "OSC";
	case PT_Invalid:
		return "Invalid";
	default:
		return "";
	}
}

/**
* Convenience function to resolve string to enum
*/
ProtocolType  ProcessingEngineConfig::ProtocolTypeFromString(String type)
{
	if (type == "OCA")
		return PT_OCAProtocol;
	if (type == "OSC")
		return PT_OSCProtocol;

	return PT_Invalid;
}

/**
* Convenience function to resolve enum to sth. human readable (e.g. in config file)
*/
String ProcessingEngineConfig::ObjectHandlingModeToString(ObjectHandlingMode ohm)
{
	switch (ohm)
	{
	case OHM_Bypass:
		return "Bypass (A<->B)";
	case OHM_Remap_A_X_Y_to_B_XY:
		return "Reroute single A (x), (y) to combi B (xy)";
	case OHM_Mux_nA_to_mB:
		return "Multiplex multiple n-ch. A to m-ch. B protocols";
	case OHM_Forward_only_valueChanges:
		return "Forward value changes only";
    case OHM_Forward_A_to_B_only:
        return "Forward data only (A->B)";
    case OHM_Reverse_B_to_A_only:
        return "Reverse data only (B->A)";
	case OHM_DS100_DeviceSimulation:
		return "Simulate DS100 object poll answers";
	default:
		return "";
	}
}

/**
* Convenience function to resolve string to enum
*/
ObjectHandlingMode ProcessingEngineConfig::ObjectHandlingModeFromString(String mode)
{
	if (mode == ObjectHandlingModeToString(OHM_Bypass))
		return OHM_Bypass;
	if (mode == ObjectHandlingModeToString(OHM_Remap_A_X_Y_to_B_XY))
		return OHM_Remap_A_X_Y_to_B_XY;
	if (mode == ObjectHandlingModeToString(OHM_Mux_nA_to_mB))
		return OHM_Mux_nA_to_mB;
	if (mode == ObjectHandlingModeToString(OHM_Forward_only_valueChanges))
		return OHM_Forward_only_valueChanges;
    if (mode == ObjectHandlingModeToString(OHM_Forward_A_to_B_only))
        return OHM_Forward_A_to_B_only;
    if (mode == ObjectHandlingModeToString(OHM_Reverse_B_to_A_only))
        return OHM_Reverse_B_to_A_only;
	if (mode == ObjectHandlingModeToString(OHM_DS100_DeviceSimulation))
		return OHM_DS100_DeviceSimulation;

	return OHM_Invalid;
}
