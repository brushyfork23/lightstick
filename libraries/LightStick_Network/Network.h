#ifndef Network_h
#define Network_h

#include <Arduino.h>

#include <Streaming.h>

// radio
#include <RFM69.h> 
#include <SPI.h> 
// wireless programming
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
// for storage of node information
#include <EEPROM.h> 

// comms settings and information

#define BROADCAST			0  // all nodes will hear this
#define GROUPID				158  // local group
#define POWERLEVEL			15 // 0-31, 31 being maximal

#define PROGRAMMER_NODE		253 // nodeID of wireless programmer

#define AUDIO_NODE			252 // nodeID of audio broadcaster

#define N_NODES				3

// system state messages
enum systemState {
	M_CALIBRATE=0,	// calibration of sensors
	M_NORMAL,		// normal operation
	M_PROGRAM,		// OTA programming
	M_REBOOT, 		// reboot the nodes
	
	N_MODES 		// track mode count
};

// pin definitions common to Moteuinos
#define LED		9 // Moteinos have LED on D9
#define FLASH_SS	8 // and FLASH SS on D8
#define FLASH_ID	0xEF30 // EF30 for windbond 4mbit flash

class Network {
  public:

  	// Payload
	word 
		animation,
		volume,
		input,
		s=0;

	// initialize radio
	void begin(byte nodeID=255, byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
	// return my node ID
	byte myNodeID;
	// return my index into arrays
	byte myIndex;
	// which can be used to understand what is to the right and left
	byte right(byte i);
	byte left(byte i);

	// check for radio traffic; return true if we have a message or state change
	boolean update();
	byte senderNodeID, targetNodeID; // from and to information for message
	unsigned long message; // message 
	systemState state; // system state
	
	// show the contents of the network information
	void showNetwork();

	// translate message -> instructions
	void decodeMessage();

	// translate instructions -> message
	void encodeMessage();

	// send message to all nodes
	void broadcastMessage();

	// send message to one node
	void sendMessage(byte toNodeID);

	void sendState(byte toNodeID=BROADCAST);

	// all of this is conducted over radio
	RFM69 radio;
	
};

extern Network N;

#endif
