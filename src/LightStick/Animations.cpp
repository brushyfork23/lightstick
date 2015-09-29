#include "Animations.h"

// see https://github.com/FastLED/FastLED/wiki

FASTLED_USING_NAMESPACE

CRGB leds[NUM_STRIPS][NUM_LEDS];

// startup
void Animation::begin() {
  this->currentAnimation = N_ANIMATIONS; // none
     
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<APA102, PIN_DATA1, PIN_CLK>(leds[0], NUM_LEDS);
  FastLED.addLeds<APA102, PIN_DATA2, PIN_CLK>(leds[1], NUM_LEDS);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

// sets the animation 
void Animation::setAnimation(byte animation, CHSV startHSV, uint16_t startPos, unsigned long framesPerSecond) {
  this->currentAnimation = animation;
  this->currentHSV = startHSV;
  this->currentPos = startPos;
  
  this->pushNextFrame.interval(1000UL/framesPerSecond);
  
  FastLED.clear();
}

// runs the animation
void Animation::runAnimation() {

  // pre-calculate the next frame
  static boolean nextFrameReady = false;
  if( ! nextFrameReady ) {
    switch( this->currentAnimation ) {
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
 
void Animation::aSolid() {
  // FastLED's built-in solid fill (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_solid( leds[s], NUM_LEDS, this->currentHSV );
  }
  this->currentHSV.hue++;
}

void Animation::aRainbow() {
  // FastLED's built-in rainbow generator (colorutils.h)
  for(int s = 0; s < NUM_STRIPS; s++) {
    fill_rainbow( leds[s], NUM_LEDS, this->currentHSV.hue, 7 );
  }
  this->currentHSV.hue++;
}

void Animation::aGlitter() {
  FastLED.clear();

  fract8 chanceOfGlitter = 80;
  if( random8() < chanceOfGlitter) {
    int l = random16(NUM_LEDS);
    leds[0][l] = this->currentHSV;
    leds[1][l] = leds[0][l];
  }

  for(int s = 0; s < NUM_STRIPS; s++) {
    blur1d( leds[s], NUM_LEDS, 64 );
  }
}

void Animation::aConfetti() {
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds[0], NUM_LEDS, 10);
  fadeToBlackBy( leds[1], NUM_LEDS, 10);

  this->currentHSV = CHSV( this->currentHSV.hue + random8(64), 200, 255 );
  this->currentPos = random16(NUM_LEDS);
  
  leds[0][this->currentPos] += this->currentHSV;
  leds[1][this->currentPos] = leds[0][this->currentPos];
}

void Animation::aCylon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);

  this->currentHSV = CHSV( ++this->currentHSV.hue, 255, 192 );
  this->currentPos = beatsin16(13,0,NUM_LEDS);
  
  leds[0][this->currentPos] += this->currentHSV;
  leds[1][this->currentPos] = leds[0][this->currentPos];
}

void Animation::aBPM() {
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[0][i] = ColorFromPalette(palette, this->currentHSV.hue+(i*2), beat-this->currentHSV.hue+(i*10));
    leds[1][i] = leds[0][i];
  }
  this->currentHSV.hue++;
}


void Animation::aJuggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds[0], NUM_LEDS, 20);
  fadeToBlackBy( leds[1], NUM_LEDS, 20);
  
  this->currentHSV.hue = 0;
  
  for( int i = 0; i < 8; i++) {
    int l = beatsin16(i+7,0,NUM_LEDS);
    leds[0][l] |= CHSV(this->currentHSV.hue, 200, 255);
    leds[1][l] = leds[0][l];
    this->currentHSV.hue += 32;
  }
}

Animation anim;


