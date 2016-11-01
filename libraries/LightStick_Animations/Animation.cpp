#include "Animations.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_LEDS];

// startup
void Animation::begin() {
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  this->setFPS();
  this->brightnessSet();

  this->startAnimation();

  this->colorSet();
  this->colorIncrement();

  this->startPosition();
  this->incrementPosition();

  this->startSeed();

  this->setActivity();

  Serial << F("Animation. Startup complete.") << endl;
}
// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->pushNextFrame.interval(1000UL/framesPerSecond);
  Serial << F("FPS= ") << framesPerSecond << F(". show update=") << 1000UL/framesPerSecond << F(" ms.") << endl;
}

// sets master brightness
void Animation::brightnessSet(byte brightness) {
  // set master brightness control
  this->brightVal = brightness;
  FastLED.setBrightness(brightness); 
  Serial << F("Brightness set= ") << brightness << endl;
}
void Animation::brightnessInc(uint8_t inc) {
  this->brightInc = inc;
  Serial << F("Brightness increment=") << this->brightInc << endl;
}
void Animation::brightnessTarget(byte brightness) {
  this->targetBright = brightness;
  Serial << F("Brightness target=") << this->targetBright << endl;
}
void Animation::brightnessStep() {
  // Bring brightVal closer to targetBright
  if (this->brightVal != this->targetBright) {
    // set to target if increment would overshoot
    if (abs(this->brightVal - this->targetBright) < this->brightInc) {
      this->brightVal = this->targetBright;
    } else if (this->brightVal < this->targetBright) {
      this->brightVal += this->brightInc;
    } else if (this->brightVal > this->targetBright) {
      this->brightVal -= this->brightInc;
    }
  }
}

// sets the animation 
void Animation::startAnimation(byte animation, boolean clearStrip) {
  this->anim = animation % (N_ANIMATIONS-1);
  if( clearStrip ) FastLED.clear();

  Serial << F("animation=") << this->anim << endl;

}
void Animation::colorSet(byte color) {
  this->colorVal = color;
  Serial << F("color set=") << this->colorVal << endl;
}
void Animation::colorIncrement(int inc) {
  this->colorInc = inc;
  Serial << F("color increment=") << this->colorInc << endl;
}
void Animation::colorTarget(byte color) {
  this->targetColor = color;
  Serial << F("color target=") << this->targetColor << endl;
}
void Animation::colorStep() {
  // Bring colorVal closer to targetcolor
  if (this->colorVal != this->targetColor) {
    // set to target if increment would overshoot
    if (abs(this->colorVal - this->targetColor) < this->colorInc) {
      this->colorVal = this->targetColor;
    } else if (this->colorVal < this->targetColor) {
      this->colorVal += this->colorInc;
    } else if (this->colorVal > this->targetColor) {
      this->colorVal -= this->colorInc;
    }
  }
}
void Animation::startPosition(byte pos) {
  this->posVal = pos % NUM_LEDS;
  Serial << F("pos=") << this->posVal << endl;
}
void Animation::incrementPosition(int inc) {
  this->posInc = inc ;
  Serial << F("increment=") << this->posInc << endl;
}
void Animation::startSeed(uint16_t seed) {
  random16_set_seed( seed );  // FastLED/lib8tion
  Serial << F("random seed=") << seed << endl;
}
void Animation::setActivity(fract8 chance) {
  if( chanceAct != chance ) {
    this->chanceAct = chance;
    Serial << F("activity chance=") << this->chanceAct << endl;
  }
}

