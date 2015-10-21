#include "Radio.h"


void Radio::begin(byte nodesStart, byte groupID, byte freq) {
  Serial << F("Radio. startup with unknown node number.") << endl;

  radio.initialize(freq, nodesStart, groupID);
  radio.setHighPower(); // using RFM69HW board
  radio.promiscuous(true); // so broadcasts are received

  // negotiate a node ID for myself
  randomSeed(analogRead(A0)); // entropy
  nNodes = 0; // initialize to none, we'll figure this out later
  
  Serial << F("Waiting....") << endl;
  delay( random(0,MAX_NODES)*25UL ); // wait a tick
  
  if( radio.receiveDone() && radio.DATALEN==sizeof(Message) ) { // someone's claimed a node number
    msg = *(Message*)radio.DATA;  // read it
    nodeID = msg.nextNode;
    msg.nextNode ++;
    msg.packetNumber ++;
    radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));

    Serial << F("Radio. startup with negotiated nodeID=") << nodeID << endl;
  } else { // nobody's claimed a node number.  let's do that
    nodeID = nodesStart;
    msg.nextNode = nodeID+1;
    msg.packetNumber = 0;
    radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));

    Serial << F("Radio. startup with claimed nodeID=") << nodeID << endl;
  }
  // save this, in case we get directed traffic
  radio.setAddress(nodeID);

  // clear rssi tracking
  for(int i=0; i<MAX_NODES; i++) {
    relRSSI[i]=1.0;
    averageRSSI[i]=-50.0;
  }
}

void Radio::update() {
  // new traffic?
  if( radio.receiveDone() && radio.DATALEN==sizeof(Message) ) {
    // read it
    msg = *(Message*)radio.DATA;  

    // record RSSI
    recordRSSI(radio.SENDERID-NODESTART, radio.readRSSI());
    
    // has someone joined?
    byte totalNodes = radio.SENDERID-NODESTART+1; 
    if( totalNodes > nNodes ) {
      nNodes = totalNodes;
      Serial << F("Radio.  new node detected.  nNodes=") << nNodes << endl;
    }

    // am I next in the round-robin transmission?
    if( msg.nextNode = nodeID ) {
      msg.nextNode++;
      if( msg.nextNode >= nNodes ) msg.nextNode = NODESTART; // loop
      msg.packetNumber++;
      radio.send(BROADCAST, (const void*)(&msg), sizeof(Message));
    }   
  }
}

// record RSSI
void Radio::recordRSSI(byte index, float rssi) {
  // running average
  const float nSamples=10.0;
  // score a delta relative to average
  relRSSI[index] = rssi/averageRSSI[index];
  // compute average
  averageRSSI[index] = (nSamples-1)/nSamples * averageRSSI[index] + 1/nSamples * rssi;
}

// trigger on delta RSSI
boolean Radio::trigger(float rssiThresh, unsigned long cooldown) {

  // track cooldown do we're not spamming triggers
  static unsigned long lastTriggered = millis();
  if( millis()-lastTriggered < cooldown ) return( false );

  // check for a trigger
  boolean ret = false;
  for(int i=0; i<nNodes; i++) {
    ret |= relRSSI[i] >= rssiThresh;
    ret |= relRSSI[i] <= 1.0/rssiThresh;
  }

  // if we've triggered, note the time
  if( ret ) lastTriggered = millis();

  return( ret );
}

Radio R;
