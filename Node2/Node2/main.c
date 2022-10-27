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
	WDT->WDT_MR = WDT_MR_WDDIS; // Turns off watchdog
    SystemInit();
	configure_uart();
	
	// MCK = 84 000 000 Hz
	// BRP = 41
	// SJW = 3
	// PROPAG = 7
	// PHASE1 = 7
	// PHASE2 = 7
	// OLD VALUE can_br = 0b00000000 00101001 00110111 01110111 
	
	// TQ = 762 ns
	// can_br = 0b00000000 00111111 00110111 01110111
	
	// uint32_t can_br = 0x00293777;
	uint32_t can_br = 0x003F3777;
	
	// can_init_def_tx_rx_mb(can_br);
	can_init_def_tx_rx_mb(can_br);
	
	
    // LED is on PA19 and PA20
    
    // PER: PIO Enable Register
	// OER: Output Enable Register
    PIOA->PIO_PER = PIO_PER_P19 | PIO_PER_P20;
    PIOA->PIO_OER = PIO_OER_P19 | PIO_OER_P20;
	
	// For PWM: PC22
	PIOC->PIO_PDR |= PIO_PDR_P22;
	PIOC->PIO_ABSR |= PIO_ABSR_P22;
		
	/* Useful PWM registers */
	// PWM_CLK
	// PWM_CMRn
	// PWM_CPRD(UPD)n
	// PWM_CDTY(UPD)n
	// PWM_ENA	(PWM_DIS)

	
	PWM->PWM_CLK |= (0x0A << PWM_CLK_PREA) | (0x0A << PWM_CLK_PREB);
	PWM->PWM_CLK |= (255 << PWM_CLK_DIVA)  | (255 << PWM_CLK_DIVB);
		
	
		
    while (1) 
    {	
		
		extern CAN_MESSAGE message;
		
		printf("ID: %d, length: %d, message: ", message.id, message.data_length);
		for (int i = 0; i < message.data_length; i++)
			printf("%d ", message.data[i]);
		printf("\n\r");
		
    }
}
