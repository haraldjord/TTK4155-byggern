/*
 * IR.c
 *
 * Created: 03.11.2022 14:17:38
 *  Author: chrimeh
 */ 

#include "sam.h"
#include "sam3x8e.h"
#include "IR.h"
#include "component\adc.h"

void IR_Init() {
	// Turns off write protection
	// ADC->ADC_WPMR &= ~ADC_WPMR_WPEN;
	
	PMC->PMC_PCER1 = (1 << ID_ADC - 32); // PMC_PCER1_PID32;
	ADC->ADC_MR = ADC_MR_FREERUN_ON;
	ADC->ADC_CHER = ADC_CHER_CH0;
	// Pin A7 on Due shield
	
	/*
	ADC_MR		Mode register
	ADC_CHER	Channel enable register
	ADC_CR		Control register
	ADC_CDRx	Converted data register
	ADC_LCDR	Last converted data register
	*/
	
	ADC->ADC_CR = ADC_CR_START;
}

int IR_read() {
	int IR = ADC->ADC_CDR[0];
	return IR;
}