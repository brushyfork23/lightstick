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

// enumerate animation modes
enum program_t {
  P_MANUAL=0,
  P_AUDIO,
  P_DELAY_TEST,
  
  N_PROGRAMS
};

// Establish My NodeId

// set Metro pushNextFrame for tracking time between frames
// and set bool nextFrameReady 

// if radio input available
//  record payload: https://github.com/LowPowerLab/RFM69/blob/master/Examples/Struct_receive/Struct_receive.ino#L95
//  if Payload.pgm != pgm
//     setProgram(pgm)
//  switch on pgm
//    case DELAY_TEST
//      increment hue value
//      set strip color to hue value
//      send ack
//    case MANUAL
//      convert Payload.num to hue value
//      set strip color to hue value
//    case AUDIO
//      convert Payload.num to brightness
//      set strip brightness

// Render next frame of animation: A.runAnimation()

// light strip

//  setProgram(pgm) {
//    init(pgm)

//  init(pgm) {
//    switch on pgm
//    case DELAY_TEST
//      set hueVal = 0
//    case MANUAL
//      set hueVal = 0
//    case AUDIO
//      set brightness = 0

char program=0;
byte hueVal;

void setup() {
  delay(500); // delay for upload
   
  Serial.begin(115200);

  // Establish My NodeId and start radio
  R.begin();

  // startup animation
  A.begin();
}


void loop() {
  // if availalbe, record a new radio payload
  R.update();

  // Update program if new
  if (R.hasUnorocessedPayload && R.pgm != program) {
    program = R.pgm;
    switch(program) {
      case P_MANUAL:
        hueVal = 0;
        break;
      case P_AUDIO:
        A.setMasterBrightness(0);
        break;
      case P_DELAY_TEST:
        break;
    }
  }

  switch (program) {
    case P_MANUAL:
      A.startHue(R.num);
      A.runAnimation();
      break;
    case P_AUDIO:
      A.setMasterBrightness(R.num);
      A.runAnimation();
      break;
    case P_DELAY_TEST:
      if (R.hasUnorocessedPayload) {
        A.incrementHue(1);
        A.runAnimation();
        R.sendAck();
      }
      break;
  }

  R.hasUnorocessedPayload = false;
}



