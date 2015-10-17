#include <FastLED.h>
#include <Metro.h>
#include <Streaming.h>
#include "Animations.h"

Metro changeAnimation(10000UL);
byte anim = A_SOLID;
byte pos = 0;
byte hue = 0;

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);
  
  // startup animation
  A.begin();
  
  // 
  A.startAnimation(A_FIRE);
  A.startHue(0);
  A.incrementHue(1);
}

void loop()
{
  // run it
  A.runAnimation();
  /*
  // swap on an interval
  if( changeAnimation.check() ) {
    pos += 10;
    hue += 10;
    anim++;

    A.startAnimation(anim);
    A.startPosition(pos);
    A.startHue(hue);
  }
  */
}

