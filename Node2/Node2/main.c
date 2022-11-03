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
#include <string.h>
#include <math.h>

int map_int(int x, int in_min, int in_max, int out_min, int out_max);

int main(void)
{
	WDT->WDT_MR = WDT_MR_WDDIS; // Turns off watchdog
    SystemInit();
	configure_uart();
	PWM_Init();
	IR_Init();

	uint32_t can_br = 0x003F3777;
	can_init_def_tx_rx_mb(can_br);
	
	int x[filter_samples];
	int y[filter_samples];
	int IR[filter_samples];
	
	int score_status = 0;
	int score = 0;
	
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



		if (filtered_IR < 2000 && !score_status) {
			score += 1;
			score_status = 1;
		}
		
		
		if (score_status && button_right) {
			score_status = 0;
		}


		printf("%d\n\r", score);
		
    }
}

int map_int(int x, int in_min, int in_max, int out_min, int out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}