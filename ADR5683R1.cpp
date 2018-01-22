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

adr5683r1::adr5683r1(uint8_t sckPin1, uint8_t syncPin1){ 
	this->syncPin=syncPin1; 
	this->sckPin=sckPin1; 
	this->controlRegister=0;
  SPI.setSCK(sckPin1);
}

uint16_t adr5683r1::getControlRegister(){
  return(this->controlRegister);
}
/*
void adr5683r1::writeInputRegister(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
		uint16_t dataToSend=(this->WRITE_INPUT_REGISTER_BIT) | value;
		noInterrupts();
		
		digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
    	SPI.transfer16(dataToSend);
    	digitalWrite(this->syncPin,HIGH);//Pull up the SYNC pin to end the transmission
		
		interrupts();
	}
}

void adr5683r1::updateDACRegister(){
	noInterrupts();
	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	SPI.transfer16(this->WRITE_DAC_REGISTER_BIT);
	//SPI.transfer(this->ZERO_BIT);
	digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	interrupts();
}
*/
void adr5683r1::writeAndUpdateRegisters(uint16_t value){
	//Check if the value is bigger than 12 Bit.
	if(value<MAX_VALUE){
	uint16_t dataToSend=this->WRITE_AND_UPDATE_REGISTER_BITS | value;
	
	noInterrupts();

	digitalWrite(this->syncPin,LOW);//Write the commmands to the DAC
	SPI.transfer16(dataToSend);
	digitalWrite(this->syncPin,HIGH);//Pull up the SYNC pin to end the transmission
	
	interrupts();
	}
}

//This function is only needed to derive a couple send functions for the controlregister from it.
void adr5683r1::genericSendFunction(bool mode, uint16_t mask){
	
	if(mode){
		this->controlRegister=this->controlRegister | mask;
	}
	else{
		this->controlRegister=this->controlRegister & ~mask;
	}

	uint16_t dataToSend = WRITE_CONTROL_REGISTER_BIT | this->controlRegister;

	//Serial.println(dataToSend,BIN);

	noInterrupts();

	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	SPI.transfer16(dataToSend);
	SPI.transfer(this->ZERO_BIT);
	digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission

	interrupts();
}

void adr5683r1::setDaisyChain(bool mode){	
	genericSendFunction(mode,this->DAISY_CHAIN_BIT);
}

void adr5683r1::setGain(bool mode){
	genericSendFunction(mode,this->GAIN_BIT);
}

void adr5683r1::setRef(bool mode){
	genericSendFunction(mode,this->REFERENCE_BIT);
}

void adr5683r1::setPD0(bool mode){
	genericSendFunction(mode,this->PD0_BIT);
}

void adr5683r1::setPD1(bool mode){
	genericSendFunction(mode,this->PD1_BIT);
}


void adr5683r1::resetDAC(){
	this->controlRegister=this->controlRegister | this->RESET_BIT;

	noInterrupts();
	
	digitalWrite(this->syncPin,LOW); //Write the commmands to the DAC
	SPI.transfer16(this->controlRegister);
 	SPI.transfer(this->ZERO_BIT);
	digitalWrite(this->syncPin,HIGH); //Pull up the SYNC pin to end the transmission
	
	interrupts();

	this->controlRegister=0;
}
void adr5683r1::beginDAC(uint32_t clockSpeed){
	SPI.begin();
	pinMode(this->syncPin,OUTPUT);
	digitalWrite(this->syncPin,HIGH);
	if(clockSpeed <= MAX_CLOCK_FREQUENCY){
		SPI.beginTransaction(SPISettings(clockSpeed,MSBFIRST,SPI_MODE2));
	}
}
