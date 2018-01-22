#include "ADR5683R1.h"

//The Pin which has to be pulled LOW to begin a transmission.
#define SYNC_PIN 10

//The Pin for the clock signal.
#define SCK_PIN 14

//The maximal clock in Mhz that the Teensy LC can handle.
#define CLOCK_SPEED 48000000

adr5683r1 dac(SCK_PIN,SYNC_PIN);

void setup() {
  pinMode(SCK_PIN,OUTPUT);
  pinMode(11,OUTPUT);
  Serial.begin(9600);
  // put your setup code here, to run once:
  dac.beginDAC(CLOCK_SPEED);
  dac.setGain(false);
  delay(1);
  
  //dac.writeAndUpdateRegisters(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  dac.writeAndUpdateRegisters(4000);
}
