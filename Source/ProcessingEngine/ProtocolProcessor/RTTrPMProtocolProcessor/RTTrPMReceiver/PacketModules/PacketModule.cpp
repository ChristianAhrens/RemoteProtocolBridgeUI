/*
  ==============================================================================

    PacketModule.cpp
    Created: 23 Oct 2020 10:07:56am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "PacketModule.h"


/**
* Constructor of the PacketModule class
*/
PacketModule::PacketModule()
{
}

/**
* Constructor of the PacketModule class.
* @param	data	Input byte data as vector reference.
* @param	readPos	Reference variable which helps to know from which bytes the next modul read should beginn
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
* @return	The module type
*/
PacketModule::PacketModuleType PacketModule::GetModuleType() const
{
	return m_moduleType;
}

/**
* Returns the size of the module
* @return The module size in bytes
*/
uint16_t PacketModule::GetModuleSize() const
{
	return m_moduleSize;
}