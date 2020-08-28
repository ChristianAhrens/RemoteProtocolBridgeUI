/*
  ==============================================================================

    Reverse_B_to_A_only.h
    Created: 21 Sep 2020 3:06:18pm
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
 * Class Reverse_B_to_A_only is a class for filtering received value data from RoleA protocols to not be forwarded
 */
class Reverse_B_to_A_only : public ObjectDataHandling_Abstract
{
public:
	Reverse_B_to_A_only(ProcessingEngineNode* parentNode);
	~Reverse_B_to_A_only();

	bool OnReceivedMessageFromProtocol(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) override;

protected:

};