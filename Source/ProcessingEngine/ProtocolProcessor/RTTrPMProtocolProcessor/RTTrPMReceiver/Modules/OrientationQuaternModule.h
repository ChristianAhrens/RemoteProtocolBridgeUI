/*
  ==============================================================================

    OrientationQuaternionModule.h
    Created: 23 Oct 2020 10:11:36am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class OrientationQuaternionModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - quaternion orientation module
*
*/
class OrientationQuaternionModule : public PacketModule
{
public:
	OrientationQuaternionModule(std::vector<unsigned char>& data, int& readPos);
	~OrientationQuaternionModule() override;

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	uint16_t GetLatency() const;
	double GetQx() const;
	double GetQy() const;
	double GetQz() const;
	double GetQw() const;

	bool isValid() const override;

private:
	uint16_t	m_latency{ 0 };
	double		m_Qx{ 0 };
	double		m_Qy{ 0 };
	double		m_Qz{ 0 };
	double		m_Qw{ 0 };
};
