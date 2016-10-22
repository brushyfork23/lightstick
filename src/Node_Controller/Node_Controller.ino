// Include libraries for serial, eeprom, and radio
#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

#include <Network.h>
#include <Animation.h>

#define INSTRUCTIONS "Commands: 1=Drop Freq, 2=Raise Freq, 3=Drop Vol, 4=Raise Vol"
enum
{
  // Commands
  kDropFreq=1  ,
  kRaiseFreq   ,
  kDropVol     ,
  kRaiseVol    ,
};

byte inVal;

uint8_t triggerBand = 4; // audio band to monitor volume on
uint16_t squash = 100;  // volume to subtract.  Dynamically drop when lights are too bright or never go completely dark.

boolean waitingForColor = false;

boolean cmdMode = false;

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
  if( Serial.available() > 0 ) {
    inVal = Serial.parseInt();
    Serial << F("Reading new value: ") << inVal << endl;
    if (inVal == 0) {
      cmdMode = !cmdMode;
      if (cmdMode) {
        Serial << F("Entering command mode.") << endl;
        printInstructions();
      } else {
        Serial << F("Entering color mode.  Enter new color value (1 - 255):") << endl;
      }
    }
    if (cmdMode) {
      switch(inVal) {
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
    } else {
      N.color = inVal;
      Serial << F("Setting color: ") << inVal << endl;

      // send new color value to all nodes
      N.encodeMessage();
      N.broadcastMessage();

      Serial << F("Enter new color value (1 - 255):") << endl;
    }
  }
}

void printInstructions() {
  Serial << F(INSTRUCTIONS) << endl;
}
