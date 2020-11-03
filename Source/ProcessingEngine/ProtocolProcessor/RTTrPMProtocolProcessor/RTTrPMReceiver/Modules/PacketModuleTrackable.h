/*
  ==============================================================================

    PacketModuleTrackable.h
    Created: 23 Oct 2020 10:08:13am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PacketModule.h"

#include <vector>
#include <string>

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
	~PacketModuleTrackable() override;

	void readData(std::vector<unsigned char>& data, int& readPos) override;

	std::string GetName() const;
	uint32_t GetSeqNumber() const;
	uint8_t GetNumberOfSubModules() const;

	bool isValid() const override;

private:
	uint8_t		m_nameLength{ 0 };
	std::string	m_name;
	uint32_t	m_seqNumber{ 0 };
	uint8_t		m_numberOfSubModules{ 0 };
};