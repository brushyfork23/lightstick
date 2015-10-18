// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/
#include <SPI.h>
#include <RFM69.h>
#include <Streaming.h>
#include <Metro.h>

#define NODEID        21    //unique for each node on same network
#define NETWORKID     188  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY     RF69_915MHZ
#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define LED           9 // Moteinos have LEDs on D9

#define SERIAL_BAUD   115200

int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buff[20];
byte sendSize=0;
boolean requestACK = false;
RFM69 radio;

void setup() {
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
}

const int nB = 11;
int rssiBuckets[nB] = {70, 75, 80, 85, 90, 95, 100, 105, 110, 115, 120};
int rssiCounts[nB];

long lastPeriod = -1;
int samples=10000, cSample=10000;

void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
    }

    if (input == 'd') //d=dump register values
      radio.readAllRegs();

  }

  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
    }
    Blink(LED,5);
    Serial.println();
  }

  // get current RSSI
  int rssi = -radio.readRSSI();
  // what bucket is it in?
  for( int i=0; i<nB; i++ ) {
    if( rssi <= rssiBuckets[i] ) {
      rssiCounts[i]++;
      break;
    }
  }
  // are we ready to print
  cSample++;
  if( cSample >= samples ) {
    cSample = 0;
    for( int i=0; i<nB; i++ ) {
      Serial << rssiBuckets[i] << "=" << float(rssiCounts[i])/float(samples) << "\t";
      rssiCounts[i]=0;
    }
    Serial << endl;
  }
  
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
