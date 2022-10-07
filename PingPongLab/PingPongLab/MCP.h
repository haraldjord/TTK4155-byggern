/*
 * MCP.h
 *
 * Created: 07.10.2022 10:32:36
 *  Author: haraljor
 */ 


#ifndef MCP_H_
#define MCP_H_
#include "mcp2515.h"

void MCP_active(void){
	PORTB &= (~(1 << PB4));
}

void MCP_deactive(void){
	PORTB |= (1 << PB4);
}

unsigned char MCP_read(char address) {
	MCP_active();
	SPI_write(MCP_READ);
	SPI_write(address);
	char data = SPI_read(); 
	MCP_deactive();
	return data;
}

void MCP_write(char address, char data) {
	MCP_active();
	SPI_write(MCP_WRITE);
	SPI_write(address);
	SPI_write(data);
	MCP_deactive();
}


void MCP_rts() {
	MCP_active();
	SPI_write(MCP_RTS_ALL);
	MCP_deactive();
}

unsigned char MCP_status(void) {
	MCP_active();
	SPI_write(MCP_READ_STATUS);
	unsigned char data = SPI_read();
	MCP_deactive();
	return data;
}

void MCP_bitmod(char address, char mask, char data) {
	MCP_active();
	SPI_write(MCP_BITMOD);
	SPI_write(address);
	SPI_write(mask);
	SPI_write(data);
	MCP_deactive();
}

void MCP_reset() {
	MCP_active();
	SPI_write(MCP_RESET);
	MCP_deactive();
}

#endif /* MCP_H_ */