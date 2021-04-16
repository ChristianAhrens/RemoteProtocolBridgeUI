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
class NodeComponent;
class ObjectHandlingConfigWindow;

/**
 * Class ObjectHandlingConfigComponent_Abstract is a container used to hold the GUI controls for modifying the node object handling configuration.
 */
class ObjectHandlingConfigComponent_Abstract :	public Component,
												public Button::Listener,
												public ProcessingEngineConfig::XmlConfigurableElement
{
public:
	ObjectHandlingConfigComponent_Abstract(ObjectHandlingMode mode = OHM_Invalid);
	~ObjectHandlingConfigComponent_Abstract();

	//==============================================================================
	virtual const std::pair<int, int> GetSuggestedSize() = 0;

	//==============================================================================
	virtual void AddListener(ObjectHandlingConfigWindow* listener);

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

private:
	virtual void paint(Graphics&) override;
	virtual void resized() override = 0;

	void buttonClicked(Button* button) override;

protected:
	std::unique_ptr<Label>			m_Headline;				/**< Headlining Label for complete object list section. */
	std::unique_ptr<TextButton>		m_applyConfigButton;	/**< Button to apply edited values to configuration and leave. */
	ObjectHandlingConfigWindow*		m_parentListener;		/**< Parent that needs to be notified when this window self-destroys. */
	ObjectHandlingMode				m_mode;					/**< The mode of this OH object. */

};

/**
 * Class OHNoConfigComponent is a container used to hold the GUI
 * for Object Handling mode modules that do not require configuration.
 */
class OHNoConfigComponent : public ObjectHandlingConfigComponent_Abstract,
	public TextEditor::Listener
{
public:
	OHNoConfigComponent(ObjectHandlingMode mode);
	~OHNoConfigComponent();

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

private:
	void resized() override;

};

/**
 * Class OHMultiplexAtoBConfigComponent is a container used to hold the GUI
 * specifically used to configure configuration of mux n A to m B protocol channel counts.
 */
class OHMultiplexAtoBConfigComponent : public ObjectHandlingConfigComponent_Abstract,
	public TextEditor::Listener
{
public:
	OHMultiplexAtoBConfigComponent(ObjectHandlingMode mode);
	~OHMultiplexAtoBConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

private:
	virtual void resized() override;

	virtual void textEditorFocusLost(TextEditor &) override;
	virtual void textEditorReturnKeyPressed(TextEditor &) override;

	std::unique_ptr<Label>		m_CountALabel;	/**< Headlining Label for enable checks. */
	std::unique_ptr<TextEditor>	m_CountAEdit;	/**< Headlining Label for channel range edits. */
	std::unique_ptr<Label>		m_CountBLabel;	/**< Headlining Label for mapping checks. */
	std::unique_ptr<TextEditor>	m_CountBEdit;	/**< Headlining Label for mapping1 checks. */

};

/**
 * Class OHForwardOnlyValueChangesConfigComponent is a container used to hold the GUI
 * specifically used to configure configuration of Forward_only_valueChanges protocol.
 */
class OHForwardOnlyValueChangesConfigComponent : public ObjectHandlingConfigComponent_Abstract,
	public ComboBox::Listener
{
	enum PrecVal
	{
		PV_INVALID,
		PV_EVEN,	// 1
		PV_CENTI,	// 0.1
		PV_MILLI,	// 0.01
		PV_MICRO,	// 0.001
	};

public:
	OHForwardOnlyValueChangesConfigComponent(ObjectHandlingMode mode);
	~OHForwardOnlyValueChangesConfigComponent();

	//==============================================================================
	virtual void resized() override;

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

	//==============================================================================
	double GetPrecision();
	void SetPrecision(double precision);

private:
	void comboBoxChanged(ComboBox* comboBox) override;

	static String PrecisionValToString(PrecVal pv);

	std::unique_ptr<Label>		m_PrecisionLabel;	/**< Label for precision values. */
	std::unique_ptr<ComboBox>	m_PrecisionSelect;	/**< Dropdown for possible precision values. */

};

/**
 * Class OHMirrorDualAwithValFilterConfigComponent is a container used to hold the GUI
 * specifically used to configure configuration of Mirror_dualA_withValFilter protocol.
 */
