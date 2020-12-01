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

#include <JuceHeader.h>


/**
 * Class MIDIProtocolProcessor is a derived class for MIDI protocol interaction.
 */
class MIDIProtocolProcessor :	public ProtocolProcessor_Abstract,
								private MessageListener,
								private MidiInputCallback
{
public:
	MIDIProtocolProcessor(const NodeId& parentNodeId);
	~MIDIProtocolProcessor();

	//==============================================================================
	void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;

	//==============================================================================
	void handleMessage(const Message& msg) override;

	//==============================================================================
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	bool Start() override;
	bool Stop() override;

	//==============================================================================
	void SetRemoteObjectsActive(XmlElement* activeObjsXmlElement) override;
	void SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement) override;

	bool SendRemoteObjectMessage(RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) override;

	String GetMIDIRemoteObjectString(RemoteObjectIdentifier id);

private:
	// This is used to dispach an incoming midi message to the message thread
	class CallbackMidiMessage : public Message
	{
	public:
		/**
		* Constructor with default initialization of message and source.
		* @param m	The midi message to handle.
		* @param s	The source the message was received from.
		*/
		CallbackMidiMessage(const juce::MidiMessage& m, const juce::String& s) : message(m), source(s) {}

		juce::MidiMessage message;
		juce::String source;
	};

	String getMidiMessageDescription(const juce::MidiMessage& m);

	std::unique_ptr<AudioDeviceManager>	m_deviceManager;		/** We use the AudioDeviceManager class to register midi callbacks and set midi devices to enabled. */
	int									m_lastInputIndex{ 0 };

	int									m_currentOnNoteNumber{ -1 };
	float								m_currentX{ 0.0f };
	float								m_currentY{ 0.0f };

};
