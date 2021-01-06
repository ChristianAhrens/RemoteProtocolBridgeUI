/*
  ==============================================================================

	DS100_DeviceSimulation.cpp
	Created: 4 Aug 2020 08:47:37am
	Author:  Christian Ahrens

  ==============================================================================
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
	SetMode(ObjectHandlingMode::OHM_DS100_DeviceSimulation);

	ScopedLock l(m_currentValLock);
	m_simulatedChCount = 0;
	m_simulatedMappingsCount = 0;

	m_refreshInterval = 50;
	m_simulationBaseValue = 0.0f;
}

/**
 * Destructor
 */
DS100_DeviceSimulation::~DS100_DeviceSimulation()
{
	stopTimerThread();
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

	stopTimerThread();
	{
		ScopedLock l(m_currentValLock);

		auto simChCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMCHCNT));
		if (simChCntXmlElement)
			m_simulatedChCount = simChCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 64);

		auto simMapingsCntXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::SIMMAPCNT));
		if (simMapingsCntXmlElement)
			m_simulatedMappingsCount = simMapingsCntXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), 1);

		auto refreshIntervalXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REFRESHINTERVAL));
		if (refreshIntervalXmlElement)
			m_refreshInterval = refreshIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), 50);
	}
	InitDataValues();

	if (m_refreshInterval > 0)
		startTimerThread(m_refreshInterval, m_refreshInterval);

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
	auto parentNode = ObjectDataHandling_Abstract::GetParentNode();
	if (parentNode)
	{
		if (IsDataRequestPollMessage(Id, msgData))
		{
			return ReplyToDataRequest(PId, Id, msgData._addrVal);
		}
		else
		{
			SetDataValue(PId, Id, msgData);

			auto protocolAIter = std::find(GetProtocolAIds().begin(), GetProtocolAIds().end(), PId);
			if (protocolAIter != GetProtocolAIds().end())
			{
				// Send to all typeB protocols
				auto sendSuccess = true;
				for (auto const protocolB : GetProtocolBIds())
					sendSuccess = sendSuccess && parentNode->SendMessageTo(protocolB, Id, msgData);

				return sendSuccess;

			}
			auto protocolBIter = std::find(GetProtocolBIds().begin(), GetProtocolBIds().end(), PId);
			if (protocolBIter != GetProtocolBIds().end())
			{
				// Send to all typeA protocols
				auto sendSuccess = true;
				for (auto const protocolA : GetProtocolAIds())
					sendSuccess = sendSuccess && parentNode->SendMessageTo(protocolA, Id, msgData);

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
	case ROI_CoordinateMapping_SourcePosition_X:
	case ROI_CoordinateMapping_SourcePosition_Y:
	case ROI_CoordinateMapping_SourcePosition_XY:
	case ROI_Positioning_SourceSpread:
	case ROI_Positioning_SourceDelayMode:
	case ROI_MatrixInput_ReverbSendGain:
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
		if (msgData._valCount == 0)
			return true;
		else
			return false;
	}
	else
		return false;
}

/**
 * Helper method to create a string representation for a given message id/data pair
 */
void DS100_DeviceSimulation::PrintMessageInfo(const std::pair<RemoteObjectIdentifier, RemoteObjectMessageData>& idDataKV)
{
	switch (idDataKV.first)
	{
	case ROI_HeartbeatPong:
		DBG("Sending Pong reply.");
		break;
	case ROI_CoordinateMapping_SourcePosition_XY:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + "Rec" + String(idDataKV.second._addrVal._second) + " XY position reply (" + String(static_cast<float*>(idDataKV.second._payload)[0]) + ", " + String(static_cast<float*>(idDataKV.second._payload)[1]) + ").");
		break;
	case ROI_CoordinateMapping_SourcePosition_X:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + " X position reply (" + String(static_cast<float*>(idDataKV.second._payload)[0]) + ").");
		break;
	case ROI_CoordinateMapping_SourcePosition_Y:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + " Y position reply (" + String(static_cast<float*>(idDataKV.second._payload)[0]) + ").");
		break;
	case ROI_Positioning_SourceSpread:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + " spread reply (" + String(static_cast<float*>(idDataKV.second._payload)[0]) + ").");
		break;
	case ROI_MatrixInput_ReverbSendGain:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + " EnSpace gain reply (" + String(static_cast<float*>(idDataKV.second._payload)[0]) + ").");
		break;
	case ROI_Positioning_SourceDelayMode:
		DBG("Sending Ch" + String(idDataKV.second._addrVal._first) + " delaymode reply (" + String(static_cast<int*>(idDataKV.second._payload)[0]) + ").");
		break;
	default:
		return;
	}
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
	const ProcessingEngineNode* parentNode = ObjectDataHandling_Abstract::GetParentNode();
	if (!parentNode)
		return false;

	ScopedLock l(m_currentValLock);
	
	if (m_currentValues.count(Id) == 0)
		return false;
	if (m_currentValues.at(Id).count(adressing) == 0)
		return false;

	auto dataReply = std::make_pair(Id, m_currentValues.at(Id).at(adressing));

	switch (Id)
	{
	case ROI_HeartbeatPing:
		jassert(m_currentValues.at(Id).at(adressing)._valType == ROVT_NONE);
		dataReply.first = ROI_HeartbeatPong;
		break;
	case ROI_CoordinateMapping_SourcePosition_XY:
		jassert(m_currentValues.at(Id).at(adressing)._valCount == 2);
		jassert(m_currentValues.at(Id).at(adressing)._valType == ROVT_FLOAT);
		dataReply.second._addrVal = adressing;
		break;
	case ROI_CoordinateMapping_SourcePosition_X:
	case ROI_CoordinateMapping_SourcePosition_Y:
	case ROI_Positioning_SourceSpread:
	case ROI_MatrixInput_ReverbSendGain:
		jassert(m_currentValues.at(Id).at(adressing)._valCount == 1);
		jassert(m_currentValues.at(Id).at(adressing)._valType == ROVT_FLOAT);
		dataReply.second._addrVal = adressing;
		break;
	case ROI_Positioning_SourceDelayMode:
		jassert(m_currentValues.at(Id).at(adressing)._valCount == 1);
		jassert(m_currentValues.at(Id).at(adressing)._valType == ROVT_INT);
		dataReply.second._addrVal = adressing;
		break;
	case ROI_HeartbeatPong:
	case ROI_Invalid:
	default:
		return false;
	}

