#ifndef LightStick_Animation_h
#define LightStick_Animation_h

#include <Arduino.h>

#include <Streaming.h>

#define PIN_DATA1      4
#define PIN_DATA2      7
#define PIN_CLK        6
#define LED_TYPE       APA102
#define NUM_STRIPS     2
#define NUM_LEDS       110

#define COLOR_START      160

#define BRIGHT_LOW     5
#define BRIGHT_HIGH    110
#define BRIGHT_START   BRIGHT_LOW

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
  A_PULSE=1,
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
  A_CENTERFIRE, // fire that radiates from a center
  A_TESTPATTERN, // test pattern to see if the LEDs are ok
  A_CLEAR,
  A_RGB,
  A_HUE,
  
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

class LightStick_Animation {
  public:
    // initialize led strips
    void begin();
    // which calls the following functions with their defaults:
    // set frames per second
    void setFPS(uint16_t framesPerSecond=30);
    // set master brightness
    void brightnessSet(byte brightness=BRIGHT_START);
    // set brightness increment
    void brightnessInc(uint8_t inc=1);
    // set target brightness
    void brightnessTarget(byte brightness=BRIGHT_START);
    // moves brightVal to targetBright by brightInc
    void brightnessStep();
    // animations control
    // sets the animation 
    void startAnimation(byte animation=A_STABLE, boolean clearStrip=true); 
    // sets the color start and increment for animation
    void colorSet(byte color=COLOR_START); // default blue
    void colorIncrement(int inc=6);
    void colorTarget(byte color=COLOR_START);
    // moves colorVal to targetColor by colorInc
    void colorStep();
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
    byte anim, colorVal=COLOR_START, targetColor=COLOR_START, brightVal=BRIGHT_START, targetBright=BRIGHT_START, posVal;   
    int colorInc, posInc, brightInc;
    fract8 chanceAct;
    
    byte position, extent;
    byte currentPosition, currentExtent;
    
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
    void aCenterFire(byte pos, byte extent);
    void aTestPattern();
    void aClear();
    void aRGB();
    void aHue();
};

extern LightStick_Animation A;

#endif
