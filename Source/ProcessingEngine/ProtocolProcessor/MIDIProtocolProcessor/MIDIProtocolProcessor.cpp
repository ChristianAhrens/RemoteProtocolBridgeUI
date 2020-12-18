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

#include "MIDIProtocolProcessor.h"

#include "../../ProcessingEngineConfig.h"


// **************************************************************************************
//    class MIDIProtocolProcessor
// **************************************************************************************
/**
 * Derived MIDI remote protocol processing class
 */
MIDIProtocolProcessor::MIDIProtocolProcessor(const NodeId& parentNodeId)
	: ProtocolProcessorBase(parentNodeId)
{
	m_type = ProtocolType::PT_MidiProtocol;

	m_deviceManager = std::make_unique<AudioDeviceManager>();
}

/**
 * Destructor
 */
MIDIProtocolProcessor::~MIDIProtocolProcessor()
{
	
	auto list = juce::MidiInput::getAvailableDevices();
	if (m_lastInputIndex >= 0 && list.size() > m_lastInputIndex)
	{
		auto oldInput = list[m_lastInputIndex];

		DBG("Deactivating MIDI input " + oldInput.name + +" (" + oldInput.identifier + ")");

		m_deviceManager->removeMidiInputDeviceCallback(oldInput.identifier, this);
	}
}

/**
 * Reimplemented from MidiInputCallback to handle midi messages.
 * @param source	The data source midi input.
 * @param message	The midi input data.
 */
void MIDIProtocolProcessor::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
	// dispatch message to queue
	postMessage(std::make_unique<CallbackMidiMessage>(message, (source == nullptr ? "UNKNOWN" : source->getName())).release());
}

/**
 * Reimplemented from MessageListener to handle messages posted to queue.
 * @param msg	The incoming message to handle
 */
