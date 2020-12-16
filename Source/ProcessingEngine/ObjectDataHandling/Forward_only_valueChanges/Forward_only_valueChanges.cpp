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

#include "Forward_only_valueChanges.h"

#include "../../ProcessingEngineNode.h"
#include "../../ProcessingEngineConfig.h"


// **************************************************************************************
//    class Forward_only_valueChanges
// **************************************************************************************
/**
 * Constructor of class Forward_only_valueChanges.
 *
 * @param parentNode	The objects' parent node that is used by derived objects to forward received message contents to.
 */
Forward_only_valueChanges::Forward_only_valueChanges(ProcessingEngineNode* parentNode)
	: ObjectDataHandling_Abstract(parentNode)
{
	SetMode(ObjectHandlingMode::OHM_Forward_only_valueChanges);
	m_precision = 0.001f;
}

/**
 * Destructor
 */
Forward_only_valueChanges::~Forward_only_valueChanges()
{
	for (std::pair<RemoteObjectIdentifier, std::map<RemoteObjectAddressing, RemoteObjectMessageData>> roi : m_currentValues)
	{
		for (std::pair<RemoteObjectAddressing, RemoteObjectMessageData> val : roi.second)
		{
            switch(val.second.valType)
            {
                case ROVT_INT:
                    delete static_cast<int*>(val.second.payload);
                    break;
                case ROVT_FLOAT:
                    delete static_cast<float*>(val.second.payload);
                    break;
                case ROVT_STRING:
                    delete static_cast<char*>(val.second.payload);
                    break;
                default:
                    break;
            }
	
			val.second.payload = nullptr;
			val.second.payloadSize = 0;
			val.second.valCount = 0;
		}
	}
	
	m_currentValues.clear();
}

/**
 * Reimplemented to set the custom parts from configuration for the datahandling object.
 *
 * @param config	The overall configuration object that can be used to query config data from
 * @param NId		The node id of the parent node this data handling object is child of (needed to access data from config)
 */
bool Forward_only_valueChanges::setStateXml(XmlElement* stateXml)
{
	if (!ObjectDataHandling_Abstract::setStateXml(stateXml))
		return false;

	if (stateXml->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)) != ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges))
		return false;

	auto precisionXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
	if (precisionXmlElement)
		m_precision = precisionXmlElement->getAllSubText().getFloatValue();
	else
		return false;

	return true;
}

/**
 * Method to be called by parent node on receiving data from node protocol with given id
 *
 * @param PId		The id of the protocol that received the data
 * @param Id		The object id to send a message for
 * @param msgData	The actual message value/content data
 * @return	True if successful sent/forwarded, false if not
 */
bool Forward_only_valueChanges::OnReceivedMessageFromProtocol(ProtocolId PId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	const ProcessingEngineNode* parentNode = ObjectDataHandling_Abstract::GetParentNode();
	if (parentNode)
	{
		if (!IsChangedDataValue(Id, msgData))
			return false;

		if (GetProtocolAIds().contains(PId))
		{
			// Send to all typeB protocols
			bool sendSuccess = true;
			int typeBProtocolCount = GetProtocolBIds().size();
			for (int i = 0; i < typeBProtocolCount; ++i)
				sendSuccess = sendSuccess && parentNode->SendMessageTo(GetProtocolBIds()[i], Id, msgData);

			return sendSuccess;

		}
		if (GetProtocolBIds().contains(PId))
		{
			// Send to all typeA protocols
			bool sendSuccess = true;
			int typeAProtocolCount = GetProtocolAIds().size();
			for (int i = 0; i < typeAProtocolCount; ++i)
				sendSuccess = sendSuccess && parentNode->SendMessageTo(GetProtocolAIds()[i], Id, msgData);

			return sendSuccess;
		}
	}

	return false;
}

/**
 * Helper method to detect if incoming value has changed in any way compared with the previously received one
 * (RemoteObjectIdentifier is taken in account as well as the channel/record addressing)
 *
 * @param Id	The ROI that was received and has to be checked
 * @param msgData	The received message data that has to be checked
 * @return True if a change has been detected, false if not
 */
