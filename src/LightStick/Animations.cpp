#include "Animations.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_STRIPS][NUM_LEDS];

// startup
void Animation::begin() {
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds[0], NUM_LEDS);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds[1], NUM_LEDS);

  this->setFPS();
  this->setMasterBrightness();
  
  this->startAnimation();
  
  this->startHue();
  this->incrementHue();  
  
  this->startPosition();
  this->incrementPosition();
  
  this->startSeed();
  
  Serial << F("Animation. Startup complete.") << endl;
}
// sets FPS
void Animation::setFPS(uint16_t framesPerSecond) {
  this->pushNextFrame.interval(1000UL/framesPerSecond);
  Serial << F("FPS= ") << framesPerSecond << F(". show update=") << 1000UL/framesPerSecond << F(" ms.") << endl;
}

// sets master brightness
void Animation::setMasterBrightness(byte masterBrightness) {
  // set master brightness control
  FastLED.setBrightness(masterBrightness); 
  Serial << F("Master brightness= ") << masterBrightness << endl;
}

// sets the animation 
void Animation::startAnimation(byte animation, boolean clearStrip) {
  this->anim = animation % N_ANIMATIONS;
  if( clearStrip ) FastLED.clear();

  Serial << F("animation=") << this->anim << endl;
    
}
void Animation::startHue(byte hue) {
  this->hueVal = hue % 255;
  Serial << F("hue start=") << this->hueVal << endl;
}
void Animation::incrementHue(int inc) {
  this->hueInc = inc;
  Serial << F("hue increment=") << this->hueInc << endl;
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

// runs the animation
void Animation::runAnimation() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if( ! nextFrameReady ) {
    switch( anim ) {
      case A_SOLID: aSolid(); break;
      case A_RAINBOW: aRainbow(); break;
      case A_GLITTER: aGlitter(); break;
      case A_CONFETTI: aConfetti(); break;
      case A_CYLON: aCylon(); break;
      case A_BPM: aBPM(); break;
      case A_JUGGLE: aJuggle(); break;
      default: break;
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

// uses: hueVal, hueInc
void Animation::aSolid() {
  // FastLED's built-in solid fill (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_solid( leds[s], NUM_LEDS, CHSV(hueVal, 255, 255) );
  }
  hueVal += hueInc;
}

// uses: hueVal, hueInc
void Animation::aRainbow() {
  // FastLED's built-in rainbow generator (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_rainbow( leds[s], NUM_LEDS, hueVal );
  }
  hueVal += hueInc;
}

// uses: hueVal, hueInc, posVal
void Animation::aGlitter() {
  fract8 chanceOfGlitter = 80;
  if( random8() < chanceOfGlitter) {
    posVal += random8(NUM_LEDS);
    posVal %= NUM_LEDS;
    leds[0][posVal] = CHSV( hueVal, 255, 255 );
    leds[1][posVal] = leds[0][posVal];
  }

  for(int s = 0; s < NUM_STRIPS; s++) {
    blur1d( leds[s], NUM_LEDS, 64 );
  }
  
  hueVal += hueInc;
}

// uses: hueVal, hueInc
void Animation::aConfetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds[0], NUM_LEDS, 10 );
  fadeToBlackBy( leds[1], NUM_LEDS, 10 );

  hueVal += hueInc;
  posVal = random8(NUM_LEDS);
  
  leds[0][posVal] += CHSV( hueVal, 200, 255 );
  leds[1][posVal] = leds[0][posVal];
}

// uses: hueVal, hueInc, posInc
void Animation::aCylon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS, abs(2*posInc) );
  fadeToBlackBy( leds[1], NUM_LEDS, abs(2*posInc) );

  hueVal += hueInc;
//  posVal = beatsin8(13, 0, NUM_LEDS); // see: lib8tion.h
  posVal = beatsin8(abs(posInc), 0, NUM_LEDS); // see: lib8tion.h
  
  leds[0][posVal] = CHSV( hueVal, 255, 255 );
  leds[1][posVal] = leds[0][posVal];
}

// uses: hueVal, hueInc
void Animation::aBPM() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  const uint8_t BeatsPerMinute = 120;
  const CRGBPalette16 palette = PartyColors_p;
  
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[0][i] = ColorFromPalette(palette, hueVal+(i*2), beat-hueVal+(i*10));
    leds[1][i] = leds[0][i];
  }
  
  hueVal += hueInc;
}

// uses: hueVal
void Animation::aJuggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);
  
//  hueVal = 0;
  
  for( int i = 0; i < 8; i++) {
    int l = beatsin16(i+7,0,NUM_LEDS);
    leds[0][l] |= CHSV(hueVal, 200, 255);
    leds[1][l] = leds[0][l];
    hueVal += 32;
  }
}

Animation A;


