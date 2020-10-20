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
CRTTrPMConnectionServer::CRTTrPMConnectionServer(int portNumber) : Thread("RTTrPM_Connection_Server")
{
	m_socket = std::make_unique<DatagramSocket>();
	m_listeningPort = portNumber;
	m_hostAddress = "127.0.0.1";
}

/**
* Destructor of the CRTTrPMConnectionServer class
*/
CRTTrPMConnectionServer::~CRTTrPMConnectionServer()
{
}

/**
* Method to start the listener thread
*/
bool CRTTrPMConnectionServer::start()
{
	return BeginWaitingForSocket(m_listeningPort, m_hostAddress);
}

/**
* Method to terminate the listener thread
*/
bool CRTTrPMConnectionServer::stop()
{
	signalThreadShouldExit();

	if (m_socket != nullptr)
		m_socket->shutdown();

	stopThread(4000);
	m_socket.reset();

	return true;
}

/**
 * Method to add a Listener to internal list.
 *
 * @param listenerToAdd	The listener object to add.
 */
void CRTTrPMConnectionServer::addListener(CRTTrPMConnectionServer::RTTrPMListener* listenerToAdd)
{
	m_listeners.add(listenerToAdd);
}

/**
 * Method to remove a Listener from internal list.
 *
 * @param listenerToRemove	The listener object to remove.
 */
void CRTTrPMConnectionServer::removeListener(CRTTrPMConnectionServer::RTTrPMListener* listenerToRemove)
{
	m_listeners.remove(listenerToRemove);
}

/**
* Reads all packet modules and sorts them within the CCentroidMod class, saves all modules into packetModules vector
* @param	dataBuffer		: An array which keeps the caught data information.
* @param	bytesRead		: Keeps the number of read bytes
* @param	packetModules	: Module vector that is filled with the modules read from the buffer
*
* @return	packetModules	: Returns the count of packet modules read into given target module content vector
*/
int CRTTrPMConnectionServer::HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, std::vector<CPacketModule*>& packetModules)
{
	std::vector<unsigned char> data(dataBuffer, dataBuffer + bytesRead);			// data: Vector that has all the caught data information
	int startPosToRead = 0;													// Counter variable to know from which byte the next module has to be read
	CRTTrP cRTTrPObject = CRTTrP(data, startPosToRead);						// Sort all RTTrP header information

	if(startPosToRead == 0)	//	If true, Signature from RTTrP header was not right. Process will be returned to the run methode
	{
		return 0;
	}

	for(int i = 0; i < cRTTrPObject.GetNumOfTrackableMods(); i++)			// As many trackable modules as the packet has
	{
		CPacketModuleTrackable trackableModule(data, startPosToRead);		// Reads the name, name length and number of sub-modules

		for(int j = 0; j < trackableModule.GetNumberOfSubModules(); j++)	// As many sub-modules as the packet has
		{
			CPacketModule packetModuleToRead(data, startPosToRead);			// Reads the module type and size

			switch(packetModuleToRead.GetModuleType())						// Decides between the different type of sub-modules
			{
			case CPacketModule::PMT_centroidPosition:
				{
					std::vector<unsigned char> centroidmoddata(data.begin() + 31, data.end());	// centroidmoddata :  Vector that receives the position of the coordinates.
					CCentroidMod *mod = new CCentroidMod(&centroidmoddata);
					packetModules.push_back(mod);						
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
			
				case CPacketModule::PMT_trackedPointPosition:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case CPacketModule::PMT_orientationQuaternion:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case CPacketModule::PMT_orientationEuler:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case CPacketModule::PMT_centroidAccelerationAndVelocity:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
				
				case CPacketModule::PMT_trackedPointAccelerationandVelocity:
				{
					startPosToRead += packetModuleToRead.GetModuleSize();
					break;
				}
			}
		}
	}

	return static_cast<int>(packetModules.size());
}

/**
* It waits until a client sends any message to the host.
* If a message arrives it will call up the HandleBuffer method which will sort the module information.
*/
void CRTTrPMConnectionServer::run()
{
	int maxBytesToRead = 512;		// Variable for the maximal size of data -> notice: if its too small data can't be read!
	unsigned char dataBuffer[512];	// An array which keeps the caught data information.
	std::vector<CPacketModule*>	modulesReadBuffer;

	while(!threadShouldExit())
	{
		int readyReturnValue = m_socket->waitUntilReady(true, 100);	//	waitUntilReady() -> true: It will wait until the socket is ready for reading, 100: give up time in ms.

		if(readyReturnValue == 0)
		{
			wait(1000);
			continue;
		}

		else if(readyReturnValue == 1)
		{
			//	bytesRead returns the number of read bytes, or -1 if there was an error.
			int bytesRead = m_socket->read(dataBuffer, maxBytesToRead, false);	//	read() -> false: the method will return as much data as is currently available without blocking.

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
				int moduleCount = HandleBuffer(dataBuffer, bytesRead, modulesReadBuffer);
				if (moduleCount > 0)
				{
					for (auto const& mod : modulesReadBuffer)
					{
						// now post the message that will trigger the handleMessage callback
						// dealing with the non-realtime listeners.
						if (m_listeners.size() > 0)
							postMessage(new CallbackMessage(*mod, m_hostAddress, m_listeningPort));
					}
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

	m_socket.reset(new DatagramSocket());	//  deletes the old object that it was previously pointing to if there was one. 

	if(m_socket->bindToPort(portNumber, bindAddress))
	{
		startThread();
		return true;
	}

	m_socket.reset();
	return false;
}

/**
* 
*/
void CRTTrPMConnectionServer::handleMessage(const Message& msg)
{
	if (auto* callbackMessage = dynamic_cast<const CallbackMessage*> (&msg))
	{
		auto& contentModule = callbackMessage->contentModule;
		auto& senderIPAddress = callbackMessage->senderIPAddress;
		auto& senderPort = callbackMessage->senderPort;

		callListeners(contentModule, senderIPAddress, senderPort);
	}
}

/**
* 
*/
void CRTTrPMConnectionServer::callListeners(const CPacketModule& contentModule, const String& senderIPAddress, const int& senderPort)
{
	if (contentModule.isValid())
	{
		m_listeners.call([&](CRTTrPMConnectionServer::RTTrPMListener& l) { l.RTTrPMModuleReceived(contentModule, senderIPAddress, senderPort); });
	}
}