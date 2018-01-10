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

ADR5683R1::ADR5683R1(uint8_t sckPin1, uint8_t syncPin1){ 
	this->syncPin=syncPin1; 
	this->sckPin=sckPin1; 
	this->controlRegister=0;
}

ADR5683R1::writeInputRegister(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
		uint16_t dataToSend=this->WRIT_INPUT_REGISTER_BIT | value;
		noInterrupts();
		
		digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	    SPI.transfer16(dataToSend);
	    digitalWrite(this->syncPin,HIGH);//Pull up the SYNC pin to end the transmission
		
		interrupts();
	}
}

ADR5683R1::updateDACRegister(){
	noInterrupts();
	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
    SPI.transfer16(this->WRITE_DAC_REGISTER_BIT);
    digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	interrupts();
}

ADR5683R1::writeAndUpdateRegisters(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
		uint16_t dataToSend=this->WRITE_AND_UPDATE_REGISTERS_BITS | value;
		
		noInterrupts();

		digitalWrite(this->syncPin,LOW);//Write the commmands to the DAC
		SPI.transfer16(dataToSend);
		digitalWrite(this->syncPin,HIGH);//Pull up the SYNC pin to end the transmission
		
		interrupts();
	}
}

//This function is only needed to derive a couple send functions for the controlregister from it.
ADR5683R1::genericSendFunction(bool mode, uint16_t mask){
	if(mode){
		this->controlRegister=this->controlRegister | this->mask;
	}
	else{
		this->controlRegister=controlRegister & ~mask;
	}

	noInterrupts();
	
	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	SPI.transfer16(controlRegister);
	SPI.transer(this->ZERO_BIT);
	digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	
	interrupts();
}

ADR5683R1::setDaisyChain(bool mode){	
	genericSendFunction(mode,this->DAISY_CHAIN_BIT);
}

ADR5683R1::setGain(bool mode){
	genericSendFunction(mode,this->GAIN_BIT);
}

ADR5683R1::setRef(bool mode){
	genericSendFunction(mode,this->REFERENCE_BIT);
}

ADR5683R1::setPD0(bool mode){
	genericSendFunction(mode,this->PDO_BIT);
}

ADR5683R1::setPD1(bool mode){
	genericSendFunction(mode,this->PD1_BIT);
}


ADR5683R1::reset(){
	this->controlRegister=this->controlRegister | this->RESET_BIT;

	noInterrupts();
	
	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	SPI.transfer16(this->controlRegister);
 	SPI.transer(ZERO_BIT);
	digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	
	interrupts();

	this->controlRegister=0;
}

ADR5683R1::begin(uint32_t clockSpeed){
	SPI.begin();
	pinMode(this->syncPin,OUTPUT);
	digitalWrite(this->syncPin,HIGH);
	if(clockSpeed <= MAX_CLOCK_FREQUENCY){
		SPI.beginTransaction(SPI(clockSpeed,MSBFIRST,SPI_MODE2));
	}
}