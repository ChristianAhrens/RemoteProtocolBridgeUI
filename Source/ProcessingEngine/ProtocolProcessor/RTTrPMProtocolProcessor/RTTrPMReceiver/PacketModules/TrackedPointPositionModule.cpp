/*
  ==============================================================================

    TrackedPointPositionModule.cpp
    Created: 23 Oct 2020 10:11:07am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "TrackedPointPositionModule.h"


/**
* Constructor of the class TrackedPointPositionModule
* @param	data	Input byte data as vector reference.
* @param	readPos	Reference variable which helps to know from which bytes the next modul read should beginn
*/
TrackedPointPositionModule::TrackedPointPositionModule(std::vector<unsigned char>& data, int& readPos)
	: PacketModule(data, readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void TrackedPointPositionModule::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	std::copy(readIter, readIter + 2, (unsigned char*)&m_latency);
	readIter += 2;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateX);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateY);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateZ);
	readIter += 8;

	std::copy(readIter, readIter + 1, (unsigned char*)&m_index);
	readIter += 1;

	readPos += (2 + (3 * 8) + 1);
}

/**
* Destructor of the TrackedPointPositionModule class
*/
TrackedPointPositionModule::~TrackedPointPositionModule()
{
}

/**
 * Reimplemented helper method to check validity of the packet module based on size greater zero and correct type.
 * @return True if valid, false if not
 */
bool TrackedPointPositionModule::isValid() const
{
	return (PacketModule::isValid() && (GetModuleType() == TrackedPointPosition));
}

/**
* Returns the latency.
* Approximate time in milliseconds since last measurement, if equal to 0xFFFF, overflow
* @return The latency value
*/
uint16_t TrackedPointPositionModule::GetLatency() const
{
	return m_latency;
}

/**
* Returns the X coordinate
* @return The x coordinate value
*/
double TrackedPointPositionModule::GetX() const
{
	return m_coordinateX;
}

/**
* Returns the Y coordinate
* @return The y coordinate value
*/
double TrackedPointPositionModule::GetY() const
{
	return m_coordinateY;
}

/**
* Returns the Z coordinate
* @return The z coordinate value
*/
double TrackedPointPositionModule::GetZ() const
{
	return m_coordinateZ;
}

/**
* Returns the point index
* @return The point index
*/
uint8_t TrackedPointPositionModule::GetPointIndex() const
{
	return m_index;
}