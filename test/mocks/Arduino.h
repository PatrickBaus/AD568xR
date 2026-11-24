/*
 * Minimal host-side fake of the Arduino core API used by the AD568xR library.
 *
 * All calls that touch a pin or the SPI bus are appended to a single, ordered
 * event log, so tests can assert on the exact sequence seen by the hardware.
 */
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stddef.h>
#include <stdint.h>

#include <deque>
#include <vector>

#define HIGH 0x1
#define LOW 0x0

#define INPUT 0x0
#define OUTPUT 0x1

#define LSBFIRST 0
#define MSBFIRST 1

namespace mock {

struct Event {
  enum Type {
    PIN_MODE,               // pin, value = mode
    DIGITAL_WRITE,          // pin, value = level
    SPI_BEGIN,              // bus
    SPI_BEGIN_TRANSACTION,  // bus, clock, bitOrder, dataMode
    SPI_TRANSFER,           // bus, value = byte sent (MOSI)
    SPI_END_TRANSACTION     // bus
  };

  Type type;
  const void* bus;  // SPIClass instance, nullptr for pin events
  uint8_t pin;
  uint8_t value;
  uint32_t clock;
  uint8_t bitOrder;
  uint8_t dataMode;
};

// The global, ordered log of everything the code under test did
std::vector<Event>& events();
// Bytes returned (MISO) by the next calls to SPIClass::transfer(). 0x00 is returned when empty.
std::deque<uint8_t>& spiResponses();
// Clear the log and the scripted responses
void reset();
// Convenience: the bytes written to MOSI, in order (optionally only for one bus)
std::vector<uint8_t> mosiBytes(const void* bus = nullptr);
// Convenience: the number of events of a given type
size_t count(Event::Type type);

}  // namespace mock

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);

#endif
