/*
==============================================================================

RTTrPMConnectionServer.h
Author:  adam.nagy

==============================================================================
*/

#pragma once

#include "RTTrPM.h"

#include <JuceHeader.h>


// **************************************************************
// class CRTTrPMConnectionServer
// **************************************************************
/**
*
* Creates an uninitialised server object, that will wait
* for client sockets to connect to a port on this host.
*/
class CRTTrPMConnectionServer : public Thread
{
public:
	CRTTrPMConnectionServer();
	~CRTTrPMConnectionServer();
	bool BeginWaitingForSocket(const int portNumber, const String &bindAddress);
	void stop();
	std::vector<CCentroidMod*>GetCentroidPosCoordinates();

private:
	void run() override;
	int HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, std::vector<CCentroidMod*>& packetModules);
	std::vector<CCentroidMod*> m_modulesFromDatenpacket;
	ScopedPointer<DatagramSocket> socket;			// Socket from JUCE to connect to
	int m_packetSize;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRTTrPMConnectionServer)
};

