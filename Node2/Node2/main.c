/*
 * Node2.c
 *
 * Created: 13.10.2022 13:08:29
 * Author : chrimeh
 */ 

#include "sam.h"
#include "sam3x8e.h"
#include "printf-stdarg.h"
#include "uart.h"
#include "can_controller.h"
#include "can_interrupt.h"
#include <string.h>

int main(void)
{
    SystemInit();
	configure_uart();
	
	// MCK = 84 000 000 Hz
	// BRP = 41
	// SJW = 3
	// PROPAG = 7
	// PHASE1 = 7
	// PHASE2 = 7
	// can_br = 0b00000000 00101001 00110111 01110111
	
	uint32_t can_br = 0x00293777;
	
	can_init_def_tx_rx_mb(can_br);
	
    // LED is on PA19 and PA20
    
    // PER: PIO Enable Register
	// OER: Output Enable Register
    PIOA->PIO_PER = PIO_PER_P19 | PIO_PER_P20;
    PIOA->PIO_OER = PIO_OER_P19 | PIO_OER_P20;
	
		
    while (1) 
    {
		printf("Test\r");
		
		CAN_MESSAGE msg;
		msg.id = 12;
		msg.data_length = 4;
		strcpy(msg.data, "Test");
		
		can_send(&msg, 0);
		
		// SODR: Set Output Data Register
		// CODR: Clear Output Data Register
	
		/*
		PIOA->PIO_SODR = PIO_SODR_P19;
		PIOA->PIO_CODR = PIO_CODR_P20;
		// delay();
		
		PIOA->PIO_CODR = PIO_CODR_P19;
		PIOA->PIO_SODR = PIO_SODR_P20;
		// delay();
		*/
    }
}
