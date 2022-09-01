/*
 * PingPongLab.c
 *
 * Created: 01.09.2022 09:26:13
 * Author : chrimeh
 */ 

#define F_CPU 5000000
#include <util/delay.h>
#include <avr/io.h>


int main(void)
{
	
	// Sets PA0 as output and PA1 as input
	DDRA = 255;
	
    while (1) 
    {
		PORTA = 0;
		_delay_ms(500);
		PORTA = 255;
		_delay_ms(500);
		
		
		// Reads PA1 and writes it to PORTA
		//PORTA = (PINA & 0b00000010) >> 1;
    }
}

