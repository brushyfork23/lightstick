#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module

#include "Animations.h"
#include "Radio.h"

#define LOWACT 20 // little bit of flare
#define HIGHACT 80 // lots of flare when triggered
#define FLAREDUR 1000UL // flare for this long when triggered

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
  
  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);
  A.setActivity(LOWACT);
  
  // start the radio
  R.begin();
}

void loop()
{
  // update the radio traffic
  R.update();

  // check for trigger
  static unsigned long lastTriggered = millis();
  const unsigned long flareFor = FLAREDUR;
  if( R.trigger() ) {
    A.setActivity(HIGHACT); // lots of fire
    lastTriggered = millis();
  }

  // cool it down after triggering
  if( millis()-lastTriggered > flareFor ) A.setActivity(LOWACT);
  
  // run it
  A.runAnimation();
}

