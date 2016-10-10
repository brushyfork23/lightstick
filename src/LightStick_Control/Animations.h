#ifndef Animations_h
#define Animations_h

#include <Arduino.h>

#include <Streaming.h>

#define PIN_DATA1      4
#define PIN_DATA2      7
#define PIN_CLK        6
#define LED_TYPE       APA102
#define NUM_STRIPS     2
#define NUM_LEDS       110

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
  A_PULSE=0,
  A_STABLE,
  A_SOLID,
  A_RAINBOW,
  A_GLITTER,
  A_CONFETTI,
  A_CYLON,
  A_BPM,
  A_JUGGLE,
  A_WHITE,// really a test pattern for maximal ampere draw estimation
  A_FIRE, // simulated fire colors
  A_TESTPATTERN, // test pattern to see if the LEDs are ok
  
  N_ANIMATIONS
};

/*
LED Colors:
red
orange
yellow
green
aqua
blue
purple
pink
*/

class Animation {
  public:
    // initialize led strips
    void begin();
    // which calls the following functions with their defaults:
    // set frames per second
    void setFPS(uint16_t framesPerSecond=30);
    // set master brightness
    void setMasterBrightness(byte masterBrightness=255);

    // animations control
    // sets the animation 
    void startAnimation(byte animation=A_STABLE, boolean clearStrip=true); 
    // sets the hue start and increment for animation
    void startHue(byte hue=160); // default blue
    void hueIncrement(int inc=1);
    void hueTarget(byte hue=160);
    // moves hueVal to targetHue by hueInc
    void stepHue();
    // sets the led start and increment for animation
    void startPosition(byte pos=0);
    void incrementPosition(int inc=1);
    // set the random seed for animation
    void startSeed(uint16_t seed = 1337);
    // set the fractional chance of animation activity (e.g "sparking")
    void setActivity(fract8 chance=80);
    
    // runs the animation
    void update();
    
  private:
    byte anim, hueVal, targetHue, targetBright, posVal;   
    int hueInc, posInc;
    fract8 chanceAct;
    
    Metro pushNextFrame;
    
    void aPulse();
    void aStable();
    void aSolid();
    void aRainbow();
    void aGlitter();
    void aConfetti();
    void aCylon();
    void aBPM();
    void aJuggle();
    void aWhite();
    void aFire();
    void aTestPattern();
};

extern Animation A;

#endif
