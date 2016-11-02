#include "Network.h"

SPIFlash flash(FLASH_SS, FLASH_ID); 

void Network::begin(byte nodeID, byte groupID, byte freq, byte powerLevel) {
	Serial << F("Network. startup.") << endl;

	// Establish NodeID in EEPROM
	const byte radioConfigLocation = 42; // EEPROM location for radio settings.
	if( nodeID == 255 ) {
		this->myNodeID = EEPROM.read(radioConfigLocation);
		Serial << F("Network. read nodeID from EEPROM=") << this->myNodeID << endl;
	} else {
		Serial << F("Network.  writing nodeID to EEPROM=") << nodeID << endl;
		EEPROM.update(radioConfigLocation, nodeID);
		this->myNodeID = nodeID;
	}
	if( this->myNodeID == 255 ) {
		Serial << F("Network. ERROR no nodeID found in EEPROM!") << endl;
		Serial << F("Enter the node number for this node:") << endl;
		while( ! Serial.available() );
		EEPROM.update(radioConfigLocation, Serial.parseInt());
		return( this->begin(nodeID, groupID, freq, powerLevel) );
	}

	radio.initialize(freq, this->myNodeID, groupID);
	radio.setHighPower(); // using RFM69HW board
	radio.promiscuous(true); // so broadcasts are received
	radio.setPowerLevel(powerLevel);
	
	this->animation = this->volume = this->input = this->s = 0;
	
	// LED usage
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);

	this->state = M_NORMAL; // default to normal operation
	
	Serial << F("Network. startup complete with myNodeID=") << this->myNodeID << endl;
	
}

boolean Network::update() {

	// new traffic?
	if( radio.receiveDone() ) {  
		// check for programming 
		if( radio.SENDERID == PROGRAMMER_NODE && radio.TARGETID == this->myNodeID ) {
			Serial << F("Network. Reprogramming message?") << endl;
			// being asked to reprogram ourselves by Gateway?
			CheckForWirelessHEX(this->radio, flash);
			
		} 

		// check for messages
		if( radio.DATALEN==sizeof(message) ) {
			
			// read it
			message = *(unsigned long*)radio.DATA; 
			targetNodeID = radio.TARGETID;
			senderNodeID = radio.SENDERID;
			
			if( targetNodeID == myNodeID && radio.ACKRequested() ) {
				  radio.sendACK();
//				  Serial << this->myNodeID << F(": ACK sent to ") << this->lastRxNodeID << endl;
			}
			return( true );
			
		} else if( radio.DATALEN==sizeof(systemState) ) {

			// read it
			state = *(systemState*)radio.DATA;
			targetNodeID = radio.TARGETID;
			senderNodeID = radio.SENDERID;

			if( targetNodeID == myNodeID && radio.ACKRequested() ) {
				  radio.sendACK();
//				  Serial << this->myNodeID << F(": ACK sent to ") << this->lastRxNodeID << endl;
			}
			
			// run the reboot commmand right now
			if( state == M_REBOOT ) {
				resetUsingWatchdog(true);
			}
			
			return( true );
			
		} else if( radio.SENDERID == PROGRAMMER_NODE ) {
			if( radio.DATALEN >= 23 && radio.DATALEN <= 25 ) {
				if( state != M_PROGRAM ) {
					Serial << F("Network. Programmer traffic.") << endl;
					Serial << F("Length=") << radio.DATALEN << endl;	
					// we need to wait until the airwaves are clear.
					state = M_PROGRAM;
				}
			}
		}
	}


	return( false );
}

void Network::showNetwork() {
	Serial << F("Network. ");
	Serial << senderNodeID << F("->") << targetNodeID;
	Serial << F("\ts=")  << s;
	Serial << F("\tmsg=") << _BIN(message);
	Serial << F("\tstate=") << state;
	Serial << endl;
}

/*
message is 32 bits:
	0-7   = animation
	8-15  = volume
	16-23 = input
	24-31 = s (8 extra MSB)

message == AAAAAAAAVVVVVVVVIIIIIIIISSSSSSSS
*/

void Network::decodeMessage() {
	this->s = this->message & 255UL;
	this->input = (this->message >> 8) & 255UL;
	this->volume = (this->message >> 16) & 255UL;
	this->animation = (this->message >> 24) & 255UL;
}

void Network::encodeMessage() {

	this->message = 0;
	
	this->message |= this->animation & 255UL;
	this->message = this->message << 8;
	
	this->message |= this->volume & 255UL;
	this->message = this->message << 8;
	
	this->message |= this->input & 255UL;
	this->message = this->message << 8;

	this->message |= this->s & 255UL;
	
}

// broadcast message to all nodes
void Network::broadcastMessage() {
	// put check in to make sure we're not clobbering messages from other transceivers
	update();

	targetNodeID = BROADCAST;
	senderNodeID = myNodeID;

	radio.send(BROADCAST, (const void*)(&message), sizeof(message));

}

void Network::sendMessage(byte toNodeID) {
	// put check in to make sure we're not clobbering messages from other transceivers
	update();

	targetNodeID = toNodeID;
	senderNodeID = myNodeID;

	radio.send(toNodeID, (const void*)(&message), sizeof(message));

}

void Network::sendState(byte toNodeID) {
	// put check in to make sure we're not clobbering messages from other transceivers
	update();

	targetNodeID = toNodeID;
	senderNodeID = myNodeID;

	radio.send(toNodeID, (const void*)(&state), sizeof(state));
}

Network N;
