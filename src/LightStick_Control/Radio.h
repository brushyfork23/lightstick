#ifndef Radio_h
#define Radio_h

#include <Arduino.h>
#include <Streaming.h>

// radio
#include <SPI.h> 
#include <RFM69.h> 
// wireless programming
#include <SPIFlash.h>
#include <avr/wdt.h>
#include <WirelessHEX69.h>
// for storage of node information
#include <EEPROM.h> 

// comms settings and information
#define BROADCAST     0  // all nodes will hear this
#define GROUPID       158  // local group
#define POWERLEVEL      15 // 0-31, 31 being maximal
#define CONTROLLER_NODE     253 // nodeID of wireless programmer

// pin definitions common to Moteuinos
#define LED   9 // Moteinos have LED on D9
#define FLASH_SS  8 // and FLASH SS on D8
#define FLASH_ID  0xEF30 // EF30 for windbond 4mbit flash

typedef struct {
  char          pgm; //program to execute
  int16_t       num; //numeric value 
} Payload;

class Radio {
  public:
    // initialize radio
    void begin(byte nodeID=255, byte groupID=GROUPID, byte freq=RF69_915MHZ, byte powerLevel=POWERLEVEL);
    void update();
    void sendAck();
    void sendProgram(byte pgm);
    void sendVal(byte val);
    // return my node ID
    byte myNodeID; // 26-28 for Lights
    char pgm;
    int16_t num;
    bool hasUnorocessedPayload;

  private:
    RFM69 radio;
    // message storage
    Payload payload;
    void sendPayload(Payload payload);
};


extern Radio R;

#endif



