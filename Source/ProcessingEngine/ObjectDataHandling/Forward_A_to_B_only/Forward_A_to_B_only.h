/*
  ==============================================================================

    Forward_A_to_B_only.h
    Created: 21 Sep 2020 3:05:02pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "../ObjectDataHandling_Abstract.h"
#include "../../../RemoteProtocolBridgeCommon.h"
#include "../../ProcessingEngineConfig.h"

#include <JuceHeader.h>

// Fwd. declarations
class ProcessingEngineNode;

/**
 * Class Forward_A_to_B_only is a class for filtering received value data from RoleB protocols to not be forwarded
 */
class Forward_A_to_B_only : public ObjectDataHandling_Abstract
{
public:
	Forward_A_to_B_only(ProcessingEngineNode* parentNode);
	~Forward_A_to_B_only();

	bool OnReceivedMessageFromProtocol(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) override;

protected:

};