// ArrayOfLedArrays - see https://github.com/FastLED/FastLED/wiki/Multiple-Controller-Examples for more info on
// using multiple controllers.  In this example, we're going to set up four NEOPIXEL strips on three
// different pins, each strip getting its own CRGB array to be played with, only this time they're going
// to be all parts of an array of arrays.

#include "FastLED.h"

#define NUM_STRIPS 2
#define NUM_LEDS_PER_STRIP 110
#define PIN_CLK 6 // shared by the strips
#define PIN_DATA1 4
#define PIN_DATA2 7
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

#define MASTER_BRIGHTNESS 255
#define FRAMES_PER_SECOND 30

// For mirroring strips, all the "special" stuff happens just in setup.  We
// just addLeds multiple times, once for each strip
void setup() {
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds[1], NUM_LEDS_PER_STRIP);
  
  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

void loop() {
  // This outer loop will go over each strip, one at a time
  for(int s = 0; s < NUM_STRIPS; s++) {
    // This inner loop will go over each led in the current strip, one at a time
    for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      leds[s][i] = CRGB::Red;
      FastLED.show();
      leds[s][i] = CRGB::Black;
      
      // insert a delay to keep the framerate modest
      FastLED.delay(1000/FRAMES_PER_SECOND); 
    }
  }
}
