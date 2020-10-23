/*
  ==============================================================================

    TrackedPointAccelAndVeloModule.h
    Created: 23 Oct 2020 10:12:50am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class TrackedPointAccelAndVeloModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - tracked point with acceleration and velocity module
*
*/
class TrackedPointAccelAndVeloModule : public PacketModule
{
public:
	TrackedPointAccelAndVeloModule(std::vector<unsigned char>& data, int& readPos);
	~TrackedPointAccelAndVeloModule();

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
	uint8_t GetPointIndex() const;

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
	uint8_t		m_index{ 0 };
};
