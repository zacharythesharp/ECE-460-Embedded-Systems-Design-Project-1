#include "DebugPorts.h"
#include "gpio_defs.h"
#include <MKL25Z4.h> 

/*
handle toggling of debug signal pins
PTB0, PTB1, PTB2, PTB3
*/

void Toggle_Debug_Ports(unsigned int zero, unsigned int one, unsigned int two, unsigned int three) {
	
	if (zero) {
		PTB->PSOR = MASK(PIN_ZERO);		// set bit
	}
	else {
		PTB->PCOR = MASK(PIN_ZERO);		// clear bit
	}
	
	if (one) {
		PTB->PSOR = MASK(PIN_ONE);		// set bit
	}
	else {
		PTB->PCOR = MASK(PIN_ONE);		// clear bit
	}
	
	if (two) {
		PTB->PSOR = MASK(PIN_TWO);		// set bit
	}
	else {
		PTB->PCOR = MASK(PIN_TWO);		// clear bit
	}
	
	if (three) {
		PTB->PSOR = MASK(PIN_THREE);		// set bit
	}
	else {
		PTB->PCOR = MASK(PIN_THREE);		// clear bit
	}
	
} // end Toggle_Debug_Ports

void Init_Debug_Ports() {
	// Enable clock to ports B
	SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
	// Make 4 pins GPIO
	PORTB->PCR[0] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[0] |= PORT_PCR_MUX(1);          
	PORTB->PCR[1] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[1] |= PORT_PCR_MUX(1);          
	PORTB->PCR[2] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[2] |= PORT_PCR_MUX(1);
	PORTB->PCR[3] &= ~PORT_PCR_MUX_MASK;          
	PORTB->PCR[3] |= PORT_PCR_MUX(1);
	
	// Set ports to outputs
	PTB->PDDR |= MASK(PIN_ZERO) | MASK(PIN_ONE) | MASK(PIN_TWO) | MASK(PIN_THREE);
	
} // end init debug ports

