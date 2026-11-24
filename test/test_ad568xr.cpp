/*
 * Host-based tests of the AD568xR library.
 *
 * The expected values are taken from the AD5683R/AD5682R/AD5681R/AD5683 datasheet:
 *  - 24 bit input shift register, MSB first: DB23..DB20 command, DB19..DB4 data (left aligned,
 *    i.e. DB19..DB8 for the 12 bit AD5681R and DB19..DB6 for the 14 bit AD5682R)
 *  - Command 0b0011: write the input register and update the DAC register
 *  - Command 0b0100: write the control register
 *  - Control register: DB19 reset, DB18 PD1, DB17 PD0, DB16 REF (disable), DB15 gain, DB14 DCEN
 *  - Data is clocked in on the falling edge of SCLK (SPI mode 1) while /SYNC is low
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <vector>

#include "SPI.h"
#include "ad568xr.h"

using mock::Event;

namespace {

const uint8_t CS_PIN = 10;

struct Fixture {
  Fixture() { mock::reset(); }
  ~Fixture() { mock::reset(); }
};

typedef std::vector<uint8_t> Bytes;

// Assemble the 24 bit frames from the bytes seen on MOSI
std::vector<uint32_t> frames() {
  const Bytes bytes = mock::mosiBytes();
  REQUIRE(bytes.size() % 3 == 0);
  std::vector<uint32_t> result;
  for (size_t i = 0; i < bytes.size(); i += 3) {
    result.push_back(((uint32_t)bytes[i] << 16) | ((uint32_t)bytes[i + 1] << 8) | bytes[i + 2]);
  }
  return result;
}

// The last (and only) frame written since the log was cleared
uint32_t singleFrame() {
  const std::vector<uint32_t> result = frames();
  REQUIRE(result.size() == 1);
  return result[0];
}

// Check that events [offset, offset+7) are exactly one well-formed 24 bit SPI frame
void checkFrameSequence(const SPIClass& bus, size_t offset, uint8_t byte0, uint8_t byte1, uint8_t byte2) {
  const std::vector<Event>& log = mock::events();
  REQUIRE(log.size() >= offset + 7);

  CHECK(log[offset + 0].type == Event::SPI_BEGIN_TRANSACTION);
  CHECK(log[offset + 0].bus == &bus);
  CHECK(log[offset + 0].clock == 1000000UL);
  CHECK(log[offset + 0].bitOrder == MSBFIRST);
  CHECK(log[offset + 0].dataMode == SPI_MODE1);

  CHECK(log[offset + 1].type == Event::DIGITAL_WRITE);
  CHECK(log[offset + 1].pin == CS_PIN);
  CHECK(log[offset + 1].value == LOW);

  const uint8_t expected[3] = {byte0, byte1, byte2};
  for (size_t i = 0; i < 3; i++) {
    CHECK(log[offset + 2 + i].type == Event::SPI_TRANSFER);
    CHECK(log[offset + 2 + i].bus == &bus);
    CHECK(log[offset + 2 + i].value == expected[i]);
  }

  CHECK(log[offset + 5].type == Event::DIGITAL_WRITE);
  CHECK(log[offset + 5].pin == CS_PIN);
  CHECK(log[offset + 5].value == HIGH);

  CHECK(log[offset + 6].type == Event::SPI_END_TRANSACTION);
  CHECK(log[offset + 6].bus == &bus);
}

}  // namespace

TEST_CASE_FIXTURE(Fixture, "the constructor does not touch the hardware") {
  AD5681R dac12(CS_PIN, SPI);
  AD5682R dac14(CS_PIN, SPI);
  AD5683R dac16(CS_PIN, SPI);

  CHECK(mock::events().empty());
  CHECK(dac12.getControlRegister() == 0);
  CHECK(dac14.getControlRegister() == 0);
  CHECK(dac16.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "begin() deselects the DAC and leaves the SPI bus alone by default") {
  AD5683R dac(CS_PIN, SPI);
  dac.begin();

  const std::vector<Event>& log = mock::events();
  REQUIRE(log.size() == 2);
  CHECK(log[0].type == Event::PIN_MODE);
  CHECK(log[0].pin == CS_PIN);
  CHECK(log[0].value == OUTPUT);
  CHECK(log[1].type == Event::DIGITAL_WRITE);
  CHECK(log[1].pin == CS_PIN);
  CHECK(log[1].value == HIGH);
}

TEST_CASE_FIXTURE(Fixture, "begin(false) does not initialise the SPI bus") {
  AD5683R dac(CS_PIN, SPI);
  dac.begin(false);

  CHECK(mock::events().size() == 2);
  CHECK(mock::count(Event::SPI_BEGIN) == 0);
}

TEST_CASE_FIXTURE(Fixture, "begin(true) initialises the SPI bus after deselecting the DAC") {
  SPIClass otherBus;
  AD5683R dac(7, otherBus);
  dac.begin(true);

  const std::vector<Event>& log = mock::events();
  REQUIRE(log.size() == 3);
  CHECK(log[0].type == Event::PIN_MODE);
  CHECK(log[0].pin == 7);
  CHECK(log[0].value == OUTPUT);
  CHECK(log[1].type == Event::DIGITAL_WRITE);
  CHECK(log[1].pin == 7);
  CHECK(log[1].value == HIGH);
  CHECK(log[2].type == Event::SPI_BEGIN);
  CHECK(log[2].bus == &otherBus);
}

TEST_CASE_FIXTURE(Fixture, "a write is one 24 bit, MSB first, mode 1 transaction framed by chip select") {
  AD5683R dac(CS_PIN, SPI);
  dac.setValue(0x1234);

  // 0b0011 (write input register and update DAC), 0x1234, 4 don't care bits
  REQUIRE(mock::events().size() == 7);
  checkFrameSequence(SPI, 0, 0x31, 0x23, 0x40);
}

TEST_CASE_FIXTURE(Fixture, "the SPI bus passed to the constructor is used") {
  SPIClass otherBus;
  AD5683R dac(CS_PIN, otherBus);
  dac.setValue(0xFFFF);

  REQUIRE(mock::events().size() == 7);
  checkFrameSequence(otherBus, 0, 0x3F, 0xFF, 0xF0);
  CHECK(mock::mosiBytes(&SPI).empty());
  CHECK(mock::mosiBytes(&otherBus) == Bytes({0x3F, 0xFF, 0xF0}));
}

TEST_CASE_FIXTURE(Fixture, "data read from the bus during a write is ignored") {
  AD5683R dac(CS_PIN, SPI);
  mock::spiResponses().push_back(0xDE);
  mock::spiResponses().push_back(0xAD);
  mock::spiResponses().push_back(0xBE);
  dac.setValue(0x8000);

  CHECK(mock::spiResponses().empty());
  CHECK(singleFrame() == 0x380000);
  CHECK(dac.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "AD5683R (16 bit) codes are left aligned at DB19..DB4") {
  AD5683R dac(CS_PIN, SPI);

  SUBCASE("zero scale") {
    dac.setValue(0x0000);
    CHECK(singleFrame() == 0x300000);
  }
  SUBCASE("1 LSB") {
    dac.setValue(0x0001);
    CHECK(singleFrame() == 0x300010);
  }
  SUBCASE("mid scale") {
    dac.setValue(0x8000);
    CHECK(singleFrame() == 0x380000);
  }
  SUBCASE("full scale") {
    dac.setValue(0xFFFF);
    CHECK(singleFrame() == 0x3FFFF0);
  }
  SUBCASE("alternating bit pattern") {
    dac.setValue(0xA5C3);
    CHECK(singleFrame() == 0x3A5C30);
  }
}

TEST_CASE_FIXTURE(Fixture, "AD5682R (14 bit) codes are left aligned at DB19..DB6") {
  AD5682R dac(CS_PIN, SPI);

  SUBCASE("zero scale") {
    dac.setValue(0x0000);
    CHECK(singleFrame() == 0x300000);
  }
  SUBCASE("1 LSB") {
    dac.setValue(0x0001);
    CHECK(singleFrame() == 0x300040);
  }
  SUBCASE("mid scale") {
    dac.setValue(0x2000);
    CHECK(singleFrame() == 0x380000);
  }
  SUBCASE("full scale") {
    dac.setValue(0x3FFF);
    CHECK(singleFrame() == 0x3FFFC0);
  }
  SUBCASE("codes above full scale are truncated to 14 bit and never alter the command bits") {
    dac.setValue(0x4000);
    CHECK(singleFrame() == 0x300000);
    mock::reset();
    dac.setValue(0xFFFF);
    CHECK(singleFrame() == 0x3FFFC0);
  }
}

TEST_CASE_FIXTURE(Fixture, "AD5681R (12 bit) codes are left aligned at DB19..DB8") {
  AD5681R dac(CS_PIN, SPI);

  SUBCASE("zero scale") {
    dac.setValue(0x0000);
    CHECK(singleFrame() == 0x300000);
  }
  SUBCASE("1 LSB") {
    dac.setValue(0x0001);
    CHECK(singleFrame() == 0x300100);
  }
  SUBCASE("mid scale") {
    dac.setValue(0x0800);
    CHECK(singleFrame() == 0x380000);
  }
  SUBCASE("full scale") {
    dac.setValue(0x0FFF);
    CHECK(singleFrame() == 0x3FFF00);
  }
  SUBCASE("codes above full scale are truncated to 12 bit and never alter the command bits") {
    dac.setValue(0x1000);
    CHECK(singleFrame() == 0x300000);
    mock::reset();
    dac.setValue(0xFFFF);
    CHECK(singleFrame() == 0x3FFF00);
  }
}

TEST_CASE_FIXTURE(Fixture, "setValue() does not modify the control register") {
  AD5683R dac(CS_PIN, SPI);
  dac.setGain(true);
  mock::reset();

  dac.setValue(0xFFFF);
  CHECK(dac.getControlRegister() == 0x008000UL);
  CHECK(singleFrame() == 0x3FFFF0);
}

TEST_CASE_FIXTURE(Fixture, "setGain() controls DB15 of the control register") {
  AD5683R dac(CS_PIN, SPI);

  dac.setGain(true);
  REQUIRE(mock::events().size() == 7);
  checkFrameSequence(SPI, 0, 0x40, 0x80, 0x00);
  CHECK(dac.getControlRegister() == 0x008000UL);

  mock::reset();
  dac.setGain(true);  // idempotent
  CHECK(singleFrame() == 0x408000);

  mock::reset();
  dac.setGain(false);
  CHECK(singleFrame() == 0x400000);
  CHECK(dac.getControlRegister() == 0);

  mock::reset();
  dac.setGain(false);  // idempotent
  CHECK(singleFrame() == 0x400000);
  CHECK(dac.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "setDaisyChain() controls DB14 of the control register") {
  AD5683R dac(CS_PIN, SPI);

  dac.setDaisyChain(true);
  REQUIRE(mock::events().size() == 7);
  checkFrameSequence(SPI, 0, 0x40, 0x40, 0x00);
  CHECK(dac.getControlRegister() == 0x004000UL);

  mock::reset();
  dac.setDaisyChain(true);  // idempotent
  CHECK(singleFrame() == 0x404000);

  mock::reset();
  dac.setDaisyChain(false);
  CHECK(singleFrame() == 0x400000);
  CHECK(dac.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "disableRefeference() controls DB16 of the control register") {
  AD5683R dac(CS_PIN, SPI);

  dac.disableRefeference(true);
  REQUIRE(mock::events().size() == 7);
  checkFrameSequence(SPI, 0, 0x41, 0x00, 0x00);
  // DB16 does not fit into 16 bit, the getter must not truncate it
  CHECK(dac.getControlRegister() == 0x010000UL);

  mock::reset();
  dac.disableRefeference(true);  // idempotent
  CHECK(singleFrame() == 0x410000);

  mock::reset();
  dac.disableRefeference(false);
  CHECK(singleFrame() == 0x400000);
  CHECK(dac.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "setPowerDownMode() writes PD1/PD0 (DB18/DB17) to the DAC") {
  AD5683R dac(CS_PIN, SPI);

  SUBCASE("1 kOhm to ground") {
    dac.setPowerDownMode(outputImpedance1k);
    REQUIRE(mock::events().size() == 7);
    checkFrameSequence(SPI, 0, 0x42, 0x00, 0x00);
    CHECK(dac.getControlRegister() == 0x020000UL);
  }
  SUBCASE("100 kOhm to ground") {
    dac.setPowerDownMode(outputImpedance100k);
    REQUIRE(mock::events().size() == 7);
    checkFrameSequence(SPI, 0, 0x44, 0x00, 0x00);
    CHECK(dac.getControlRegister() == 0x040000UL);
  }
  SUBCASE("three-state") {
    dac.setPowerDownMode(tristate);
    REQUIRE(mock::events().size() == 7);
    checkFrameSequence(SPI, 0, 0x46, 0x00, 0x00);
    CHECK(dac.getControlRegister() == 0x060000UL);
  }
  SUBCASE("normal operation") {
    dac.setPowerDownMode(normal);
    REQUIRE(mock::events().size() == 7);
    checkFrameSequence(SPI, 0, 0x40, 0x00, 0x00);
    CHECK(dac.getControlRegister() == 0);
  }
}

TEST_CASE_FIXTURE(Fixture, "setPowerDownMode() replaces both power down bits") {
  AD5683R dac(CS_PIN, SPI);

  const PowerDownMode modes[] = {normal, outputImpedance1k, outputImpedance100k, tristate};
  for (PowerDownMode from : modes) {
    for (PowerDownMode to : modes) {
      CAPTURE(from);
      CAPTURE(to);
      dac.setPowerDownMode(from);
      mock::reset();
      dac.setPowerDownMode(to);
      CHECK(singleFrame() == (0x400000UL | ((uint32_t)to << 17)));
      CHECK(dac.getControlRegister() == ((uint32_t)to << 17));
    }
  }
}

TEST_CASE_FIXTURE(Fixture, "the PowerDownMode values match the PD1/PD0 encoding of the datasheet") {
  CHECK(normal == 0);
  CHECK(outputImpedance1k == 1);
  CHECK(outputImpedance100k == 2);
  CHECK(tristate == 3);
}

TEST_CASE_FIXTURE(Fixture, "control register options are independent of each other") {
  AD5683R dac(CS_PIN, SPI);

  dac.setGain(true);
  dac.setDaisyChain(true);
  dac.disableRefeference(true);
  dac.setPowerDownMode(tristate);
  CHECK(dac.getControlRegister() == 0x07C000UL);
  CHECK(frames() == std::vector<uint32_t>({0x408000, 0x40C000, 0x41C000, 0x47C000}));

  mock::reset();
  dac.setPowerDownMode(outputImpedance1k);
  CHECK(singleFrame() == 0x43C000);

  mock::reset();
  dac.setGain(false);
  CHECK(singleFrame() == 0x434000);

  mock::reset();
  dac.disableRefeference(false);
  CHECK(singleFrame() == 0x424000);

  mock::reset();
  dac.setDaisyChain(false);
  CHECK(singleFrame() == 0x420000);

  mock::reset();
  dac.setPowerDownMode(normal);
  CHECK(singleFrame() == 0x400000);
  CHECK(dac.getControlRegister() == 0);
}

TEST_CASE_FIXTURE(Fixture, "the control register layout is the same for all family members") {
  AD5681R dac12(CS_PIN, SPI);
  AD5682R dac14(CS_PIN, SPI);

  dac12.setGain(true);
  dac14.disableRefeference(true);
  CHECK(frames() == std::vector<uint32_t>({0x408000, 0x410000}));
}

TEST_CASE_FIXTURE(Fixture, "reset() sets DB19 and restores the power-on defaults of the shadow register") {
  AD5683R dac(CS_PIN, SPI);

  SUBCASE("from the power-on state") {
    dac.reset();
    REQUIRE(mock::events().size() == 7);
    checkFrameSequence(SPI, 0, 0x48, 0x00, 0x00);
    CHECK(dac.getControlRegister() == 0);
  }
  SUBCASE("the reset bit is not sticky") {
    dac.reset();
    mock::reset();
    dac.setGain(true);
    CHECK(singleFrame() == 0x408000);
    CHECK(dac.getControlRegister() == 0x008000UL);
  }
  SUBCASE("previously set options are cleared") {
    dac.setGain(true);
    dac.disableRefeference(true);
    dac.setPowerDownMode(tristate);
    mock::reset();

    dac.reset();
    CHECK((singleFrame() & 0xF80000UL) == 0x480000UL);
    CHECK(dac.getControlRegister() == 0);

    mock::reset();
    dac.setDaisyChain(true);
    CHECK(singleFrame() == 0x404000);
  }
}

TEST_CASE_FIXTURE(Fixture, "two DACs on the same bus use their own chip select and shadow register") {
  AD5683R dacA(CS_PIN, SPI);
  AD5681R dacB(9, SPI);

  dacA.setGain(true);
  dacB.setValue(0x0FFF);

  const std::vector<Event>& log = mock::events();
  REQUIRE(log.size() == 14);
  checkFrameSequence(SPI, 0, 0x40, 0x80, 0x00);
  CHECK(log[8].type == Event::DIGITAL_WRITE);
  CHECK(log[8].pin == 9);
  CHECK(log[8].value == LOW);
  CHECK(log[12].type == Event::DIGITAL_WRITE);
  CHECK(log[12].pin == 9);
  CHECK(log[12].value == HIGH);
  CHECK(mock::mosiBytes() == Bytes({0x40, 0x80, 0x00, 0x3F, 0xFF, 0x00}));
  CHECK(dacA.getControlRegister() == 0x008000UL);
  CHECK(dacB.getControlRegister() == 0);
}
