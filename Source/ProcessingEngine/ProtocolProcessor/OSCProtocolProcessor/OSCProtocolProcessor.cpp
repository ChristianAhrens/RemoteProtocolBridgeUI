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

#include "OSCProtocolProcessor.h"

#include "../../ProcessingEngineConfig.h"


// **************************************************************************************
//    class OSCProtocolProcessor
// **************************************************************************************
/**
 * Derived OSC remote protocol processing class
 */
OSCProtocolProcessor::OSCProtocolProcessor(const NodeId& parentNodeId, int listenerPortNumber)
	: ProtocolProcessor_Abstract(parentNodeId), m_oscReceiver(listenerPortNumber)
{
	m_type = ProtocolType::PT_OSCProtocol;
	m_oscMsgRate = ET_DefaultPollingRate;

	// OSCProtocolProcessor derives from OSCReceiver::Listener
	m_oscReceiver.addListener(this);
}

/**
 * Destructor
 */
OSCProtocolProcessor::~OSCProtocolProcessor()
{
	Stop();

	m_oscReceiver.removeListener(this);
}

/**
 * Overloaded method to start the protocol processing object.
 * Usually called after configuration has been set.
 */
bool OSCProtocolProcessor::Start()
{
	bool successS = false;
	bool successR = false;

	// Connect both sender and receiver  
	successS = m_oscSender.connect(m_ipAddress, m_clientPort);
	jassert(successS);

	successR = m_oscReceiver.connect();
	jassert(successR);

	m_IsRunning = (successS && successR);

	return m_IsRunning;
}

/**
 * Overloaded method to stop to protocol processing object.
 */
bool OSCProtocolProcessor::Stop()
{
	m_IsRunning = false;

	bool successS = false;
	bool successR = false;

	// Connect both sender and receiver  
	successS = m_oscSender.disconnect();
	jassert(successS);

	successR = m_oscReceiver.disconnect();
	jassert(successR);

	return (successS && successR);
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
bool OSCProtocolProcessor::setStateXml(XmlElement *stateXml)
{
	if (!ProtocolProcessor_Abstract::setStateXml(stateXml))
		return false;
	else
	{
		auto pollingIntervalXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
		if (pollingIntervalXmlElement)
		{
			m_oscMsgRate = pollingIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL));
			return true;
		}
		else
			return false;
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
void OSCProtocolProcessor::SetRemoteObjectsActive(XmlElement* activeObjsXmlElement)
{
	ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, m_activeRemoteObjects);

	// Start timer callback if objects are to be polled
	if (m_activeRemoteObjects.size() > 0)
	{
		startTimer(m_oscMsgRate);
	}
	else
	{
		stopTimer();
	}
}

/**
 * Setter for remote object channels to not forward for further processing.
 * This uses a helper method from engine config to get a list of 
 * object ids into the corresponding internal member.
 * 
 * @param mutedObjChsXmlElement	The xml element that has to be parsed to get the object data
 */
void OSCProtocolProcessor::SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement)
{
	ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, m_mutedRemoteObjectChannels);
}

/**
 * Method to trigger sending of a message
 *
 * @param Id		The id of the object to send a message for
 * @param msgData	The message payload and metadata
 */
