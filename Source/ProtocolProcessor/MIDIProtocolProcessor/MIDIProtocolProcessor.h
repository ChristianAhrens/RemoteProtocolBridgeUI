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

#include "../../Common.h"
#include "../ProtocolProcessor_Abstract.h"

#include <JuceHeader.h>


/**
 * Class MIDIProtocolProcessor is a derived class for MIDI protocol interaction.
 * This currently is only a dummy for potential future functionality.
 * Feel free to implement something yourself here.
 */
class MIDIProtocolProcessor : public ProtocolProcessor_Abstract
{
public:
	MIDIProtocolProcessor();
	~MIDIProtocolProcessor();

	void SetProtocolConfigurationData(const ProcessingEngineConfig::ProtocolData &protocolData, const Array<RemoteObject> &activeObjs, NodeId NId,
									  ProtocolId PId) override;

	bool Start() override;
	bool Stop() override;
	void SetRemoteObjectsActive(const Array<RemoteObject>& Objs) override;
	bool SendMessage(RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) override;

	String GetMIDIRemoteObjectString(RemoteObjectIdentifier id);

};
