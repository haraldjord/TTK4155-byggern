/*
 * PingPongLab.c
 *
 * Created: 01.09.2022 09:26:13
 * Author : chrimeh
 */ 

#define F_CPU 4915200
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#include <util/delay.h>
#include <avr/io.h>


int main(void)
{
	// Sets PA0 as output and PA1 as input
	DDRA = 255;
	USART_Init(MYUBRR);
	
	unsigned char counter = 0;
	
    while (1) 
    {
		USART_Transmit(counter);
		counter++;
		
		PORTA = 0;
		_delay_ms(500);
		PORTA = 255;
		_delay_ms(500);
    }
}

void USART_Init(unsigned int ubrr)
{
	// Writes baud rate to USART Baud Rate Register H/L for port 0
	UBRR0H = ((unsigned char)(ubrr>>8)) & 0b00001111;
	UBRR0L = (unsigned char)ubrr;
	// Enables RX and TX in USART Control and Status Register B for port 0
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Asynchronous operation, 2 stop bits, 8 data bits, even parity (2<<UPM00)
	UCSR0C = (1<<URSEL0)|(1<<USBS0)|(3<<UCSZ00)|(2<<UPM00);
}

void USART_Transmit(unsigned char data)
{
	// While register is not empty, do nothing
	while( !(UCSR0A & (1<<UDRE0)) );
	
	// Write data to USART Data Register for port 0
	UDR0 = data;
}

unsigned char USART_Receive()
{
	// Wait for USART Receive Complete flag to be raised
	while( !(UCSR0A & (1<<RXC0)) );
	
	return UDR0;
}
