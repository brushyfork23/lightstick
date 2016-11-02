// Include libraries for serial, eeprom, and radio
#include <Streaming.h>
#include <Metro.h>
#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <FiniteStateMachine.h>

#include <Network.h>
#include <Animation.h>

#define INSTRUCTIONS "Commands: 1=Drop Freq, 2=Raise Freq, 3=Drop Vol, 4=Raise Vol"
#define COLOR_INSTRUCTIONS "Colors: 1=Red, 2=Orange, 3=Yellow, 4=Green, 5=Aqua, 6=Blue, 7=Purple, 8=Pink, 9=White"
enum
{
  // Commands
  kDropFreq=1  ,
  kRaiseFreq   ,
  kDropVol     ,
  kRaiseVol    ,
};

byte inVal;

uint8_t lastAnim, lastColor = 0;

boolean waitingForColor = false;

// Controller sends commands to either Audio node or LightStick nodes.
// Audio node receives instructions on tuning volume and frequency level.
// LightStick nodes receive instructions of which animation to display.
// light <--> audio

/* Prototypes for state methods */
void audioEnter(), audioUpdate(), audioExit();
void lightEnter(), lightUpdate(), lightExit();

State audio = State(audioEnter, audioUpdate, audioExit);

State light = State(lightEnter, lightUpdate, lightExit);

FSM controller = FSM(light); //initialize state machine, start in state: light

//***** Audio Commands
void audioEnter() {
  Serial << F("LightStick: ->audio") << endl;
  printInstructions();
}
void audioUpdate() {
  if( Serial.available() > 0 ) {
    inVal = Serial.parseInt();
    Serial << F("Reading new value: ") << inVal << endl;
    switch(inVal) {
      case 0:
        controller.transitionTo(light);
        break;
      case kDropFreq:
        Serial << F("Instructing audio node to lower trigger frequency.") << endl;
        N.input = M_LOWER_FREQ;
        break;
      case kRaiseFreq:
        Serial << F("Instructing audio node to raise trigger frequency.") << endl;
        N.input = M_RAISE_FREQ;
        break;
      case kDropVol:
        Serial << F("Instructing audio node to lower volume.") << endl;
        N.input = M_LOWER_VOL;
        break;
      case kRaiseVol:
        Serial << F("Instructing audio node to raise volume.") << endl;
        N.input = M_RAISE_VOL;
        break;
    }
    // Send new command to audio node
    N.encodeMessage();
    N.sendMessage(AUDIO_NODE);

    printInstructions();
  }
}
void audioExit() {
}

//***** LightStick Commands
void lightEnter() {
  Serial << F("LightStick: ->light") << endl;
  printColors();
}
void lightUpdate() {
  if( Serial.available() > 0 ) {
    inVal = Serial.parseInt();
    Serial << F("Reading new value: ") << inVal << endl;
    switch (inVal) {
      case 0:
        controller.transitionTo(audio);
        break;
      case 1:
        toggleHue(HUE_RED);
        break;
      case 2:
        toggleHue(HUE_ORANGE);
        break;
      case 3:
        toggleHue(HUE_YELLOW);
        break;
      case 4:
        toggleHue(HUE_GREEN);
        break;
      case 5:
        toggleHue(HUE_AQUA);
        break;
      case 6:
        toggleHue(HUE_BLUE);
        break;
      case 7:
        toggleHue(HUE_PURPLE);
        break;
      case 8:
        toggleHue(HUE_PINK);
        break;
      case 9:
        toggleRGB(CRGB::White);
        break;
    }

    N.encodeMessage();
    N.broadcastMessage();

    printColors();
  }
}
void lightExit() {
}

void toggleHue(uint8_t hue) {
  if (lastAnim == A_HUE
    && lastColor == hue) {
    // This hue is being toggled off
    N.animation = A_CLEAR;
    Serial << F("Clearing") << endl;
  } else {
    N.animation = A_HUE;
    N.input = hue;
  }
}
void toggleRGB(uint8_t rgb) {
  if (lastAnim == A_RGB
    && lastColor == rgb) {
    // This color is being toggled off
    N.animation = A_CLEAR;
    Serial << F("Clearing") << endl;
  } else {
    N.animation = A_RGB;
    N.input = rgb;
  }
}

void setup() {
  delay(500);

  Serial.begin(115200);

  // Establish Control NodeId and start radio
  N.begin(CONTROLLER_NODE);
 
  N.animation = A_CLEAR;
  N.input = 0;

  Serial.println("Starting Controller node.");
  printInstructions();
}

void loop() {
  controller.update();
}

void printInstructions() {
  Serial << F(INSTRUCTIONS) << endl;
}
void printColors() {
  Serial << F(COLOR_INSTRUCTIONS) << endl;
}
