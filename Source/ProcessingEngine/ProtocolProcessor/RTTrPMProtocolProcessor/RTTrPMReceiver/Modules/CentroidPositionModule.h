/*
  ==============================================================================

    CentroidPositionModule.h
    Created: 23 Oct 2020 10:10:37am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class CentroidPositionModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - Centroid position module 
*
*/
class CentroidPositionModule : public PacketModule
{
public:
	CentroidPositionModule(std::vector<unsigned char>& data, int& readPos);
	~CentroidPositionModule() override;

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	uint16_t GetLatency() const;
	double GetX() const;
	double GetY() const;
	double GetZ() const;

	bool isValid() const override;

private:
	uint16_t	m_latency{ 0 };
	double		m_coordinateX{ 0 }; 
	double		m_coordinateY{ 0 };
	double		m_coordinateZ{ 0 };
};