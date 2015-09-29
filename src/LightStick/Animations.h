#ifndef Animations_h
#define Animations_h

#include <Arduino.h>

#define PIN_DATA1      4
#define PIN_DATA2      7
#define PIN_CLK        6
#define LED_TYPE       APA102
#define NUM_STRIPS     2
#define NUM_LEDS       110
#define MASTER_BRIGHTNESS  255

#include <Metro.h>
#include <FastLED.h>

// at 60mA per pixel on white * 220 pixels, 13.2A @ 5VDC.
// by ammeter, I get:
// 6A @ 5VDC (White)
// 3.4A (rainbow)
// 1A sparse animations

// 4A supply seems about right




FASTLED_USING_NAMESPACE

// enumerate animation modes
enum animation_t {
  A_SOLID=0,
  A_RAINBOW,
  A_GLITTER,
  A_CONFETTI,
  A_CYLON,
  A_BPM,
  A_JUGGLE,
  
  N_ANIMATIONS
};

class Animation {
  public:
    // startup
    void begin();
    // sets the animation 
    void setAnimation(byte animation, CHSV startHSV, uint16_t startPos, unsigned long framesPerSecond); 
    // runs the animation
    void runAnimation();
    
  private:
    byte currentAnimation;
    
    CHSV currentHSV;
    uint16_t currentPos;
    
    Metro pushNextFrame;
    
    void aSolid();
    void aRainbow();
    void aGlitter();
    void aConfetti();
    void aCylon();
    void aBPM();
    void aJuggle();
};

extern Animation anim;

#endif
