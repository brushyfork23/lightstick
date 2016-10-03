#include "Radio.h"

SPIFlash flash(FLASH_SS, FLASH_ID); 

Payload payload;

void Radio::begin(byte nodeID, byte groupID, byte freq, byte powerLevel) {
  // Establish My NodeId
  // EEPROM location for radio settings.
  const byte radioConfigLocation = 42;
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
    return( this->begin(nodeID, GROUPID, freq, powerLevel) );
  }

  // Start Radio

  radio.initialize(freq, this->myNodeID, groupID);
  radio.setHighPower(); // using RFM69HW board
  radio.promiscuous(true); // so broadcasts are received
  radio.setPowerLevel(powerLevel);

  // LED usage
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
}

void Radio::update() {
  // check for new data
  if (radio.receiveDone() && radio.DATALEN == sizeof(Payload)) {
    // update Payload object
    payload = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
    this->pgm = payload.pgm;
    this->num = payload.num;
    this->hasUnorocessedPayload = true;
  }
}

void Radio::sendAck() {
  radio.sendACK();
}

void Radio::sendProgram(byte pgm) {
  payload.pgm = pgm;
  sendPayload(payload);
}

void Radio::sendVal(byte val) {
  payload.num = val;
  sendPayload(payload);
}

void Radio::sendPayload(Payload payload) {
  Serial << F("Sending struct (") << sizeof(payload) << F(" bytes) ... ") << endl;
  radio.send(BROADCAST, (const void*)(&payload), sizeof(Payload));
}

Radio R;

