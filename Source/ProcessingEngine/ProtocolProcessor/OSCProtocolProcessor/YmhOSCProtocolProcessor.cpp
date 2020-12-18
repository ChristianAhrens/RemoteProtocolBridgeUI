/*
  ==============================================================================

    YmhOSCProtocolProcessor.cpp
    Created: 18 Dec 2020 07:55:00am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "YmhOSCProtocolProcessor.h"


// **************************************************************************************
//    class YmhOSCProtocolProcessor
// **************************************************************************************
/**
 * Derived OSC remote protocol processing class
 */
YmhOSCProtocolProcessor::YmhOSCProtocolProcessor(const NodeId& parentNodeId, int listenerPortNumber)
	: OSCProtocolProcessor(parentNodeId, listenerPortNumber)
{
	m_type = ProtocolType::PT_YamahaOSCProtocol;
}

/**
 * Destructor
 */
YmhOSCProtocolProcessor::~YmhOSCProtocolProcessor()
{
}


/**
 * Sets the xml configuration for the protocol processor object.
 *
 * @param stateXml	The XmlElement containing configuration for this protocol processor instance
 * @return True on success, False on failure
 */
bool YmhOSCProtocolProcessor::setStateXml(XmlElement* stateXml)
{
	if (!OSCProtocolProcessor::setStateXml(stateXml))
		return false;
	else
	{
		auto mappingAreaXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
		if (mappingAreaXmlElement)
		{
			m_mappingAreaId = static_cast<MappingAreaId>(mappingAreaXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID)));
			return true;
		}
		else
			return false;
	}
}

/**
 * Method to trigger sending of a message
 *
 * @param Id		The id of the object to send a message for
 * @param msgData	The message payload and metadata
 */
bool YmhOSCProtocolProcessor::SendRemoteObjectMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
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
 * Called when the OSCReceiver receives a new OSC message and parses its contents to
 * pass the received data to parent node for further handling
 *
 * @param message			The received OSC message.
 * @param senderIPAddress	The ip the message originates from.
 * @param senderPort			The port this message was received on.
 */
void YmhOSCProtocolProcessor::oscMessageReceived(const OSCMessage &message, const String& senderIPAddress, const int& senderPort)
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
		RemoteObjectIdentifier newObjectId = ROI_Invalid;
		SourceId channelId = INVALID_ADDRESS_VALUE;
		MappingId recordId = INVALID_ADDRESS_VALUE;

		// check if the osc message is actually one of yamaha domain type
		if (!addressString.startsWith(GetRemoteObjectDomainString()))
			return;

		// get the channel info if the object is supposed to provide it
		if (ProcessingEngineConfig::IsChannelAddressingObject(newObjectId))
		{
			// Parse the Channel ID
			channelId = static_cast<SourceId>((addressString.fromLastOccurrenceOf(GetRemoteObjectDomainString(), false, true)).getIntValue());
			jassert(channelId > 0);
			if (channelId <= 0)
				return;
		}

		// If the received channel (source) is set to muted, return without further processing
		if (IsChannelMuted(channelId))
			return;

		// Determine which parameter was changed depending on the incoming message's address pattern.
		if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourceSpread)))
			newObjectId = ROI_Positioning_SourceSpread;
		else if (addressString.startsWith(GetRemoteObjectString(ROI_Positioning_SourceDelayMode)))
			newObjectId = ROI_Positioning_SourceDelayMode;
		else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition_X)))
			newObjectId = ROI_CoordinateMapping_SourcePosition_X;
		else if (addressString.startsWith(GetRemoteObjectString(ROI_CoordinateMapping_SourcePosition_Y)))
			newObjectId = ROI_CoordinateMapping_SourcePosition_Y;
		else if (addressString.startsWith(GetRemoteObjectString(ROI_MatrixInput_ReverbSendGain)))
			newObjectId = ROI_MatrixInput_ReverbSendGain;
		else
			newObjectId = ROI_Invalid;

		// set the record info if the object needs it
		if (ProcessingEngineConfig::IsRecordAddressingObject(newObjectId))
		{
			recordId = static_cast<MappingId>(m_mappingAreaId);
		}

		newMsgData.addrVal.first = channelId;
		newMsgData.addrVal.second = recordId;
		newMsgData.valType = ROVT_FLOAT;

		switch (newObjectId)
		{
		case ROI_Error_GnrlErr:
		case ROI_MatrixInput_Select:
		case ROI_MatrixInput_Mute:
		case ROI_MatrixInput_DelayEnable:
		case ROI_MatrixInput_EqEnable:
		case ROI_MatrixInput_Polarity:
		case ROI_MatrixNode_Enable:
		case ROI_MatrixNode_DelayEnable:
		case ROI_MatrixOutput_Mute:
		case ROI_MatrixOutput_DelayEnable:
		case ROI_MatrixOutput_EqEnable:
		case ROI_MatrixOutput_Polarity:
		case ROI_Positioning_SourceDelayMode:
		case ROI_MatrixSettings_ReverbRoomId:
		case ROI_ReverbInputProcessing_Mute:
		case ROI_ReverbInputProcessing_EqEnable:
		case ROI_Scene_Recall:
		case ROI_RemoteProtocolBridge_SoundObjectSelect:
		case ROI_RemoteProtocolBridge_UIElementIndexSelect:
			createIntMessageData(message, newMsgData);
			break;
		case ROI_MatrixInput_Gain:
		case ROI_MatrixInput_Delay:
		case ROI_MatrixInput_LevelMeterPreMute:
		case ROI_MatrixInput_LevelMeterPostMute:
		case ROI_MatrixNode_Gain:
		case ROI_MatrixNode_Delay:
		case ROI_MatrixOutput_Gain:
		case ROI_MatrixOutput_Delay:
		case ROI_MatrixOutput_LevelMeterPreMute:
		case ROI_MatrixOutput_LevelMeterPostMute:
		case ROI_Positioning_SourceSpread:
		case ROI_Positioning_SourcePosition_XY:
		case ROI_Positioning_SourcePosition_X:
		case ROI_Positioning_SourcePosition_Y:
		case ROI_Positioning_SourcePosition:
		case ROI_MatrixSettings_ReverbPredelayFactor:
		case ROI_MatrixSettings_RevebRearLevel:
		case ROI_MatrixInput_ReverbSendGain:
		case ROI_ReverbInput_Gain:
		case ROI_ReverbInputProcessing_Gain:
		case ROI_ReverbInputProcessing_LevelMeter:
		case ROI_CoordinateMapping_SourcePosition_XY:
		case ROI_CoordinateMapping_SourcePosition_X:
		case ROI_CoordinateMapping_SourcePosition_Y:
		case ROI_CoordinateMapping_SourcePosition:
			createFloatMessageData(message, newMsgData);
			break;
		case ROI_Scene_SceneIndex:
		case ROI_Settings_DeviceName:
		case ROI_Error_ErrorText:
		case ROI_Status_StatusText:
		case ROI_MatrixInput_ChannelName:
		case ROI_MatrixOutput_ChannelName:
		case ROI_Scene_SceneName:
		case ROI_Scene_SceneComment:
			createStringMessageData(message, newMsgData);
			break;
		case ROI_Device_Clear:
		case ROI_Scene_Previous:
		case ROI_Scene_Next:
			break;
		default:
			jassertfalse;
			break;
		}

		// provide the received message to parent node
		if (m_messageListener)
			m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
	}
}

