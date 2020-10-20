/*
==============================================================================

RTTrPM.h
Author:  adam.nagy

==============================================================================
*/
#include <iostream>
#include <cstdint>
#include <vector>
#include <winsock2.h>

#pragma once


// **************************************************************
// class CRTTrP 
// **************************************************************
/**
* A class to sort the header information from RTTrP
* 
*/
class CRTTrP
{
public:
	CRTTrP();
	CRTTrP(std::vector<unsigned char> data, int &startPosToRead);
	~CRTTrP();
	uint8_t GetNumOfTrackableMods();

private:
	uint16_t m_intHeader{ 0 }; // The RTTrP Header has two types, one with float and one with integer
	uint16_t m_fltHeader{ 0 };
	uint16_t m_version{ 0 };
	uint32_t m_packetID{ 0 };
	uint8_t m_packetForm{ 0 };
	uint16_t m_packetSize{ 0 };
	uint32_t m_context{ 0 };
	uint8_t m_numMods{ 0 };
};


// **************************************************************
// class CPacketModule 
// **************************************************************
/**
* A class to save the basic packet module information
*/
class CPacketModule
{
public:
	typedef uint8_t	PacketModuleType;
	static constexpr PacketModuleType PMT_invalid								= 0x00;
	static constexpr PacketModuleType PMT_withTimestamp							= 0x51;
	static constexpr PacketModuleType PMT_withoutTimestamp						= 0x01;
	static constexpr PacketModuleType PMT_centroidPosition						= 0x02;
	static constexpr PacketModuleType PMT_trackedPointPosition					= 0x06;
	static constexpr PacketModuleType PMT_orientationQuaternion					= 0x03;
	static constexpr PacketModuleType PMT_orientationEuler						= 0x04;
	static constexpr PacketModuleType PMT_centroidAccelerationAndVelocity		= 0x20;
	static constexpr PacketModuleType PMT_trackedPointAccelerationandVelocity	= 0x21;

public:
	CPacketModule();
	CPacketModule(std::vector<unsigned char> data, int &startPosToRead);
	~CPacketModule();

	PacketModuleType GetModuleType() const;
	uint16_t GetModuleSize() const;

	virtual bool isValid() const;

private:
	PacketModuleType	m_moduleType{ PMT_invalid };	//	Type of the packet module
	uint16_t			m_moduleSize{ 0 };	//	Size of the module
};

// **************************************************************
// class CPacketModuleTrackable 
// **************************************************************
/**
* A class to save the trackable packet module information
*/
class CPacketModuleTrackable : public CPacketModule
{
public:
	CPacketModuleTrackable(std::vector<unsigned char> data, int &startPosToRead);
	~CPacketModuleTrackable();

	int GetNumberOfSubModules() const;

	bool isValid() const override;

private:
	uint8_t m_lengthOfname;
	char m_name;
	int m_numberOfSubModules;
};

// **************************************************************
// class CCentroidMod
// **************************************************************
/**
* A class to sort the information from RTTrPM - Centroid module 
*
*/
class CCentroidMod : public CPacketModule
{
public:
	CCentroidMod();
	CCentroidMod(std::vector<unsigned char> *data);
	~CCentroidMod();

	void SetClearAllVariables();
	double GetXCoordinate() const;
	double GetYCoordinate() const;
	double GetZCoordinate() const;

	bool isValid() const override;

private:
	double m_coordinateX; 
	double m_coordinateY;
	double m_coordinateZ;
};