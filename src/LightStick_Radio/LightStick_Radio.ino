#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module

#include "Animations.h"
#include "Radio.h"

#define LOWACT 15 // little bit of flare
#define HIGHACT 100 // lots of flare when triggered
#define FLAREDUR 5000UL // flare for this long when triggered

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
   
  // start the radio
  R.begin();

  // startup animation
  A.begin();
  
  // see Animations.h for other options.
  A.startAnimation(A_FIRE);
  A.setActivity(LOWACT);

}

void loop()
{
  // update the radio traffic
  R.update();

  // check for trigger
  static unsigned long lastTriggered = millis();
  boolean canTrigger = millis()-lastTriggered > FLAREDUR;
  
  if( R.trigger() && canTrigger ) {
    Serial << F("** TRIGGER! **") << endl;
    A.setActivity(HIGHACT); // lots of fire
    lastTriggered = millis();
  } 
  
  boolean cooldown = millis()-lastTriggered > FLAREDUR;
  if( cooldown) A.setActivity(LOWACT); // cool it down after triggering
  
  // run it
  A.runAnimation();
  
}

