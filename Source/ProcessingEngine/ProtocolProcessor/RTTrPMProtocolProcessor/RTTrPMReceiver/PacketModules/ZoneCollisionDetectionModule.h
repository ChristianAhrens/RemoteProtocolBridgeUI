/*
  ==============================================================================

    ZoneCollisionDetectionModule.h
    Created: 23 Oct 2020 10:10:37am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>
#include <string>
#include <memory>


// **************************************************************
// class ZoneObjectSubModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - ZoneObject sub-module
*
*/
class ZoneObjectSubModule : public PacketModule
{
public:
	ZoneObjectSubModule(std::vector<unsigned char>& data, int& readPos);
	~ZoneObjectSubModule();

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	uint8_t GetSize() const;
	std::string GetName() const;

	bool isValid() const override;

private:
	uint8_t		m_size{ 0 };
	uint8_t		m_nameLength{ 0 };
	std::string m_name;
};


// **************************************************************
// class ZoneCollisionDetectionModule
// **************************************************************
/**
* A class to sort the information from RTTrPM - Zone collision detection module
*
*/
class ZoneCollisionDetectionModule : public PacketModule
{
public:
	ZoneCollisionDetectionModule(std::vector<unsigned char>& data, int& readPos);
	~ZoneCollisionDetectionModule();

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	const std::vector<std::unique_ptr<ZoneObjectSubModule>>& GetZoneSubModules() const;

	bool isValid() const override;

private:
	uint8_t		m_numberOfZoneSubModules{ 0 };
	std::vector<std::unique_ptr<ZoneObjectSubModule>>	m_zoneObjectSubModules;
};