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

#include <JuceHeader.h>

#include <RemoteProtocolBridgeCommon.h>
#include <ProcessingEngine/ProcessingEngine.h>
#include <ProcessingEngine/ProcessingEngineConfig.h>

// Fwd. Declarations
class ProtocolComponent;
class ProtocolConfigWindow;

/**
 * Class ProtocolConfigComponent_Abstract is a container used to hold the GUI controls for modifying the app configuration.
 */
class ProtocolConfigComponent_Abstract :	public Component,
											public Button::Listener,
											public ProcessingEngineConfig::XmlConfigurableElement
{
public:
	ProtocolConfigComponent_Abstract(ProtocolRole role);
	~ProtocolConfigComponent_Abstract();

	//==============================================================================
	virtual const std::pair<int, int> GetSuggestedSize() = 0;

	//==============================================================================
	virtual void AddListener(ProtocolConfigWindow* listener);

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

protected:
	//==============================================================================
	virtual bool						DumpActiveHandlingUsed() = 0;
	virtual std::vector<RemoteObject>	DumpActiveRemoteObjects() = 0;
	virtual std::pair<int, int>			DumpProtocolPorts();
	virtual void						SetActiveHandlingUsed(bool active);
	virtual void						FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs) = 0;
	virtual void						FillProtocolPorts(const std::pair<int, int>& ports);

private:
	void paint(Graphics&) override;
	void resized() override = 0;

	void buttonClicked(Button* button) override;

protected:
	std::unique_ptr<Label>		m_HostPortLabel;		/**< Label as description of host port edit. */
	std::unique_ptr<TextEditor>	m_HostPortEdit;			/**< Edit for editing of host port. */
	std::unique_ptr<Label>		m_ClientPortLabel;		/**< Label as description of client port edit. */
	std::unique_ptr<TextEditor> m_ClientPortEdit;		/**< Edit for editing of client port. */

	std::unique_ptr<Label>		m_Headline;				/**< Headlining Label for complete object list section. */
	std::unique_ptr<TextButton> m_applyConfigButton;	/**< Button to apply edited values to configuration and leave. */
	ProtocolConfigWindow*		m_parentListener;		/**< Parent that needs to be notified when this window self-destroys. */

	ProtocolRole				m_ProtocolRole;			/**< This protocols' role (A or B). */

	std::unique_ptr<XmlElement>	m_protocolXmlElement;	/**< This protocols chached xml config element, needed to modify individual values and still be able to deliver the complete element when requested. */

};

/**
 * Class BasicProtocolConfigComponent is a container used to hold the GUI controls
 * for modifying the protocol configuration in a very basic way.
 */
class BasicProtocolConfigComponent : public ProtocolConfigComponent_Abstract
{
public:
	BasicProtocolConfigComponent(ProtocolRole role);
	~BasicProtocolConfigComponent();

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

	//==============================================================================
	void AddListener(ProtocolConfigWindow* listener) override;

protected:
	//==============================================================================
	bool				DumpActiveHandlingUsed() override;
	std::vector<RemoteObject> DumpActiveRemoteObjects() override;
	void				SetActiveHandlingUsed(bool active) override;
	void				FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs) override;

private:
	void resized() override;

	void buttonClicked(Button* button) override;

	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjEnableChecks;		/**< Enable checkboxes for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<Label>>			m_RemObjNameLabels;			/**< Name labels for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<TextEditor>>		m_RemObjActiveChannelEdits;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<TextEditor>>		m_RemObjActiveRecordEdits;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */

	std::unique_ptr<Label>			m_EnableHeadlineLabel;		/**< Headlining Label for enable checks. */
	std::unique_ptr<Label>			m_ChannelHeadlineLabel;		/**< Headlining Label for channel range edits. */
	std::unique_ptr<Label>			m_RecordHeadlineLabel;		/**< Headlining Label for record range edits. */

	std::unique_ptr<ToggleButton>	m_UseActiveHandlingCheck;	/**< Checkbox to toggle active remote object handling setting. */
	std::unique_ptr<Label>			m_UseActiveHandlingLabel;	/**< Descriptive label for active remote object handling checkbox. */

};

/**
 * Class ActiveObjectScrollContentsComponent is a container 
 * used to hold the GUI elements to configure what remote objects
 * to activly handle for a protocol.
 */
class ActiveObjectScrollContentsComponent : public Component
{
public:
	ActiveObjectScrollContentsComponent();
	~ActiveObjectScrollContentsComponent();

	//==============================================================================
	bool IsActiveHandlingEnabled();
	std::vector<RemoteObject> GetActiveRemoteObjects();
	void SetActiveRemoteObjects(const std::vector<RemoteObject>& Objs);

private:
	void resized() override;

	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjEnableChecks;		/**< Enable checkboxes for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<Label>>			m_RemObjNameLabels;			/**< Name labels for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<TextEditor>>		m_RemObjActiveChannelEdits;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjMappingArea1Checks;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjMappingArea2Checks;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjMappingArea3Checks;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */
	std::map<int, std::unique_ptr<ToggleButton>>	m_RemObjMappingArea4Checks;	/**< Channel Range editing fields for all remote object to be configured/listed on ui. */

};

/**
 * Class OSCProtocolConfigComponent is a container used to hold the GUI controls
 * specifically used to configure d&b OSC protocol configuration.
 */
