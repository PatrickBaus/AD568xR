/*    
 *  The ADR5683R1 library is free software: you can redistribute it and/or
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
#ifndef ADR5683R1
#define ADR5683R1

// include the Arduino standard library.
#include <Arduino.h>
// include the SPI library.
#include <SPI.h>

class ADR5683R1
{
public:

	//The pins that are used are declared in the constructor.
	ADR5683R1(uint8_t mosiPin, uint8_t sckPin, uint8_t syncPin);
	ADR5683R1(uint8_t sckPin, uint8_t syncPin);
	ADR5683R1(uint8_t syncPin);
	//Output stuff

	//The output has capacity of 12 bit
	writeInputRegister(uint16_t value);
	
	updateDACRegister();
	
	outputVoltage(uint16_t value);
	
	//Control register stuff
	setDaisyChain(bool mode);
	setGain(bool mode);
	setREF(bool mode);
	setPD0(bool mode);
	setPD1(bool mode);
	reset(bool mode);
	
	//Initialze everything with begin().
	begin();

protected:

	//The bits that changes the input register.
	static const uint16_t writeInputRegisterBits=0b00010000000000000000;
	//The bits that produces an output of the DA with the value in the input register.
	static const uint16_t writeDACRegisterBits=0b0010000000000000;
	//Changes the input register and outputs the written values.
	static const uint16_t writeOutputBits=0b0011000000000000;
	//The bits that are send to write control register.
	static const uint16_t writeControlRegisterBits=0b0100000000000000;

	//Save the control register of the DAC so that the other values are not changed when write a new value it.
	//The control register is 0 by default.
	uint16_t controlRegister = 0b0000000000000000;
	uint8_t mosiPin;
	uint8_t sckPin;
	uint8_t syncPin;
};


#endif