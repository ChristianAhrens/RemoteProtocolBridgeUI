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

using namespace std;

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
	CRTTrP(vector<unsigned char> data, int &startPosToRead);
	~CRTTrP();
	uint8_t GetNumOfTrackableMods();

private:
	uint16_t m_intHeader; // The RTTrP Header has two types, one with float and one with integer
	uint16_t m_fltHeader;
	uint16_t m_version;
	uint32_t m_packetID;
	uint8_t m_packetForm;
	uint16_t m_packetSize;
	uint32_t m_context;
	uint8_t m_numMods;
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
	CPacketModule();
	CPacketModule(vector<unsigned char> data, int &startPosToRead);
	~CPacketModule();
	uint8_t GetModuleType();
	uint16_t GetModuleSize();

protected:
	uint8_t m_moduleType;	//	Type of the packet module
	uint16_t m_moduleSize;	//	Size of the module
};

// **************************************************************
// class CPacketModuleTrackable 
// **************************************************************
/**
* A class to save the trackable packet module information
*/
class CPacketModuleTrackable : CPacketModule
{
public:
	CPacketModuleTrackable(vector<unsigned char> data, int &startPosToRead);
	~CPacketModuleTrackable();
	int GetNumberOfSubModules();

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
	CCentroidMod(vector<unsigned char> *data);
	~CCentroidMod();

	void SetClearAllVariables();
	double GetXCoordinat();
	double GetYCoordinat();
	double GetZCoordinat();

private:
	double m_coordinateX; 
	double m_coordinateY;
	double m_coordinateZ;
};