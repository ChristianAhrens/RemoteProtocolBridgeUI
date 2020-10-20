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
class CRTTrPMConnectionServer : private Thread,
	private MessageListener
{
public:
	//==============================================================================
	/**
	 * Implementation of a RTTrPM message. This is designed similar to SenderAwareOSCReceiver::SAOPimpl 
	 * that itself is taken from JUCEs' OSCReceiver::pimpl
	 */
	struct CallbackMessage : public Message
	{
		/**
		* Constructor with default initialization of sender ip and port.
		*
		* @param rttrpmModule	The rttrpm module data to use in this message.
		*/
		CallbackMessage(CPacketModule rttrpmModule) : contentModule(rttrpmModule), senderIPAddress(String()), senderPort(0) {}

		/**
		* Constructor with default initialization of sender ip and port.
		*
		* @param rttrpmModule	The rttrpm module data to use in this message.
		* @param sndIP	The sender ip of this message.
		* @param sndPort The port this message was received on.
		*/
		CallbackMessage(CPacketModule rttrpmModule, String sndIP, int sndPort) : contentModule(rttrpmModule), senderIPAddress(sndIP), senderPort(sndPort) {}

		CPacketModule	contentModule;			/**< The payload of the message. */
		String			senderIPAddress;	/**< The sender ip address from whom the message was received. */
		int				senderPort;			/**< The sender port from where the message was received. */
	};
	
	//==============================================================================
	/** 
	 * A class for receiving RTTrPM data from CRTTrPMConnectionServer.
	 */
	class RTTrPMListener
	{
	public:
		/** Destructor. */
		virtual ~RTTrPMListener() = default;

		/** Called when the CRTTrPMConnectionServer receives new RTTrPM module(s). */
		virtual void RTTrPMModuleReceived(const CPacketModule& module, const String& senderIPAddress, const int& senderPort) = 0;
	};

public:
	CRTTrPMConnectionServer(int portNumber);
	~CRTTrPMConnectionServer();

	//==============================================================================
	bool start();
	bool stop();

	//==============================================================================
	void addListener(CRTTrPMConnectionServer::RTTrPMListener* listenerToAdd);
	void removeListener(CRTTrPMConnectionServer::RTTrPMListener* listenerToRemove);

private:
	//==============================================================================
	bool BeginWaitingForSocket(const int portNumber, const String &bindAddress);

	void run() override;
	int HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, std::vector<CPacketModule*>& packetModules);

	//==============================================================================
	void handleMessage(const Message& msg) override;
	void callListeners(const CPacketModule& content, const String& senderIPAddress, const int& senderPort);

	//==============================================================================
	std::unique_ptr<DatagramSocket>	m_socket;

	//==============================================================================
	int														m_listeningPort{ 0 };
	String													m_hostAddress;
	ListenerList<CRTTrPMConnectionServer::RTTrPMListener>	m_listeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CRTTrPMConnectionServer)
};

