#ifndef LightStick_Network_h
#define LightStick_Network_h

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

#define CONTROLLER_NODE		252 // nodeID of controlloer which broadcasts new instructions

#define AUDIO_NODE			251 // nodeID of audio broadcaster

#define N_NODES				3

// pin definitions common to Moteuinos
#define LED		9 // Moteinos have LED on D9
#define FLASH_SS	8 // and FLASH SS on D8
#define FLASH_ID	0xEF30 // EF30 for windbond 4mbit flash



// system state messages
enum systemState {
	M_CALIBRATE=0,	// calibration of sensors
	M_NORMAL,		// normal operation
	M_PROGRAM,		// OTA programming
	M_REBOOT, 		// reboot the nodes
	
	N_MODES 		// track mode count
};

// controller instructions
enum controlInstructions {
	M_RAISE_VOL=0,	// Raise volume broadcast value (by lowering audio squash)
	M_LOWER_VOL,	// Lower volume broadcast value (by raising audio squash)
	M_RAISE_FREQ,	// Raise trigger frequency band
	M_LOWER_FREQ,	// Lower trigger frequency band

	N_INSTRUCTIONS
};

class LightStick_Network {
  public:
	// initialize radio
	void begin(byte nodeID=255, byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
	// check for radio traffic; return true if we have a message or state change
	boolean update();
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
	systemState state; // system state
	// return my node ID
	byte myNodeID;
	byte senderNodeID, targetNodeID; // from and to information for message
  	// Payload
	word 
		animation,
		volume,
		input,
		s;
	// all of this is conducted over radio
	RFM69 radio;
  private:
	unsigned long message; // message
};

extern LightStick_Network N;

#endif
