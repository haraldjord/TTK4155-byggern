/*
 * Node2.c
 *
 * Created: 13.10.2022 13:08:29
 * Author : chrimeh
 */ 

#define filter_samples 50
#define x_offset 50
#define START 0
#define FIRST_READ 1
#define SECOND_READ 2

// For setting and clearing PIOD
#define DIR		(1 << 10)
#define EN		(1 << 9)
#define SEL		(1 << 2)
#define NOT_RST (1 << 1)
#define NOT_OE  (1 << 0)	

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
volatile int pulse_counter = 0;
	

// Ziegler-Nichols tuning
// Ku = 1.80;
// Tu = 0.81;
float Kp = 0.81; // 0.45 * Ku;
float Ki = 0.20; // 0.54 * Ku/Tu;

// PID variables
const float T = 0.02f; // For 50 Hz
volatile int e = 0;
volatile int ei = 0;
volatile int u = 0;

void Encoder_update(void);
void Encoder_reset(void);
volatile char encoder_stage = START; // Stage of reading encoder
char encoder_flag = 0;				 // high flag indicates 20 us has passed and a read can be performed
char encoder_rts = 1;				 // Ready to start read operation from stage START.
int encoder_pos = 0;				 // Encoder position to be used in main loop
int  encoder_temp = 0;				 // Used to store high byte during read operation
int  reference_pos = 0;				 // Reference position for PID control (in terms of encoder value)
char encoder_update_requested = 1;
	
	
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
	int prev_button_left = 0;
	
	
	
	// PIN51 3v3 solenoid control
	PIOC->PIO_PER = PIO_PER_P12;
	PIOC->PIO_OER = PIO_OER_P12;
	
	// Motor interface box encoder pins
	PIOC->PIO_ODR = (0xFF << 1);
	PMC->PMC_PCER0 = PMC_PCER0_PID13;
	PMC->PMC_PCR |= PMC_PCR_PID(13) | PMC_PCR_EN;
	
	/* Motor interface box control pins
	DIR		PIN32	PD10
	EN 		PIN30	PD9
	SEL		PIN27	PD2
	NOT_RST	PIN26	PD1
	NOT_OE	PIN25	PD0
	*/

	PIOD->PIO_PER = PIO_PER_P10 | PIO_PER_P9 | PIO_PER_P2 | PIO_PER_P1 | PIO_PER_P0;
	PIOD->PIO_OER = PIO_OER_P10 | PIO_OER_P9 | PIO_OER_P2 | PIO_OER_P1 | PIO_OER_P0;
	
	PIOD->PIO_SODR = DIR | EN | NOT_RST;
	
	
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
		/*
		int dac_val;
		if (remapped_filtered_x > 0) {
			dac_val = map_int(remapped_filtered_x, 0, 100, 0, 4096);
			PIOD->PIO_SODR = DIR;
		}
		else {
			dac_val = map_int(-remapped_filtered_x, 0, 100, 0, 4096);
			PIOD->PIO_CODR = DIR;
		}
		DAC_write(dac_val*3/4);
		*/
		
		// reference_pos = map_int(-remapped_filtered_x, -100, 100, -5000, 5000);; //map_int(remapped_filtered_x, -100, 100, -32768, 32767);
		reference_pos = map_int(-slider_right, -255, 0, -5000, 5000);; //map_int(remapped_filtered_x, -100, 100, -32768, 32767);
		// printf("%d, %d\n\r", slider_right, reference_pos);
		if (u > 0) {
			PIOD->PIO_CODR = DIR;
		}
		else {
			PIOD->PIO_SODR = DIR;
		}
		DAC_write(abs(u));
		
		
		printf("%d\n\r", slider_right);
		

		
		
		if (!prev_button_left && button_left) {
			Encoder_reset();
		}
		
		/* Pulse control for solenoid */
		if (!prev_button_right && button_right) {
			// solenoid enable
			PIOC->PIO_CODR = PIO_SODR_P12; // PIN51 on Due
			pulse_counter = 0;
		}
		if (pulse_counter > 10) {
			// solenoid disable
			PIOC->PIO_SODR = PIO_CODR_P12;
		}
		if (pulse_counter > 60000) // In case of overflow
			pulse_counter = 100;
		
		
		
		// Update encoder position every 20 ms
		if (encoder_update_requested) {
			encoder_update_requested = 0;
			//printf("%d\n\r", encoder_pos);
			Encoder_update();
		}
			
		prev_button_left = button_left;
		prev_button_right = button_right;
    }
}










