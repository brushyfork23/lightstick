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

#include "Radio.h"

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




byte pgm=77;

char input[64]; //serial input buffer

bool hasNewInput = false;

void setup() {
  delay(500);

  Serial.begin(115200);

  // Establish Control NodeId and start radio
  R.begin(CONTROLLER_NODE);
 
  Serial.println("Start Controller...");
}

void loop() {
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

  char buff[inputLen-1];
  switch(pgm) {
    case 68: // D
      break;
    case 77: // M
      if (hasNewInput) {
        for(int i = 1; i < inputLen; i++) {
          buff[i-1] += input[i];
        }
        buff[inputLen-1] = '\0';
        Serial << F("Sending value: ") << atoi(buff) << endl;
        R.sendVal(atoi(buff));
      }
      break;
    case 65: // A
      break;
  }

  hasNewInput = false;
}



