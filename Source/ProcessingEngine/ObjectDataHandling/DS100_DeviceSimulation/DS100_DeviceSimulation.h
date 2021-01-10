/*
  ==============================================================================

	DS100_DeviceSimulation.h
	Created: 4 Aug 2020 08:47:37am
	Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "../ObjectDataHandling_Abstract.h"
#include "../../../RemoteProtocolBridgeCommon.h"
#include "../../ProcessingEngineConfig.h"
#include "../../TimerThreadBase.h"

#include <JuceHeader.h>

// Fwd. declarations
class ProcessingEngineNode;

/**
 * Class DS100_DeviceSimulation is a class for filtering received value data to only forward changed values.
 */
class DS100_DeviceSimulation : public ObjectDataHandling_Abstract, public TimerThreadBase
{
public:
	DS100_DeviceSimulation(ProcessingEngineNode* parentNode);
	~DS100_DeviceSimulation();

	//==============================================================================
	bool setStateXml(XmlElement* stateXml) override;

	//==============================================================================
	bool OnReceivedMessageFromProtocol(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) override;

	//==============================================================================
	void timerThreadCallback() override;

private:
	bool IsDataRequestPollMessage(const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData);
	bool ReplyToDataRequest(const ProtocolId PId, const RemoteObjectIdentifier Id, const RemoteObjectAddressing adressing);

	void InitDataValues();
	void SetDataValue(const ProtocolId PId, const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData);

	void PrintMessageSendInfo(const std::pair<RemoteObjectIdentifier, RemoteObjectMessageData>& idDataKV);
	void PrintDataUpdateInfo(const std::pair<RemoteObjectIdentifier, RemoteObjectMessageData>& idDataKV);
	
	CriticalSection						m_currentValLock;			/**< For securing access to current values map and other member variables. */
	std::map<RemoteObjectIdentifier, std::map<RemoteObjectAddressing, RemoteObjectMessageData>>	m_currentValues;	/**< Map of current value data to use to compare to incoming data regarding value changes. */
	std::vector<RemoteObjectIdentifier>	m_simulatedRemoteObjects;	/**< The remote objects that are simulated. */
	int									m_simulatedChCount;			/**< Count of channels that are currently simulated. */
	int									m_simulatedMappingsCount;	/**< Count of mapping areas (mappings) that are currently simulated. */
	int									m_refreshInterval;			/**< Refresh interval to update internal simulation. */

	float								m_simulationBaseValue;		/**< Rolling value that is used as base for simulated object values. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DS100_DeviceSimulation)
};
