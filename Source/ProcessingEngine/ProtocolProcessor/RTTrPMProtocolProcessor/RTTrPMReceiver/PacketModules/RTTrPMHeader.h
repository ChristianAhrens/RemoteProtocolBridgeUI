/*
  ==============================================================================

    RTTrPMHeader.h
    Created: 23 Oct 2020 10:08:34am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <vector>


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
	RTTrPMHeader();
	RTTrPMHeader(std::vector<unsigned char>& data, int& readPos);
	~RTTrPMHeader();

	void readData(std::vector<unsigned char>& data, int& readPos);

	PacketModuleSignature GetIntSignature() const;
	PacketModuleSignature GetFloatSignature() const;
	uint16_t GetVersion() const;
	uint32_t GetPacketID() const;
	PacketModuleFormat GetPacketFormat() const;
	uint16_t GetPacketSize() const;
	uint32_t GetContext() const;
	uint8_t GetNumberOfModules() const;

private:
	PacketModuleSignature m_intSignature{ 0 };
	PacketModuleSignature m_floatSignature{ 0 };
	uint16_t m_version{ 0 };
	uint32_t m_packetID{ 0 };
	PacketModuleFormat m_packetFormat{ 0 };
	uint16_t m_packetSize{ 0 };
	uint32_t m_context{ 0 };
	uint8_t m_numModules{ 0 };
};