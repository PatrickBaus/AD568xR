#include "Arduino.h"
#include "SPI.h"

namespace mock {

std::vector<Event>& events() {
  static std::vector<Event> log;
  return log;
}

std::deque<uint8_t>& spiResponses() {
  static std::deque<uint8_t> responses;
  return responses;
}

void reset() {
  events().clear();
  spiResponses().clear();
}

std::vector<uint8_t> mosiBytes(const void* bus) {
  std::vector<uint8_t> bytes;
  for (const Event& event : events()) {
    if (event.type == Event::SPI_TRANSFER && (bus == nullptr || event.bus == bus)) {
      bytes.push_back(event.value);
    }
  }
  return bytes;
}

size_t count(Event::Type type) {
  size_t result = 0;
  for (const Event& event : events()) {
    if (event.type == type) {
      result++;
    }
  }
  return result;
}

static void record(Event::Type type, const void* bus, uint8_t pin, uint8_t value, uint32_t clock = 0, uint8_t bitOrder = 0, uint8_t dataMode = 0) {
  Event event = {type, bus, pin, value, clock, bitOrder, dataMode};
  events().push_back(event);
}

}  // namespace mock

void pinMode(uint8_t pin, uint8_t mode) {
  mock::record(mock::Event::PIN_MODE, nullptr, pin, mode);
}

void digitalWrite(uint8_t pin, uint8_t value) {
  mock::record(mock::Event::DIGITAL_WRITE, nullptr, pin, value);
}

SPIClass SPI;

void SPIClass::begin() {
  mock::record(mock::Event::SPI_BEGIN, this, 0, 0);
}

void SPIClass::beginTransaction(SPISettings settings) {
  mock::record(mock::Event::SPI_BEGIN_TRANSACTION, this, 0, 0, settings.clock, settings.bitOrder, settings.dataMode);
}

uint8_t SPIClass::transfer(uint8_t data) {
  mock::record(mock::Event::SPI_TRANSFER, this, 0, data);
  uint8_t response = 0x00;
  if (!mock::spiResponses().empty()) {
    response = mock::spiResponses().front();
    mock::spiResponses().pop_front();
  }
  return response;
}

void SPIClass::endTransaction() {
  mock::record(mock::Event::SPI_END_TRANSACTION, this, 0, 0);
}
