
//#include <CmdMessenger.h> // Serial Console

// Include libraries for serial, eeprom, and radio
#include <Streaming.h>
#include <Metro.h>

#include <RFM69.h> // RFM69HW radio transmitter module
#include <SPI.h> // for radio board 
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
#include <EEPROM.h>

//#include <FiniteStateMachine.h>

#include "Radio.h"

//------ Input units.
#include "Mic.h" // Microphone

#define INSTRUCTIONS "Commands: 0=Audio, 1=Color Fast, 2=Color Slow, 3=Set Color, 4=Hard On, 5=Drop Freq, 6=Raise Freq, 7=Drop Vol, 8=Raise Vol"
enum
{
  // Commands
  kAudio=0     ,
  kColorFast   ,
  kColorSlow   ,
  kSetColor    ,
  kHardOn      ,
  kDropFreq    ,
  kRaiseFreq   ,
  kDropVol     ,
  kRaiseVol    ,
};

// Attach a new CmdMessenger object to the default Serial port
//CmdMessenger cmdMessenger = CmdMessenger(Serial);

byte inVal;

uint8_t triggerBand = 4; // audio band to monitor volume on
uint16_t squash = 100;  // volume to subtract.  Dynamically drop when lights are too bright or never go completely dark.

#define SAMPLES   60                                          // Length of buffer for dynamic level adjustment
#define TOP       1023                                         // Allow peak to be slightly off scale

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

boolean waitingForColor = false;

uint8_t amplitude = 0;                                        // Current intensity of audio

/*void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kColorFast, OnSetColorCmd);
  cmdMessenger.attach(kAudio, OnAudioCmd);
}
// ------------------  C A L L B A C K S -----------------------

// Called when a received command has no attached function
void OnUnknownCommand()
{
  Serial << F("Changing program to: Set Audio") << endl;

  byte pgm=kAudio;

  R.sendProgram(pgm);

  recordNewInVal();
}

// Callback function that responds that Arduino is ready (has booted up)
void OnAudioCmd()
{
  Serial << F("Changing program to: Audio") << endl;

  byte pgm=kAudio;

  R.sendProgram(pgm);
}

// Callback function calculates the sum of the two received float values
void OnSetColorCmd()
{
  Serial << F("Changing program to: Set Color") << endl;

  byte pgm=kColorFast;

  R.sendProgram(pgm);

  recordNewInVal();
}

void recordNewInVal() {
  // Retreive first parameter as int
  inVal = cmdMessenger.readInt16Arg();
  hasNewInput = true;
}
*/

// ------------------ M A I N  ----------------------

void setup() {
  delay(500);

  Serial.begin(115200);

  //------ Input units.
  listenLine.begin(MIC_RESET_PIN, MIC_STROBE_PIN, MIC_OUT_PIN); // only listens to line in
  
  // Establish Control NodeId and start radio
  R.begin(CONTROLLER_NODE);
 
  R.pgm = kAudio;
  R.bright = 100;
  // Attach my application's user-defined callback methods
//  attachCommandCallbacks();

  Serial.println("Start Controller...");
  printInstructions();
}

void loop() {

  //cmdMessenger.feedinSerialData();

  if( Serial.available() > 0 ) {
    inVal = Serial.parseInt();
    Serial << F("Reading new value: ") << inVal << endl;
    if (waitingForColor) {
      R.hue = Serial.parseInt();
      waitingForColor = false;
    } else {
      switch(inVal) {
        case kAudio:
          Serial << F("Changing program to: Audio") << endl;

          R.pgm = kAudio;
          break;
        case kColorFast:
          if (!R.pgm == kColorFast) {
            Serial << F("Changing program to: Fast Color Change") << endl;
            
            R.pgm = kColorFast;
          }
          break;
        case kColorSlow:
          if (!R.pgm == kColorSlow) {
            Serial << F("Changing program to: Slow Color Change") << endl;
            
            R.pgm=kColorSlow;
          }
          break;
        case kSetColor:
          Serial << F("Enter new color value (0 - 255):") << endl;
          waitingForColor = true;
          break;
        case kHardOn:
          Serial << F("Setting brightness hard on.") << endl;

          R.pgm=kHardOn;

          R.bright = 100;
          break;
        case kRaiseFreq:
          if (triggerBand < 6) {
            triggerBand = triggerBand + 1;
            Serial << F("Raising trigger band to: ") << triggerBand << endl;
          } else {
            Serial << F("Trigger band at ceiling") << endl;
          }
          break;
        case kDropVol:
          if (squash < 1003) {
            squash = squash + 20;
            Serial << F("Raising squash to: ") << squash << endl;
          } else {
            Serial << F("Squash at ceiling") << endl;
          }
          break;
        case kRaiseVol:
          if (squash < 21) {
            squash = squash - 20;
            Serial << F("Dropping squash to: ") << squash << endl;
          } else {
            Serial << F("Squash at floor") << endl;
          }
          break;
      }
      printInstructions();
    }
  }
  
  /*
  byte inputLen = 0;
  if( Serial.available() > 0 ) {
    inputLen = readSerialLine(input, 10, 64, 100); //readSerialLine(char* input, char endOfLineChar=10, byte maxLength=64, uint16_t timeout=1000);
    Serial << F("Received input with len: ") << inputLen << endl;
    hasNewInput = true;
  }

  if (inputLen > 0) {
    String inStr = String(input);
    Serial << F("Input: ") << inStr << endl;
  }

  if (inputLen == 2 && input[0] == 80) { // 80 = P
    Serial << F("Changing program to: ") << input[1] << endl;
    //Serial << F("Enter new program cmd (Options: D = Delay Test, M = Manual, A = Audio:") << endl;
    //while( ! Serial.available() );
    //pgm = Serial.read();
    pgm = input[1];
    R.sendProgram(input[1]);
  }
  */

  switch(R.pgm) {
    case kAudio:
      updateAmplitude();
      break;
  }

  R.sendPayload();
}

void updateAmplitude() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
   
  /*
  int n = analogRead(MIC_PIN);                                    // Raw reading from mic
  n = abs(n - 512 - DC_OFFSET);                               // Center on zero
  */
  listenLine.update();
  int n = listenLine.getVol(triggerBand);
  
  n = (n <= squash) ? 0 : (n - squash);                         // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;                                 // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  //amplitude = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);
  amplitude = map(lvl, 0, TOP, 0, 255); 
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
  if((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
  minLvlAvg = (minLvlAvg * 63 + minLvl) >> 6;                 // Dampen min/max levels
  maxLvlAvg = (maxLvlAvg * 63 + maxLvl) >> 6;                 // (fake rolling average)
}

void printInstructions() {
  Serial << F(INSTRUCTIONS) << endl;
}
