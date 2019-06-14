AD5681R/AD5682R/AD5683R Arduino Library
===================

This repository contains an Arduino library for the Analog Devices AD5681R/AD5682R/AD5683R family of nanoDAC+s written in C++. The library uses SPI Transactions and allows to select the SPI controller if there is more than one.

Usage
-----
```cpp
#include "ad568xr.h"

#define CS_PIN 10
#define SCK_PIN 14

AD5681R dac(CS_PIN, &SPI);    // To use the 14-bit version use AD5682R, or AD5683R for the 16-bit brethren

void setup() {
  pinMode(SCK_PIN,OUTPUT);
  SPI.setSCK(SCK_PIN);
  dac.begin();          // Set all pins required by the DAC and calls SPI.begin()
  // dac.begin(false);  // Call begin(false) if you do not want to run SPI.begin()
  dac.reset();          // Reset the internal DAC registers
  dac.setGain(true);    // Set the maximum output voltage to 2*Vref = 5 V

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {
  static uint16_t outputValue = 0;
  dac.setValue(outputValue++);    // Increment the DAC output every 10 ms
  delay(10);
}
```

Installation
-----
Currently the library does not support the Arduino library manager, so it is highly recommended to copy the full library to a subfolder called
```
src/
```
within your Arduino project.
