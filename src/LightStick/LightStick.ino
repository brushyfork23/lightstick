#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include "Animations.h"

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

