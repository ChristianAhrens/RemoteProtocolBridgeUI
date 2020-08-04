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

#include "DS100_DeviceSimulation.h"

#include "../../ProcessingEngineNode.h"
#include "../../ProcessingEngineConfig.h"


// **************************************************************************************
//    class DS100_DeviceSimulation
// **************************************************************************************
/**
 * Constructor of class DS100_DeviceSimulation.
 *
 * @param parentNode	The objects' parent node that is used by derived objects to forward received message contents to.
 */
DS100_DeviceSimulation::DS100_DeviceSimulation(ProcessingEngineNode* parentNode)
	: ObjectDataHandling_Abstract(parentNode)
{
	m_mode = ObjectHandlingMode::OHM_DS100_DeviceSimulation;

	m_simulatedChCount = 0;
	m_simulatedMappingsCount = 0;
	m_refreshInterval = 50;
}

/**
 * Destructor
 */
DS100_DeviceSimulation::~DS100_DeviceSimulation()
{
	for (auto const & idValuesKV : m_currentValues)
	{
		for (auto value : idValuesKV.second)
		{
			delete value.second.payload;
		}
	}
	
	m_currentValues.clear();
}

/**
 * Reimplemented to set the custom parts from configuration for the datahandling object.
 *
 * @param config	The overall configuration object that can be used to query config data from
 * @param NId		The node id of the parent node this data handling object is child of (needed to access data from config)
 */
bool DS100_DeviceSimulation::setStateXml(XmlElement* stateXml)
{
	if (!ObjectDataHandling_Abstract::setStateXml(stateXml))
		return false;

	if (stateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)) != ProcessingEngineConfig::ObjectHandlingModeToString(OHM_DS100_DeviceSimulation))
		return false;

	stopTimer();

	auto simChCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMCHCNT));
	if (simChCntXmlElement)
		m_simulatedChCount = simChCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 64);

	auto simMapingsCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMMAPCNT));
	if (simMapingsCntXmlElement)
		m_simulatedMappingsCount = simMapingsCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 1);

	auto refreshIntervalXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REFRESHINTERVAL));
	if (refreshIntervalXmlElement)
		m_refreshInterval = refreshIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), 50);

	InitDataValues();

	startTimer(m_refreshInterval);

	return true;
}

/**
 * Method to be called by parent node on receiving data from node protocol with given id
 *
 * @param PId		The id of the protocol that received the data
 * @param Id		The object id to send a message for
 * @param msgData	The actual message value/content data
 * @return	True if successful sent/forwarded, false if not
 */
bool DS100_DeviceSimulation::OnReceivedMessageFromProtocol(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (m_parentNode)
	{
		if (IsDataRequestPollMessage(Id, msgData))
		{
			return ReplyToDataRequest(PId, Id, msgData.addrVal);
		}
		else
		{
			if (m_protocolAIds.contains(PId))
			{
				// Send to all typeB protocols
				bool sendSuccess = true;
				int typeBProtocolCount = m_protocolBIds.size();
				for (int i = 0; i < typeBProtocolCount; ++i)
					sendSuccess = sendSuccess && m_parentNode->SendMessageTo(m_protocolBIds[i], Id, msgData);

				return sendSuccess;

			}
			if (m_protocolBIds.contains(PId))
			{
				// Send to all typeA protocols
				bool sendSuccess = true;
				int typeAProtocolCount = m_protocolAIds.size();
				for (int i = 0; i < typeAProtocolCount; ++i)
					sendSuccess = sendSuccess && m_parentNode->SendMessageTo(m_protocolAIds[i], Id, msgData);

				return sendSuccess;
			}

			return false;
		}
	}

	return false;
}

/**
 * Helper method to detect if incoming message is an osc message adressing a valid object without
 * actual data value (meaning a reply with the current data value of the object is expected).
 *
 * @param Id	The ROI that was received and has to be checked
 * @param msgData	The received message data that has to be checked
 * @return True if the object is valid and no data is contained, false if not
 */
bool DS100_DeviceSimulation::IsDataRequestPollMessage(const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	bool remoteObjectRequiresReply = false;
	switch (Id)
	{
	case ROI_HeartbeatPing:
	case ROI_SoundObject_Position_X:
	case ROI_SoundObject_Position_Y:
	case ROI_SoundObject_Position_XY:
	case ROI_SoundObject_Spread:
	case ROI_SoundObject_DelayMode:
	case ROI_ReverbSendGain:
		remoteObjectRequiresReply = true;
		break;
	case ROI_HeartbeatPong:
	case ROI_Invalid:
	default:
		remoteObjectRequiresReply = false;
		break;
	}

	if (remoteObjectRequiresReply)
	{
		if (msgData.valCount == 0)
			return true;
		else
			return false;
	}
	else
		return false;
}

/**
 * Helper method to reply the correct current simulated data to a received request.
 *
 * @param PId		The id of the protocol that received the request and needs to be sent back the current value
 * @param Id		The ROI that was received and has to be checked
 * @param adressing	The adressing (ch+rec) that was queried and requires answering
 * @return True if the requested reply was successful, false if not
 */
