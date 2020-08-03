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

#include "ProcessingEngineNode.h"

#include "ObjectDataHandling.h"
#include "ProcessingEngine.h"
#include "ProcessingEngineConfig.h"

#include "ProtocolProcessor/OCAProtocolProcessor/OCAProtocolProcessor.h"
#include "ProtocolProcessor/OSCProtocolProcessor/OSCProtocolProcessor.h"
#include "ProtocolProcessor/MIDIProtocolProcessor/MIDIProtocolProcessor.h"

// **************************************************************************************
//    class ProcessingEngineNode
// **************************************************************************************
/**
 * Constructor
 */
ProcessingEngineNode::ProcessingEngineNode()
{
	m_dataHandling	= 0;
}

/**
 * Constructor including initialization of internal parent object
 *
 * @param parentEngine	The engine object to be used as internal parent
 */
ProcessingEngineNode::ProcessingEngineNode(ProcessingEngineNode::NodeListener* listener)
	: ProcessingEngineNode()
{
	AddListener(listener);
}

/**
 * Destructor
 */
ProcessingEngineNode::~ProcessingEngineNode()
{
	Stop();
}

/**
 * Method to register a listener object to be called when the node has received the respective data via a node protocol.
 * @param listener	The listener object to add to the internal list of listeners
 */
void ProcessingEngineNode::AddListener(ProcessingEngineNode::NodeListener* listener)
{
	if (listener)
		m_listeners.push_back(listener);
}

/**
 * Getter for the id of this processing node object
 *
 * @return The id of this processing node object
 */
NodeId ProcessingEngineNode::GetId()
{
	return m_nodeId;
}

/**
 * Starts the node object and therefor the internal protocol processing objects as well.
 *
 * @return	True if both internal processing objects have been started successfully. False if not.
 */
bool ProcessingEngineNode::Start()
{
	m_shouldBeRunning = true;

	bool successfullyStartedA = m_typeAProtocols.size() > 0;
	bool successfullyStartedB = true;

	for (std::map<ProtocolId, std::unique_ptr<ProtocolProcessor_Abstract>>::iterator paiter = m_typeAProtocols.begin(); successfullyStartedA && paiter != m_typeAProtocols.end(); ++paiter)
		successfullyStartedA = successfullyStartedA && paiter->second->Start();

	for (std::map<ProtocolId, std::unique_ptr<ProtocolProcessor_Abstract>>::iterator pbiter = m_typeBProtocols.begin(); successfullyStartedB && pbiter != m_typeBProtocols.end(); ++pbiter)
		successfullyStartedB = successfullyStartedB && pbiter->second->Start();

	// if one of the protocol processors did not start successfully,
	// enshure the other is not running without purpose
	if (!successfullyStartedA || !successfullyStartedB)
	{
		Stop();
	}

	return (successfullyStartedA && successfullyStartedB);
}

/**
 * Stops the node object and therefor the internal protocol processing objects as well.
 *
 * @return	True if both internal processing objects have been stopped successfully. False if not.
 */
bool ProcessingEngineNode::Stop()
{
	m_shouldBeRunning = false;

	bool successfullyStoppedA = true;
	bool successfullyStoppedB = true;

	for (std::map<ProtocolId, std::unique_ptr<ProtocolProcessor_Abstract>>::iterator paiter = m_typeAProtocols.begin(); paiter != m_typeAProtocols.end(); ++paiter)
		successfullyStoppedA = successfullyStoppedA && paiter->second->Stop();

	for (std::map<ProtocolId, std::unique_ptr<ProtocolProcessor_Abstract>>::iterator pbiter = m_typeAProtocols.begin(); pbiter != m_typeAProtocols.end(); ++pbiter)
		successfullyStoppedB = successfullyStoppedB && pbiter->second->Stop();

	return (successfullyStoppedA && successfullyStoppedB);
}

/**
 *
 */
std::unique_ptr<XmlElement> ProcessingEngineNode::createStateXml()
{
	auto nodeXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	nodeXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), static_cast<int>(m_nodeId));

	nodeXmlElement->addChildElement(m_dataHandling->createStateXml().release());



	return std::move(nodeXmlElement);
}

/**
 * Setter for the configuration of this node object.
 * Includes initializing the internal node id that is used to access relevant config info of the given config object.
 *
 * @param config	The application configuration object to use to access config data
 * @param NId	The node id to use for this node object and to access data from config object
 */
