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

#include <Network.h>
#include <Animation.h>

uint8_t anim = A_CLEAR;

// Stick is in idle state when no radio activity is detected.  This way if connectivity is lost, the stick does not freeze.
// Transition from radio to idle when radio has been inactive for a set length of time.
// Transition from idle to radio when a few radio packets have been received.
// idle <-> radio

/* Prototypes for state methods */
void radioEnter(), radioUpdate(), radioExit();
void idleEnter(), idleUpdate(), idleExit();

State radio = State(radioEnter, radioUpdate, radioExit);

State idle = State(idleEnter, idleUpdate, idleExit);

FSM lightstick = FSM(idle); //initialize state machine, start in state: idle

// Ensure a stable radio connection before transitioning to radio state
Metro radioStable(250UL);

// If radio contact is lost, transition to a non-radio-dependent state
Metro radioTimeout(1500UL);

//***** Radio Controlled
void radioEnter() {
  Serial << F("LightStick: ->radio") << endl;
  radioTimeout.reset();
}
void radioUpdate() {
  // check Network for new radio message
  if ( N.update()
    && N.targetNodeID == BROADCAST
  ) {
    radioTimeout.reset();
    N.decodeMessage(); // decode the message to instructions
    switch (N.animation) {
      case A_CENTERFIRE:
        if (anim != N.animation) {
          A.startAnimation(A_CENTERFIRE, true);
          A.brightnessSet(255);
        }
        break;
      case A_CLEAR:
        A.startAnimation(A_CLEAR, true);
        break;
      case A_RGB:
        if (anim != N.animation) {
          A.startAnimation(A_RGB, true);
        }
        A.colorTarget(N.input);
        A.brightnessSet(N.volume);
        break;
      case A_HUE:
        if (anim != N.animation) {
          A.startAnimation(A_HUE, true);
        }
        A.colorTarget(N.input);
        A.brightnessSet(N.volume);
        break;
      default:
        if (anim != N.animation) {
          A.startAnimation(A_STABLE, true);
        }
        A.colorTarget(N.input);
        A.brightnessSet(N.volume);
        break;
    }
    anim = N.animation;
  } else {
    // if it has been too long since last communication, transition to idle state
    if (radioTimeout.check()) {
      Serial << F("Lost radio contact.") << endl;
      lightstick.transitionTo(idle);
    }
  }
  
  A.update();
}
void radioExit() {
}

//***** Idle
void idleEnter() {
  Serial << F("LightStick: ->idle") << endl;
  A.startAnimation(A_CLEAR, true);
  A.update();

  radioStable.reset();  
}
void idleUpdate() {
  // check radio for new transmission
  if ( N.update() ) {
    radioTimeout.reset();
    // ensure connection is stable
    if (radioStable.check()) {
      Serial << F("Sustained radio contact aquired.") << endl;
      lightstick.transitionTo(radio);
    }
  } else {
    if (radioTimeout.check()) {
      radioStable.reset();
    }
  }
}
void idleExit() { 
}

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);

  // establish NodeId and start radio
  N.begin();

  // startup animation
  A.begin();
}


void loop() {
  lightstick.update();
}