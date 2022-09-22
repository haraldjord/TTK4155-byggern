/*
 * PingPongLab.c
 *
 * Created: 01.09.2022 09:26:13
 * Author : chrimeh
 */ 

#define F_CPU 4915200
#define BAUD 9600
#define MYUBRR 31 //F_CPU/16/BAUD-1

#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

void latch_test(char received_char);
void SRAM_test(void);
void USART_Init(unsigned int ubrr);
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void);
void CLK_Init(int TOP);
void SRAM_Init(void);
void OLED_Init(void);
void Write_OLED(char msg, char cd);


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
	
	USART_Init(MYUBRR);
	CLK_Init(0);
	SRAM_Init();
	OLED_Init();
	
	// Setup for printf
	stdout = &uart_stdio;
	stdin = &uart_stdio;
	
	unsigned char received_char = 0;
	
    while (1) {
				
		received_char = USART_Receive();
		char button_left = !!(PIND & (1 << PIND3));
		char button_right = !!(PIND & (1 << PIND2));
		
		
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
			int x_pos = (100*analog0 - 12800)/128;
			int y_pos = (100*analog1 - 12800)/128;
			
			Calibrate_Joystick();
		}	
		
		else if (received_char == 'r') {
						
			OLED_Init();
			
			printf("r\n");
			
			Write_OLED(0b00000000, 1);
			Write_OLED(0b00011000, 1);
			Write_OLED(0b00011000, 1);
			Write_OLED(0b00011000, 1);
			Write_OLED(0b01111110, 1);
			Write_OLED(0b00111100, 1);
			Write_OLED(0b00011000, 1);
			Write_OLED(0b00000000, 1);
		}

	}
}


// Copied directly from datasheet
void OLED_Init(void) {
	Write_OLED(0xae, 0); // display off
	Write_OLED(0xa1, 0); //segment remap
	Write_OLED(0xda, 0); //common pads hardware: alternative
	Write_OLED(0x12, 0);
	Write_OLED(0xc8, 0); //common output scan direction:com63~com0
	Write_OLED(0xa8, 0); //multiplex ration mode:63
	Write_OLED(0x3f, 0);
	Write_OLED(0xd5, 0); //display divide ratio/osc. freq. mode
	Write_OLED(0x80, 0);
	Write_OLED(0x81, 0); //contrast control
	Write_OLED(0x50, 0);
	Write_OLED(0xd9, 0); //set pre-charge period
	Write_OLED(0x21, 0);
	Write_OLED(0x20, 0); //Set Memory Addressing Mode
	Write_OLED(0x02, 0);
	Write_OLED(0xdb, 0); //VCOM deselect level mode
	Write_OLED(0x30, 0);
	Write_OLED(0xad, 0); //master configuration
	Write_OLED(0x00, 0);
	Write_OLED(0xa4, 0); //out follows RAM content
	Write_OLED(0xa6, 0); //set normal display
	Write_OLED(0xaf, 0); // display on 
}

void Write_OLED(char msg, char cd) {
	// C/!D
	if (cd) {
		PORTE |= (1 << PINE2);
		PORTC &= 0x11110000;  // Enables CS
	}
	else {
		PORTE &= !(1 << PINE2);
		PORTC &= 0x11110010;
		PORTC |= 0x00000010;
	}
	
	_delay_ms(1);
	PORTD &= !(1 << 6);   // Sets !WR low
	_delay_ms(1);	
	PORTA = msg;		  // Puts msg on data bus
	_delay_ms(1);
	PORTD |= (1 << 6);	  // Disables CS
	_delay_ms(1);
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
	// Set PD5 as output
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