class OSCProtocolConfigComponent : public ProtocolConfigComponent_Abstract
{
public:
	OSCProtocolConfigComponent(ProtocolRole role);
	~OSCProtocolConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

	//==============================================================================
	void AddListener(ProtocolConfigWindow* listener) override;

protected:
	//==============================================================================
	bool						DumpActiveHandlingUsed() override;
	std::vector<RemoteObject>	DumpActiveRemoteObjects() override;
	void						FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs) override;

private:
	void resized() override;

	void buttonClicked(Button* button) override;

	void FillPollingInterval(int PollingInterval);
	int DumpPollingInterval();

	std::unique_ptr<Label>		m_EnableHeadlineLabel;		/**< Headlining Label for enable checks. */
	std::unique_ptr<Label>		m_ChannelHeadlineLabel;		/**< Headlining Label for channel range edits. */
	std::unique_ptr<Label>		m_MappingsHeadlineLabel;	/**< Headlining Label for mapping checks. */
	std::unique_ptr<Label>		m_Mapping1HeadlineLabel;	/**< Headlining Label for mapping1 checks. */
	std::unique_ptr<Label>		m_Mapping2HeadlineLabel;	/**< Headlining Label for mapping2 checks. */
	std::unique_ptr<Label>		m_Mapping3HeadlineLabel;	/**< Headlining Label for mapping3 checks. */
	std::unique_ptr<Label>		m_Mapping4HeadlineLabel;	/**< Headlining Label for mapping4 checks. */

	std::unique_ptr<ActiveObjectScrollContentsComponent>	m_activeObjectsListComponent;
	std::unique_ptr<Viewport>								m_activeObjectsListScrollView;

	std::unique_ptr<Label>		m_PollingIntervalLabel;		/**< Label as description of polling interval edit. */
	std::unique_ptr<TextEditor> m_PollingIntervalEdit;		/**< Edit for editing of polling interval. */

};

/**
 * Class RTTrPMProtocolConfigComponent is a container used to hold the GUI controls
 * specifically used to configure Blacktrax RTTrPM protocol configuration.
 */
class RTTrPMProtocolConfigComponent : public ProtocolConfigComponent_Abstract,
	public TextEditor::Listener
{
public:
	RTTrPMProtocolConfigComponent(ProtocolRole role);
	~RTTrPMProtocolConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

	//==============================================================================
	void AddListener(ProtocolConfigWindow* listener) override;

protected:
	//==============================================================================
	bool DumpActiveHandlingUsed() override;
	std::vector<RemoteObject> DumpActiveRemoteObjects() override;
	void FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs) override;

private:
	virtual void resized() override;

	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;

	void buttonClicked(Button* button) override;

	void FillMappingAreaId(int MappingAreaId);
	int DumpMappingAreaId();

	std::unique_ptr<Label>		m_MappingAreaIdLabel;		/**< Label as description of MappingArea id edit. */
	std::unique_ptr<TextEditor> m_MappingAreaIdEdit;		/**< Edit for editing of MappingArea id. */
};

/**
 * Class MIDIProtocolConfigComponent is a container used to hold the GUI controls
 * specifically used to configure generic MIDI protocol configuration.
 */
class MIDIProtocolConfigComponent : public ProtocolConfigComponent_Abstract,
	public TextEditor::Listener
{
public:
	MIDIProtocolConfigComponent(ProtocolRole role);
	~MIDIProtocolConfigComponent();

	void setMidiInput(int index);

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

	//==============================================================================
	void AddListener(ProtocolConfigWindow* listener) override;

protected:
	//==============================================================================
	bool DumpActiveHandlingUsed() override;
	std::vector<RemoteObject> DumpActiveRemoteObjects() override;
	void FillActiveRemoteObjects(const std::vector<RemoteObject>& Objs) override;

private:
	virtual void resized() override;

	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;

	void buttonClicked(Button* button) override;

	void FillSelectedMidiInputIndex(int MidiInputIndex);
	int DumpSelectedMidiInputIndex();

	std::unique_ptr<AudioDeviceManager>	m_deviceManager;		/** We use the AudioDeviceManager class to find which MIDI input devices are enabled. */
	std::unique_ptr<ComboBox>			m_midiInputList;        /** We display the names of the MIDI input devices in this combo-box for the user to select.. */
	std::unique_ptr<Label>				m_midiInputListLabel;
};

/**
 * Class ProtocolConfigWindow provides a window that embedds a ProtocolConfigComponent_Abstract
 */
class ProtocolConfigWindow : public DialogWindow, public ProcessingEngineConfig::XmlConfigurableElement
{
public:
	//==============================================================================
	ProtocolConfigWindow(const String &name, Colour backgroundColour, bool escapeKeyTriggersCloseButton, NodeId NId, ProtocolId PId,
						 ProtocolRole role, ProtocolType Type, bool addToDesktop = true);
	~ProtocolConfigWindow();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	void OnEditingFinished();

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize();

	//==============================================================================
	void AddListener(ProtocolComponent* listener);

private:
	void closeButtonPressed() override;

	std::unique_ptr<ProtocolConfigComponent_Abstract>	m_configComponent;	/**< Actual config content component to reside in window. */
	ProtocolComponent*		m_parentListener;	/**< Parent that needs to be notified when this window self-destroys. */
	NodeId					m_NId;				/**< ID of the node this config dialog refers to. */
	ProtocolId				m_PId;				/**< ID of the nodes' protocl this config dialog refers to. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProtocolConfigWindow)
};