class OHMirrorDualAwithValFilterConfigComponent : public OHForwardOnlyValueChangesConfigComponent,
	public TextEditor::Listener
{
public:
	OHMirrorDualAwithValFilterConfigComponent(ObjectHandlingMode mode);
	~OHMirrorDualAwithValFilterConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

private:
	virtual void resized() override;

	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;

	std::unique_ptr<Label>		m_failoverTimeLabel;	/**< Headlining Label for failover time edit. */
	std::unique_ptr<TextEditor>	m_failoverTimeEdit;		/**< Headlining Label for failover time edit. */

};

/**
 * Class OHDS100SimConfigComponent is a container used to hold the GUI
 * specifically used to configure configuration of DS100_DeviceSimulation protocol value filter precision.
 */
class OHDS100SimConfigComponent : public ObjectHandlingConfigComponent_Abstract,
	public TextEditor::Listener
{
public:
	OHDS100SimConfigComponent(ObjectHandlingMode mode);
	~OHDS100SimConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

private:
	virtual void resized() override;

	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;

	std::unique_ptr<Label>		m_CountChannelsLabel;	/**< . */
	std::unique_ptr<TextEditor>	m_CountChannelsEdit;	/**< . */
	std::unique_ptr<Label>		m_CountMappingsLabel;	/**< . */
	std::unique_ptr<TextEditor>	m_CountMappingsEdit;	/**< . */
	std::unique_ptr<Label>		m_RefreshIntervalLabel;	/**< . */
	std::unique_ptr<TextEditor>	m_RefreshIntervalEdit;	/**< . */

};

/**
 * Class OHMuxAtoBOnlyValueChangesConfigComponent is a container used to hold the GUI
 * specifically used to configure configuration of mux n A to m B protocol channel counts
 * and precision for detecting value changes to be forwarded.
 */
class OHMuxAtoBOnlyValueChangesConfigComponent : public ObjectHandlingConfigComponent_Abstract,
	public TextEditor::Listener,
	public ComboBox::Listener
{
	enum PrecVal
	{
		PV_INVALID,
		PV_EVEN,	// 1
		PV_CENTI,	// 0.1
		PV_MILLI,	// 0.01
		PV_MICRO,	// 0.001
	};
public:
	OHMuxAtoBOnlyValueChangesConfigComponent(ObjectHandlingMode mode);
	~OHMuxAtoBOnlyValueChangesConfigComponent();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize() override;

private:
	//==============================================================================
	virtual void resized() override;

	//==============================================================================
	virtual void textEditorFocusLost(TextEditor&) override;
	virtual void textEditorReturnKeyPressed(TextEditor&) override;

	//==============================================================================
	void comboBoxChanged(ComboBox* comboBox) override;

	//==============================================================================
	static String PrecisionValToString(PrecVal pv);

	//==============================================================================
	std::unique_ptr<Label>		m_CountALabel;	/**< Headlining Label for enable checks. */
	std::unique_ptr<TextEditor>	m_CountAEdit;	/**< Headlining Label for channel range edits. */
	std::unique_ptr<Label>		m_CountBLabel;	/**< Headlining Label for mapping checks. */
	std::unique_ptr<TextEditor>	m_CountBEdit;	/**< Headlining Label for mapping1 checks. */
	std::unique_ptr<Label>		m_PrecisionLabel;	/**< Label for precision values. */
	std::unique_ptr<ComboBox>	m_PrecisionSelect;	/**< Dropdown for possible precision values. */

};

/**
 * Class ObjectHandlingConfigWindow provides a window that embedds an ObjectHandlingConfigComponent_Abstract
 */
class ObjectHandlingConfigWindow : public DialogWindow, 
	public ProcessingEngineConfig::XmlConfigurableElement
{
public:
	//==============================================================================
	ObjectHandlingConfigWindow(const String &name, Colour backgroundColour, bool escapeKeyTriggersCloseButton, NodeId NId, ObjectHandlingMode mode,
							   bool addToDesktop = true);
	~ObjectHandlingConfigWindow();

	//==============================================================================
	void OnEditingFinished();

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	const std::pair<int, int> GetSuggestedSize();

	//==============================================================================
	void AddListener(NodeComponent* listener);

private:
	void closeButtonPressed() override;

	std::unique_ptr<ObjectHandlingConfigComponent_Abstract>	m_configComponent;	/**< Actual config content component to reside in window. */
	NodeComponent*									m_parentListener;	/**< Parent that needs to be notified when this window self-destroys. */
	NodeId											m_NId;				/**< ID of the node this config dialog refers to. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ObjectHandlingConfigWindow)
};
