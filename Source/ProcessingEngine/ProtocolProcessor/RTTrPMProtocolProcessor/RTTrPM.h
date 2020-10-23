/*
==============================================================================

RTTrPM.h
Author:  adam.nagy

==============================================================================
*/
#include <iostream>
#include <cstdint>
#include <vector>

#pragma once


// **************************************************************
// class RTTrPMHeader
// **************************************************************
/**
* A class to read a databuffer containing RTTrPM packet modules
* 
*/
class RTTrPMHeader
{
public:
	static constexpr uint16_t PacketModuleHeaderVersion = 0x0002;

	typedef uint16_t PacketModuleSignature;
	static constexpr PacketModuleSignature BigEndianInt			= 0x4154;
	static constexpr PacketModuleSignature LittleEndianInt		= 0x5441;
	static constexpr PacketModuleSignature BigEndianFloat		= 0x4334;
	static constexpr PacketModuleSignature LittleEndianFloat	= 0x3443;

	typedef uint8_t PacketModuleFormat;
	static constexpr PacketModuleFormat Raw			= 0x00;
	static constexpr PacketModuleFormat Protobuf	= 0x01;
	static constexpr PacketModuleFormat Thrift		= 0x02;

public:
	RTTrPMHeader(std::vector<unsigned char>& data, int& readPos);
	~RTTrPMHeader();

	void readData(std::vector<unsigned char>& data, int& readPos);

	uint8_t GetNumOfTrackableMods();

private:
	PacketModuleSignature m_intSignature{ 0 };
	PacketModuleSignature m_floatSignature{ 0 };
	uint16_t m_version{ 0 };
	uint32_t m_packetID{ 0 };
	PacketModuleFormat m_packetFormat{ 0 };
	uint16_t m_packetSize{ 0 };
	uint32_t m_context{ 0 };
	uint8_t m_numMods{ 0 };
};


// **************************************************************
// class PacketModule 
// **************************************************************
/**
* A class to save the basic packet module information
*/
class PacketModule
{
public:
	typedef uint8_t	PacketModuleType;
	static constexpr PacketModuleType Invalid								= 0x00;
	static constexpr PacketModuleType WithTimestamp							= 0x51;
	static constexpr PacketModuleType WithoutTimestamp						= 0x01;
	static constexpr PacketModuleType CentroidPosition						= 0x02;
	static constexpr PacketModuleType TrackedPointPosition					= 0x06;
	static constexpr PacketModuleType OrientationQuaternion					= 0x03;
	static constexpr PacketModuleType OrientationEuler						= 0x04;
	static constexpr PacketModuleType CentroidAccelerationAndVelocity		= 0x20;
	static constexpr PacketModuleType TrackedPointAccelerationandVelocity	= 0x21;

public:
	PacketModule();
	PacketModule(std::vector<unsigned char>& data, int & readPos);
	~PacketModule();

	virtual void readData(std::vector<unsigned char>& data, int& readPos);

	PacketModuleType GetModuleType() const;
	uint16_t GetModuleSize() const;

	virtual bool isValid() const;

private:
	PacketModuleType	m_moduleType{ Invalid };	//	Type of the packet module
	uint16_t			m_moduleSize{ 0 };	//	Size of the module
};


// **************************************************************
// class PacketModuleTrackable 
// **************************************************************
/**
* A class to save the trackable packet module information
*/
class PacketModuleTrackable : public PacketModule
{
public:
	PacketModuleTrackable(std::vector<unsigned char>& data, int & readPos);
	~PacketModuleTrackable();

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	std::string GetName() const;
	uint32_t GetSeqNumber() const;
	uint8_t GetNumberOfSubModules() const;

	bool isValid() const override;

private:
	uint8_t		m_lengthOfname{ 0 };
	std::string	m_name;
	uint32_t	m_seqNumber{ 0 };
	uint8_t		m_numberOfSubModules{ 0 };
};


// **************************************************************
// class CentroidModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - Centroid module 
*
*/
class CentroidModule : public PacketModule
{
public:
	CentroidModule(std::vector<unsigned char>& data, int& readPos);
	~CentroidModule();

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
