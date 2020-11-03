/*
  ==============================================================================

    TrackedPointModule.h
    Created: 23 Oct 2020 10:11:07am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class TrackedPointPositionModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - tracked point module
*
*/
class TrackedPointPositionModule : public PacketModule
{
public:
	TrackedPointPositionModule(std::vector<unsigned char>& data, int& readPos);
	~TrackedPointPositionModule() override;

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	uint16_t GetLatency() const;
	double GetX() const;
	double GetY() const;
	double GetZ() const;
	uint8_t GetPointIndex() const;

	bool isValid() const override;

private:
	uint16_t	m_latency{ 0 };
	double		m_coordinateX{ 0 };
	double		m_coordinateY{ 0 };
	double		m_coordinateZ{ 0 };
	uint8_t		m_index{ 0 };
};