void MIDIProtocolProcessor::handleMessage(const Message& msg)
{
	if (auto* callbackMessage = dynamic_cast<const CallbackMidiMessage*> (&msg))
	{
		auto const& midiMessage = callbackMessage->message;

		DBG("MIDI received: " + getMidiMessageDescription(midiMessage));

		RemoteObjectIdentifier newObjectId = ROI_Invalid;
		RemoteObjectMessageData newMsgData;
		newMsgData.addrVal.first = INVALID_ADDRESS_VALUE;
		newMsgData.addrVal.second = INVALID_ADDRESS_VALUE;
		newMsgData.valType = ROVT_NONE;
		newMsgData.valCount = 0;
		newMsgData.payload = 0;
		newMsgData.payloadSize = 0;

		// NoteOn/Off is hardcoded as matrixinput select message trigger
		if (midiMessage.isNoteOn() || midiMessage.isNoteOff())
		{
			m_currentNoteNumber = midiMessage.getNoteNumber() - 47;

			if (midiMessage.isNoteOn())
				m_intValueBuffer[0] = 1;
			if (midiMessage.isNoteOff())
				m_intValueBuffer[0] = 0;

			newObjectId = ROI_MatrixInput_Select;

			newMsgData.addrVal.first = m_currentNoteNumber;
			newMsgData.addrVal.second = 0;
			newMsgData.valType = ROVT_INT;
			newMsgData.valCount = 1;
			newMsgData.payload = &m_intValueBuffer;
			newMsgData.payloadSize = sizeof(int);

			if (m_currentNoteNumber > -1 && m_messageListener && !IsChannelMuted(m_currentNoteNumber))
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);

			if (midiMessage.isNoteOff())
				m_currentNoteNumber = -1;
		}

		// If the received channel (source) is set to muted, return without further processing
		if (IsChannelMuted(m_currentNoteNumber))
			return;

		// Pitchwheel or controllervalue (controller 1) is hardcoded as xy message trigger
		if (midiMessage.isPitchWheel() || (midiMessage.isController() && midiMessage.getControllerNumber() == 1))
		{
			if (midiMessage.isPitchWheel())
				m_currentX = midiMessage.getPitchWheelValue() / 16383.0f;
			if (midiMessage.isController())
				m_currentY = midiMessage.getControllerValue() / 127.0f;

			newObjectId = ROI_CoordinateMapping_SourcePosition_XY;

			m_floatValueBuffer[0] = m_currentX;
			m_floatValueBuffer[1] = m_currentY;

			newMsgData.addrVal.first = m_currentNoteNumber;
			newMsgData.addrVal.second = 1;
			newMsgData.valType = ROVT_FLOAT;
			newMsgData.valCount = 2;
			newMsgData.payload = &m_floatValueBuffer;
			newMsgData.payloadSize = 2 * sizeof(float);

			if (m_currentNoteNumber > -1 && m_messageListener)
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
		}

		// Controllervalue (controller 5) is hardcoded as reverbsendgain message trigger
		if (midiMessage.isController() && midiMessage.getControllerNumber() == 5)
		{
			auto normValue = midiMessage.getControllerValue() / 127.0f;
		
			newObjectId = ROI_MatrixInput_ReverbSendGain;
		
			m_floatValueBuffer[0] = (normValue * 144.0f) - 120.0f; // -120 ... +24
		
			newMsgData.addrVal.first = m_currentNoteNumber;
			newMsgData.addrVal.second = 1;
			newMsgData.valType = ROVT_FLOAT;
			newMsgData.valCount = 1;
			newMsgData.payload = &m_floatValueBuffer;
			newMsgData.payloadSize = sizeof(float);
		
			if (m_currentNoteNumber > -1 && m_messageListener)
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
		}

		// Controllervalue (controller 6) is hardcoded as spread factor message trigger
		if (midiMessage.isController() && midiMessage.getControllerNumber() == 6)
		{
			auto normValue = midiMessage.getControllerValue() / 127.0f;

			newObjectId = ROI_Positioning_SourceSpread;

			m_floatValueBuffer[0] = normValue;

			newMsgData.addrVal.first = m_currentNoteNumber;
			newMsgData.addrVal.second = 1;
			newMsgData.valType = ROVT_FLOAT;
			newMsgData.valCount = 1;
			newMsgData.payload = &m_floatValueBuffer;
			newMsgData.payloadSize = sizeof(float);

			if (m_currentNoteNumber > -1 && m_messageListener)
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
		}

		// Controllervalue (controller 7) is hardcoded as delaymode message trigger
		if (midiMessage.isController() && midiMessage.getControllerNumber() == 7)
		{
			auto normValue = midiMessage.getControllerValue() / 127.0f;

			newObjectId = ROI_Positioning_SourceDelayMode;

			m_intValueBuffer[0] = 2 * static_cast<int>(normValue); // 0 - 2

			newMsgData.addrVal.first = m_currentNoteNumber;
			newMsgData.addrVal.second = 1;
			newMsgData.valType = ROVT_INT;
			newMsgData.valCount = 1;
			newMsgData.payload = &m_intValueBuffer;
			newMsgData.payloadSize = sizeof(float);

			if (m_currentNoteNumber > -1 && m_messageListener)
				m_messageListener->OnProtocolMessageReceived(this, newObjectId, newMsgData);
		}
	}
}

/**
 * Debugging helper method taken from JUCE's "HandlingMidiEventsTutorial"
 */
String MIDIProtocolProcessor::getMidiMessageDescription(const juce::MidiMessage& m)
{
	if (m.isNoteOn())           return "Note on " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
	if (m.isNoteOff())          return "Note off " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3);
	if (m.isProgramChange())    return "Program change " + juce::String(m.getProgramChangeNumber());
	if (m.isPitchWheel())       return "Pitch wheel " + juce::String(m.getPitchWheelValue());
	if (m.isAftertouch())       return "After touch " + juce::MidiMessage::getMidiNoteName(m.getNoteNumber(), true, true, 3) + ": " + juce::String(m.getAfterTouchValue());
	if (m.isChannelPressure())  return "Channel pressure " + juce::String(m.getChannelPressureValue());
	if (m.isAllNotesOff())      return "All notes off";
	if (m.isAllSoundOff())      return "All sound off";
	if (m.isMetaEvent())        return "Meta event";

	if (m.isController())
	{
		juce::String name(juce::MidiMessage::getControllerName(m.getControllerNumber()));

		if (name.isEmpty())
			name = "[" + juce::String(m.getControllerNumber()) + "]";

		return "Controller " + name + ": " + juce::String(m.getControllerValue());
	}

	return juce::String::toHexString(m.getRawData(), m.getRawDataSize());
}

