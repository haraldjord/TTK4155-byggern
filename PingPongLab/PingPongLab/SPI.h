/*
 * SPI.h
 *
 * Created: 06.10.2022 11:59:52
 * Author: haraljor
 */ 


#ifndef SPI_H_
#define SPI_H_
#include <avr/io.h>

void SPI_Init(void) {
	// Sets MOSI, SCK, !SS as outputs and MISO as input
	DDRB |= (1 << DDB4) | (1 << DDB5) | (1 << DDB7);
	DDRB &= (~(1 << DDB6));
	
	// Enable SPI as master, clock rate f_osc/16
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);	
}

// Loopback mode
// XFh: 010 00111

void SPI_write(unsigned char data){
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
}

unsigned char SPI_read(){
	SPDR = 0xFF;
	while(!(SPSR & (1 << SPIF)));
	return SPDR;
}


#endif /* SPI_H_ */