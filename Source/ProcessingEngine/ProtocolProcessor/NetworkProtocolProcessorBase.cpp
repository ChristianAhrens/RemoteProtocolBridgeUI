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

#include "NetworkProtocolProcessorBase.h"

#include "../ProcessingEngineNode.h"

// **************************************************************************************
//    class NetworkProtocolProcessorBase
// **************************************************************************************

/**
 * Constructor of abstract class NetworkProtocolProcessorBase.
 */
NetworkProtocolProcessorBase::NetworkProtocolProcessorBase(const NodeId& parentNodeId)
	: ProtocolProcessorBase(parentNodeId)
{
}

/**
 * Destructor
 */
NetworkProtocolProcessorBase::~NetworkProtocolProcessorBase()
{

}

/**
 * Sets the configuration data for the protocol processor object.
 *
 * @param stateXml	The configuration data to parse and set active
 * @return True on success, false on failure
 */
bool NetworkProtocolProcessorBase::setStateXml(XmlElement* stateXml)
{
	if (!ProtocolProcessorBase::setStateXml(stateXml))
		return false;

	auto retVal = true;

	auto ipAdressXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressXmlElement)
		m_ipAddress = ipAdressXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS));
	else
		retVal = false;

	auto clientPortXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		m_clientPort = clientPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
	else
		retVal = false;

	auto hostPortXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		m_hostPort = hostPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
	else
		retVal = false;

	return retVal;
}
