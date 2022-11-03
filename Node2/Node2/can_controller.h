/*
 * can_controller.h
 *
 * Author: Gustav O. Often and Eivind H. J�lsgard
 *
 * For use in TTK4155 Embedded and Industrial Computer Systems Design
 * NTNU - Norwegian University of Science and Technology
 *
 */ 


	// MCK = 84 000 000 Hz
	// BRP = 41
	// SJW = 3
	// PROPAG = 7
	// PHASE1 = 7
	// PHASE2 = 7
	// OLD VALUE can_br = 0b00000000 00101001 00110111 01110111
	
	// TQ = 762 ns
	// can_br = 0b00000000 00111111 00110111 01110111
	
	// uint32_t can_br = 0x00293777; OLD VALUE


#ifndef CAN_CONTROLLER_H_
#define CAN_CONTROLLER_H_

#include <stdint.h>

typedef struct can_message_t
{
	uint16_t id;
	char data_length;
	char data[8];
} CAN_MESSAGE;

uint8_t can_init_def_tx_rx_mb(uint32_t can_br);
uint8_t can_init(uint32_t can_br, uint8_t num_tx_mb, uint8_t num_rx_mb);

uint8_t can_send(CAN_MESSAGE* can_msg, uint8_t mailbox_id);
uint8_t can_receive(CAN_MESSAGE* can_msg, uint8_t mailbox_id);

#endif /* CAN_CONTROLLER_H_ */