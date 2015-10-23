#ifndef Radio_h
#define Radio_h

#include <Arduino.h>

#include <Metro.h>
#include <Streaming.h>
#include <SPI.h> // for radio board 
#include <RFM69.h> // RFM69HW radio transmitter module

#define BROADCAST 0  // all nodes will hear this
#define GROUPID 188  // local group
#define NODESTART 21 // nodes start at this number
#define MAX_NODES 10 // for memory allocation

#define LED_PIN 9    // blink when sending.

#define PING_EVERY 100UL // or thereabouts

typedef struct {
  byte packetNumber;
  byte powerLevel;
} Message;

class Radio {
  public:
    // initialize led strips
    void begin(byte groupID=GROUPID, byte freq=RF69_915MHZ);
    // check for radio traffic; record the rssi if received; send a packet if it's our turn
    void update();
    // returns true if the rssi data indicates a trigger
    boolean trigger(float rssiThresh=1.04, unsigned long cooldown=50UL); // e.g. returns true if (last/average) RSSI >= 1.05 or <= (1/1.05), but only every cooldown interval

  private:
    // radio instance
    void setNodeID(byte node);
    void setPowerLevel(byte level=0);
    byte nodeID, powerLevel;
    RFM69 radio;
    // message storage
    Message msg;
    // RSSI storage
    float averageRSSI[MAX_NODES], relRSSI[MAX_NODES];

    // record RSSI
    void recordRSSI(byte index, int rssi);

};

extern Radio R;

#endif
