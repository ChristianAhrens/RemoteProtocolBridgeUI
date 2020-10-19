/*
==============================================================================

RTTrPM.cpp
Author:  adam.nagy

==============================================================================
*/

#include "RTTrPM.h"

/**
* Constructor of the class CRTTrP
* Initialize the member variables with null.
*/
CRTTrP::CRTTrP()
{
	this->m_intHeader = NULL;
	this->m_fltHeader = NULL;
	this->m_version = NULL;
	this->m_packetID = NULL;
	this->m_packetForm = NULL;
	this->m_packetSize = NULL;
	this->m_context = NULL;
	this->m_numMods = NULL;
}

/**
* Constructor of the class CRTTrP. It sorts all RTTrP header information and saves it within the member variables
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
CRTTrP::CRTTrP(vector<unsigned char> data, int &startPosToRead)
{
	CRTTrP();	// initialise the variables with null
	std::copy(data.begin(), data.begin() + 2, (unsigned char*)&this->m_intHeader);
	data.erase(data.begin(), data.begin() + 2);

	copy(data.begin(), data.begin() + 2, (unsigned char*)&this->m_fltHeader);
	data.erase(data.begin(), data.begin() + 2);

	if((m_intHeader == 0x4154) && (m_fltHeader == 0x4334))
	{
		copy(data.begin(), data.begin() + 2, (uint16_t*)&this->m_version);
		data.erase(data.begin(), data.begin() + 2);

		copy(data.begin(), data.begin() + 4, (uint32_t*)&this->m_packetID);
		data.erase(data.begin(), data.begin() + 4);

		copy(data.begin(), data.begin() + 1, (uint8_t*)&this->m_packetForm);
		data.erase(data.begin(), data.begin() + 1);

		copy(data.begin(), data.begin() + 2, (uint16_t*)&this->m_packetSize);
		data.erase(data.begin(), data.begin() + 2);

		copy(data.begin(), data.begin() + 4, (uint32_t*)&this->m_context);
		data.erase(data.begin(), data.begin() + 4);

		copy(data.begin(), data.begin() + 1, (uint8_t*)&this->m_numMods);
		data.erase(data.begin(), data.begin() + 1);

		startPosToRead = 18;
	}

	else
	{
		startPosToRead = 0;
	}
}

/**
* Destructor of the class CRTTrP
*/
CRTTrP::~CRTTrP()
{
}

/**
* Method that returns the number of trackable modules
*
* @return	m_numMods :	Number of trackable modules
*/
uint8_t CRTTrP::GetNumOfTrackableMods()
{ 
	return m_numMods; 
}

/**
* Constructor of the CPacketModuleTrackable class. It reads the number of sub-modules in the packet.
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
CPacketModuleTrackable::CPacketModuleTrackable(vector<unsigned char> data, int &startPosToRead) : CPacketModule(data, startPosToRead)
{
	if(GetModuleType() == 0x51)
	{
		copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (uint8_t *)&this->m_lengthOfname);
		data.erase(data.begin() + startPosToRead, data.begin() + startPosToRead+1);

		copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (unsigned char *)&this->m_name);
		data.erase(data.begin() + startPosToRead + 1, data.begin() + startPosToRead+1);

		copy(data.begin() + startPosToRead + 5, data.begin() + startPosToRead + 6, (int *)&this->m_numberOfSubModules);
		data.erase(data.begin() + startPosToRead + 5, data.begin() + startPosToRead + 6);
	
		startPosToRead = 28;	// So the read starts by the first byte (type) of the next sub-module
	}

	else if(GetModuleType() == 0x01)
	{
		copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (uint8_t *)&this->m_lengthOfname);
		data.erase(data.begin() + startPosToRead, data.begin() + startPosToRead + 1);

		copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (unsigned char *)&this->m_name);
		data.erase(data.begin() + startPosToRead + 1, data.begin() + startPosToRead + 1);

		copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (int *)&this->m_numberOfSubModules);
		data.erase(data.begin() + startPosToRead, data.begin() + startPosToRead + 1);
		startPosToRead = 22;
	}
}

/**
* Destructor of the class CPacketModuleTrackable
*/
CPacketModuleTrackable::~CPacketModuleTrackable()
{
}

/**
* Returns the number of sub-modules
*
* @return	m_numberOfSubModules :	Number of sub-modules
*/
int CPacketModuleTrackable::GetNumberOfSubModules()
{
	return m_numberOfSubModules;
}

/**
* Constructor of the CPacketModule class
*
*/
CPacketModule::CPacketModule()
{
}

/**
* Constructor of the CPacketModule class. It saves the type and size of the module.
*
* @param	data			: Keeps the caught data information.
* @param	&startPosToRead	: Reference variable which helps to know from which bytes the next modul read should beginn
*/
CPacketModule::CPacketModule(vector<unsigned char> data, int &startPosToRead)
{
	copy(data.begin() + startPosToRead, data.begin() + startPosToRead + 1, (uint8_t *)&this->m_moduleType);
	data.erase(data.begin() + startPosToRead, data.begin() + startPosToRead+1);

	m_moduleSize = *(data.begin() + startPosToRead);		// Reads the first byte

	// Adjust the startPosToRead if the current module type is trackable
	if(m_moduleType == 0x51 || m_moduleType == 0x01)
	{
		startPosToRead = 21;	// So the next read starts by the name (3th. byte of the module) of the trackable module
	}
}

/**
* Destructor of the CPacketModule class
*/
CPacketModule::~CPacketModule()
{
}

/**
* Returns the type of the module
*
*/
uint8_t CPacketModule::GetModuleType()
{
	return m_moduleType;
}

/**
* Returns the size of the module
*
*/
uint16_t CPacketModule::GetModuleSize()
{
	return m_moduleSize;
}

/**
* Constructor of the CCentroidMod class
* 
*/
CCentroidMod::CCentroidMod()
{
}

/**
* Constructor of the class CCentroidMod
* @param	data	: keeps the caught data information.
*/
CCentroidMod::CCentroidMod(vector<unsigned char> *data)
{
	SetClearAllVariables();		// initialise the variables with null

	data->erase(data->begin(), data->begin() + 2);
	copy(data->begin(), data->begin() + 8, (unsigned char *)&this->m_coordinateX);
	data->erase(data->begin(), data->begin() + 8);

	copy(data->begin(), data->begin() + 8, (unsigned char *)&this->m_coordinateY);
	data->erase(data->begin(), data->begin() + 8);

	copy(data->begin(), data->begin() + 8, (unsigned char *)&this->m_coordinateZ);
	data->erase(data->begin(), data->begin() + 8);
}

/**
* Destructor of the CCentroidMod class
*/
CCentroidMod::~CCentroidMod()
{
}

/**
* Clears all member variables, before calling them up in the CCentroidMod constructor
*/
void CCentroidMod::SetClearAllVariables()
{
	m_coordinateX = 0;
	m_coordinateY = 0;
	m_coordinateZ = 0;
}

/**
* Returns the X coordinate
*/
double CCentroidMod::GetXCoordinat()
{ 
	return m_coordinateX; 
}

/**
* Returns the Y coordinate
*/
double CCentroidMod::GetYCoordinat()
{
	return m_coordinateY; 
}

/**
* Returns the Z coordinate
*/
double CCentroidMod::GetZCoordinat()
{ 
	return m_coordinateZ; 
}