/*
  ==============================================================================

    RTTrPMReceiver.h
    Created: 23 Oct 2020 10:07:01am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PacketModules/RTTrPMHeader.h"
#include "PacketModules/PacketModules.h"


// **************************************************************
// class RTTrPMReceiver
// **************************************************************
/**
*
* Creates an uninitialised server object, that will wait
* for client sockets to connect to a port on this host.
*/
class RTTrPMReceiver : private Thread,
	private MessageListener
{
public:
	//==============================================================================
	/**
	 * Helper type that combindes RTTrPM header information
	 * with list of content modules
	 */
	struct RTTrPMMessage
	{
		/**
		 * Constructor with default initialization header and modules.
		 */
		RTTrPMMessage() : header(), modules() {}

		RTTrPMHeader								header;
		std::vector<std::unique_ptr<PacketModule>>	modules;
	};

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
		CallbackMessage(RTTrPMMessage& rttrpmMessage) : contentRTTrPM(std::move(rttrpmMessage)), senderIPAddress(String()), senderPort(0) {}

		/**
		* Constructor with default initialization of sender ip and port.
		*
		* @param rttrpmModule	The rttrpm module data to use in this message.
		* @param sndIP	The sender ip of this message.
		* @param sndPort The port this message was received on.
		*/
		CallbackMessage(RTTrPMMessage& rttrpmMessage, String sndIP, int sndPort) : contentRTTrPM(std::move(rttrpmMessage)), senderIPAddress(sndIP), senderPort(sndPort) {}

		RTTrPMMessage	contentRTTrPM;		/**< The payload of the message. */
		String			senderIPAddress;	/**< The sender ip address from whom the message was received. */
		int				senderPort;			/**< The sender port from where the message was received. */
	};
	
	//==============================================================================
	/** 
	 * A class for receiving RTTrPM data from RTTrPMReceiver.
	 */
	class DataListener
	{
	public:
		/** Destructor. */
		virtual ~DataListener() = default;

		/** Called when the RTTrPMReceiver receives new RTTrPM module(s). */
		virtual void RTTrPMModuleReceived(const RTTrPMMessage& module, const String& senderIPAddress, const int& senderPort) = 0;
	};

public:
	RTTrPMReceiver(int portNumber);
	~RTTrPMReceiver();

	//==============================================================================
	bool start();
	bool stop();

	//==============================================================================
	void addListener(RTTrPMReceiver::DataListener* listenerToAdd);
	void removeListener(RTTrPMReceiver::DataListener* listenerToRemove);

private:
	//==============================================================================
	bool BeginWaitingForSocket(const int portNumber, const String &bindAddress = String());

	void run() override;
	int HandleBuffer(unsigned char* dataBuffer, size_t bytesRead, RTTrPMMessage& decodedMessage);

	//==============================================================================
	void handleMessage(const Message& msg) override;
	void callListeners(const RTTrPMMessage& content, const String& senderIPAddress, const int& senderPort);

	//==============================================================================
	std::unique_ptr<DatagramSocket>	m_socket;

	//==============================================================================
	int											m_listeningPort{ 0 };
	ListenerList<RTTrPMReceiver::DataListener>	m_listeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RTTrPMReceiver)
};