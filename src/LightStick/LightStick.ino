#include <FastLED.h>
#include <Metro.h>
#include "Animations.h"

#define FPS 100

CHSV Blue(HUE_BLUE, 255, 255);
Metro changeAnimation(10000UL);
byte currentAnimation = A_SOLID;

void setup() {
  delay(500); // delay for upload
   
  // startup animation
  anim.begin();
  anim.setAnimation(A_SOLID, Blue, 0, 30);
//  anim.setAnimation(A_RAINBOW, Blue, 0, FPS);
//  anim.setAnimation(A_GLITTER, Blue, 0, FPS);
//  anim.setAnimation(A_CONFETTI, Blue, 0, FPS);
//  anim.setAnimation(A_CYLON, Blue, 0, FPS);
//  anim.setAnimation(A_BPM, Blue, 0, FPS);
//  anim.setAnimation(A_JUGGLE, Blue, 0, FPS);
}

void loop()
{
  // run it
  anim.runAnimation();
  
  // swap on an interval
  if( changeAnimation.check() ) {
    currentAnimation++;
    if( currentAnimation == N_ANIMATIONS ) currentAnimation = 0;
    anim.setAnimation(currentAnimation, Blue, 0, 30);
  }
}

