/*
  ==============================================================================

    YmhOSCProtocolProcessor.h
    Created: 18 Dec 2020 07:55:00am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "OSCProtocolProcessor.h"

#include <JuceHeader.h>

/**
 * Class YmhOSCProtocolProcessor is a derived class for special Yamaha RIVAGES OSC protocol interaction.
 */
class YmhOSCProtocolProcessor : public OSCProtocolProcessor
{
public:
	enum MappingAreaId
	{
		MAI_Invalid = -1,
		MAI_First = 1,
		MAI_Second,
		MAI_Third,
		MAI_Fourth,
	};

public:
	YmhOSCProtocolProcessor(const NodeId& parentNodeId, int listenerPortNumber);
	virtual ~YmhOSCProtocolProcessor() override;

	bool setStateXml(XmlElement* stateXml) override;

	bool SendRemoteObjectMessage(RemoteObjectIdentifier id, RemoteObjectMessageData& msgData) override;

	static String GetRemoteObjectDomainString();
	static String GetRemoteObjectParameterTypeString(RemoteObjectIdentifier id);

	virtual void oscMessageReceived(const OSCMessage &message, const String& senderIPAddress, const int& senderPort) override;

private:
	MappingAreaId	m_mappingAreaId{ MAI_Invalid };	/**< The DS100 mapping area to be used when converting incoming coords into relative messages. If this is MAI_Invalid, absolute messages will be generated. */

};
