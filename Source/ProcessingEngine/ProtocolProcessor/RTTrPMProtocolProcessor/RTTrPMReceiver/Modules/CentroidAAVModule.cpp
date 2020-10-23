/*
  ==============================================================================

    CentroidAccelAndVeloModule.cpp
    Created: 23 Oct 2020 10:12:37am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "CentroidAAVModule.h"


/**
* Constructor of the class CentroidAccelAndVeloModule
* @param	data	Input byte data as vector reference.
* @param	readPos	Reference variable which helps to know from which bytes the next modul read should beginn
*/
CentroidAccelAndVeloModule::CentroidAccelAndVeloModule(std::vector<unsigned char>& data, int& readPos)
	: PacketModule(data, readPos)
{
	readData(data, readPos);
}

/**
 * Helper method to parse the input data vector starting at given read position.
 * @param data	The input data in a byte vector.
 * @param readPos	The position in the given vector where reading shall be started.
 */
void CentroidAccelAndVeloModule::readData(std::vector<unsigned char>& data, int& readPos)
{
	auto readIter = data.begin() + readPos;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateX);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateY);
	readIter += 8;

	std::copy(readIter, readIter + 8, (unsigned char*)&m_coordinateZ);
	readIter += 8;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_accelerationX);
	readIter += 4;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_accelerationY);
	readIter += 4;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_accelerationZ);
	readIter += 4;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_velocityX);
	readIter += 4;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_velocityY);
	readIter += 4;

	std::copy(readIter, readIter + 4, (unsigned char*)&m_velocityZ);
	readIter += 4;

	readPos += ((3 * 8) + (6 * 4));
}

/**
* Destructor of the CentroidAccelAndVeloModule class
*/
CentroidAccelAndVeloModule::~CentroidAccelAndVeloModule()
{
}

/**
 * Reimplemented helper method to check validity of the packet module based on size greater zero and correct type.
 * @return True if valid, false if not
 */
bool CentroidAccelAndVeloModule::isValid() const
{
	return (PacketModule::isValid() && (GetModuleType() == CentroidAccelerationAndVelocity));
}

/**
* Returns the position of the X coordinate
* @return The x coordinate value
*/
double CentroidAccelAndVeloModule::GetXCoordinate() const
{
	return m_coordinateX;
}

/**
* Returns the position of the Y coordinate
* @return The y coordinate value
*/
double CentroidAccelAndVeloModule::GetYCoordinate() const
{
	return m_coordinateY;
}

/**
* Returns the position of the Z coordinate
* @return The z coordinate value
*/
double CentroidAccelAndVeloModule::GetZCoordinate() const
{
	return m_coordinateZ;
}

/**
* Returns the calculated centroid acceleration in the X direction
* @return The x acceleration value
*/
float CentroidAccelAndVeloModule::GetXAcceleration() const
{
	return m_accelerationX;
}

/**
* Returns the calculated centroid acceleration in the Y direction
* @return The y acceleration value
*/
float CentroidAccelAndVeloModule::GetYAcceleration() const
{
	return m_accelerationY;
}

/**
* Returns the calculated centroid acceleration in the Z direction
* @return The z acceleration value
*/
float CentroidAccelAndVeloModule::GetZAcceleration() const
{
	return m_accelerationZ;
}

/**
* Returns the calculated centroid velocity in the X direction
* @return The x velocity value
*/
float CentroidAccelAndVeloModule::GetXVelocity() const
{
	return m_velocityX;
}

/**
* Returns the calculated centroid velocity in the Y direction
* @return The y velocity value
*/
float CentroidAccelAndVeloModule::GetYVelocity() const
{
	return m_velocityY;
}

/**
* Returns the calculated centroid velocity in the Z direction
* @return The z velocity value
*/
float CentroidAccelAndVeloModule::GetZVelocity() const
{
	return m_velocityZ;
}