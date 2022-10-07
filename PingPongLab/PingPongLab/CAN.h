/*
 * CAN.h
 *
 * Created: 07.10.2022 11:39:04
 *  Author: haraljor
 */ 


#ifndef CAN_H_
#define CAN_H_
#include "MCP.h"
#include "mcp2515.h"


typedef struct Message {
	int ID;
	char data[8];
	char length;
} message;


void CAN_Init() {
	// Reset MCP and put it in loopback mode
	MCP_reset();
	MCP_write(MCP_CANCTRL, MODE_LOOPBACK);
	MCP_bitmod(0x60, 0b1100000, 0xFF);			// Turn masks/filters off
}

void CAN_send(message msg) {
	MCP_write( 0x31, (char)(msg.ID >> 3) );		// RXB0SIDH, set message identifier high bits
	MCP_write( 0x32, (char)(msg.ID << 5) );		// RXB0SIDL, set message identifier low bits
	MCP_bitmod( 0x35, 0b0001111, msg.length );	// TXB0DLC, set message length and select standard frame
	
	for (int m = 0; m < msg.length; m++) {
		MCP_write(0x36+m, msg.data[m]);			// TXB0Dm, set data byte m 
	}	
	
	MCP_bitmod(MCP_TXB0CTRL, 0b00001000, 0xFF);	// Set Message Transmit request bit high
}

message CAN_receive() {
	message msg;
	
	char SIDH = MCP_read(0x61);
	char SIDL = MCP_read(0x62);
	int SID = (((int)SIDH << 8) | ((int)SIDL)) >> 5;
	msg.ID = SID;
	
	char DLC = MCP_read(0x65) & 0b00001111;
	if (DLC > 8) DLC = 8;
	msg.length = DLC;
	
	for (int m = 0; m < DLC; m++) {
		char DM = MCP_read(0x66+m);
		msg.data[m] = DM;
	}
	
	return msg;
}

#endif /* CAN_H_ */