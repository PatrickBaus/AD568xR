AD5681R/AD5682R/AD5683R Arduino Library
===================

[![CI](https://github.com/PatrickBaus/AD568xR/actions/workflows/ci.yml/badge.svg)](https://github.com/PatrickBaus/AD568xR/actions/workflows/ci.yml)

This repository contains an Arduino library for the Analog Devices AD5681R/AD5682R/AD5683R family of nanoDAC+s written in C++. The library uses SPI Transactions and allows to select the SPI controller if there is more than one.

Usage
-----
```cpp
#include <SPI.h>
#include <ad568xr.h>

#define CS_PIN 10
#define SCK_PIN 14   // Only used on Teensy boards

AD5681R dac(CS_PIN, SPI);    // To use the 14-bit version use AD5682R, or AD5683R for the 16-bit brethren

void setup() {
#if defined(TEENSYDUINO)
  // Teensy only: use the alternate SCK pin, because the default SCK pin (13) drives the LED below
  pinMode(SCK_PIN,OUTPUT);
  SPI.setSCK(SCK_PIN);
#endif
  SPI.begin();          // Not needed if you pass `true` to dac.begin()
  dac.begin();          // Call dac.begin(true) if you want to automatically run SPI.begin()
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
The library is not (yet) available via the Arduino library manager. Either download this repository as a ZIP file and install it via `Sketch -> Include Library -> Add .ZIP Library...` or clone it into the `libraries` folder of your sketchbook.

Tests
-----
The library comes with a host-based test suite, that runs on a PC without any hardware. The Arduino and SPI APIs are replaced by small fakes found in `test/mocks/`, which record all pin and bus activity. The tests are written using [doctest](https://github.com/doctest/doctest), which is downloaded by CMake. To run the tests you need CMake (>= 3.14), a C++11 compiler (GCC is required for the coverage report) and [gcovr](https://gcovr.com/):

```bash
cmake -S test -B test/build
cmake --build test/build
ctest --test-dir test/build --output-on-failure
```

To create the coverage report for `src/` and check that both the line and the branch coverage are 100 %, run the following command after the tests:

```bash
gcovr --root . --filter src/ --exclude-throw-branches --exclude-unreachable-branches \
  --fail-under-line 100 --fail-under-branch 100 --print-summary --txt-metric branch
```
