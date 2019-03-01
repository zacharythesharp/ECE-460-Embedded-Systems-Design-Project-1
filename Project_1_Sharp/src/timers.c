#include "timers.h"
#include <MKL25Z4.h> 
#include "LEDs.h"

void Init_Timer_Output(){
	// Enable clock to port A
//	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;;
	
	// PTA12 connected to FTM1_CH0, Mux Alt 3
	// Set pin to FTM
//	PORTA->PCR[12] &= ~PORT_PCR_MUX_MASK;          
//	PORTA->PCR[12] |= PORT_PCR_MUX(3);          

	// Configure TPM
	SIM->SCGC6 |= SIM_SCGC6_TPM0_MASK;

	//set clock source for tpm: 48 MHz
	SIM->SOPT2 |= (SIM_SOPT2_TPMSRC(1) | SIM_SOPT2_PLLFLLSEL_MASK);

	//load the counter and mod
	TPM0->MOD = 240-1;//(BASE_CLK/(DESIRED_CLK*32))-1; // 48MHz / (100kHz * 32)  

	//set TPM count direction to up with a divide by 2 prescaler 
	TPM0->SC = TPM_SC_PS(1);

	// Continue operation in debug mode
	TPM0->CONF |= TPM_CONF_DBGMODE(3);

	// Start TPM
	TPM0->SC |= TPM_SC_CMOD(1);

}


// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