bool DS100_DeviceSimulation::ReplyToDataRequest(const ProtocolId PId, const RemoteObjectIdentifier Id, const RemoteObjectAddressing adressing)
{
	if (m_currentValues.count(Id) == 0)
		return false;
	if (m_currentValues.at(Id).count(adressing) == 0)
		return false;
	
	RemoteObjectMessageData emptyReplyMessageData;
	emptyReplyMessageData.payload = nullptr;
	emptyReplyMessageData.payloadSize = 0;
	emptyReplyMessageData.valCount = 0;
	emptyReplyMessageData.valType = ROVT_NONE;
	emptyReplyMessageData.addrVal.first = 0;
	emptyReplyMessageData.addrVal.second = 0;

	switch (Id)
	{
	case ROI_HeartbeatPing:
		return m_parentNode->SendMessageTo(PId, ROI_HeartbeatPong, emptyReplyMessageData);
	case ROI_SoundObject_Position_XY:
		jassert(m_currentValues.at(Id).at(adressing).valCount == 2);
		jassert(m_currentValues.at(Id).at(adressing).valType == ROVT_FLOAT);
		break;
	case ROI_SoundObject_Position_X:
	case ROI_SoundObject_Position_Y:
	case ROI_SoundObject_Spread:
	case ROI_ReverbSendGain:
		jassert(m_currentValues.at(Id).at(adressing).valCount == 1);
		jassert(m_currentValues.at(Id).at(adressing).valType == ROVT_FLOAT);
		break;
	case ROI_SoundObject_DelayMode:
		jassert(m_currentValues.at(Id).at(adressing).valCount == 1);
		jassert(m_currentValues.at(Id).at(adressing).valType == ROVT_INT);
		break;
	case ROI_HeartbeatPong:
	case ROI_Invalid:
	default:
		return false;
	}

	return m_parentNode->SendMessageTo(PId, Id, m_currentValues.at(Id).at(adressing));
}

/**
 * Helper method to set a new RemoteObjectMessageData obj. to internal map of current values.
 * Takes care of cleaning up previously stored data if required.
 *
 * @param Id	The ROI that shall be stored
 * @param msgData	The message data that shall be stored
 */
void DS100_DeviceSimulation::InitDataValues()
{
	for (int i = ROI_Invalid + 1; i < ROI_UserMAX; i++)
	{
		RemoteObjectIdentifier roi = static_cast<RemoteObjectIdentifier>(i);
		auto& remoteAdressValueMap = m_currentValues[roi];

		for (juce::int16 mapping = 1; mapping <= m_simulatedMappingsCount; mapping++)
		{
			for (juce::int16 channel = 1; channel <= m_simulatedChCount; channel++)
			{
				RemoteObjectAddressing adressing(channel, mapping);
				auto& remoteValue = remoteAdressValueMap[adressing];

				switch (roi)
				{
				case ROI_SoundObject_Position_XY:
					remoteValue.valCount = 2;
					remoteValue.valType = ROVT_FLOAT;
					remoteValue.payload = new float[2];
					static_cast<float*>(remoteValue.payload)[0] = 0.0f;
					static_cast<float*>(remoteValue.payload)[1] = 0.0f;
					remoteValue.payloadSize = 2 * sizeof(float);
					break;
				case ROI_SoundObject_Position_X:
				case ROI_SoundObject_Position_Y:
				case ROI_SoundObject_Spread:
				case ROI_ReverbSendGain:
					remoteValue.valCount = 1;
					remoteValue.valType = ROVT_FLOAT;
					remoteValue.payload = new float;
					*static_cast<float*>(remoteValue.payload) = 0.0f;
					remoteValue.payloadSize = sizeof(float);
					break;
				case ROI_SoundObject_DelayMode:
					remoteValue.valCount = 1;
					remoteValue.valType = ROVT_INT;
					remoteValue.payload = new int;
					*static_cast<int*>(remoteValue.payload) = 0;
					remoteValue.payloadSize = sizeof(int);
					break;
				case ROI_HeartbeatPing:
				case ROI_HeartbeatPong:
				case ROI_Invalid:
				default:
					remoteValue.valCount = 0;
					remoteValue.valType = ROVT_NONE;
					remoteValue.payload = nullptr;
					remoteValue.payloadSize = 0;
					break;
				}
			}
		}
	}
}

/**
 * Reimplemented from Timer to tick 
 */
void DS100_DeviceSimulation::timerCallback()
{
	m_simulationBaseValue += 0.1f;

	for (int i = ROI_Invalid + 1; i < ROI_UserMAX; i++)
	{
		RemoteObjectIdentifier roi = static_cast<RemoteObjectIdentifier>(i);
		auto & remoteAdressValueMap = m_currentValues.at(roi);

		for (juce::int16 mapping = 1; mapping <= m_simulatedMappingsCount; mapping++)
		{
			for (juce::int16 channel = 1; channel <= m_simulatedChCount; channel++)
			{
				RemoteObjectAddressing adressing(channel, mapping);
				auto& remoteValue = remoteAdressValueMap.at(adressing);

				float val1 = sin(m_simulationBaseValue + channel);
				float val2 = cos(m_simulationBaseValue + channel);

				switch (remoteValue.valType)
				{
				case ROVT_FLOAT:
					if (remoteValue.valCount == 1)
					{
						static_cast<float*>(remoteValue.payload)[0] = val1;
					}
					else if (remoteValue.valCount == 2)
					{
						static_cast<float*>(remoteValue.payload)[0] = val1;
						static_cast<float*>(remoteValue.payload)[1] = val2;
					}
					DBG(String(i) + " " + String(channel) + "/" + String(mapping) + " val12 " + String(int(val1)) + " " + String(int(val2)));
					break;
				case ROVT_INT:
					if (remoteValue.valCount == 1)
					{
						static_cast<int*>(remoteValue.payload)[0] = static_cast<int>(val1);
					}
					else if (remoteValue.valCount == 2)
					{
						static_cast<int*>(remoteValue.payload)[0] = static_cast<int>(val1);
						static_cast<int*>(remoteValue.payload)[1] = static_cast<int>(val2);
					}
					DBG(String(i) + " " + String(channel) + "/" + String(mapping) + " val12 " + String(val1) + " " + String(val2));
					break;
				case ROVT_STRING:
				case ROVT_NONE:
				default:
					break;
				}
			}
		}
	}
}