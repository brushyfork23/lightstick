#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module

#include "Animations.h"

// radio instance
RFM69 radio;
// signal strength, proportional (somehow) to distance and interference
int rssi;
char payload[] = "ABCDE";
int sendSize = 5;

Metro sendPacket(500UL);

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
  
  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);

  // radio intialization  
  radio.initialize(RF69_915MHZ, 21, 188); // freq, node, group
  radio.setHighPower(); // for HW boards.
  radio.promiscuous(true); // so broadcasts are received.
  
  randomSeed(analogRead(A3)); // randomness
}

void loop()
{
  // run it
  A.runAnimation();
  
  // check for comms traffic
  if ( radio.receiveDone() ) {
    rssi = radio.readRSSI();
    Serial << F("Rx.  RSSI=") << rssi << endl;
  }
  else if( sendPacket.check() ) { 
    radio.send(0, payload, sendSize);
//    sendPacket.interval(random(1000UL, 10000UL));
    sendPacket.reset();
    Serial << F("Tx.") << endl;
  }  
  
}

