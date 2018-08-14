#include "AD568xR.h"

#define CS_PIN 10
#define SCK_PIN 14

AD5681R dac(CS_PIN, &SPI);    // To use the 14-bit version use AD5682R, or AD5683R for the 16-bit brethren

void setup() {
  pinMode(SCK_PIN,OUTPUT);
  SPI.setSCK(SCK_PIN);
  SPI.begin();
  dac.begin();          // Set all pins required by the DAC
  dac.reset();          // Reset the internal DAC registers
  dac.setGain(true);    // Set the maximum output voltage to 2*Vref = 5 V

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {
  static uint16_t outputValue = 0;
  dac.setValue(outputValue++);
  delay(10);
}