bool OSCProtocolProcessor::SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (!m_IsRunning)
		return false;

	bool sendSuccess = false;

	String addressString = GetRemoteObjectString(Id);

	if (msgData.addrVal.second != INVALID_ADDRESS_VALUE)
		addressString += String::formatted("/%d", msgData.addrVal.second);

	if (msgData.addrVal.first != INVALID_ADDRESS_VALUE)
		addressString += String::formatted("/%d", msgData.addrVal.first);

	uint16 valSize;
	switch (msgData.valType)
	{
	case ROVT_INT:
		valSize = sizeof(int);
		break;
	case ROVT_FLOAT:
		valSize = sizeof(float);
		break;
	case ROVT_STRING:
		jassertfalse; // String not (yet?) supported
		valSize = 0;
		break;
	case ROVT_NONE:
	default:
		valSize = 0;
		break;
	}

	jassert((msgData.valCount*valSize) == msgData.payloadSize);

	switch (msgData.valType)
	{
	case ROVT_INT:
		{
		jassert(msgData.valCount < 4); // max known d&b OSC msg val cnt would be positioning xyz
		int multivalues[3];

		for (int i = 0; i < msgData.valCount; ++i)
			multivalues[i] = ((int*)msgData.payload)[i];

		if (msgData.valCount == 1)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0]));
		else if (msgData.valCount == 2)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0], multivalues[1]));
		else if (msgData.valCount == 3)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0], multivalues[1], multivalues[2]));
		else
			sendSuccess = m_oscSender.send(OSCMessage(addressString));
		}
		break;
	case ROVT_FLOAT:
		{
		jassert(msgData.valCount < 4); // max known d&b OSC msg val cnt would be positioning xyz
		float multivalues[3];

		for (int i = 0; i < msgData.valCount; ++i)
			multivalues[i] = ((float*)msgData.payload)[i];

		if (msgData.valCount == 1)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0]));
		else if (msgData.valCount == 2)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0], multivalues[1]));
		else if (msgData.valCount == 3)
			sendSuccess = m_oscSender.send(OSCMessage(addressString, multivalues[0], multivalues[1], multivalues[2]));
		else
			sendSuccess = m_oscSender.send(OSCMessage(addressString));
		}
		break;
	case ROVT_NONE:
		sendSuccess = m_oscSender.send(OSCMessage(addressString));
		break;
	case ROVT_STRING:
	default:
		break;
	}

	return sendSuccess;
}

/**
* Called when the OSCReceiver receives a new OSC bundle.
* The bundle is processed and all contained individual messages passed on
* to oscMessageReceived for further handling.
*
* @param bundle				The received OSC bundle.
* @param senderIPAddress	The ip the bundle originates from.
* @param senderPort			The port this bundle was received on.
*/
void OSCProtocolProcessor::oscBundleReceived(const OSCBundle &bundle, const String& senderIPAddress, const int& senderPort)
{
	if (senderIPAddress != m_ipAddress)
	{
#ifdef DEBUG
		DBG("NId"+String(m_parentNodeId) 
			+ " PId"+String(m_protocolProcessorId) + ": ignore unexpected OSC bundle from " 
			+ senderIPAddress + " (" + m_ipAddress + " expected)");
#endif
		return;
	}

	for (int i = 0; i < bundle.size(); ++i)
	{
		if (bundle[i].isBundle())
			oscBundleReceived(bundle[i].getBundle(), senderIPAddress, senderPort);
		else if (bundle[i].isMessage())
			oscMessageReceived(bundle[i].getMessage(), senderIPAddress, senderPort);
	}
}

/**
 * Called when the OSCReceiver receives a new OSC message and parses its contents to
 * pass the received data to parent node for further handling
 *
 * @param message			The received OSC message.
* @param senderIPAddress	The ip the message originates from.
* @param senderPort			The port this message was received on.
 */
