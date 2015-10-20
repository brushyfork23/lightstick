#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module
#include <EEPROM.h> // save node ID

#include "Animations.h"
#include "Radio.h"

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
  
  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);
}

void loop()
{
  // run it
  A.runAnimation();
}

