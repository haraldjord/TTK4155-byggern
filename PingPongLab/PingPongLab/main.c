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


static FILE uart_stdio = FDEV_SETUP_STREAM(
	(int(*)(char, struct __file *))USART_Transmit, 
	(int(*)(struct __file *))USART_Receive, 
	_FDEV_SETUP_RW
);

int main(void)
{
	// Sets PA0 as output and PA1 as input
	USART_Init(MYUBRR);
	DDRA = 0xFF;
	DDRE |= 0x02;
	
	// Enable external memory interface
	MCUCR |= (1 << SRE);
	// Mask PC7-PC4
	SFIOR |= (1 << XMM2);
	
	//USART_Receive();
	stdout = &uart_stdio;
	stdin = &uart_stdio;
	
	unsigned char received_char;
	//unsigned char rceived_char_old;
	
    while (1)  {

		received_char = USART_Receive();
		// printf("%c", received_char);

		if (received_char == 's') SRAM_test();
		
	}
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