/*
 * PWM.c
 *
 * Created: 03.11.2022 12:07:24
 *  Author: chrimeh
 */ 

#include "sam.h"
#include "sam3x8e.h"
#include "PWM.h"
#include "pwm.h"
#include "printf-stdarg.h"

int pwm_c = 0;

void PWM_Handler(void) {
	pwm_c += 1;
	printf("%d\n\r", pwm_c/200);
}

void PWM_Init() {
	// Enables writing to PWM register group 0 (PWM_CLK config)
	// PWM->PWM_WPCR = PWM_WPCR_WPRG0 | PWM_WPCR_WPKEY(0x50574D);
	// Sets f_CLKA to MCK/1024/164 = 500.19 Hz
	// PWM->PWM_CLK |= PWM_CLK_PREA(0x0A) | PWM_CLK_DIVA(164);
	
	PMC->PMC_PCER1 |= PMC_PCER1_PID36;
	
	PIOC->PIO_OER |= PIO_OER_P19;
	PIOC->PIO_PDR |= PIO_PDR_P19;
	PIOC->PIO_ABSR |= PIO_ABSR_P19;
	
	PWM->PWM_WPCR &= ~PIO_WPMR_WPEN;
	REG_PWM_CMR5 = PWM_CMR_CPRE_MCK_DIV_1024 | PWM_CMR_CPOL; // PWM_CMR
	REG_PWM_CPRD5 = 1640; // 1680000
	
	// Sets the period to the middle value and enables PWM
	REG_PWM_CDTY5 = 123; // 126000; // 75 600 to 176 400 (0.9 ms to 2.1 ms)
	REG_PWM_ENA = PWM_ENA_CHID5;
	
	/*Setup interrupts for the PID controller*/
	REG_PWM_CMPM5 |= PWM_CMPM_CEN;
	//__disable_irq();
	//PWM->PWM_IER1 |= PWM_IER1_CHID5;
	//PWM->PWM_IER2 |= PWM_IER2_CMPM5;
	//__enable_irq();
	//NVIC_EnableIRQ(ID_PWM);
}	

void PWM_pos(int pos) {
	// maps from (-100, 100) to (75600, 176400)
	int CDTY = -pos/2 + 123; // 504*pos + 126000;
	
	/*
	// Limits the values
	if (CDTY < 75600) CDTY = 75600;
	if (CDTY > 176400) CDTY = 176400;
	*/
	
	if (CDTY < 74) CDTY = 74;
	if (CDTY > 172) CDTY = 172;
	
	// 75 600 to 176 400 (0.9 ms to 2.1 ms)
	REG_PWM_CDTYUPD5 = CDTY;
}