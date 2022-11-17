/*
 * DAC.c
 *
 * Created: 04.11.2022 10:12:04
 *  Author: haraljor
 */ 

#include "DAC.h"
#include "sam.h"
#include "sam3x8e.h"
#include "printf-stdarg.h"


/* Useful registers */
// DACC_MR
// DACC_CHER
// DACC_CDR
		
		
		
void DAC_Init() {
	// PID38
	PMC->PMC_PCER1 = PMC_PCER1_PID38;
	DACC->DACC_MR = DACC_MR_USER_SEL_CHANNEL1;
	DACC->DACC_CHER = DACC_CHER_CH1;
}


void DAC_write(int val) {
	DACC->DACC_CDR = val;
}