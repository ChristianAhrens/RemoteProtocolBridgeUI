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

#include "RTTrPMProtocolProcessor.h"

#include "../../../ProcessingEngine/ProcessingEngineConfig.h"


// **************************************************************************************
//    class RTTrPMProtocolProcessor
// **************************************************************************************
/**
 * Derived RTTrPM remote protocol processing class
 */
RTTrPMProtocolProcessor::RTTrPMProtocolProcessor(const NodeId& parentNodeId, int listenerPortNumber)
	: ProtocolProcessor_Abstract(parentNodeId), m_rttrpmReceiver(listenerPortNumber)
{
	m_type = ProtocolType::PT_RTTrPMProtocol;

	// RTTrPMProtocolProcessor derives from OSCReceiver::Listener
	m_rttrpmReceiver.addListener(this);
}

/**
 * Destructor
 */
RTTrPMProtocolProcessor::~RTTrPMProtocolProcessor()
{
	Stop();

	m_rttrpmReceiver.removeListener(this);
}

/**
 * Overloaded method to start the protocol processing object.
 * Usually called after configuration has been set.
 */
bool RTTrPMProtocolProcessor::Start()
{
	m_IsRunning = m_rttrpmReceiver.start();

	return m_IsRunning;
}

/**
 * Overloaded method to stop to protocol processing object.
 */
bool RTTrPMProtocolProcessor::Stop()
{
	m_IsRunning = !m_rttrpmReceiver.stop();

	return m_IsRunning;
}

/**
 * Reimplemented setter for protocol config data.
 * This calls the base implementation and in addition
 * takes care of setting polling interval.
 *
 * @param protocolData	The configuration data struct with config data
 * @param activeObjs	The objects to use as 'active' for this protocol
 * @param NId			The node id of the parent node this protocol processing object is child of (needed to access data from config)
 * @param PId			The protocol id of this protocol processing object (needed to access data from config)
 */
bool RTTrPMProtocolProcessor::setStateXml(XmlElement* stateXml)
{
	if (!ProtocolProcessor_Abstract::setStateXml(stateXml))
		return false;
	else
	{
		return true;
	}
}

/**
 * Setter for remote object to specifically activate.
 * For OSC processing this is used to activate internal polling
 * of the object values.
 * In case an empty list of objects is passed, polling is stopped and
 * the internal list is cleared.
 *
 * @param activeObjsXmlElement	The xml element that has to be parsed to get the object data
 */
void RTTrPMProtocolProcessor::SetRemoteObjectsActive(XmlElement* activeObjsXmlElement)
{
	ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, m_activeRemoteObjects);
}

/**
 * Setter for remote object channels to not forward for further processing.
 * This uses a helper method from engine config to get a list of
 * object ids into the corresponding internal member.
 *
 * @param mutedObjChsXmlElement	The xml element that has to be parsed to get the object data
 */
void RTTrPMProtocolProcessor::SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement)
{
	ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, m_mutedRemoteObjectChannels);
}

/**
 * Method to trigger sending of a message
 *
 * @param Id		The id of the object to send a message for
 * @param msgData	The message payload and metadata
 */
bool RTTrPMProtocolProcessor::SendRemoteObjectMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	ignoreUnused(Id);
	ignoreUnused(msgData);

	// currently, RTTrPM Protocol implementation does not support sending data
	return false;
}

/**
 * Called when the RTTrPM server receives a new RTTrPM packet module
 *
 * @param module			The received RTTrPM module.
 * @param senderIPAddress	The ip the message originates from.
 * @param senderPort			The port this message was received on.
 */
void RTTrPMProtocolProcessor::RTTrPMModuleReceived(const CPacketModule& RTTrPMmodule, const String& senderIPAddress, const int& senderPort)
{
	ignoreUnused(senderPort);
	if (senderIPAddress != m_ipAddress)
	{
#ifdef DEBUG
		DBG("NId" + String(m_parentNodeId)
			+ " PId" + String(m_protocolProcessorId) + ": ignore unexpected OSC message from " 
			+ senderIPAddress + " (" + m_ipAddress + " expected)");
#endif
		return;
	}

	RemoteObjectMessageData newMsgData;
	newMsgData.addrVal.first = INVALID_ADDRESS_VALUE;
	newMsgData.addrVal.second = INVALID_ADDRESS_VALUE;
	newMsgData.valType = ROVT_NONE;
	newMsgData.valCount = 0;
	newMsgData.payload = 0;
	newMsgData.payloadSize = 0;

	RemoteObjectIdentifier newObjectId = ROI_Invalid;
	
	//float newFloatValue;
	float newDualFloatValue[2];
	//int newIntValue;

	if (RTTrPMmodule.isValid())
	{
		switch (RTTrPMmodule.GetModuleType())
		{
		case CPacketModule::PMT_withTimestamp:
			break;
		case CPacketModule::PMT_withoutTimestamp:
			break;
		case CPacketModule::PMT_centroidPosition:
			{
			const CCentroidMod* centroid = dynamic_cast<const CCentroidMod*>(&RTTrPMmodule);
				if (centroid)
				{
					newObjectId = ROI_Positioning_SourcePosition_XY;

					/*dbg*/newMsgData.addrVal.first = int16(1);
					/*dbg*/newMsgData.addrVal.second = int16(INVALID_ADDRESS_VALUE);

					newDualFloatValue[0] = static_cast<float>(centroid->GetXCoordinate());
					newDualFloatValue[1] = static_cast<float>(centroid->GetYCoordinate());

					newMsgData.valType = ROVT_FLOAT;
					newMsgData.valCount = 2;
					newMsgData.payload = &newDualFloatValue;
					newMsgData.payloadSize = 2 * sizeof(float);
				}
			}
			break;
		case CPacketModule::PMT_trackedPointPosition:
			break;
		case CPacketModule::PMT_orientationQuaternion:
			break;
		case CPacketModule::PMT_orientationEuler:
			break;
		case CPacketModule::PMT_centroidAccelerationAndVelocity:
			break;
		case CPacketModule::PMT_trackedPointAccelerationandVelocity:
			break;
		case CPacketModule::PMT_invalid:
			break;
		default:
			break;
		}
	}
	else
	{
		newObjectId = ROI_Invalid;
	}

	// provide the received message to parent node
	if (m_messageListener)
		m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
}
