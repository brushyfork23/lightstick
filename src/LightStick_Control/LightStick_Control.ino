#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <FastLED.h>
#include <FiniteStateMachine.h>

#include "Radio.h"

#include "Animations.h"

enum
{
  // Commands
  kAudio=0     ,
  kColorFast   ,
  kColorSlow   ,
};

byte program=kAudio;

// Stick is in pulse state when no radio activity is detected.  This way if connectivity is lost, the stick does not freeze.
// Transition from react to pulse when radio has been inactive for a set length of time.
// Transition from pulse to react when a radio packet is received.
// pulse <-> react
/*
State pulse = State(pulseEnter, pulseUpdate, pulseExit);

FSM lightstick = FSM(pulse); //initialize state machine, start in state: pulse

//***** Pulse
void pulseEnter() {
  Serial << F("LightStick: ->pulse") << endl;

  // Set target color and brightness
  A.targetHue(160); // blue

  // Set animation to pulse
  A.startAnimation(A_Pulse, false);
}
void pulseUpdate() {
  A.update();
  light.animate(A_Idle);
  // check buttons for game play start
  if ( touch.anyButtonPressed() ) {
    // going to start a game
    Serial << F("LightStick: pulse->audio") << endl;

    sound.stopAll();
    sound.setLeveling(1, 1); // 1x tone and 1x track
    //rockTrack = sound.playRock(501);

    // let's play a game
    simon.transitionTo(game);
  } else if ( idleBeforeFanfare.check() ) {

    // do some fanfare
    fanfareLevel = IDLE;
    simon.transitionTo(fanfare);
  } else if ( sensor.modeChange() ) {
    // run tests
    simon.transitionTo(test);
  }

  scoreboard.showBackerMessages();
  scoreboard.showSimonTeam();
}
void pulseExit() {
}
*/

/* Prototypes for state methods */
void audioEnter(), audioUpdate(), audioExit();
void pulseEnter(), pulseUpdate(), pulseExit();

State audio = State(audioEnter, audioUpdate, audioExit);

State pulse = State(pulseEnter, pulseUpdate, pulseExit);

FSM lightstick = FSM(pulse); //initialize state machine, start in state: audio

// Ensure a stable radio connection before transitioning to audio state
Metro radioStable(200UL);

// If radio contact is lost, transition to a non-radio-dependent state
Metro radioTimeout(200UL);

//***** Audio Reactive
void audioEnter() {
  Serial << F("LightStick: ->audio") << endl;
}
void audioUpdate() {
  A.update();
  R.update();

  // check radio for new transmission
  if (R.hasUnorocessedPayload ) {
    R.hasUnorocessedPayload = false;
    radioTimeout.reset();

    A.hueTarget(R.hue);
    A.brightnessSet(R.bright);

    // check for new directive
    if (R.pgm != program) {
      program = R.pgm;
      switch(program) {
        case kColorFast:
          Serial << F("Fast Color Change") << endl;
          A.hueIncrement(20);
          break;
        case kColorSlow:
          Serial << F("Slow Color Change") << endl;
          A.hueIncrement(1);
          break;
      }
    }
  } else {
    // if it has been too long since last communication, transition to pulse state
    if (radioTimeout.check()) {
      radioStable.reset();
      Serial << F("Lost radio contact.") << endl;
      lightstick.transitionTo(pulse);
    }
  }
}
void audioExit() {
}

//***** Pulse
void pulseEnter() {
  Serial << F("LightStick: ->pulse") << endl;

  // Setup for quick transition to dim brightness
  A.brightnessTarget(BRIGHT_LOW);
  A.brightnessInc(10);

  // Set animation to pulse but do not clear current settings
  A.startAnimation(A_PULSE, false);
}
void pulseUpdate() {
  A.update();
  R.update();

  // check radio for new transmission
  if (R.hasUnorocessedPayload ) {
    R.hasUnorocessedPayload = false;

    // ensure connection is stable
    if (radioStable.check()) {
      radioTimeout.reset();
      Serial << F("Sustained radio contact aquired.") << endl;
      lightstick.transitionTo(audio);
    }
  } else {
    radioStable.reset();
  }
}
void pulseExit() {
}

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);

  // Establish My NodeId and start radio
  R.begin();

  // startup animation
  A.begin();
}


void loop() {
  lightstick.update();
  /*
  // if availalbe, record a new radio payload
  R.update();

  // Update program if new
  if (R.hasUnorocessedPayload && R.pgm != program) {
    Serial << F("Unprocessed Payload: ");
    program = R.pgm;
    switch(program) {
      case kAudio:
        Serial << F("Audio") << endl;
        break;
      case kColorFast:
        Serial << F("Fast Color Change") << endl;
        A.hueIncrement(20);
        break;
      case kColorSlow:
        Serial << F("Slow Color Change") << endl;
        A.hueIncrement(1);
        break;
    }
  }

  if (R.hasUnorocessedPayload) {
    A.hueTarget(R.hue);
    A.setMasterBrightness(R.bright);
  }
  
  A.update();

  R.hasUnorocessedPayload = false;
  */
}