int map_int(int x, int in_min, int in_max, int out_min, int out_max) {
	int val = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	
	// Limiting
	if (val > out_max) return out_max;
	else if (val < out_min) return out_min;
	return val;
}

void CLK_Init() {
	PMC->PMC_PCER0 = PMC_PCER0_PID27 | (1 << ID_TC3); // ID_TC3 corresponds to Counter 1 channel 0
	PMC->PMC_PCR = PMC_PCR_PID(27) | PMC_PCR_EN;
	PMC->PMC_PCR = PMC_PCR_PID(28) | PMC_PCR_EN;
	
	
	// Clock 0 for solenoid timing and PID updates
	REG_TC0_RC0 = 840000; // 50 Hz (T = 20 ms)
	REG_TC0_CMR0 = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVSEL_UP_RC;
	
	// Clock 1 for 20 us read delay on encoder
	TC1->TC_CHANNEL[0].TC_RC = 840; // 50 000 Hz (T = 20 us)
	TC1->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 | TC_CMR_WAVSEL_UP  | TC_CMR_CPCSTOP; // | TC_CMR_CPCDIS; // Stop on RC compare
	
	REG_TC0_IER0 = TC_IER_CPCS; // Interrupt on C compare
	TC1->TC_CHANNEL[0].TC_IER = TC_IER_CPCS; // Interrupt on C compare
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_EnableIRQ(TC3_IRQn);
	
	REG_TC0_CCR0 = TC_CCR_CLKEN | TC_CCR_SWTRG;
	TC1->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN;
	
}

void TC0_Handler() {
	/* Called every 20 ms to update PI-controller value */
	int k = REG_TC0_SR0 & TC_SR_CPCS;
	int g = REG_TC0_SR1 & TC_SR_CPCS;
	
	pulse_counter += 1;
	
	int encoder_pos_signed = encoder_pos;
	if (encoder_pos_signed >= 32768)
		encoder_pos_signed -= 65536;
	
	e = reference_pos - encoder_pos_signed;
	// printf("%d\n\r", encoder_pos_signed);
	ei += e;
	// if (ei > )
	u = Kp*e + T*Ki*ei; // Maybe limit values
	encoder_update_requested = 1;
}

void TC3_Handler() {
	/* Called when TC1 has counted to 840 (20 us) */
	
	int k = TC1->TC_CHANNEL[0].TC_SR;
	encoder_flag = 1; // Set to signify that 20 us has passed
	
	// Update next update stage
	if (encoder_stage == START) encoder_stage = FIRST_READ;
	else if (encoder_stage == FIRST_READ) encoder_stage = SECOND_READ;
	else encoder_stage = START;

}

void Encoder_update() {
	/* Called to update the encoder position */
	
	/* Motor interface box control pins
	DIR		PIN32	PD10
	EN 		PIN30	PD9
	SEL		PIN27	PD2
	NOT_RST	PIN26	PD1
	NOT_OE	PIN25	PD0
	*/

	if (encoder_stage == START && encoder_rts) {
		encoder_rts = 0;
		encoder_flag = 0;
		PIOD->PIO_CODR = NOT_OE; // Set !OE low
		PIOD->PIO_CODR = SEL;	 // Set SEL low to output high byte
		TC1->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;
	}
	else if (encoder_stage == FIRST_READ && encoder_flag) {
		encoder_flag = 0;
		int MJ2 = (PIOC->PIO_PDSR & (0xFF << 1)) >> 1;
		encoder_temp = (MJ2 << 8);
		PIOD->PIO_SODR = SEL;	// Set SEL high to output low byte
		TC1->TC_CHANNEL[0].TC_CCR = TC_CCR_SWTRG;
	}
	else if (encoder_stage == SECOND_READ && encoder_flag) {
		encoder_flag = 0;
		int MJ2 = (PIOC->PIO_PDSR & (0xFF << 1)) >> 1;
		PIOD->PIO_SODR = NOT_OE; // Set !OE high
		
		// Overwrite global variable for position
		encoder_pos = encoder_temp | MJ2;
		encoder_rts = 1;
		encoder_stage = START;
	}
	
} 

void Encoder_reset() {
	/* Resets encoder position */
	int prev_pulse = pulse_counter;
	PIOD->PIO_CODR = NOT_RST;
	while (pulse_counter == prev_pulse);
	PIOD->PIO_SODR = NOT_RST;
	
	// Reset the error integral
	ei = 0;
}