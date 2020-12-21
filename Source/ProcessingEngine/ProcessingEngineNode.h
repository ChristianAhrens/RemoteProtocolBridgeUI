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

#pragma once

#include "../RemoteProtocolBridgeCommon.h"

#include "ProcessingEngineConfig.h"
#include "ProtocolProcessor/ProtocolProcessorBase.h"

// Fwd. declarations
class ObjectDataHandling_Abstract;
class ProcessingEngine;

/**
 * Class ProcessingEngineNode is a class to hold a processing element handled by engine class.
 */
class ProcessingEngineNode :	public ProtocolProcessorBase::Listener,
								public ProcessingEngineConfig::XmlConfigurableElement,
								private MessageListener
{
public:
	/**
	 * Implementation of a node internal message.
	 */
	struct NodeCallbackMessage : public Message
	{
		/**
		 * Constructor with default initialization.
		 *
		 * @param nodeId				The node id to initialize the member with.
		 * @param senderProtocolId		The sender protocol id to initialize the member with.
		 * @param senderProtocolType	The sender protocol type to initialize the member with.
		 * @param Id					The remote object id to initialize the member with.
		 * @param msgData				The message data to initialize the member with.
		 */
		NodeCallbackMessage(NodeId nodeId, ProtocolId senderProtocolId, ProtocolType senderProtocolType, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) : 
			_nodeId(nodeId),
			_senderProtocolId(senderProtocolId),
			_senderProtocolType(senderProtocolType),
			_Id(Id),
			_msgData(msgData) {}

		/**
		 * Constructor with default initialization.
		 *
		 * @param senderProtocolId		The sender protocol id to initialize the member with.
		 * @param Id					The remote object id to initialize the member with.
		 * @param msgData				The message data to initialize the member with.
		 */
		NodeCallbackMessage(ProtocolId senderProtocolId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) :
			_senderProtocolId(senderProtocolId),
			_Id(Id),
			_msgData(msgData) {}

		NodeId						_nodeId{ static_cast<NodeId>(INVALID_ADDRESS_VALUE) };
		ProtocolId					_senderProtocolId{ static_cast<ProtocolId>(INVALID_ADDRESS_VALUE) };
		ProtocolType				_senderProtocolType{ PT_Invalid };
		RemoteObjectIdentifier		_Id{ ROI_Invalid };
		RemoteObjectMessageData&	_msgData;
	};

	/**
	 * Abstract embedded interface class for message data handling
	 */
	class NodeListener
	{
	public:
		NodeListener() {};
		virtual ~NodeListener() {};

		/**
		 * Method to be overloaded by ancestors to act as an interface
		 * for handling of received message data
		 */
		virtual void HandleNodeData(const NodeCallbackMessage* callbackMessage) = 0;
	};

public:
	ProcessingEngineNode();
	ProcessingEngineNode(ProcessingEngineNode::NodeListener* listener);
	~ProcessingEngineNode();

	void AddListener(ProcessingEngineNode::NodeListener* listener);

	NodeId GetId();

	bool SendMessageTo(ProtocolId PId, RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) const;

	bool Start();
	bool Stop();

	//==============================================================================
	virtual std::unique_ptr<XmlElement> createStateXml() override;
	virtual bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	void OnProtocolMessageReceived(ProtocolProcessorBase* receiver, RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) override;

	//==============================================================================
	void handleMessage(const Message& msg) override;

private:
	ProtocolProcessorBase* CreateProtocolProcessor(ProtocolType type, int listenerPortNumber);
	ObjectDataHandling_Abstract* CreateObjectDataHandling(ObjectHandlingMode mode);

	std::unique_ptr<ObjectDataHandling_Abstract>					m_dataHandling;		/**< The object data handling object (to be initialized with instance of derived class). */

	NodeId															m_nodeId;			/**< The id of the bridging node object. */

	std::map<ProtocolId, std::unique_ptr<ProtocolProcessorBase>>	m_typeAProtocols;	/**< The remote protocols that act with role A of this node. */
	std::map<ProtocolId, std::unique_ptr<ProtocolProcessorBase>>	m_typeBProtocols;	/**< The remote protocols that act with role B of this node. */

	std::vector<ProcessingEngineNode::NodeListener*>				m_listeners;		/**< The listner objects, for e.g. logging message traffic. */

	bool															m_protocolsRunning;

};