bool Forward_only_valueChanges::IsChangedDataValue(const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	if (m_precision == 0)
		return true;

	auto isChangedDataValue = false;

	// if our hash does not yet contain our ROI, initialize it
	if ((m_currentValues.count(Id) == 0) || (m_currentValues.at(Id).count(msgData.addrVal) == 0))
	{
		isChangedDataValue = true;
	}
	else
	{
		const RemoteObjectMessageData& currentVal = m_currentValues.at(Id).at(msgData.addrVal);
		if ((currentVal.valType != msgData.valType) || (currentVal.valCount != msgData.valCount) || (currentVal.payloadSize != msgData.payloadSize))
		{
			isChangedDataValue = true;
		}
		else
		{
			uint16 valCount = currentVal.valCount;
			RemoteObjectValueType valType = currentVal.valType;
			auto refData = currentVal.payload;
			auto newData = msgData.payload;
	
			auto referencePrecisionValue = 0;
			auto newPrecisionValue = 0;
			
			auto changeFound = false;
			for (int i = 0; i < valCount; ++i)
			{
				switch (valType)
				{
				case ROVT_INT:
					{
					// convert payload to correct pointer type
					auto refVal = static_cast<int*>(refData);
					auto newVal = static_cast<int*>(newData);
					// grab actual value
					referencePrecisionValue = *refVal;
					newPrecisionValue = *newVal;
					// increase pointer to next value (to access it in next valCount loop iteration)
					refData = refVal+1;
					newData = newVal+1;
					}
					break;
				case ROVT_FLOAT:
					{
					// convert payload to correct pointer type
					auto refVal = static_cast<float*>(refData);
					auto newVal = static_cast<float*>(newData);
					// grab actual value and apply precision to get a comparable value
					referencePrecisionValue = static_cast<int>(std::roundf(*refVal / m_precision));
					newPrecisionValue		= static_cast<int>(std::roundf(*newVal / m_precision));
					// increase pointer to next value (to access it in next valCount loop iteration)
					refData = refVal+1;
					newData = newVal+1;
					}
					break;
				case ROVT_STRING:
					jassertfalse; // String not (yet?) supported
					changeFound = true;
					break;
				case ROVT_NONE:
				default:
					changeFound = true;
					break;
				}
	
	
				if (referencePrecisionValue != newPrecisionValue)
					changeFound = true;
			}
	
			isChangedDataValue = changeFound;
		}
	}

	if (isChangedDataValue)
		SetCurrentDataValue(Id, msgData);

	return isChangedDataValue;
}

/**
 * Helper method to set a new RemoteObjectMessageData obj. to internal map of current values.
 * Takes care of cleaning up previously stored data if required.
 *
 * @param Id	The ROI that shall be stored
 * @param msgData	The message data that shall be stored
 */
void Forward_only_valueChanges::SetCurrentDataValue(const RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	auto dataValAddr = msgData.addrVal;

	// Check if the new data value addressing is currently not present in internal hash
	// or if it differs in its value size and needs to be reinitialized
	if ((m_currentValues.count(Id) == 0) || (m_currentValues.at(Id).count(dataValAddr) == 0) || 
		(m_currentValues.at(Id).at(dataValAddr).payloadSize != msgData.payloadSize))
	{
		// If the data value exists, but has wrong size, reinitialize it
		if((m_currentValues.count(Id) != 0) && (m_currentValues.at(Id).count(dataValAddr) != 0) && 
			(m_currentValues.at(Id).at(dataValAddr).payloadSize != msgData.payloadSize))
		{
            switch(m_currentValues.at(Id).at(msgData.addrVal).valType)
            {
                case ROVT_INT:
                    delete static_cast<int*>(m_currentValues.at(Id).at(dataValAddr).payload);
                    break;
                case ROVT_FLOAT:
                    delete static_cast<float*>(m_currentValues.at(Id).at(dataValAddr).payload);
                    break;
                case ROVT_STRING:
                    delete static_cast<char*>(m_currentValues.at(Id).at(dataValAddr).payload);
                    break;
                default:
                    break;
            }
    
            m_currentValues.at(Id).at(dataValAddr).payload = nullptr;
            m_currentValues.at(Id).at(dataValAddr).payloadSize = 0;
            m_currentValues.at(Id).at(dataValAddr).valCount = 0;
		}
	
		RemoteObjectMessageData dataCopy = msgData;
	
		dataCopy.payload = new unsigned char[msgData.payloadSize];
		memcpy(dataCopy.payload, msgData.payload, msgData.payloadSize);
	
		m_currentValues[Id][msgData.addrVal] = dataCopy;
	}
	else
	{
		// do not copy entire data struct, since we need to keep our payload ptr
		m_currentValues.at(Id).at(dataValAddr).addrVal = msgData.addrVal;
		m_currentValues.at(Id).at(dataValAddr).valCount = msgData.valCount;
		m_currentValues.at(Id).at(dataValAddr).valType = msgData.valType;
		memcpy(m_currentValues.at(Id).at(dataValAddr).payload, msgData.payload, msgData.payloadSize);
	}
}

/**
 * Getter for the private precision value
 * @return	The internal precision value.
 */
float Forward_only_valueChanges::GetPrecision()
{
	return m_precision;
}

/**
 * Setter for the private precision value
 * @param precision	The precision value to set.
 */
void Forward_only_valueChanges::SetPrecision(float precision)
{
	m_precision = precision;
}