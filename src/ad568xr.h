/*    
 *  The ADR5681R library is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as 
 *  published by the Free Software Foundation, either version 3 of the 
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef AD5681R_H
#define AD5681R_H

// include the Arduino SPI library.
#include <SPI.h>

#define AD568XR_SPI_CLOCK_FREQUENCY (1*1000*1000) //The maximum clockfrequency of the DAC is 50 Mhz

enum PowerDownMode {
  normal = 0b00,
  outputImpedance1k = 0b01,
  outputImpedance100k = 0b10,
  tristate = 0b11,
};

class AD568XR {
  public:
    AD568XR(const uint8_t cs_pin, SPIClass& spi, const uint8_t VALUE_OFFSET);
    
    void setValue(const uint16_t value);
    void setDaisyChain(const bool enable);
    void setGain(const bool enable);
    void disableRefeference(const bool disable);
    void setPowerDownMode(const PowerDownMode mode);
    void reset();

    uint16_t getControlRegister();
    void begin(bool initSPI=false);

  private:
    const uint8_t VALUE_OFFSET;
    SPIClass* spi;

  protected:
    void writeSPI(const uint32_t value);

    SPISettings spi_settings;
    uint32_t controlRegister = 0;
    uint8_t cs_pin;

    static const uint32_t COMMAND_WRITE_INPUT_REGISTER=0b0001 << 20;
    static const uint32_t COMMAND_UPDATE_INPUT_REGISTER=0b0010 << 20;
    static const uint32_t COMMAND_WRITE_CONTROL_REGISTER=0b0100 << 20;

    static const uint32_t OPTION_ENABLE_DAISY_CHAIN = 1 << 14;  
    static const uint32_t OPTION_ENABLE_GAIN = 1 << 15;
    static const uint32_t OPTION_DISABLE_REFERENCE = 1 << 16;
    static const uint32_t OPTION_PD0 = 1 << 17;
    static const uint32_t OPTION_PD1 = 1 << 18;
    static const uint32_t OPTION_RESET = 1 << 19;
};

class AD5681R: public AD568XR {
  public:
    AD5681R(const uint8_t cs_pin, SPIClass& spi) : AD568XR(cs_pin, spi, 4+4 /*VALUE_OFFSET*/) {}
};

class AD5682R: public AD568XR {
  public:
    AD5682R(const uint8_t cs_pin, SPIClass& spi) : AD568XR(cs_pin, spi, 4+2 /*VALUE_OFFSET*/) {}
};

class AD5683R: public AD568XR {
  public:
    AD5683R(const uint8_t cs_pin, SPIClass& spi) : AD568XR(cs_pin, spi, 4 /*VALUE_OFFSET*/) {}
};
#endif