#ifdef DEBUG
	PrintMessageInfo(dataReply);
#endif

	return parentNode->SendMessageTo(PId, dataReply.first, dataReply.second);
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
	RemoteObjectMessageData emptyReplyMessageData;
	{
		emptyReplyMessageData._payload = nullptr;
		emptyReplyMessageData._payloadSize = 0;
		emptyReplyMessageData._valCount = 0;
		emptyReplyMessageData._valType = ROVT_NONE;
		emptyReplyMessageData._addrVal._first = INVALID_ADDRESS_VALUE;
		emptyReplyMessageData._addrVal._second = INVALID_ADDRESS_VALUE;
		ScopedLock l(m_currentValLock);
		m_currentValues[ROI_HeartbeatPing].insert(std::make_pair(emptyReplyMessageData._addrVal, emptyReplyMessageData));
		m_currentValues[ROI_HeartbeatPong].insert(std::make_pair(emptyReplyMessageData._addrVal, emptyReplyMessageData));
	}

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; i++)
	{
		RemoteObjectIdentifier roi = static_cast<RemoteObjectIdentifier>(i);
		auto remoteAdressValueMap = std::map<RemoteObjectAddressing, RemoteObjectMessageData>{};

		for (MappingId mapping = 1; mapping <= m_simulatedMappingsCount; mapping++)
		{
			for (SourceId channel = 1; channel <= m_simulatedChCount; channel++)
			{
				RemoteObjectAddressing adressing(channel, mapping);
				auto remoteValue = RemoteObjectMessageData{};

				switch (roi)
				{
				case ROI_CoordinateMapping_SourcePosition_XY:
					remoteValue._valCount = 2;
					remoteValue._valType = ROVT_FLOAT;
					remoteValue._payload = new float[2];
					remoteValue._payloadOwned = true;
					static_cast<float*>(remoteValue._payload)[0] = 0.0f;
					static_cast<float*>(remoteValue._payload)[1] = 0.0f;
					remoteValue._payloadSize = 2 * sizeof(float);
					break;
				case ROI_CoordinateMapping_SourcePosition_X:
				case ROI_CoordinateMapping_SourcePosition_Y:
				case ROI_Positioning_SourceSpread:
				case ROI_MatrixInput_ReverbSendGain:
					remoteValue._valCount = 1;
					remoteValue._valType = ROVT_FLOAT;
					remoteValue._payload = new float;
					remoteValue._payloadOwned = true;
					*static_cast<float*>(remoteValue._payload) = 0.0f;
					remoteValue._payloadSize = sizeof(float);
					break;
				case ROI_Positioning_SourceDelayMode:
					remoteValue._valCount = 1;
					remoteValue._valType = ROVT_INT;
					remoteValue._payload = new int;
					remoteValue._payloadOwned = true;
					*static_cast<int*>(remoteValue._payload) = 0;
					remoteValue._payloadSize = sizeof(int);
					break;
				case ROI_HeartbeatPing:
				case ROI_HeartbeatPong:
				case ROI_Invalid:
				default:
					remoteValue._valCount = 0;
					remoteValue._valType = ROVT_NONE;
					remoteValue._payload = nullptr;
					remoteValue._payloadSize = 0;
					break;
				}

				remoteAdressValueMap.insert(std::make_pair(adressing, remoteValue));
			}
		}

		ScopedLock l(m_currentValLock);
		m_currentValues.insert(std::make_pair(roi, remoteAdressValueMap));
	}

	return;
}

