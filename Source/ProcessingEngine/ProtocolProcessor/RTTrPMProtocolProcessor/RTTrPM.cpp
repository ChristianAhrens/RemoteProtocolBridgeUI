/*
==============================================================================

RTTrPM.cpp
Author:  adam.nagy

==============================================================================
*/

#include "RTTrPM.h"

/**
* Constructor of the class RTTrPMHeader. It sorts all RTTrP header information and saves it within the member variables
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
RTTrPMHeader::RTTrPMHeader(std::vector<unsigned char>& data, int& readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void RTTrPMHeader::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	std::copy(readIter, readIter + 2, (unsigned char*)&m_intSignature);
	readIter += 2;
	std::copy(readIter, readIter + 2, (unsigned char*)&m_floatSignature);
	readIter += 2;

	readPos += 4;

	if((m_intSignature == BigEndianInt) && (m_floatSignature == BigEndianFloat))
	{
		std::copy(readIter, readIter + 2, (uint16_t*)&m_version);
		readIter += 2;
		std::copy(readIter, readIter + 4, (uint32_t*)&m_packetID);	   
		readIter += 4;
		std::copy(readIter, readIter + 1, (uint8_t*)&m_packetFormat);   
		readIter += 1;
		std::copy(readIter, readIter + 2, (uint16_t*)&m_packetSize);  
		readIter += 2;
		std::copy(readIter, readIter + 4, (uint32_t*)&m_context);	   
		readIter += 4;
		std::copy(readIter, readIter + 1, (uint8_t*)&m_numMods);	   
		readIter += 1;

		readPos += 14;
	}
}

/**
* Destructor of the class RTTrPMHeader
*/
RTTrPMHeader::~RTTrPMHeader()
{
}

/**
* Method that returns the number of trackable modules
*
* @return	m_numMods :	Number of trackable modules
*/
uint8_t RTTrPMHeader::GetNumOfTrackableMods()
{ 
	return m_numMods; 
}


/**
* Constructor of the PacketModule class
*/
PacketModule::PacketModule()
{
}

/**
* Constructor of the PacketModule class. It saves the type and size of the module.
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
PacketModule::PacketModule(std::vector<unsigned char>& data, int& readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void PacketModule::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	std::copy(readIter, readIter + 1, (uint8_t*)&m_moduleType);
	readIter += 1;
	std::copy(readIter, readIter + 2, (uint16_t*)&m_moduleSize);
	readIter += 2;

	readPos += 3;
}

/**
* Destructor of the PacketModule class
*/
PacketModule::~PacketModule()
{
}

/**
 * Helper method to check validity of the packet module based on size greater zero and correct type.
 */
bool PacketModule::isValid() const
{
	return ((m_moduleSize > 0) && (m_moduleType != Invalid));
}

/**
* Returns the type of the module
*
*/
PacketModule::PacketModuleType PacketModule::GetModuleType() const
{
	return m_moduleType;
}

/**
* Returns the size of the module
*
*/
uint16_t PacketModule::GetModuleSize() const
{
	return m_moduleSize;
}


/**
* Constructor of the PacketModuleTrackable class. It reads the number of sub-modules in the packet.
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
PacketModuleTrackable::PacketModuleTrackable(std::vector<unsigned char>& data, int& readPos)
	: PacketModule(data, readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void PacketModuleTrackable::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	if(GetModuleType() == WithTimestamp)
	{
		std::copy(readIter, readIter + 1, (uint8_t *)&m_lengthOfname);
		readIter += 1;

		std::unique_ptr<char> nameBuf = std::make_unique<char>(m_lengthOfname);
		std::copy(readIter, readIter + m_lengthOfname, nameBuf.get());
		m_name = std::string(nameBuf.get(), m_lengthOfname);
		readIter += m_lengthOfname;

		std::copy(readIter, readIter + 4, (uint32_t*)&m_seqNumber);
		readIter += 4;

		std::copy(readIter, readIter + 1, (int *)&m_numberOfSubModules);
		readIter += 1;

		readPos += (1 + m_lengthOfname + 4 + 1);
	}

	else if(GetModuleType() == WithoutTimestamp)
	{
		std::copy(readIter, readIter + 1, (uint8_t*)&m_lengthOfname);
		readIter += 1;

		std::copy(readIter, readIter + m_lengthOfname, (unsigned char*)&m_name);
		readIter += m_lengthOfname;

		std::copy(readIter, readIter + 1, (int*)&m_numberOfSubModules);
		readIter += 1;
		
		readPos += (1 + m_lengthOfname + 1);
	}
}

/**
* Destructor of the class PacketModuleTrackable
*/
PacketModuleTrackable::~PacketModuleTrackable()
{
}

/**
 * Getter for the packet module name
 * @return The name string
 */
std::string PacketModuleTrackable::GetName() const
{
	return m_name;
}

/**
 * Getter for the packet module sequence number
 * @return The sequence number
 */
uint32_t PacketModuleTrackable::GetSeqNumber() const
{
	return m_seqNumber;
}

/**
* Returns the number of sub-modules
*
* @return	m_numberOfSubModules :	Number of sub-modules
*/
uint8_t PacketModuleTrackable::GetNumberOfSubModules() const
{
	return m_numberOfSubModules;
}

/**
 * Reimplemented helper method to check validity of the packet module based on size greater zero and correct type.
 */
bool PacketModuleTrackable::isValid() const
{
	return ((GetModuleSize() > 0) && ((GetModuleType() == WithTimestamp) || (GetModuleType() == WithoutTimestamp)));
}


/**
* Constructor of the class CentroidModule
* @param	data	: keeps the caught data information.
*/
CentroidModule::CentroidModule(std::vector<unsigned char>& data, int& readPos)
	: PacketModule(data, readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void CentroidModule::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	std::copy(readIter, readIter + 2, (unsigned char*)&m_latency);
	readIter += 2;

	std::copy(readIter, readIter + 8, (unsigned char *)&m_coordinateX);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char *)&m_coordinateY);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char *)&m_coordinateZ);
	readIter += 8;

	readPos += (2 + 8 + 8 + 8);
}

/**
* Destructor of the CentroidModule class
*/
CentroidModule::~CentroidModule()
{
}

/**
 * Reimplemented helper method to check validity of the packet module based on size greater zero and correct type.
 */
bool CentroidModule::isValid() const
{
	return ((GetModuleSize() > 0) && (GetModuleType() == CentroidPosition));
}

/**
* Returns the latency.
* Approximate time in milliseconds since last measurement, if equal to 0xFFFF, overflow
*/
uint16_t CentroidModule::GetLatency() const
{
	return m_latency;
}

/**
* Returns the X coordinate
*/
double CentroidModule::GetX() const
{ 
	return m_coordinateX; 
}

/**
* Returns the Y coordinate
*/
double CentroidModule::GetY() const
{
	return m_coordinateY; 
}

/**
* Returns the Z coordinate
*/
double CentroidModule::GetZ() const
{ 
	return m_coordinateZ; 
}