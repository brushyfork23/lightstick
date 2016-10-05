
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




#define TRIGGER_BAND 6 // audio band to monitor volume on


enum
{
  // Commands
  kManual=0      ,
  kAudio       , 
  kDelayTest   ,
};

// Attach a new CmdMessenger object to the default Serial port
//CmdMessenger cmdMessenger = CmdMessenger(Serial);

int inVal = 0;
bool hasNewInput = false;
byte pgm=kManual;

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
      listenLine.update();   // populate avg
      listenLine.update();   // populate avg
      int vol = listenLine.getAvg(TRIGGER_BAND);
      Serial << F("Sending vol value: ") << vol << endl;
      R.sendVal(vol);
      break;
  }
}