void OSCProtocolProcessor::oscMessageReceived(const OSCMessage &message, const String& senderIPAddress, const int& senderPort)
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

	int messageSize = message.size();
	bool isContentMessage = (messageSize > 0); // the value count is reported by JUCE as OSCMessage::size

	RemoteObjectMessageData newMsgData;
	newMsgData.addrVal.first = INVALID_ADDRESS_VALUE;
	newMsgData.addrVal.second = INVALID_ADDRESS_VALUE;
	newMsgData.valType = ROVT_NONE;
	newMsgData.valCount = 0;
	newMsgData.payload = 0;
	newMsgData.payloadSize = 0;

	String addressString = message.getAddressPattern().toString();
	// Check if the incoming message is a response to a sent "ping" heartbeat.
	if (addressString.startsWith(GetRemoteObjectString(ROI_HeartbeatPong)) && m_messageListener)
		m_messageListener->OnProtocolMessageReceived(this, ROI_HeartbeatPong, newMsgData);
	// Check if the incoming message is a response to a sent "pong" heartbeat.
	else if (addressString.startsWith(GetRemoteObjectString(ROI_HeartbeatPing)) && m_messageListener)
		m_messageListener->OnProtocolMessageReceived(this, ROI_HeartbeatPing, newMsgData);
	// Handle the incoming message contents.
	else
	{
		// Parse the Source ID
		int sourceId = (addressString.fromLastOccurrenceOf("/", false, true)).getIntValue();
		jassert(sourceId > 0);
		if (sourceId > 0)
		{
			RemoteObjectIdentifier newObjectId;

			newMsgData.addrVal.first = int16(sourceId);
			newMsgData.valType = ROVT_FLOAT;
		
			float newFloatValue;
			float newDualFloatValue[2];
			float newTripleFloatValue[3];
			int newIntValue;
			int newDualIntValue[2];

			// Determine which parameter was changed depending on the incoming message's address pattern.
			//ROI_Settings_DeviceName;
			if (addressString.startsWith(GetRemoteObjectString(ROI_Error_GnrlErr)))
			{
				newObjectId = ROI_Error_GnrlErr;

				if (isContentMessage)
				{
					// gnrlerr should be an int
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else
						newIntValue = 0;

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			//ROI_Error_ErrorText;
			//ROI_Status_StatusText;
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_Mute)))
			{
				newObjectId = ROI_MatrixInput_Mute;

				if (isContentMessage)
				{
					// matrixinput mute should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_Gain)))
			{
				newObjectId = ROI_MatrixInput_Gain;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_Delay)))
			{
				newObjectId = ROI_MatrixInput_Delay;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_DelayEnable)))
			{
				newObjectId = ROI_MatrixInput_DelayEnable;

				if (isContentMessage)
				{
					// matrixinput delayenable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_EqEnable)))
			{
				newObjectId = ROI_MatrixInput_EqEnable;

				if (isContentMessage)
				{
					// matrixinput eqenable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_Polarity)))
			{
				newObjectId = ROI_MatrixInput_Polarity;

				if (isContentMessage)
				{
					// matrixinput polarity should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			//ROI_MatrixInput_ChannelName;
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_LevelMeterPreMute)))
			{
			newObjectId = ROI_MatrixInput_LevelMeterPreMute;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_LevelMeterPostMute)))
			{
			newObjectId = ROI_MatrixInput_LevelMeterPostMute;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixNode_Enable)))
			{
				newObjectId = ROI_MatrixNode_Enable;

				if (isContentMessage)
				{
					// matrixnode enable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixNode_Gain)))
			{
			newObjectId = ROI_MatrixNode_Gain;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixNode_DelayEnable)))
			{
				newObjectId = ROI_MatrixNode_DelayEnable;

				if (isContentMessage)
				{
					// matrixnode delayenable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_Polarity)))
			{
				newObjectId = ROI_MatrixInput_Polarity;

				if (isContentMessage)
				{
					// matrixinput polarity should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixNode_Delay)))
			{
			newObjectId = ROI_MatrixNode_Delay;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_Mute)))
			{
				newObjectId = ROI_MatrixOutput_Mute;

				if (isContentMessage)
				{
					// matrixnode enable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_Gain)))
			{
			newObjectId = ROI_MatrixOutput_Gain;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_Delay)))
			{
			newObjectId = ROI_MatrixOutput_Delay;

			if (isContentMessage)
			{
				newFloatValue = message[0].getFloat32();

				newMsgData.valCount = 1;
				newMsgData.payload = &newFloatValue;
				newMsgData.payloadSize = sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_DelayEnable)))
			{
				newObjectId = ROI_MatrixOutput_DelayEnable;

				if (isContentMessage)
				{
					// matrixnode enable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_EqEnable)))
			{
				newObjectId = ROI_MatrixOutput_EqEnable;

				if (isContentMessage)
				{
					// matrixnode enable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_Polarity)))
			{
				newObjectId = ROI_MatrixOutput_Polarity;

				if (isContentMessage)
				{
					// matrixnode enable should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			//ROI_MatrixOutput_ChannelName;
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_LevelMeterPreMute)))
			{
				newObjectId = ROI_MatrixOutput_LevelMeterPreMute;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixOutput_LevelMeterPostMute)))
			{
				newObjectId = ROI_MatrixOutput_LevelMeterPostMute;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourceSpread)))
			{
				newObjectId = ROI_Positioning_SourceSpread;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourceDelayMode)))
			{
				newObjectId = ROI_Positioning_SourceDelayMode;

				if (isContentMessage)
				{
					// delaymode should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourcePosition)))
			{
			newObjectId = ROI_Positioning_SourcePosition;

			if (isContentMessage)
			{
				newTripleFloatValue[0] = message[0].getFloat32();
				newTripleFloatValue[1] = message[1].getFloat32();
				newTripleFloatValue[2] = message[2].getFloat32();

				newMsgData.valCount = 3;
				newMsgData.payload = &newTripleFloatValue;
				newMsgData.payloadSize = 3 * sizeof(float);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourcePosition_XY)))
			{
				newObjectId = ROI_Positioning_SourcePosition_XY;

				if (isContentMessage)
				{
					newDualFloatValue[0] = message[0].getFloat32();
					newDualFloatValue[1] = message[1].getFloat32();

					newMsgData.valCount = 2;
					newMsgData.payload = &newDualFloatValue;
					newMsgData.payloadSize = 2 * sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourcePosition_X)))
			{
				newObjectId = ROI_Positioning_SourcePosition_X;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourcePosition_Y)))
			{
				newObjectId = ROI_Positioning_SourcePosition_Y;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition)))
			{
				// Parse the Mapping ID
				addressString = addressString.upToLastOccurrenceOf("/", false, true);
				newMsgData.addrVal.second = int16((addressString.fromLastOccurrenceOf("/", false, true)).getIntValue());
				jassert(newMsgData.addrVal.second > 0);

				newObjectId = ROI_CoordinateMapping_SourcePosition;

				if (isContentMessage)
				{
					newTripleFloatValue[0] = message[0].getFloat32();
					newTripleFloatValue[1] = message[1].getFloat32();
					newTripleFloatValue[2] = message[2].getFloat32();

					newMsgData.valCount = 2;
					newMsgData.payload = &newTripleFloatValue;
					newMsgData.payloadSize = 2 * sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition_XY)))
			{
				// Parse the Mapping ID
				addressString = addressString.upToLastOccurrenceOf("/", false, true);
				newMsgData.addrVal.second = int16((addressString.fromLastOccurrenceOf("/", false, true)).getIntValue());
				jassert(newMsgData.addrVal.second > 0);

				newObjectId = ROI_CoordinateMapping_SourcePosition_XY;

				if (isContentMessage)
				{
					newDualFloatValue[0] = message[0].getFloat32();
					newDualFloatValue[1] = message[1].getFloat32();

					newMsgData.valCount = 2;
					newMsgData.payload = &newDualFloatValue;
					newMsgData.payloadSize = 2 * sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition_X)))
			{
				// Parse the Mapping ID
				addressString = addressString.upToLastOccurrenceOf("/", false, true);
				newMsgData.addrVal.second = int16((addressString.fromLastOccurrenceOf("/", false, true)).getIntValue());
				jassert(newMsgData.addrVal.second > 0);

				newObjectId = ROI_CoordinateMapping_SourcePosition_X;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition_Y)))
			{
				// Parse the Mapping ID
				addressString = addressString.upToLastOccurrenceOf("/", false, true);
				newMsgData.addrVal.second = int16((addressString.fromLastOccurrenceOf("/", false, true)).getIntValue());
				jassert(newMsgData.addrVal.second > 0);

				newObjectId = ROI_CoordinateMapping_SourcePosition_Y;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixSettings_ReverbRoomId)))
			{
			newObjectId = ROI_MatrixSettings_ReverbRoomId;

			if (isContentMessage)
			{
				// delaymode should be an int, but since some OSC appliances can only process floats,
				// we need to be prepared to optionally accept float as well
				if (message[0].isInt32())
					newIntValue = message[0].getInt32();
				else if (message[0].isFloat32())
					newIntValue = (int)round(message[0].getFloat32());

				newMsgData.valType = ROVT_INT;
				newMsgData.valCount = 1;
				newMsgData.payload = &newIntValue;
				newMsgData.payloadSize = sizeof(int);
			}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixSettings_ReverbPredelayFactor)))
			{
				newObjectId = ROI_MatrixSettings_ReverbPredelayFactor;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixSettings_RevebRearLevel)))
			{
				newObjectId = ROI_MatrixSettings_RevebRearLevel;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_ReverbSendGain)))
			{
				newObjectId = ROI_MatrixInput_ReverbSendGain;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_ReverbInput_Gain)))
			{
				newObjectId = ROI_ReverbInput_Gain;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_ReverbInputProcessing_Mute)))
			{
				newObjectId = ROI_ReverbInputProcessing_Mute;

				if (isContentMessage)
				{
					// delaymode should be an int, but since some OSC appliances can only process floats,
					// we need to be prepared to optionally accept float as well
					if (message[0].isInt32())
						newIntValue = message[0].getInt32();
					else if (message[0].isFloat32())
						newIntValue = (int)round(message[0].getFloat32());

					newMsgData.valType = ROVT_INT;
					newMsgData.valCount = 1;
					newMsgData.payload = &newIntValue;
					newMsgData.payloadSize = sizeof(int);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_ReverbInputProcessing_Gain)))
			{
				newObjectId = ROI_ReverbInputProcessing_Gain;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_ReverbInputProcessing_LevelMeter)))
			{
				newObjectId = ROI_ReverbInputProcessing_LevelMeter;

				if (isContentMessage)
				{
					newFloatValue = message[0].getFloat32();

					newMsgData.valCount = 1;
					newMsgData.payload = &newFloatValue;
					newMsgData.payloadSize = sizeof(float);
				}
			}
			else if (addressString.startsWith(GetRemoteObjectString(ROI_ReverbInputProcessing_EqEnable)))
			{
			newObjectId = ROI_ReverbInputProcessing_EqEnable;

			if (isContentMessage)
			{
				// delaymode should be an int, but since some OSC appliances can only process floats,
				// we need to be prepared to optionally accept float as well
				if (message[0].isInt32())
					newIntValue = message[0].getInt32();
				else if (message[0].isFloat32())
					newIntValue = (int)round(message[0].getFloat32());

				newMsgData.valType = ROVT_INT;
				newMsgData.valCount = 1;
				newMsgData.payload = &newIntValue;
				newMsgData.payloadSize = sizeof(int);
			}
			}
			//ROI_Device_Clear;
			//ROI_Scene_Previous;
			//ROI_Scene_Next;
			//ROI_Scene_Recall;
			else if (addressString.startsWith(GetRemoteObjectString(ROI_Scene_Recall)))
			{
				newObjectId = ROI_Scene_Recall;

				if (isContentMessage)
				{
					if (message.size() == 1)
					{
						// delaymode should be an int, but since some OSC appliances can only process floats,
						// we need to be prepared to optionally accept float as well
						if (message[0].isInt32())
							newIntValue = message[0].getInt32();
						else if (message[0].isFloat32())
							newIntValue = (int)round(message[0].getFloat32());

						newMsgData.valType = ROVT_INT;
						newMsgData.valCount = 1;
						newMsgData.payload = &newIntValue;
						newMsgData.payloadSize = sizeof(int);
					}
					else if (message.size() == 2)
					{
						// delaymode should be an int, but since some OSC appliances can only process floats,
						// we need to be prepared to optionally accept float as well
						if (message[0].isInt32())
							newDualIntValue[0] = message[0].getInt32();
						else if (message[0].isFloat32())
							newDualIntValue[0] = (int)round(message[0].getFloat32());
						if (message[1].isInt32())
							newDualIntValue[1] = message[1].getInt32();
						else if (message[1].isFloat32())
							newDualIntValue[1] = (int)round(message[1].getFloat32());

						newMsgData.valType = ROVT_INT;
						newMsgData.valCount = 2;
						newMsgData.payload = &newDualIntValue;
						newMsgData.payloadSize = 2 * sizeof(int);
					}
				}
			}
			//ROI_Scene_SceneIndex;
			//ROI_Scene_SceneName;
			//ROI_Scene_SceneComment;
			else
			{
				newObjectId = ROI_Invalid;
			}

			// provide the received message to parent node
			if (m_messageListener)
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
		}
	}
}

