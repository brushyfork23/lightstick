#include "FastLED.h"

FASTLED_USING_NAMESPACE

// FastLED "100-lines-of-code" demo reel, showing just a few 
// of the kinds of animation patterns you can quickly and easily 
// compose using FastLED.  
//
// This example also shows one easy way to define multiple 
// animations patterns and have them automatically rotate.
//
// -Mark Kriegsman, December 2014

#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define PIN_DATA1    4
#define PIN_CLK   6
#define PIN_DATA2 7
#define LED_TYPE    APA102
#define COLOR_ORDER RGB
#define NUM_STRIPS 2
#define NUM_LEDS    110
CRGB leds[NUM_STRIPS][NUM_LEDS];

// at 60mA per pixel on white * 220 pixels, 13.2A @ 5VDC.

#define BRIGHTNESS          255
#define FRAMES_PER_SECOND  120

void setup() {
  delay(500); // delay for upload
  
  // tell FastLED about the LED strip configuration
//  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
//  FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds[0], NUM_LEDS);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds[1], NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(255);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { allWhite, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
  
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void allWhite() {
  // FastLED's built-in solid fill (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_solid( leds[s], NUM_LEDS, CRGB::White );
  }
}

void rainbow() 
{
  // FastLED's built-in rainbow generator (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_rainbow( leds[s], NUM_LEDS, gHue, 7);
  }
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    int l = random16(NUM_LEDS);
    leds[0][l] += CRGB::White;
    leds[1][l] = leds[0][l];
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds[0], NUM_LEDS, 10);
  fadeToBlackBy( leds[1], NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[0][pos] += CHSV( gHue + random8(64), 200, 255);
  leds[1][pos] = leds[0][pos];
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[0][pos] += CHSV( gHue, 255, 192);
  leds[1][pos] = leds[0][pos];
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[0][i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    leds[1][i] = leds[0][i];
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    int l = beatsin16(i+7,0,NUM_LEDS);
    leds[0][l] |= CHSV(dothue, 200, 255);
    leds[1][l] = leds[0][l];
    dothue += 32;
  }
}

