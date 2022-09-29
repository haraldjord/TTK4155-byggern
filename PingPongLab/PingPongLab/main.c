/*
 * PingPongLab.c
 *
 * Created: 01.09.2022 09:26:13
 * Author : chrimeh
 */ 

#define F_CPU 4915200
#define BAUD 9600
#define MYUBRR 31 //F_CPU/16/BAUD-1

#define CENTER 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define DEAD 5

#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include "fonts.h"
#include "OLED.h"

void latch_test(char received_char);
void SRAM_test(void);
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void);
void CLK_Init(int TOP);
void SRAM_Init(void);
void screen0(void);
void screen1(void);

static FILE uart_stdio = FDEV_SETUP_STREAM(
	(int(*)(char, struct __file *))USART_Transmit, 
	(int(*)(struct __file *))USART_Receive, 
	_FDEV_SETUP_RW
);

volatile char analog0 = 0;
volatile char analog1 = 0;
volatile char analog2 = 0;
volatile char analog3 = 0;
int offset_x = 0;
int offset_y = 0;

int main(void)
{
	DDRA = 0xFF;
	DDRE |= (0x02);
	
	// Joystick button inputs
	DDRD &= !( (1 << PD2) | (1 << PD3) );
	// Data/!Command output
	DDRE |= (1 << PE2);
	// Joystick button
	DDRE &= !(1 << PE0);
	
	USART_Init(MYUBRR);
	CLK_Init(0);
	SRAM_Init();
	OLED_Init();
	
	// Setup for printf
	stdout = &uart_stdio;
	stdin = &uart_stdio;
	
	unsigned char received_char = 0;
	char prev_button_left = 0;
	char prev_button_right = 0;
	char prev_joystick_button = 0;
	char ready_to_move = 1;
	char arrow_pos = 0;
	char min_pos = 0;
	char max_pos = 0;
	char screen = 0;
	char prev_screen = 1;
	
	OLED_send_command(0xA6);
	OLED_reset();
	screen1();
	
	
	
    while (1) {
		printf("a\n");
		// received_char = USART_Receive();
		/*
		char button_left = !!(PIND & (1 << PIND3));
		char button_right = !!(PIND & (1 << PIND2));
		char joystick_button = !(PINE & (1 << PINE0));
		
		char button_right_pressed, button_left_pressed, joystick_button_pressed = 0;
		
		if (button_right && !prev_button_right)
			button_right_pressed = 1;
		if (button_left && !prev_button_left)
			button_left_pressed = 1;
		if (joystick_button && !prev_joystick_button)
			joystick_button_pressed = 1;
		
		
		if (screen != prev_screen) {
			if (screen == 0) {
				screen0();
				min_pos = 2;
				max_pos = 3;
			}
			if (screen == 1) {
				screen1();
				min_pos = 0;
				max_pos = 7;
			}
			arrow_pos = min_pos;
			OLED_print_arrow(arrow_pos, 0);
		} 
		
		
		if (screen == 0 && joystick_button_pressed) {
			if (arrow_pos == min_pos) {
				printf("Button pressed\n");
			}
			if (arrow_pos == min_pos + 1) {
				screen = 1;
			}
		}
		
		
		Read_ADC();
		int x = (100*(analog0 - 128))/128 - offset_x;
		int y = (100*(analog1 - 128))/128 - offset_y;

		unsigned int direction = CENTER;
		if      (x > 0 && abs(y) < abs(x)) direction = RIGHT; 
		else if (y > 0 && abs(x) < abs(y)) direction = UP;
		else if (x < 0 && abs(y) < abs(x)) direction = LEFT;
		else if (y < 0 && abs(x) < abs(y)) direction = DOWN; 
		
		if (x*x + y*y < 80*80) direction = DEAD;
		if (x*x + y*y < 60*60) direction = CENTER;
		
		
		if (direction == CENTER)
			ready_to_move = 1;
		
		if (ready_to_move) {
			if (direction == UP) {
				OLED_goto_column(0);
				OLED_print(" ", 1);
				if (arrow_pos > min_pos) 
					arrow_pos -= 1;
				OLED_print_arrow(arrow_pos, 0);
				ready_to_move = 0;
			}
			else if (direction == DOWN) {
				OLED_goto_column(0);
				OLED_print(" ", 1);
				if (arrow_pos < max_pos) 
					arrow_pos += 1;
				OLED_print_arrow(arrow_pos, 0);
				ready_to_move = 0;
			}
			else if (direction == LEFT) {
				
				ready_to_move = 0;
			}
			else if (direction == RIGHT) {
				
				ready_to_move = 0;
			}
		}
		
		
		
		
		
		
		

		
		if (received_char == 's') {
			SRAM_test();	
		}
		else if (received_char == 'a') {
			Read_ADC();
			
			int x_pos = (100*(analog0 - 128))/128 - offset_x;
			int y_pos = (100*(analog1 - 128))/128 - offset_y;
			
			if (x_pos < -100) x_pos = -100;
			else if (x_pos > 100) x_pos = 100;
			if (y_pos < -100) y_pos = -100;
			else if (y_pos > 100) y_pos = 100;
					
			printf("%d, %d, %d, %d\n", x_pos, y_pos, analog2, analog3);
			printf("Left: %u     Right: %u\n", button_left, button_right);
		}
		
		else if (received_char == 'c') {
			x = (100*analog0 - 12800)/128;
			y = (100*analog1 - 12800)/128;
			
			Calibrate_Joystick();
		}	
		
		else if (received_char == 'r') {

			printf("r\n");			

			OLED_print_arrow(0, 0);
			char msg[] = "Hello, world!";
			int length = sizeof(msg) / sizeof(char);
			OLED_print(msg, length);
			
		}
		else if (received_char == 'k') OLED_reset();
		else if (received_char == 'i') OLED_Init();
		
		
		
		
		prev_button_left = button_left;
		prev_button_right = button_right;
		prev_screen = screen;
		prev_joystick_button = joystick_button;
		*/
	}
	
	
}







