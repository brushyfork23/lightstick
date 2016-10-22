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

//------ Input units.
#include "Mic.h" // Microphone

uint8_t triggerBand = 4; // audio band to monitor volume on
uint16_t squash = 100;  // volume to subtract.  Dynamically drop when lights are too bright or never go completely dark.

#define SAMPLES   60                                          // Length of buffer for dynamic level adjustment
#define TOP       110                                         // Allow peak to be slightly off scale

#define DELAYED_FALL false                                    // Toggle delayed height falling
#define FALL_RATE 20                                          // Rate height falling

byte
  volCount    = 0,                                            // Frame counter for storing past volume data
  height      = 0,                                            // Used for falling height
  delayFallCount = 0;                                         // Frame counter for delaying height falling speed
int
  vol[SAMPLES],                                               // Collection of prior volume samples
  lvl       = 10,                                             // Current "dampened" audio level
  minLvlAvg = 0,                                              // For dynamic adjustment of graph low & high
  maxLvlAvg = 1023;

uint8_t amplitude = 0;                                        // Current intensity of audio

void setup() {
  delay(500);

  Serial.begin(115200);

  //------ Input units.
  listenLine.begin(MIC_RESET_PIN, MIC_STROBE_PIN, MIC_OUT_PIN); // only listens to line in
  
  // Establish Control NodeId and start radio
  N.begin(AUDIO_NODE);
 
  N.volume = 100;

  Serial.println("Starting Audio Node.");
}

void loop() {
  // Check for instructions to change squash or frequency
  if ( N.update()
    && N.senderNodeID == CONTROLLER_NODE
    && N.targetNodeID == AUDIO_NODE
  ) {
    N.decodeMessage();
    switch (N.input) {
      case M_RAISE_VOL:
        if (squash >= 25) {
            squash = squash - 25;
            Serial << F("Dropping squash to: ") << squash << endl;
          } else {
            Serial << F("Squash at floor") << endl;
          }
        break;
      case M_LOWER_VOL:
        if (squash < 1000) {
          squash = squash + 25;
          Serial << F("Raising squash to: ") << squash << endl;
        } else {
          Serial << F("Squash at ceiling") << endl;
        }
        break;
      case M_RAISE_FREQ:
        if (triggerBand < 6) {
          triggerBand++;
          Serial << F("Raising trigger band to: ") << triggerBand << endl;
        } else {
          Serial << F("Trigger band at ceiling") << endl;
        }
        break;
      case M_LOWER_FREQ:
        if (triggerBand > 0) {
          triggerBand--;
          Serial << F("Dropping trigger band to: ") << triggerBand << endl;
        } else {
          Serial << F("Trigger band at floor") << endl;
        }
        break;
    }
  }

  // Read volume and broadcast
  updateAmplitude();
  N.volume = amplitude;

  N.encodeMessage();
  N.broadcastMessage();
}

void updateAmplitude() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
  
  listenLine.update();
  int n = listenLine.getVol(triggerBand);
  //Serial << F("line in: ") << n << endl;
  n = (n <= squash) ? 0 : (n - squash);                         // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;                                 // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  amplitude = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);
  //amplitude = map(lvl, 0, TOP, 0, 255); 
  //amplitude = map(lvl, minLvlAvg, maxLvlAvg, 0, 255); 

  //Serial << F("n, l, amp: ") << n << ", " << lvl << ", " << amplitude << endl;
  
  if (amplitude < 0L)       amplitude = 0;                          // Clip output
  else if (amplitude > TOP) amplitude = TOP;
  
  vol[volCount] = n;                                          // Save sample for dynamic leveling
  if (++volCount >= SAMPLES) volCount = 0;                    // Advance/rollover sample counter
 
  // Get volume range of prior frames
  minLvl = maxLvl = vol[0];
  for (i=1; i<SAMPLES; i++) {
    if (vol[i] < minLvl)      minLvl = vol[i];
    else if (vol[i] > maxLvl) maxLvl = vol[i];
  }
  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if((maxLvl - minLvl) <= TOP) maxLvl = minLvl + TOP;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6;                 // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6;                 // (fake rolling average)
}