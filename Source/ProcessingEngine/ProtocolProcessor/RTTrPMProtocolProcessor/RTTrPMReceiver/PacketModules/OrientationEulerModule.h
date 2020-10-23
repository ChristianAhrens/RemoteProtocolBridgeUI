/*
  ==============================================================================

    OrientationEulerModule.h
    Created: 23 Oct 2020 10:12:16am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>


// **************************************************************
// class OrientationEulerModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - euler orientation module
*
*/
class OrientationEulerModule : public PacketModule
{
public:
	typedef uint16_t EulerOrder;
	static constexpr EulerOrder X1X2X3 = 0x0111;
	static constexpr EulerOrder X1X2Y3 = 0x0112;
	static constexpr EulerOrder X1X2Z3 = 0x0113;
	static constexpr EulerOrder X1Y2X3 = 0x0121;
	static constexpr EulerOrder X1Y2Y3 = 0x0122;
	static constexpr EulerOrder X1Y2Z3 = 0x0123;
	static constexpr EulerOrder X1Z2X3 = 0x0131;
	static constexpr EulerOrder X1Z2Y3 = 0x0132;
	static constexpr EulerOrder X1Z2Z3 = 0x0133;
	static constexpr EulerOrder Y1X2X3 = 0x0211;
	static constexpr EulerOrder Y1X2Y3 = 0x0212;
	static constexpr EulerOrder Y1X2Z3 = 0x0213;
	static constexpr EulerOrder Y1Y2X3 = 0x0221;
	static constexpr EulerOrder Y1Y2Y3 = 0x0222;
	static constexpr EulerOrder Y1Y2Z3 = 0x0223;
	static constexpr EulerOrder Y1Z2X3 = 0x0231;
	static constexpr EulerOrder Y1Z2Y3 = 0x0232;
	static constexpr EulerOrder Y1Z2Z3 = 0x0233;
	static constexpr EulerOrder Z1X2X3 = 0x0311;
	static constexpr EulerOrder Z1X2Y3 = 0x0312;
	static constexpr EulerOrder Z1X2Z3 = 0x0313;
	static constexpr EulerOrder Z1Y2X3 = 0x0321;
	static constexpr EulerOrder Z1Y2Y3 = 0x0322;
	static constexpr EulerOrder Z1Y2Z3 = 0x0323;
	static constexpr EulerOrder Z1Z2X3 = 0x0331;
	static constexpr EulerOrder Z1Z2Y3 = 0x0332;
	static constexpr EulerOrder Z1Z2Z3 = 0x0333;

public:
	OrientationEulerModule(std::vector<unsigned char>& data, int& readPos);
	~OrientationEulerModule();

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	uint16_t GetLatency() const;
	EulerOrder GetOrder() const;
	double GetR1() const;
	double GetR2() const;
	double GetR3() const;

	bool isValid() const override;

private:
	uint16_t	m_latency{ 0 };
	EulerOrder	m_order{ 0 };
	double		m_R1{ 0 };
	double		m_R2{ 0 };
	double		m_R3{ 0 };
};
