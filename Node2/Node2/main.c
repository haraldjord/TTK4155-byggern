/*
 * Node2.c
 *
 * Created: 13.10.2022 13:08:29
 * Author : chrimeh
 */ 

#define filter_samples 50
#define x_offset 50

#include "sam.h"
#include "sam3x8e.h"
#include "printf-stdarg.h"
#include "uart.h"
#include "can_controller.h" 
#include "can_interrupt.h"
#include "PWM.h"
#include "IR.h"
#include "DAC.h"
#include <string.h>

int map_int(int x, int in_min, int in_max, int out_min, int out_max);

void CLK_Init(void);
int counter_val = 0;
int pulse_counter = 0;
	
// PID variables
const float Kp = 1;
const float Ki = 1;
const float T = 0.02f; // For 50 Hz
volatile float e = 0;
volatile float ei = 0;
volatile float u = 0;
	
int main(void)
{
	WDT->WDT_MR = WDT_MR_WDDIS; // Turns off watchdog
    SystemInit();
	configure_uart();
	PWM_Init();
	IR_Init();
	DAC_Init();
	CLK_Init();

	uint32_t can_br = 0x003F3777;
	can_init_def_tx_rx_mb(can_br);
	
	int x[filter_samples];
	int y[filter_samples];
	int IR[filter_samples];

	int score_status = 0;
	int score = 0;
	
	int prev_button_right = 0;
	
	
	PIOD->PIO_PER = PIO_PER_P10;
	PIOD->PIO_OER = PIO_OER_P10;
	
	PIOC->PIO_PER = PIO_PER_P12;
	PIOC->PIO_OER = PIO_OER_P12;
	
	
    while (1) 
    {	
		
		extern CAN_MESSAGE message;
		
		/*
		printf("ID: %d, length: %d, message: ", message.id, message.data_length);
		for (int i = 0; i < message.data_length; i++)
			printf("%d ", message.data[i]);
		printf("\n\r");
		*/
		
		x[0] = message.data[0]-100;
		y[0] = message.data[1]-100;
		int slider_left = message.data[2];
		int slider_right = message.data[3];
		int button_left = message.data[4];
		int button_right = message.data[5];
		IR[0] = IR_read();
		
		/* Crude low-pass (Kalman) filter */
		int filtered_x = 0;
		int filtered_y = 0;
		int filtered_IR = 0;
		for (int i = filter_samples - 1; i > 0; i--) {
			x[i] = x[i-1];
			y[i] = y[i-1];
			IR[i] = IR[i-1];
		}
		for (int i = 0; i < filter_samples; i++) {
			filtered_x += x[i];
			filtered_y += y[i];
			filtered_IR += IR[i]/filter_samples;
		}
		filtered_x /= filter_samples;
		filtered_y /= filter_samples;
				
		int remapped_filtered_x = 0;
		if (filtered_x < x_offset) 
			remapped_filtered_x = map_int(filtered_x, -100, x_offset, -100, 0);
		else
			remapped_filtered_x = map_int(filtered_x, x_offset, 100, 0, 100);
					
		PWM_pos(remapped_filtered_x);


		/* Score detection */
		if (filtered_IR < 2000 && !score_status) {
			score += 1;
			score_status = 1;
		}
		if (score_status && button_right) {
			score_status = 0;
		}
		
		/* Writing DAC value */
		int dac_val;
		if (remapped_filtered_x > 0) {
			dac_val = map_int(remapped_filtered_x, 0, 100, 0, 4096);
			PIOD->PIO_SODR = PIO_SODR_P10;
		}
		else {
			dac_val = map_int(-remapped_filtered_x, 0, 100, 0, 4096);
			PIOD->PIO_CODR = PIO_CODR_P10;
		}
		DAC_write(dac_val*3/4);
		
		// PC1 to PC8 = encoder pos
		PIOB->PIO_ODR = (0xFF << 1);
		printf("%d\n\r", PIOB->PIO_PDSR);
		
		
		
		/* Pulse control for solenoid */
		if (!prev_button_right && button_right) {
			// solenoid enable
			PIOC->PIO_CODR = PIO_SODR_P12;
			pulse_counter = 0;
		}
		if (pulse_counter > 10) {
			// solenoid disable
			PIOC->PIO_SODR = PIO_CODR_P12;
		}
		
		
		
				
		
		prev_button_right = button_right;
    }
}

int map_int(int x, int in_min, int in_max, int out_min, int out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void CLK_Init() {
	PMC->PMC_PCER0 = PMC_PCER0_PID27;
	PMC->PMC_PCR = PMC_PCR_PID(27) | PMC_PCR_EN;
	
	REG_TC0_RC0 = 840000; // 50 Hz
	REG_TC0_CMR0 = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVSEL_UP_RC;
	
	//__disable_irq();
	REG_TC0_IER0 = TC_IER_CPCS; // Interrupt on C compare
	//__enable_irq();
	NVIC_EnableIRQ(ID_TC0);
	
	REG_TC0_CCR0 = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

void TC0_Handler() {
	int k = REG_TC0_SR0;
	counter_val += 1;
	pulse_counter += 1;
	
	//if (counter_val % 50 == 0)
	//	printf("%d\n\r", counter_val/50);
	
	ei += e;
	u = Kp*e + T*Ki*ei;
}