void screen0(void) {
	OLED_reset();
	OLED_pos(0, 10);
	OLED_print("Main menu", 9);
	OLED_pos(1, 10);
	for (int i = 0; i < 9*8; i++) OLED_send_data(0b00000001);
	OLED_pos(2, 10);
	OLED_print("Print test", 10);
	OLED_pos(3, 10);
	OLED_print("Options", 7);
}

void screen1(void) {
	OLED_reset();
	for (char i = 0; i < 8; i++) {
		OLED_pos(i, 10);
		OLED_print("Option ", 7);
		OLED_print_char(49 + i);
	}
}

void Calibrate_Joystick(void) {
	offset_x = 0;
	offset_y = 0;
	for (int i = 0; i < 50; i++) {
		Read_ADC();
		int x_pos = (100*analog0 - 12800)/128;
		int y_pos = (100*analog1 - 12800)/128;
		offset_x += x_pos;
		offset_y += y_pos;
	}
	offset_x /= 50;
	offset_y /= 50;
} 

void Read_ADC(void) {
	volatile char *adc_address = 0x1400;
	adc_address[0] = 0b00010000; // CH0, INH = 1, POT_X
	adc_address[0] = 0b00010010; // CH1, INH = 1, POT_Y
	adc_address[0] = 0b00010001; // CH0, INH = 1, SLIDER_L
	adc_address[0] = 0b00000011; // CH1, INH = 0, SLIDER_R
	_delay_ms(1); // wait for t_conv < 50us
	// Sequentially read channel 0-3
	analog0 = adc_address[0];
	analog1 = adc_address[0];
	analog2 = adc_address[0];
	analog3 = adc_address[0];
}

void USART_Init(unsigned int ubrr) {
	// Writes baud rate to USART Baud Rate Register H/L for port 0
	UBRR0H = ((unsigned char)(ubrr>>8));
	UBRR0L = (unsigned char)ubrr;
	// Enables RX and TX in USART Control and Status Register B for port 0
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Asynchronous operation, 2 stop bits, 8 data bits, even parity (2<<UPM00) for port 0
	UCSR0C = (1<<URSEL0)|(1<<USBS0)|(3<<UCSZ00)|(2<<UPM00);
}

void USART_Transmit(unsigned char data) {
	// While register is not empty, do nothing
	while( !(UCSR0A & (1<<UDRE0)) ){}
	
	// Write data to USART Data Register for port 0
	UDR0 = data;
}

unsigned char USART_Receive(void) {
	// Wait for USART Receive Complete flag to be raised
	while( !(UCSR0A & (1<<RXC0)) );
	
	//UCSR0A |= (1 << RXC0);
	return UDR0;
}

void latch_test(char received_char) {
	switch (received_char) {
		case 'e': PORTE &= 0b11111101; break;
		case 'd': PORTE |= 0b00000010; break;	
			
		case '1': PORTA = 0b00000001; break;
		case '2': PORTA = 0b00000010; break;
		case '3': PORTA = 0b00000100; break;
		case '4': PORTA = 0b00001000; break;
		case '5': PORTA = 0b00010000; break;
		case '6': PORTA = 0b00100000; break;
		case '7': PORTA = 0b01000000; break;
		case '8': PORTA = 0b10000000; break;
		
		default:
		printf("No port values changed");
		break;
	}
}

void SRAM_test(void)
{
	volatile char *ext_ram = (char *) 0x1800; // Start address for the SRAM
	uint16_t ext_ram_size = 0x800;
	uint16_t write_errors = 0;
	uint16_t retrieval_errors = 0;
	printf("Starting SRAM test...\n");
	// rand() stores some internal state, so calling this function in a loop will
	// yield different seeds each time (unless srand() is called before this function)
	uint16_t seed = rand();
	// Write phase: Immediately check that the correct value was stored
	srand(seed);
	for (uint16_t i = 0; i < ext_ram_size; i++) {
		uint8_t some_value = rand();
		ext_ram[i] = some_value;
		uint8_t retreived_value = ext_ram[i];
		if (retreived_value != some_value) {
			printf("Write phase error: ext_ram[%4d] = %02X (should be %02X)\n", i, retreived_value, some_value);
			write_errors++;
		}
	}
	// Retrieval phase: Check that no values were changed during or after the write phase
	srand(seed);
	// reset the PRNG to the state it had before the write phase
	for (uint16_t i = 0; i < ext_ram_size; i++) {
		uint8_t some_value = rand();
		uint8_t retreived_value = ext_ram[i];
		if (retreived_value != some_value) {
			printf("Retrieval phase error: ext_ram[%4d] = %02X (should be %02X)\n", i, retreived_value, some_value);
			retrieval_errors++;
		}
	}
	printf("SRAM test completed with \n%4d errors in write phase and \n%4d errors in retrieval phase\n\n", write_errors, retrieval_errors);
}

void SRAM_Init(void) {
	// Enable external memory interface
	MCUCR |= (1 << SRE);
	// Mask PC7-PC4
	SFIOR |= (1 << XMM2);
}

void CLK_Init(int TOP) {
	// Set PD4 as output
	DDRD |= (1 << DDD4);
	
	// Compare output mode: Toggle compare match
	TCCR3A |= (1 << COM3A0); 
	
	// Mode 4, CTC
	TCCR3B |= (1 << WGM32);
	
	// Set counter TOP value
	OCR3A = TOP;
	
	// Prescaler 1
	TCCR3B |= (1 << CS30);
}