bool ProcessingEngineNode::setStateXml(XmlElement* stateXml)
{
	Stop();

	if (!stateXml || stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE))
		return false;

	auto retVal = true;

	m_nodeId = stateXml->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));

	auto objectHandlingStateXml = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (objectHandlingStateXml)
	{
		m_dataHandling = std::unique_ptr<ObjectDataHandling_Abstract>(CreateObjectDataHandling(ProcessingEngineConfig::ObjectHandlingModeFromString(objectHandlingStateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)))));
		if (m_dataHandling)
			m_dataHandling->setStateXml(objectHandlingStateXml);
		else
			retVal = false;
	}

	XmlElement* protocolXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
	while (protocolXmlElement != nullptr)
	{
		// create the protocol processing objects of correct type as defined in config
		auto protocolId = protocolXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
		auto protocolType = ProcessingEngineConfig::ProtocolTypeFromString(protocolXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE)));
		auto hostPort = 0;
		auto hostPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPort = hostPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
		else
			retVal = false;

		ProtocolProcessor_Abstract* protocol = CreateProtocolProcessor(protocolType, hostPort);

		// set up the protocol processing objects of correct type as defined in config
		if (protocol)
		{
			protocol->AddListener(this);
			protocol->setStateXml(protocolXmlElement);
			if (protocol->GetRole() == ProtocolRole::PR_A)
				m_typeAProtocols[protocolId] = std::unique_ptr<ProtocolProcessor_Abstract>(protocol);
			else if (protocol->GetRole() == ProtocolRole::PR_B)
				m_typeBProtocols[protocolId] = std::unique_ptr<ProtocolProcessor_Abstract>(protocol);

			// add the protocolnodetype A to datahandlings' list of ProtocolIds for A protocols
			if (m_dataHandling)
				m_dataHandling->AddProtocolAId(protocolId);
			else
				retVal = false;
		}
		else
			retVal = false;

		auto nextProtocolXmlElement = protocolXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
		if(nextProtocolXmlElement == nullptr)
			nextProtocolXmlElement = protocolXmlElement->getNextElementWithTagName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
		protocolXmlElement = nextProtocolXmlElement;
	}

	if(m_shouldBeRunning)
		Start();

	return retVal;
}

/**
 * Method that creates the protocol processing object corresponding to the given type.
 *
 * @param type					The protocol type to create the corresponding processor for
 * @param listenerPortNumber	The port for the processor to listen on for client interaction
 * @return	The created processor object
 */
ProtocolProcessor_Abstract *ProcessingEngineNode::CreateProtocolProcessor(ProtocolType type, int listenerPortNumber)
{
	switch(type)
	{
		case PT_OSCProtocol:
			return new OSCProtocolProcessor(m_nodeId, listenerPortNumber);
		case PT_OCAProtocol:
			return new OCAProtocolProcessor(m_nodeId);
		case PT_DummyMidiProtocol:
			return new MIDIProtocolProcessor(m_nodeId);
		default:
			return 0;
	}
}

/**
 * Method that creates the data handling object corresponding to the given handling mode.
 *
 * @param mode	The data handling mode to create the corresponding handling object for
 * @return	The created data handling object
 */
ObjectDataHandling_Abstract* ProcessingEngineNode::CreateObjectDataHandling(ObjectHandlingMode mode)
{
	switch (mode)
	{
	case OHM_Forward_only_valueChanges:
		return new Forward_only_valueChanges(this);
	case OHM_Remap_A_X_Y_to_B_XY:
		return new Remap_A_X_Y_to_B_XY_Handling(this);
	case OHM_Mux_nA_to_mB:
		return new Mux_nA_to_mB(this);
	case OHM_Bypass:
		return new BypassHandling(this);
	case OHM_Invalid:
	default:
		return 0;
	}
}

/**
 * Method to handle incoming message data from the processing protocol objects (they are members of the node object).
 * This is achieved by the member processing protocol objects accessing their parent with this handling method.
 *
 * @param receiver	The protocol processing object that has received the message
 * @param id		The message object id that corresponds to the received message
 * @param msgData	The actual message data that was received
 */
void ProcessingEngineNode::OnProtocolMessageReceived(ProtocolProcessor_Abstract* receiver, RemoteObjectIdentifier id, RemoteObjectMessageData& msgData)
{
	// broadcast received data to all listeners
	for (auto listener : m_listeners)
		listener->HandleNodeData(this->GetId(), receiver->GetId(), receiver->GetType(), id, msgData);
	
	if (m_dataHandling)
		m_dataHandling->OnReceivedMessageFromProtocol(receiver->GetId(), id, msgData);
}

/**
 * Method to forward a message to member protocol with given id
 *
 * @param PId		The id of the protocol to send the RemoteObject to
 * @param Id		The message object id that corresponds to the message to be sent
 * @param msgData	The actual message data that was received
 */
bool ProcessingEngineNode::SendMessageTo(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (m_typeAProtocols.count(PId))
		return m_typeAProtocols[PId]->SendMessage(Id, msgData);
	else if (m_typeBProtocols.count(PId))
		return m_typeBProtocols[PId]->SendMessage(Id, msgData);
	else
		return false;
}