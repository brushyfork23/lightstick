#include "Radio.h"


void Radio::begin(byte groupID, byte freq) {
  Serial << F("Radio. startup with unknown node number.") << endl;

  radio.initialize(freq, NODESTART, groupID);
  setNodeID(NODESTART);
  radio.setHighPower(); // using RFM69HW board
  radio.promiscuous(true); // so broadcasts are received
  setPowerLevel();
  
  Serial << F("Waiting....") << endl;
  delay( random(0,MAX_NODES)*25UL ); // wait a tick
  
  // clear rssi tracking
  for(int i=0; i<MAX_NODES; i++) {
    relRSSI[i]=1.0;
    averageRSSI[i]=CSMA_LIMIT;
  }
  
  // set up pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void Radio::setNodeID(byte node) {
  if( node < NODESTART || node > (NODESTART+MAX_NODES)) this->nodeID = NODESTART;
  else this->nodeID = node; // wrap, if needed
  
  Serial << F("set nodeID=") << this->nodeID << endl;
  radio.setAddress(this->nodeID);
}
void Radio::setPowerLevel(byte level) {
  if( level>31 ) return; // wrap
  Serial << F("set power level=") << level << endl;
  
  this->powerLevel = level;
  msg.powerLevel = level;
  radio.setPowerLevel(level);
  
  msg.doTrigger = false; // not yet
}

void Radio::update() {
  // new traffic?
  if( radio.receiveDone() && radio.DATALEN==sizeof(Message) ) {
    digitalWrite(LED_PIN, HIGH);
    
    // read it
    msg = *(Message*)radio.DATA;  

    // record RSSI
//    recordRSSI(radio.SENDERID-NODESTART, radio.readRSSI());
    recordRSSI(radio.SENDERID-NODESTART, radio.RSSI);
    
    // has someone claimed my nodeID already?
    if( radio.SENDERID == nodeID ) setNodeID(nodeID+1);
    // is the RSSI too high?  cut the power down
//    if( radio.RSSI > RSSI_TARGET ) setPowerLevel(powerLevel-1);
    // someone else could have called for this, too.
//    if( msg.powerLevel != powerLevel ) setPowerLevel(msg.powerLevel);
    
    digitalWrite(LED_PIN, LOW);
  }
  
  // am I next in the round-robin transmission?
  static unsigned long lastSend = millis();

  if( millis()-lastSend > PING_EVERY && radio.readRSSI() > CSMA_LIMIT ) {       
    msg.packetNumber++;
    msg.doTrigger = false;
    Serial << F("sending ping from myNode:") << nodeID; 
    radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
    lastSend = millis();
    Serial << F(".  done.") << endl;
  }   

}

// record RSSI
void Radio::recordRSSI(byte index, int rssi) {
/*
  // check for realistic RSSI
  if( rssi < (CSMA_LIMIT+RSSI_THRESH) ) {
    Serial << "bad rssi=" << rssi << endl;
    return;
  }
*/
  if( averageRSSI[index]==CSMA_LIMIT ) averageRSSI[index]=rssi; // dirty startup
  
  // running average
  const float nSamples=10.0;
  // score a delta relative to average
  relRSSI[index] = rssi/averageRSSI[index];
  
  // compute average
  averageRSSI[index] = (nSamples-1)/nSamples * averageRSSI[index] + 1/nSamples * rssi;
 
  Serial << F("Record node:") << index+NODESTART << F("\trssi=") << rssi << F("\tavg rssi=");
  Serial << averageRSSI[index] << F("\trel rssi=") << relRSSI[index] << endl;   
}

// trigger on delta RSSI
boolean Radio::trigger(float rssiThresh, unsigned long cooldown) {

  // track cooldown do we're not spamming triggers
  static unsigned long lastTriggered = millis();
  if( millis()-lastTriggered < cooldown ) return( false );

  // check for a trigger by RSSI
  boolean ret = false;
  for(int i=0; i<MAX_NODES; i++) {
    ret |= relRSSI[i] >= rssiThresh;
    ret |= relRSSI[i] <= 1.0/rssiThresh;
  }

  // check for a trigger by packet
  if( ret && !msg.doTrigger ) {
    // announce by packet that we'll trigger
    msg.packetNumber++;
    msg.doTrigger = true;
    Serial << F("trigger by RSSI.  sending trigger by radio from myNode:") << nodeID;   
    radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
    Serial << F(". done.") << endl;
    
    ret |= true; 
    msg.doTrigger = false;
  } else if( msg.doTrigger ) {
    Serial << F("got trigger by radio.") << endl;
    ret |= true;
  }

  // if we've triggered, note the time
  if( ret ) lastTriggered = millis();

  return( ret );
}

Radio R;
