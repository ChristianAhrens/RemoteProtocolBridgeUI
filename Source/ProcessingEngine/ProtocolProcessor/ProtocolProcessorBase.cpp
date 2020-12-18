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

#include "ProtocolProcessorBase.h"

#include "../ProcessingEngineNode.h"

// **************************************************************************************
//    class ProtocolProcessorBase
// **************************************************************************************
/**
 * @fn bool ProtocolProcessorBase::Start()
 * Pure virtual function to start the derived processor object
 */

/**
 * @fn bool ProtocolProcessorBase::Stop()
 * Pure virtual function to stop the derived processor object
 */

/**
 * @fn bool ProtocolProcessorBase::SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
 * @param Id		The object id to send a message for
 * @param msgData	The actual message value/content data
 * Pure virtual function to trigger sending a message by derived processor object
 */

/**
 * Constructor of abstract class ProtocolProcessorBase.
 */
ProtocolProcessorBase::ProtocolProcessorBase(const NodeId& parentNodeId)
{
	m_parentNodeId = parentNodeId;
	m_type = ProtocolType::PT_Invalid;
	m_IsRunning = false;
	m_messageListener = nullptr;
}

/**
 * Destructor
 */
ProtocolProcessorBase::~ProtocolProcessorBase()
{

}

/**
 * Sets the message listener object to be used for callback on message received.
 *
 * @param messageListener	The listener object
 */
void ProtocolProcessorBase::AddListener(Listener *messageListener)
{
	m_messageListener = messageListener;
}	

/**
 * Sets the configuration data for the protocol processor object.
 *
 * @param stateXml	The configuration data to parse and set active
 * @return True on success, false on failure
 */
bool ProtocolProcessorBase::setStateXml(XmlElement* stateXml)
{
	if (!stateXml)
		return false;

	auto retVal = true;

	auto isProtocolTypeA = stateXml->getTagName() == ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA);
	auto isProtocolTypeB = stateXml->getTagName() == ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB);

	if (!isProtocolTypeA && !isProtocolTypeB)
		retVal = false;

	m_protocolProcessorRole = stateXml->getTagName() == ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA) ? ProtocolRole::PR_A : ProtocolRole::PR_B;

	m_protocolProcessorId = stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));

	auto mutedObjChsXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
	if (mutedObjChsXmlElement)
		SetRemoteObjectChannelsMuted(mutedObjChsXmlElement);

	return retVal;
}

/**
 * Setter for remote object channels to not forward for further processing.
 * This uses a helper method from engine config to get a list of
 * object ids into the corresponding internal member.
 *
 * @param mutedObjChsXmlElement	The xml element that has to be parsed to get the object data
 */
void ProtocolProcessorBase::SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement)
{
	ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, m_mutedRemoteObjectChannels);
}

/**
 * Getter for the mute state of a given channel.
 * @return	True if the channel is muted (contained in internal list of muted channels), false if not.
 */
bool ProtocolProcessorBase::IsChannelMuted(int channelNumber)
{
	return m_mutedRemoteObjectChannels.contains(channelNumber);
}

/**
 * Getter for the type of this protocol processing object
 *
 * @return The type of this protocol processing object
 */
ProtocolType ProtocolProcessorBase::GetType()
{
	return m_type;
}

/**
 * Getter for the id of this protocol processing object
 *
 * @return The id of this protocol processing object
 */
ProtocolId ProtocolProcessorBase::GetId()
{
	return m_protocolProcessorId;
}

/**
 * Getter for the role of this protocol processing object
 *
 * @return The role of this protocol processing object
 */
ProtocolRole ProtocolProcessorBase::GetRole()
{
	return m_protocolProcessorRole;
}
