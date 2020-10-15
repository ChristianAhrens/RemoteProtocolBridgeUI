/*
==============================================================================

RTTrPMConnectionServer.h
Author:  adam.nagy

==============================================================================
*/

#pragma once

#include "MainComponent.h"
#include "RTTrPM.h"

using namespace std;

enum 
{
	packetModule_withTimestamp = 0x51,
	packetModule_withoutTimestamp = 0x1,
	packetModule_centroidPosition = 0x02,
	packetModule_trackedPointPosition = 0x06,
	packetModule_orientationQuaternion = 0x03,
	packetModule_orientationEuler = 0x04,
	packetModule_centroidAccelerationAndVelocity = 0x20,
	packetModule_trackedPointAccelerationandVelocity = 0x21
};

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
	vector<CCentroidMod*>GetCentroidPosCoordinates();

private:
	void run() override;
	int HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, vector<CCentroidMod*>& packetModules);
	vector<CCentroidMod*> m_modulesFromDatenpacket;
	ScopedPointer<DatagramSocket> socket;			// Socket from JUCE to connect to
	int m_packetSize;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRTTrPMConnectionServer)
};

