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

#include <LightStick_Network.h>
#include <LightStick_Animation.h>

uint8_t curAnim = A_CLEAR;

// Stick is in idle state when no audio node activity is detected.  This way if connectivity is lost, the stick does not freeze.
// Transition from audio to idle when audio node has been inactive for a set length of time.
// Transition from idle to audio when a few audio packets have been received.
// idle <-> audio

/* Prototypes for state methods */
void audioEnter(), audioUpdate(), audioExit();
void idleEnter(), idleUpdate(), idleExit();

State audio = State(audioEnter, audioUpdate, audioExit);

State idle = State(idleEnter, idleUpdate, idleExit);

FSM lightstick = FSM(idle); //initialize state machine, start in state: idle

// Ensure a stable audio connection before transitioning to audio state
Metro audioStable(250UL);

// If audio contact is lost, transition to a non-audio-dependent state
Metro audioTimeout(1500UL);

//***** Audio Controlled
void audioEnter() {
  Serial << F("LightStick: ->audio") << endl;
  audioTimeout.reset();
}
void audioUpdate() {
  // check Network for new radio message
  bool radioUpdated = N.update();

  // if it has been too long since last audio transmission, transition to idle state
  if (!radioUpdated || N.senderNodeID != AUDIO_NODE) {
    if (audioTimeout.check()) {
      Serial << F("Lost audio contact.") << endl;
      lightstick.transitionTo(idle);
    }
  }

  // handle new audio or animation instructions
  if ( radioUpdated ) {
    N.decodeMessage(); // decode the message to instructions

    if (N.senderNodeID == AUDIO_NODE) {
      audioTimeout.reset();

      // Audio node sends volume; use to update brightness
      switch (curAnim) {
        case A_HUE:
        case A_RGB:
        case A_STABLE:
          A.brightnessSet(N.volume);
          break;
      }
    } else if (N.senderNodeID == CONTROLLER_NODE) {
      handleControllerMessage(N.animation, N.input);
    }
  }
  
  A.update();
}
void audioExit() {
}

//***** Idle
void idleEnter() {
  Serial << F("LightStick: ->idle") << endl;
  A.startAnimation(A_PULSE, true);

  audioStable.reset();  
}
void idleUpdate() {
  A.update();

  // check Network for new radio message
  bool radioUpdated = N.update();

  // when a audio packet is received, check that it is sustained
  if (radioUpdated && N.senderNodeID == AUDIO_NODE) {
    audioTimeout.reset(); // debouncer for stable audio
    if (audioStable.check()) {
      // audio transmissions are stable, transition to audio state
      Serial << F("Sustained audio contact aquired.") << endl;
      lightstick.transitionTo(audio);
    }
  }

  if (radioUpdated && N.senderNodeID == CONTROLLER_NODE) {
    N.decodeMessage();
    handleControllerMessage(N.animation, N.input);
  }

  // consult the audio signal debouncer
  if (audioTimeout.check()) {
    audioStable.reset();
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


void handleControllerMessage(word anim, word input) {
  // Controller node sends animation instructions; transition to new anim
  switch (anim) {
    case A_CENTERFIRE:
      if (curAnim != anim) {
        A.startAnimation(anim, true);
        A.brightnessSet(255);
      }
      break;
    case A_DRAGONTEARS:
      if (curAnim != anim) {
        A.startAnimation(anim, true);
        A.brightnessSet(255);
      }
      break;
    case A_CLEAR:
      A.startAnimation(A_CLEAR, true);
      break;
    default:
      if (curAnim != anim) {
        A.startAnimation(anim, true);
      }
      A.colorTarget(input);
      break;
  }
  curAnim = N.animation;
}


void loop() {
  lightstick.update();
}