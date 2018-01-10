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
#include "ADR5683R1.h"

#define MAX_CLOCK_FREQUENCY 50000000 //The maximal clockfrequency of the DAC is 50 Mhz 

ADR5683R1::ADR5683R1(uint8_t sckPin, uint8_t syncPin) : syncPin(syncPin), sckPin(sckPin), controlRegister=0 {}

ADR5683R1::writeInputRegister(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
		uint16_t dataToSend=WRIT_INPUT_REGISTER_BIT | value;
		noInterrupts();
		
		digitalWrite(syncPin,LOW); //Write the commmands to the DAC
	    SPI.transfer16(dataToSend);
	    digitalWrite(syncPin,HIGH);//Pull up the SYNC pin to end the transmission
		
		interrupts();
	}
}

ADR5683R1::updateDACRegister(){
	noInterrupts();
	digitalWrite(syncPin,LOW);
    //Write the commmands to the DAC
    SPI.transfer16(writeDACRegisterBits);
    //Pull up the SYNC pin to end the transmission
    digitalWrite(syncPin,HIGH);
	interrupts();
}

ADR5683R1::writeAndUpdateRegisters(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
		uint16_t dataToSend=WRITE_AND_UPDATE_REGISTERS_BITS | value;
		
		noInterrupts();

		digitalWrite(syncPin,LOW);//Write the commmands to the DAC
	    SPI.transfer16(dataToSend);
		digitalWrite(syncPin,HIGH);//Pull up the SYNC pin to end the transmission
		
		interrupts();
	}
}

ADR5683R1::setDaisyChain(bool mode){	
	
	if(mode){
		controlRegister=controlRegister | DAISY_CHAIN_BIT;
	}
	else{
		controlRegister=controlRegister & ~(DAISY_CHAIN_BIT);
	}

	noInterrupts();
	
	digitalWrite(syncPin,LOW); //Write the commmands to the DAC
    SPI.transfer16(controlRegister);
    SPI.transer(ZERO_BIT);
	digitalWrite(syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	
	interrupts();
}

ADR5683R1::setGain(bool mode){
	if(mode){
		controlRegister=controlRegister | GAIN_BIT;
	}
	else{
		controlRegister=controlRegister & ~(GAIN_BIT);
	}

	noInterrupts();
	
	digitalWrite(syncPin,LOW); //Write the commmands to the DAC
    SPI.transfer16(controlRegister);
    SPI.transer(ZERO_BIT);
	digitalWrite(syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	
	interrupts();
}

