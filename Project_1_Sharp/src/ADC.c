#include "ADC.h"
#include "DebugPorts.h"
#include "LEDs.h"
#include <MKL25Z4.H>

extern uint16_t g_Sample[6000]; 				// hold the sample
extern uint16_t g_numSamplesRemaining; 	// holds number of sample remaining to be taken

/*
INITIALIZE ADC CRITERIA
- single ended, 12 bit conversion w/o averaging
- hardware triggering from TPM0
- be configurable on user selected channel
*/
void Init_ADC() {
	
  NVIC_SetPriority(ADC0_IRQn, 3);
  NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);
	
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;			// sends clock to ADC to enable it
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTC_MASK;

	// PTE20: select analog for pin
	PORTE->PCR[20] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[20] |= PORT_PCR_MUX(0);
	// PTE22: select analog for pin
	PORTE->PCR[22] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[22] |= PORT_PCR_MUX(0);
	// PTD5: select analog for pin
	PORTD->PCR[5] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[5] |= PORT_PCR_MUX(0);
	// PTC2: select analog for pin
	PORTC->PCR[2] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[2] |= PORT_PCR_MUX(0);
	// PTC0: select analog for pin
	PORTC->PCR[0] &= ~PORT_PCR_MUX_MASK;
	PORTC->PCR[0] |= PORT_PCR_MUX(0);
	// PTE30: select analog for pin
	PORTE->PCR[30] &= ~PORT_PCR_MUX_MASK;
	PORTE->PCR[30] |= PORT_PCR_MUX(0);
		
	// choose hardware trigger from TPM0
	SIM->SOPT7 |= SIM_SOPT7_ADC0ALTTRGEN_MASK | SIM_SOPT7_ADC0TRGSEL(8);	
	
	// 12 bit single ended conversion
	// ADC0->CFG1 = ADC_CFG1_ADLPC_MASK | ADC_CFG1_ADIV(0) | ADC_CFG1_ADLSMP_MASK | ADC_CFG1_MODE(1) | ADC_CFG1_ADICLK(0);
	ADC0->CFG1 = ADC_CFG1_MODE(1) | ADC_CFG1_ADICLK(0);

	// choose default reference voltage locations
	ADC0->SC2 = ADC_SC2_REFSEL(0);	
	
	// switches muxes to channel B for PTD5 (channel 6). 
	// This does not effect other channels
	ADC0->CFG2 = ADC_CFG2_MUXSEL(1); 
	
	// enables channel 27 (the band gap 1V reference
	PMC->REGSC = PMC_REGSC_BGBE(1);
		
} // end Init_ADC

/*
while there are remaining samples, need to store ADC sample into sample array, 
decrement remaining samples, clear interrupt flag
*/
void ADC0_IRQHandler() {
	
	Control_RGB_LEDs(1, 1, 0); 									// yellow
	Toggle_Debug_Ports(0, 0, 1, 0);							// PTB2 high, others low
	
	g_Sample[1000 - g_numSamplesRemaining] = ADC0->R[0];		// takes and saves the data
	
	g_numSamplesRemaining--;										// decrements number of remaining samples
	NVIC_ClearPendingIRQ(ADC0_IRQn);						// clears
} // end IRQ Handler

/*
starts the ADC sampling from the hardware overflow and selects an input channel
*/
void ADC_start(uint8_t channel_number) {
	ADC0->SC1[0] = !ADC_SC1_AIEN_MASK | ADC_SC1_AIEN(1) | ADC_SC1_ADCH(channel_number); // triggers interrupt when a sample conversion has finished
	ADC0->SC2 |=  ADC_SC2_ADTRG(1); 																										// Enable hardware triggering via TPM0
} // end ADC_start

/*
halts the ADC sampling from the hardware overflow and selects an input channel
*/
void ADC_halt() {
	ADC0->SC2 &= ~ADC_SC2_ADTRG(1);
} // end ADC_halt
