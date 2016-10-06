
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

// Establish Control NodeId

// define char inChar,
//             pgm

// if serial available
//    record serial char as inChar
//    if inChar == P
//      log "Enter new program cmd (options: D = Delay Test, M = Manual, A = Audio)"
//      wait for serial available
//      record serial char as pgm
//      switch pgm
//        case D
//          send NEW_PROGRAM cmd for DELAY_TEST
//        case M
//          send NEW_PROGRAM cmd for MANUAL
//        case A
//          send NEW_PROGRAM cmd for AUDIO

// Switch on current program pgm
// case D
//    record time
//    ping single node, requesting ack
//    recieve ack; record time delta
//    compute new average delta
//    log average delta after modulo attempts
// case M
//    send inChar to all nodes in group
//    wait pre-calculated transmission time
// case A
//    take analog reading
//    compute new average amplitude
//    send amplitude to all nodes in group
//    wait pre-calculated transmission time



enum
{
  // Commands
  kManual=0      ,
  kAudio       , 
  kDelayTest   ,
};

// Attach a new CmdMessenger object to the default Serial port
//CmdMessenger cmdMessenger = CmdMessenger(Serial);

bool hasNewInput = false;
byte pgm=kAudio,
     inVal;

#define TRIGGER_BAND 6 // audio band to monitor volume on
#define NOISE     100                                         // Noise/hum/interference in mic signal and increased value until it went quiet
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
  maxLvlAvg = 1023,
  amplitude = 0;                                              // Current amplitude value
  int highest = 0;

/*void attachCommandCallbacks()
{
  // Attach callback methods
  cmdMessenger.attach(OnUnknownCommand);
  cmdMessenger.attach(kManual, OnManualCmd);
  cmdMessenger.attach(kAudio, OnAudioCmd);
  cmdMessenger.attach(kDelayTest, OnDelayCmd);
}
// ------------------  C A L L B A C K S -----------------------

// Called when a received command has no attached function
void OnUnknownCommand()
{
  Serial << F("Changing program to: Manual") << endl;

  byte pgm=kManual;

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
void OnDelayCmd()
{
  Serial << F("Changing program to: Delay Test") << endl;

  byte pgm=kDelayTest;

  R.sendProgram(pgm);
}

// Callback function calculates the sum of the two received float values
void OnManualCmd()
{
  Serial << F("Changing program to: Manual") << endl;

  byte pgm=kManual;

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
 
  // Attach my application's user-defined callback methods
//  attachCommandCallbacks();

  Serial.println("Start Controller...");
}

void loop() {

  //cmdMessenger.feedinSerialData();

  if( Serial.available() > 0 ) {
    inVal = Serial.parseInt();
    Serial << F("Reading new value: ") << inVal << endl;
    hasNewInput = true;
    switch(inVal) {
      case kManual:
        if (!pgm == kManual) {
          Serial << F("Changing program to: Manual") << endl;
          
          pgm=kManual;
  
          R.sendProgram(pgm);
          
          hasNewInput = false;
        }
        break;
      case kAudio:
        Serial << F("Changing program to: Audio") << endl;

        pgm=kAudio;
      
        R.sendProgram(pgm);
        break;
      case kDelayTest:
        Serial << F("Changing program to: Delay Test") << endl;

        pgm=kDelayTest;
      
        R.sendProgram(pgm);
        break;
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

  switch(pgm) {
    case kDelayTest:
      break;
    case kManual:
      if (hasNewInput) {
        Serial << F("Sending manual value: ") << inVal << endl;
        R.sendVal(inVal);
        hasNewInput = false;
      }
      break;
    case kAudio:
      updateAmplitude();
      /*
      listenLine.update();   // populate avg
      int amplitude = listenLine.getAvg(TRIGGER_BAND);
      amplitude = massageVol(amplitude);
      */
      //Serial << F("Sending avg vol: ") << amplitude << endl;
      R.sendVal(amplitude);
      break;
  }
}

void updateAmplitude() {
  uint8_t  i;
  uint16_t minLvl, maxLvl;
   
  /*
  int n = analogRead(MIC_PIN);                                    // Raw reading from mic
  n = abs(n - 512 - DC_OFFSET);                               // Center on zero
  */
  listenLine.update();
  int n = listenLine.getVol(TRIGGER_BAND);
  
  n = (n <= NOISE) ? 0 : (n - NOISE);                         // Remove noise/hum
  lvl = ((lvl * 7) + n) >> 3;                                 // "Dampened" reading (else looks twitchy)

  // Calculate bar height based on dynamic min/max levels (fixed point):
  //amplitude = TOP * (lvl - minLvlAvg) / (long)(maxLvlAvg - minLvlAvg);
  amplitude = map(lvl, 0, TOP, 0, 255); 
  //amplitude = map(lvl, minLvlAvg, maxLvlAvg, 0, 255); 

  Serial << F("n, l, amp: ") << n << ", " << lvl << ", " << amplitude << endl;
  
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