/**
 * Private method to get OSC object specific ObjectName string
 *
 * @param id	The object id to get the OSC specific object name
 * @return		The OSC specific object name
 */
String OSCProtocolProcessor::GetRemoteObjectString(RemoteObjectIdentifier id)
{
	switch (id)
	{
	case ROI_HeartbeatPong:
		return "/pong";
	case ROI_HeartbeatPing:
		return "/ping";
	case ROI_Settings_DeviceName:
		return "/dbaudio1/settings/devicename";
	case ROI_Error_GnrlErr:
		return "/dbaudio1/error/gnrlerr";
	case ROI_Error_ErrorText:
		return "/dbaudio1/error/errortext";
	case ROI_Status_StatusText:
		return "/dbaudio1/status/statustext";
	case ROI_MatrixInput_Mute:
		return "/dbaudio1/matrixinput/mute";
	case ROI_MatrixInput_Gain:
		return "/dbaudio1/matrixinput/gain";
	case ROI_MatrixInput_Delay:
		return "/dbaudio1/matrixinput/delay";
	case ROI_MatrixInput_DelayEnable:
		return "/dbaudio1/matrixinput/delayenable";
	case ROI_MatrixInput_EqEnable:
		return "/dbaudio1/matrixinput/eqenable";
	case ROI_MatrixInput_Polarity:
		return "/dbaudio1/matrixinput/polarity";
	case ROI_MatrixInput_ChannelName:
		return "/dbaudio1/matrixinput/channelname";
	case ROI_MatrixInput_LevelMeterPreMute:
		return "/dbaudio1/matrixinput/levelmeterpremute";
	case ROI_MatrixInput_LevelMeterPostMute:
		return "/dbaudio1/matrixinput/levelmeterpostmute";
	case ROI_MatrixNode_Enable:
		return "/dbaudio1/matrixnode/enable";
	case ROI_MatrixNode_Gain:
		return "/dbaudio1/matrixnode/gain";
	case ROI_MatrixNode_DelayEnable:
		return "/dbaudio1/matrixnode/delayenable";
	case ROI_MatrixNode_Delay:
		return "/dbaudio1/matrixnode/delay";
	case ROI_MatrixOutput_Mute:
		return "/dbaudio1/matrixoutput/mute";
	case ROI_MatrixOutput_Gain:
		return "/dbaudio1/matrixoutput/gain";
	case ROI_MatrixOutput_Delay:
		return "/dbaudio1/matrixoutput/delay";
	case ROI_MatrixOutput_DelayEnable:
		return "/dbaudio1/matrixoutput/delayenable";
	case ROI_MatrixOutput_EqEnable:
		return "/dbaudio1/matrixoutput/eqenable";
	case ROI_MatrixOutput_Polarity:
		return "/dbaudio1/matrixoutput/polarity";
	case ROI_MatrixOutput_ChannelName:
		return "/dbaudio1/matrixoutput/channelname";
	case ROI_MatrixOutput_LevelMeterPreMute:
		return "/dbaudio1/matrixoutput/levelmeterpremute";
	case ROI_MatrixOutput_LevelMeterPostMute:
		return "/dbaudio1/matrixoutput/levelmeterpostmute";
	case ROI_Positioning_SourceSpread:
		return "/dbaudio1/positioning/source_spread";
	case ROI_Positioning_SourceDelayMode:
		return "/dbaudio1/positioning/source_delaymode";
	case ROI_Positioning_SourcePosition:
		return "/dbaudio1/positioning/source_position";
	case ROI_Positioning_SourcePosition_XY:
		return "/dbaudio1/positioning/source_position_xy";
	case ROI_Positioning_SourcePosition_X:
		return "/dbaudio1/positioning/source_position_x";
	case ROI_Positioning_SourcePosition_Y:
		return "/dbaudio1/positioning/source_position_y";
	case ROI_CoordinateMapping_SourcePosition:
		return "/dbaudio1/coordinatemapping/source_position";
	case ROI_CoordinateMapping_SourcePosition_X:
		return "/dbaudio1/coordinatemapping/source_position_x";
	case ROI_CoordinateMapping_SourcePosition_Y:
		return "/dbaudio1/coordinatemapping/source_position_y";
	case ROI_CoordinateMapping_SourcePosition_XY:
		return "/dbaudio1/coordinatemapping/source_position_xy";
	case ROI_MatrixSettings_ReverbRoomId:
		return "/dbaudio1/matrixsettings/reverbroomid";
	case ROI_MatrixSettings_ReverbPredelayFactor:
		return "/dbaudio1/matrixsettings/reverbpredelayfactor";
	case ROI_MatrixSettings_RevebRearLevel:
		return "/dbaudio1/matrixsettings/reverbrearlevel";
	case ROI_MatrixInput_ReverbSendGain:
		return "/dbaudio1/matrixinput/reverbsendgain";
	case ROI_ReverbInput_Gain:
		return "/dbaudio1/reverbinput/gain";
	case ROI_ReverbInputProcessing_Mute:
		return "/dbaudio1/reverbinputprocessing/mute";
	case ROI_ReverbInputProcessing_Gain:
		return "/dbaudio1/reverbinputprocessing/gain";
	case ROI_ReverbInputProcessing_LevelMeter:
		return "/dbaudio1/reverbinputprocessing/levelmeter";
	case ROI_ReverbInputProcessing_EqEnable:
		return "/dbaudio1/reverbinputprocessing/eqenable";
	case ROI_Device_Clear:
		return "/dbaudio1/device/clear";
	case ROI_Scene_Previous:
		return "/dbaudio1/scene/previous";
	case ROI_Scene_Next:
		return "/dbaudio1/scene/next";
	case ROI_Scene_Recall:
		return "/dbaudio1/scene/recall";
	case ROI_Scene_SceneIndex:
		return "/dbaudio1/scene/sceneindex";
	case ROI_Scene_SceneName:
		return "/dbaudio1/scene/scenename";
	case ROI_Scene_SceneComment:
		return "/dbaudio1/scene/scenecomment";
	default:
		return "";
	}
}

/**
 * Timer callback function, which will be called at regular intervals to
 * send out OSC poll messages.
 */
void OSCProtocolProcessor::timerCallback()
{
	int objectCount = m_activeRemoteObjects.size();
	for (int i = 0; i < objectCount; i++)
	{
		RemoteObjectMessageData msgData;
		msgData.addrVal = m_activeRemoteObjects[i].Addr;
		msgData.valCount = 0;
		msgData.valType = ROVT_NONE;
		msgData.payload = 0;
		msgData.payloadSize = 0;
		
		SendMessage(m_activeRemoteObjects[i].Id, msgData);
	}
}