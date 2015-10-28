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

  // where are now
  unsigned long now = millis();
  
  // check for trigger
  static unsigned long lastTriggered = now;
  boolean canTrigger = now-lastTriggered > FLAREDUR;

  // track threshold for triggering
  static float threshold = 1.04;
  static Metro sensAdjust(30000UL);
  if( sensAdjust.check() && canTrigger ) {
    // too long between triggers.  increase sensitivity
    if( threshold > 1.0 ) threshold -= 0.01; // decrease sensitivity
    Serial << F("trigger threshold now: ") << threshold << endl;
  }
  
  if( R.trigger(threshold) && canTrigger ) {
    Serial << F("** TRIGGER! **") << endl;
    A.setActivity(HIGHACT); // lots of fire
    lastTriggered = now;
    threshold += 0.01; // decrease sensitivity
    Serial << F("trigger threshold now: ") << threshold << endl;
  } 
  
  boolean cooldown = now-lastTriggered > FLAREDUR;
  if( cooldown) A.setActivity(LOWACT); // cool it down after triggering
  
  // run it
  A.runAnimation();
  
}

