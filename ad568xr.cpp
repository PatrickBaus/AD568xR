/*    
 *  The ADR568xR library is free software: you can redistribute it and/or
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
#include "ad568xr.h"

#define AD5681R_SPI_CLOCK_FREQUENCY (1*1000*1000) //The maximum clockfrequency of the DAC is 50 Mhz 

AD568XR::AD568XR(const uint8_t _cs_pin, SPIClass* _spi, const uint8_t _VALUE_OFFSET) :
    VALUE_OFFSET(_VALUE_OFFSET), spi(_spi), spi_settings(SPISettings(AD5681R_SPI_CLOCK_FREQUENCY, MSBFIRST, SPI_MODE1)), cs_pin(_cs_pin) {};

const uint16_t AD568XR::getControlRegister(){
  return this->controlRegister;
}

void AD568XR::writeSPI(const uint32_t value) {
  Serial.println(value);
  Serial.println(value, BIN);
  Serial.write(0x00);
  this->spi->beginTransaction(this->spi_settings);
  digitalWriteFast(this->cs_pin, LOW);

  // The input shift register is 24 bits wide. So we do not care about the top byte of the 32 bit integer
  this->spi->transfer((value >> 16) & 0xFF);
  this->spi->transfer((value >> 8) & 0xFF);
  this->spi->transfer((value >> 0) & 0xFF);

  digitalWriteFast(this->cs_pin, HIGH);
  this->spi->endTransaction();
}

void AD568XR::setValue(const uint16_t value){
  this->writeSPI(this->COMMAND_WRITE_INPUT_REGISTER | this->COMMAND_UPDATE_INPUT_REGISTER | (((uint32_t)value << this->VALUE_OFFSET) & 0xFFFFF));
}

void AD568XR::setDaisyChain(const bool enable){
  // -(uint32_t)1 is 0b111...111 (all ones)
  // This means we invert the controlRegister depending on enable
  this->controlRegister ^= (-(uint32_t)enable ^ this->controlRegister) & this->OPTION_ENABLE_DAISY_CHAIN;    // set the OPTION_ENABLE_DAISY_CHAIN bit according to the input value

  this->writeSPI(this->COMMAND_WRITE_CONTROL_REGISTER | this->controlRegister);
}

void AD568XR::setGain(const bool enable){
  this->controlRegister ^= (-(uint32_t)enable ^ this->controlRegister) & this->OPTION_ENABLE_GAIN;   // set the OPTION_ENABLE_GAIN bit according to the input value

  this->writeSPI(this->COMMAND_WRITE_CONTROL_REGISTER | this->controlRegister);
}

void AD568XR::disableRefeference(const bool disable){
  this->controlRegister ^= (-(uint32_t)disable ^ this->controlRegister) & this->OPTION_DISABLE_REFERENCE;    // set the OPTION_DISABLE_REFERENCE bit according to the input value

  this->writeSPI(this->COMMAND_WRITE_CONTROL_REGISTER | this->controlRegister);
}

void AD568XR::setPowerDownMode(const PowerDownMode mode) {
  this->controlRegister ^= ((-(uint32_t)(mode & 0b1) ^ this->controlRegister) & this->OPTION_PD0) | ((-(uint32_t)((mode >> 1) & 0b1) ^ this->controlRegister) & this->OPTION_PD1);    // set *both* the OPTION_PD0 and OPTION_PD1 bits according to the input value
}

void AD568XR::reset() {
  this->controlRegister |= this->OPTION_RESET;
  this->writeSPI(this->COMMAND_WRITE_CONTROL_REGISTER | this->controlRegister);
  this->controlRegister = 0;
}

void AD568XR::begin() {
	pinMode(this->cs_pin, OUTPUT);
	digitalWriteFast(this->cs_pin, HIGH);
}
