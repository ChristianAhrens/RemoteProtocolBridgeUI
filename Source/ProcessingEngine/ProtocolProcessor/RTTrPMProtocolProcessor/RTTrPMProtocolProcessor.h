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

#include "../../../RemoteProtocolBridgeCommon.h"
#include "../ProtocolProcessor_Abstract.h"

#include "RTTrPMConnectionServer.h"

#include <JuceHeader.h>

/**
 * Class RTTrPMProtocolProcessor is a derived class for OSC protocol interaction.
 */
class RTTrPMProtocolProcessor : public CRTTrPMConnectionServer::RTTrPMListener,
	public ProtocolProcessor_Abstract
{
public:
	RTTrPMProtocolProcessor(const NodeId& parentNodeId, int listenerPortNumber);
	~RTTrPMProtocolProcessor();

	bool setStateXml(XmlElement* stateXml) override;

	bool Start() override;
	bool Stop() override;

	void SetRemoteObjectsActive(XmlElement* activeObjsXmlElement) override;
	void SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement) override;

	bool SendRemoteObjectMessage(RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) override;

	void RTTrPMModuleReceived(const CPacketModule& RTTrPMmodule, const String& senderIPAddress, const int& senderPort) override;

private:
	CRTTrPMConnectionServer	m_rttrpmReceiver;		/**< An receiver object for BlackTrax RTTrPM protocol that binds to a network port to receive data
													   * via UDP, parse it, and forwards the included RTTrPM packet modules to its listeners. */
	Array<RemoteObject>		m_activeRemoteObjects;	/**< List of remote objects to be activly handled. */
	Array<int>				m_mutedRemoteObjectChannels;	/**< List of remote object channelss to be muted. */

	float m_floatValueBuffer[3] = { 0.0f, 0.0f, 0.0f };
	int m_intValueBuffer[2] = { 0, 0 };
};