/**
 * Sets the xml configuration for the protocol processor object.
 *
 * @param stateXml	The XmlElement containing configuration for this protocol processor instance
 * @return True on success, False on failure
 */
bool MIDIProtocolProcessor::setStateXml(XmlElement* stateXml)
{
	if (!ProtocolProcessorBase::setStateXml(stateXml))
		return false;
	else
	{
		auto MidiInputIndex = -1;
		auto midiInputIndexXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
		if (midiInputIndexXmlElement)
			MidiInputIndex = midiInputIndexXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEINDEX));

		auto list = juce::MidiInput::getAvailableDevices();
		if (list.size() <= m_lastInputIndex)
			return false;
		if (list.size() <= MidiInputIndex)
			return false;

		auto oldInput = list[m_lastInputIndex];
		auto newInput = list[MidiInputIndex];

		if (MidiInputIndex < 0 && m_lastInputIndex >= 0)
		{
			DBG(String(__FUNCTION__) + " Deactivating MIDI input " + oldInput.name + +" (" + oldInput.identifier + ")");

			m_deviceManager->removeMidiInputDeviceCallback(oldInput.identifier, this);
			return false;
		}
		else if (m_lastInputIndex != MidiInputIndex)
		{
			m_deviceManager->removeMidiInputDeviceCallback(oldInput.identifier, this);

			DBG(String(__FUNCTION__) + " Activating MIDI input " + newInput.name + +" (" + newInput.identifier + ")");

			if (!m_deviceManager->isMidiInputDeviceEnabled(newInput.identifier))
				m_deviceManager->setMidiInputDeviceEnabled(newInput.identifier, true);

			m_deviceManager->addMidiInputDeviceCallback(newInput.identifier, this);
			m_lastInputIndex = MidiInputIndex;
			return true;
		}
		else
			return false;
	}
}

/**
 * Overloaded method to start the protocol processing object.
 * Usually called after configuration has been set.
 */
bool MIDIProtocolProcessor::Start()
{
	return true;
}

/**
 * Overloaded method to stop to protocol processing object.
 */
bool MIDIProtocolProcessor::Stop()
{
	return true;
}

/**
 * Setter for remote object to specifically activate.
 * For MIDI processing this is used to initialize MIDI Object Subscriptions
 *
 * @param Objs	The list of RemoteObjects that shall be activated
 */
void MIDIProtocolProcessor::SetRemoteObjectsActive(XmlElement* activeObjsXmlElement)
{
	ignoreUnused(activeObjsXmlElement);
}

/**
 * Setter for remote objects to not forward for further processing.
 * NOT YET IMPLEMENTED
 *
 * @param mutedObjChsXmlElement	The xml element that has to be parsed to get the object data
 */
void MIDIProtocolProcessor::SetRemoteObjectChannelsMuted(XmlElement* mutedObjChsXmlElement)
{
	ignoreUnused(mutedObjChsXmlElement);
}

/**
 * Method to trigger sending of a message
 * NOT YET IMPLEMENTED
 *
 * @param Id		The id of the object to send a message for
 * @param msgData	The message payload and metadata
 */
bool MIDIProtocolProcessor::SendRemoteObjectMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	ignoreUnused(Id);
	ignoreUnused(msgData);

	return false;
}

/**
 * Private method to get MIDI object specific ObjectName string
 * 
 * @param id	The object id to get the MIDI specific object name
 * @return		The MIDI specific object name
 */
String MIDIProtocolProcessor::GetMIDIRemoteObjectString(RemoteObjectIdentifier id)
{
	switch (id)
	{
	case ROI_CoordinateMapping_SourcePosition_X:
		return "Positioning_Source_Position_X";
	case ROI_CoordinateMapping_SourcePosition_Y:
		return "Positioning_Source_Position_Y";
	case ROI_Positioning_SourceSpread:
		return "Positioning_Source_Spread";
	case ROI_Positioning_SourceDelayMode:
		return "Positioning_Source_DelayMode";
	case ROI_MatrixInput_ReverbSendGain:
		return "MatrixInput_ReverbSendGain";
	default:
		return "";
	}
}