/**
 * static method to get Yamaha OSC-domain specific preceding string
 * @return		The Yamaha specific OSC address preceding string
 */
String YmhOSCProtocolProcessor::GetRemoteObjectDomainString()
{
	return "/ymh/src/";
}

/**
 * static method to get Yamaha specific OSC address string parameter definition trailer
 *
 * @param id	The object id to get the OSC specific parameter def string for
 * @return		The parameter defining string, empty if not available
 */
String YmhOSCProtocolProcessor::GetRemoteObjectParameterTypeString(RemoteObjectIdentifier id)
{
	switch (id)
	{
	case ROI_HeartbeatPong:
	case ROI_HeartbeatPing:
	case ROI_Settings_DeviceName:
	case ROI_Error_GnrlErr:
	case ROI_Error_ErrorText:
	case ROI_Status_StatusText:
	case ROI_MatrixInput_Select:
	case ROI_MatrixInput_Mute:
	case ROI_MatrixInput_Gain:
	case ROI_MatrixInput_Delay:
	case ROI_MatrixInput_DelayEnable:
	case ROI_MatrixInput_EqEnable:
	case ROI_MatrixInput_Polarity:
	case ROI_MatrixInput_ChannelName:
	case ROI_MatrixInput_LevelMeterPreMute:
	case ROI_MatrixInput_LevelMeterPostMute:
	case ROI_MatrixNode_Enable:
	case ROI_MatrixNode_Gain:
	case ROI_MatrixNode_DelayEnable:
	case ROI_MatrixNode_Delay:
	case ROI_MatrixOutput_Mute:
	case ROI_MatrixOutput_Gain:
	case ROI_MatrixOutput_Delay:
	case ROI_MatrixOutput_DelayEnable:
	case ROI_MatrixOutput_EqEnable:
	case ROI_MatrixOutput_Polarity:
	case ROI_MatrixOutput_ChannelName:
	case ROI_MatrixOutput_LevelMeterPreMute:
	case ROI_MatrixOutput_LevelMeterPostMute:
		return "";
	case ROI_Positioning_SourceSpread:
		return "/w";								// width parameter (0-1, can be mapped directly to SO spread)
	case ROI_Positioning_SourceDelayMode:
	case ROI_Positioning_SourcePosition:
	case ROI_Positioning_SourcePosition_XY:
	case ROI_Positioning_SourcePosition_X:
	case ROI_Positioning_SourcePosition_Y:
	case ROI_CoordinateMapping_SourcePosition:
		return "";
	case ROI_CoordinateMapping_SourcePosition_X:
		return "/p";								// pan parameter (0-1, cartesian, can be mapped directly to X SO position)
	case ROI_CoordinateMapping_SourcePosition_Y:
		return "/d";								//distance parameter (0-1, cartesian, can be mapped directly to Y SO position)
	case ROI_CoordinateMapping_SourcePosition_XY:
	case ROI_MatrixSettings_ReverbRoomId:
	case ROI_MatrixSettings_ReverbPredelayFactor:
	case ROI_MatrixSettings_RevebRearLevel:
	case ROI_MatrixInput_ReverbSendGain:
		return "/s";								// aux send (0-1, linear, need log and scale to -120 +24 to be in dB mapped to EnSpace send)
	case ROI_ReverbInput_Gain:
	case ROI_ReverbInputProcessing_Mute:
	case ROI_ReverbInputProcessing_Gain:
	case ROI_ReverbInputProcessing_LevelMeter:
	case ROI_ReverbInputProcessing_EqEnable:
	case ROI_Device_Clear:
	case ROI_Scene_Previous:
	case ROI_Scene_Next:
	case ROI_Scene_Recall:
	case ROI_Scene_SceneIndex:
	case ROI_Scene_SceneName:
	case ROI_Scene_SceneComment:
	case ROI_RemoteProtocolBridge_SoundObjectSelect:
	case ROI_RemoteProtocolBridge_UIElementIndexSelect:
	default:
		return "";
	}
}