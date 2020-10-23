/*
  ==============================================================================

    CentroidAccelerationAndVelocityModule.h
    Created: 23 Oct 2020 10:12:37am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class CentroidAccelerationAndVelocityModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - centroid with acceleration and velocity module
*
*/
class CentroidAccelerationAndVelocityModule : public PacketModule
{
public:
	CentroidAccelerationAndVelocityModule(std::vector<unsigned char>& data, int& readPos);
	~CentroidAccelerationAndVelocityModule();

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	double GetXCoordinate() const;
	double GetYCoordinate() const;
	double GetZCoordinate() const;
	float GetXAcceleration() const;
	float GetYAcceleration() const;
	float GetZAcceleration() const;
	float GetXVelocity() const;
	float GetYVelocity() const;
	float GetZVelocity() const;

	bool isValid() const override;

private:
	double		m_coordinateX{ 0 };
	double		m_coordinateY{ 0 };
	double		m_coordinateZ{ 0 };
	float		m_accelerationX{ 0 };
	float		m_accelerationY{ 0 };
	float		m_accelerationZ{ 0 };
	float		m_velocityX{ 0 };
	float		m_velocityY{ 0 };
	float		m_velocityZ{ 0 };
};