// runs the animation
void Animation::update() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if( ! nextFrameReady ) {
    switch( anim ) {
    case A_PULSE: 
      aPulse(); 
      break;
    case A_STABLE: 
      aStable(); 
      break;
    case A_SOLID: 
      aSolid(); 
      break;
    case A_RAINBOW: 
      aRainbow(); 
      break;
    case A_GLITTER: 
      aGlitter(); 
      break;
    case A_CONFETTI: 
      aConfetti(); 
      break;
    case A_CYLON: 
      aCylon(); 
      break;
    case A_BPM: 
      aBPM(); 
      break;
    case A_JUGGLE: 
      aJuggle(); 
      break;
    case A_WHITE: 
      aWhite(); 
      break;
    case A_FIRE: 
      aFire(); 
      break;
    case A_CENTERFIRE:
      aCenterFire(55, 0);
      break;
    case A_TESTPATTERN: 
      aTestPattern(); 
      break;
    case A_CLEAR:
      aClear();
      break;
    case A_RGB: 
      aRGB(); 
      break;
    case A_HUE: 
      aHue(); 
      break;
    default: 
      break;
    }
    nextFrameReady = true;
  }    

  // ready to push next frame?  
  if( pushNextFrame.check() ) {
    FastLED.show(); // push

    nextFrameReady = false; // setup for next frame calculation
    pushNextFrame.reset();  // setup for next frame push
  } 

}

// uses: colorVal, targetColor, colorInc, brightVal, targetBright, brightInc
void Animation::aPulse() {
  if (this->brightVal == BRIGHT_LOW) {
    brightnessTarget(BRIGHT_HIGH);
  } else if (this->brightVal == BRIGHT_HIGH) {
    brightnessTarget(BRIGHT_LOW);
  }

  colorStep();
  brightnessStep();
  brightnessSet(this->brightVal);
  fill_solid( leds, NUM_LEDS, CHSV(colorVal, 255, 255) );
  //fill_solid( leds, NUM_LEDS, CHSV(colorVal, 255, this->brightVal) );
}

// uses: colorVal
void Animation::aStable() {
  //colorStep();
  // FastLED's built-in solid fill (colorutils.h)
  brightnessSet(this->brightVal);
  fill_solid( leds, NUM_LEDS, CHSV(this->targetColor, 255, 255) );
  //fill_solid( leds, NUM_LEDS, CHSV(colorVal, 255, 255) );
  //fill_solid( leds, NUM_LEDS, CHSV(colorVal, 255, this->brightVal) );
}

// uses: colorVal, colorInc
void Animation::aSolid() {
  // FastLED's built-in solid fill (colorutils.h)
  fill_solid( leds, NUM_LEDS, CHSV(colorVal, 255, 255) );
  colorVal += colorInc;
}

// uses: colorVal, colorInc
void Animation::aRainbow() {
  // FastLED's built-in rainbow generator (colorutils.h)
  fill_rainbow( leds, NUM_LEDS, colorVal );
  colorVal += colorInc;
}

// uses: colorVal, colorInc, posVal, chanceAct
void Animation::aGlitter() {
  if( random8() < chanceAct ) {
    posVal += random8(NUM_LEDS);
    posVal %= NUM_LEDS;
    leds[posVal] = CHSV( colorVal, 255, 255 );
  }

  blur1d( leds, NUM_LEDS, 64 );

  colorVal += colorInc;
}

// uses: colorVal, colorInc
void Animation::aConfetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10 );

  colorVal += colorInc;
  posVal = random8(NUM_LEDS);

  leds[posVal] += CHSV( colorVal, 200, 255 );
}

// uses: colorVal, colorInc, posInc
void Animation::aCylon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, abs(2*posInc) );

  colorVal += colorInc;
  //  posVal = beatsin8(13, 0, NUM_LEDS); // see: lib8tion.h
  posVal = beatsin8(abs(posInc), 0, NUM_LEDS); // see: lib8tion.h

  leds[posVal] = CHSV( colorVal, 255, 255 );
}

// uses: colorVal, colorInc
void Animation::aBPM() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  const uint8_t BeatsPerMinute = 120;
  const CRGBPalette16 palette = PartyColors_p;

  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);

  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, colorVal+(i*2), beat-colorVal+(i*10));
  }

  colorVal += colorInc;
}