/**
 * Helper method to set the data from an incoming message to internal map of simulated values.
 * (If timer is active, this will be overwritten on next timer timeout - otherwise 
 * a new poll request will trigger an answer with the value set in this method.)
 *
 * @param PId	The id of the protocol that received the message.
 * @param Id	The ROI that was received
 * @param msgData	The received message data from which the value shall be taken
 */
void DS100_DeviceSimulation::SetDataValue(const ProtocolId PId, const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	ignoreUnused(PId);

	auto newMsgData = msgData;

	ScopedLock l(m_currentValLock);

	if (m_currentValues.count(Id) > 0)
	{
		if (m_currentValues.at(Id).count(msgData._addrVal) > 0)
		{
			m_currentValues.at(Id).at(msgData._addrVal) = newMsgData;
		}
		else
		{
			m_currentValues.at(Id).insert(std::make_pair(msgData._addrVal, newMsgData));
		}
	}
	else
	{
		m_currentValues.insert(std::pair<RemoteObjectIdentifier, std::map<RemoteObjectAddressing, RemoteObjectMessageData>>(Id, std::map<RemoteObjectAddressing, RemoteObjectMessageData>()));
		m_currentValues.at(Id).insert(std::make_pair(msgData._addrVal, newMsgData));
	}
}

/**
 * Reimplemented from Timer to tick 
 */
void DS100_DeviceSimulation::timerThreadCallback()
{
	{
		ScopedLock l(m_currentValLock);
		m_simulationBaseValue += 0.1f;
	}

	for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; i++)
	{
		RemoteObjectIdentifier roi = static_cast<RemoteObjectIdentifier>(i);

		ScopedLock l(m_currentValLock);
		auto & remoteAdressValueMap = m_currentValues.at(roi);

		for (MappingId mapping = 1; mapping <= m_simulatedMappingsCount; mapping++)
		{
			for (SourceId channel = 1; channel <= m_simulatedChCount; channel++)
			{
				RemoteObjectAddressing adressing(channel, mapping);
				auto& remoteValue = remoteAdressValueMap.at(adressing);

				float val1 = (sin(m_simulationBaseValue + (channel * 0.1f)) + 1.0f) * 0.5f;
				float val2 = (cos(m_simulationBaseValue + (channel * 0.1f)) + 1.0f) * 0.5f;

				switch (remoteValue._valType)
				{
				case ROVT_FLOAT:
					if (remoteValue._valCount == 1)
					{
						if (roi == ROI_MatrixInput_ReverbSendGain)
							val1 = (val1 * 100.0f) - 100.0f;

						if (roi == ROI_CoordinateMapping_SourcePosition_Y)
							static_cast<float*>(remoteValue._payload)[0] = val2;
						else
							static_cast<float*>(remoteValue._payload)[0] = val1;
					}
					else if (remoteValue._valCount == 2)
					{
						static_cast<float*>(remoteValue._payload)[0] = val1;
						static_cast<float*>(remoteValue._payload)[1] = val2;
					}
					break;
				case ROVT_INT:
					if (remoteValue._valCount == 1)
					{
						if (roi == ROI_Positioning_SourceDelayMode)
							val1 = val1 * 3.0f;
						
						static_cast<int*>(remoteValue._payload)[0] = static_cast<int>(val1);
					}
					else if (remoteValue._valCount == 2)
					{
						static_cast<int*>(remoteValue._payload)[0] = static_cast<int>(val1);
						static_cast<int*>(remoteValue._payload)[1] = static_cast<int>(val2);
					}
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
