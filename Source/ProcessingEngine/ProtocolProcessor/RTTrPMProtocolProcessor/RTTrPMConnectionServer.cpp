/*
==============================================================================

RTTrPMConnectionServer.cpp
Author:  adam.nagy

==============================================================================
*/

#include "RTTrPMConnectionServer.h"

/**
* Constructor of the CRTTrPMConnectionServer class
*/
CRTTrPMConnectionServer::CRTTrPMConnectionServer() : Thread("RTTrPM_Connection_Server")
{
	
}

/**
* Destructor of the CRTTrPMConnectionServer class
*/
CRTTrPMConnectionServer::~CRTTrPMConnectionServer()
{
}


/**
* Reads all packet modules and sorts them within the CCentroidMod class, saves all modules into packetModules vector
* @param	dataBuffer		: An array which keeps the caught data information.
* @param	bytesRead		: Keeps the number of read bytes
* @param	packetModules	: Keeps the read modules from the packet
*
* @return	packetModules	: Returns the size of the packet modules
*/
int CRTTrPMConnectionServer::HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, vector<CCentroidMod*>& packetModules)
{
	vector<unsigned char> data(dataBuffer, dataBuffer + bytesRead);			// data: Vector that has all the caught data information
	int startPosToRead = 0;													// Counter variable to know from which byte the next module has to be read
	CRTTrP cRTTrPObject = CRTTrP(data, startPosToRead);						// Sort all RTTrP header information

	if(startPosToRead == 0)	//	If true, Signature from RTTrP header was not right. Process will be returned to the run methode
	{
		return 0;
	}

	for(int i = 0; i < cRTTrPObject.GetNumOfTrackableMods(); i++)			// As many trackable modules as the packet has
	{
		CPacketModuleTrackable trackableModule(data, startPosToRead);		// Reads the name, name length and number of sub-modules

		for(int i = 0; i < trackableModule.GetNumberOfSubModules(); i++)	// As many sub-modules as the packet has
		{
			CPacketModule packetModuleToRead(data, startPosToRead);			// Reads the module type and size

			switch(packetModuleToRead.GetModuleType())						// Decides between the different type of sub-modules
			{
			case packetModule_centroidPosition:
				{
					vector<unsigned char> centroidmoddata(data.begin() + 31, data.end());	// centroidmoddata :  Vector that receives the position of the coordinates.
					CCentroidMod *mod = new CCentroidMod(&centroidmoddata);
					packetModules.push_back(mod);						
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
			
				case packetModule_trackedPointPosition:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case packetModule_orientationQuaternion:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case packetModule_orientationEuler:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case packetModule_centroidAccelerationAndVelocity:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case packetModule_trackedPointAccelerationandVelocity:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
			}
		}
	}
	return packetModules.size();
}

/**
* It waits until a client sends any message to the host.
* If a message arrives it will call up the HandleBuffer method which will sort the module information.
*/
void CRTTrPMConnectionServer::run()
{
	int maxBytesToRead = 512;		// Variable for the maximal size of data -> notice: if its too small data can't be read!
	unsigned char dataBuffer[512];	// An array which keeps the caught data information.

	while(!threadShouldExit())
	{
		int readyReturnValue = socket->waitUntilReady(true, 100);	//	waitUntilReady() -> true: It will wait until the socket is ready for reading, 100: give up time in ms.

		if(readyReturnValue == 0)
		{
			wait(1000);
			continue;
		}

		else if(readyReturnValue == 1)
		{
			//	bytesRead returns the number of read bytes, or -1 if there was an error.
			int bytesRead = socket->read(dataBuffer, maxBytesToRead, false);	//	read() -> false: the method will return as much data as is currently available without blocking.

			if(bytesRead == -1)
			{
				DBG("Read error");
			}
			else if(bytesRead == 0)
			{
				DBG("No data to read ");
			}
			else
			{
				m_packetSize = HandleBuffer(dataBuffer, bytesRead, m_modulesFromDatenpacket);
				if(m_packetSize == 0)
				{
					jassert(m_packetSize == 0);
					continue;
				}
			}
		}
		else
		{
			DBG("Read error");
		}
	}
}

/**
* Method for binding the socket to the specified local port and local address.
*
* @param	portNumber		: The port on which the server will receive connections
* @param	bindAddress		: The address on which the server will listen for connections.
*							  An empty string indicates that it should listen on all addresses assigned to this machine.
* @return	bool			: if true, thread is running. If false, it clears the pointer
*/
bool CRTTrPMConnectionServer::BeginWaitingForSocket(const int portNumber, const String &bindAddress)
{
	stop();

	socket.reset(new DatagramSocket());	//  deletes the old object that it was previously pointing to if there was one. 

	if(socket->bindToPort(portNumber, bindAddress))
	{
		startThread();
		return true;
	}

	socket.reset();
	return false;
}

/**
* Method to terminate the listener thread
*/
void CRTTrPMConnectionServer::stop()
{
	signalThreadShouldExit();

	if(socket != nullptr)
		socket->shutdown();

	stopThread(4000);
	socket.reset();
}

/**
* Returns the centroid packet module vector, which keeps the x, y, z coordinates
*/
vector<CCentroidMod*>CRTTrPMConnectionServer::GetCentroidPosCoordinates()
{
	return m_modulesFromDatenpacket;
}