// uses: colorVal
void Animation::aJuggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);

  //  colorVal = 0;

  for( int i = 0; i < 8; i++) {
    int l = beatsin16(i+7,0,NUM_LEDS);
    leds[l] |= CHSV(colorVal, 200, 255);
    colorVal += 32;
  }
}

// uses: colorVal (as brightness), colorInc
void Animation::aWhite() {
  // FastLED's built-in solid fill (colorutils.h)
  fill_solid( leds, NUM_LEDS, CHSV(0, 0, colorVal) );
  colorVal += colorInc;
}

// uses: colorVal, colorInc
void Animation::aFire() {
  
  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;
  
  // COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100 
#define COOLING  55
//#define COOLING  20

// 54 W with a lot of fire: 10A
// 36 W with these settings: 7.2A

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
//#define SPARKING 50
//#define SPARKING 200

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if( random8() < chanceAct ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160,255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if( gReverseDirection ) {
      pixelnumber = (NUM_LEDS-1) - j;
    } 
    else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }

}

void Animation::aCenterFire(byte center, byte extent) {

  const CRGBPalette16 gPal = HeatColors_p;
  const bool gReverseDirection = false;

  byte leftPos = constrain(center, 0, NUM_LEDS - 1);          //constrain(this->currentPos, 0, NUM_LEDS - 1);
  byte rightPos = constrain(center, 0, NUM_LEDS - 1);  //constrain(this->currentPos + 1, 0, NUM_LEDS - 1);

  byte cooling = map(this->currentExtent, 0, 192, 16, 8);

  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  // Less cooling = taller flames.  More cooling = shorter flames.
  for ( int k = 0; k < NUM_LEDS; k++) {
    heat[k] = qsub8( heat[k],  random8(0, cooling));
  }

  // Step 2.  Heat from each cell drifts left and right and diffuses a little
  for ( int k = 0; k <= (int)leftPos - 2; k++) {
    heat[k] = (heat[k + 1] + heat[k + 2] + heat[k + 3] ) / 3;
  }
  for ( int k = NUM_LEDS - 1; k >= (int)rightPos - 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 3] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near center position
  if ( random8() < 32 ) {
    int y = random(16);

    heat[leftPos - y] = qadd8( heat[leftPos], map(this->currentExtent,0,128,64,64) );
    heat[rightPos + y] = qadd8( heat[rightPos], map(this->currentExtent,0,128,64,64) );
  }

  // Step 3.5. Randomly create more central sparks
  if ( random8() < 64 ) {
    heat[leftPos] = qadd8( heat[leftPos], map(this->currentExtent,0,192,128,128) );
    heat[rightPos] = qadd8( heat[rightPos], map(this->currentExtent,0,192,128,128) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    CRGB color = ColorFromPalette( gPal, scale8( heat[j]+2, 240));
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    }
    else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

// uses: posVal, posInc, colorVal
void Animation::aTestPattern() {
  // zap the strips
  FastLED.clear();

  // cycle through the LED actuals.
  switch( colorVal ) {
    case HUE_RED: colorVal = HUE_GREEN; break;
    case HUE_GREEN: colorVal = HUE_BLUE; break;
    default: colorVal = HUE_RED; break;
  }
  
  // just look at start of strip and end of strip.
  switch( posVal ) {
    case 0: posVal = NUM_LEDS-1; break;
    default: posVal = 0; break;
  }
  
  leds[posVal] = CHSV( colorVal, 255, 255 );
}

void Animation::aClear() {
  FastLED.clear();
  fill_solid( leds, NUM_LEDS, CRGB::Black );
}

// uses: colorVal
void Animation::aRGB() {
  brightnessSet(this->brightVal);
  fill_solid( leds, NUM_LEDS, this->targetColor );
}

// uses: colorVal
void Animation::aHue() {
  brightnessSet(this->brightVal);
  fill_solid( leds, NUM_LEDS, CHSV(this->targetColor, 255, 255) );
}

Animation A;