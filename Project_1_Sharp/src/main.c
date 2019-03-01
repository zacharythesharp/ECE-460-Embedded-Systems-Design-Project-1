/*----------------------------------------------------------------------------
Author: In collaboration with code from Alexander Dean, Zach Sharp
 *----------------------------------------------------------------------------*/
#include <MKL25Z4.H>
#include <stdio.h>
#include "LEDs.h"
#include "gpio_defs.h"
#include "UART.h"
#include "delay.h"
#include "timers.h"
#include "ADC.h"
#include "DebugPorts.h"

//--------------- GLOBALS ------------------------------------------------------
uint16_t g_Sample[6000]; 								// hold the sample
uint16_t g_numSamplesRemaining = 1000; 	// holds number of sample remaining to be taken

/*----------------------------------------------------------------------------
  MAIN function
	
	Change definition of USE_UART_INTERRUPTS in UART.h to select polled or 
	interrupt-driven communication.
	 *----------------------------------------------------------------------------*/
int main (void) {
	
	// BUFFER LOCALS	
	float Vtemp25 = 948.0058;								// room temp calibration value
	float m_rising = 1.769;									// rising temp slope
	float m_falling = 1.646;								// falling temp slope
	char volts[50];													// holds the temporary voltage float value to be printed via UART 
	unsigned int i = 1;											// used for loop indexing
	float voltage = 0.0;										// temp voltage value converted from the ADC
	float temperature = 0.0;								// temperature value converted from the ADC
	uint8_t channel_number = 0;							// default channel number of zero
	uint8_t buffer[80], c, * bp, * temp_bp;	// used to parse commands coming in from UART terminal
	bp = &buffer[0]; 												// points to first element of buffer array
	temp_bp = &buffer[0];										// temp bp used in printing the character in UART

	
	// BASIC INITS
	Init_UART0(115200);						// initiliazes UART
	Init_RGB_LEDs();							// initializes LEDs
	Init_Debug_Ports();						// initializes Debug Ports
	Init_Timer_Output();					// Initializes TPM0 
	__enable_irq();								// enable IRQ functionality
	Init_ADC();										// Initializes ADC
	
#if USE_UART_INTERRUPTS==0 // Polling version of code

	Send_String_Poll("\n\rZach Sharp - ECE460: \n\n\r");
	Send_String_Poll("Valid Commands:\n\r");
	Send_String_Poll("\tR<enter>\n\r\tS<enter>\n\r\tC#<enter>\n\r\tC##<enter>\n\n\r");
	

	// Code listing 8.9, p. 233
	while (1) {
		
		// Finite State Machine
		static enum {SD, CLI, AS, SS} next_state = CLI;
		switch (next_state) {
			// ------------------------------------------------------------------------------------
			case CLI: // command line interface
				Control_RGB_LEDs(0, 0, 1);			// blue
				Toggle_Debug_Ports(1, 0, 0, 0);	// PTB0 high, others low
			
				c = UART0_Receive_Poll();				// receive current UART character
		
				/* if statement that captures unique terminal character through UART, 
				stores it in buffer array, and increments pointers */
				if (c != 0x0D && c != NULL) {
					*bp = c;											// contents at buffer index = c
					bp++;													// increment bp pointer to next index address
					UART0_Transmit_Poll(c);				// print current character for live terminal feed
				} 
		
				/* if statment that prints entire command in the buffer 
				and sets the next state for next state logic */
				if (c == 0x0D) {
					*bp = c;											// contents at buffer index = c
					bp++;													// increment bp pointer to next index address
					Send_String_Poll("\n\r");			// prints new line and carriage return to terminal
					
					if (buffer[0] == 0x52 && buffer[1] == 0x0D) {				// R input
						next_state = AS;
						Send_String_Poll("\n\rStart Recording.\n\r");
					}
					
					else if (buffer[0] == 0x53 && buffer[1] == 0x0D) { 	// S input
						next_state = SD;
						Send_String_Poll("\n\rSending Data (1000 samples).\n\r");
					}
					
					else if (buffer[0] == 0x43) {	// C input
						if ((buffer[1] == 0x30 || buffer[1] == 0x33 || buffer[1] == 0x36) && buffer[2] == 0x0D) { // 0/3/6 input
							next_state = CLI;
							channel_number = buffer[1];
							Send_String_Poll("\n\rChannel ");
							Send_String_Poll(&channel_number);
							Send_String_Poll(" Selected.\n\r");
							channel_number = buffer[1] - 0x30;
						}
						// 11/14/23/26/27 input
						else if (((buffer[1] == 0x31 && buffer[2] == 0x31) || (buffer[1] == 0x31 && buffer[2] == 0x34) || 
											(buffer[1] == 0x32 && buffer[2] == 0x33) || (buffer[1] == 0x32 && buffer[2] == 0x36) ||
											(buffer[1] == 0x32 && buffer[2] == 0x37)) && buffer[3] == 0x0D) {
							next_state = CLI;

							// Saving channel number
							Send_String_Poll("\n\rChannel ");
												
							if (buffer[2] == 0x31) {
								channel_number = 11;
								Send_String_Poll("1");
						    Send_String_Poll("1");
							}
							else if (buffer[2] == 0x34) {
								channel_number = 14;
								Send_String_Poll("1");
						    Send_String_Poll("4");
							}
							else if (buffer[2] == 0x33) {
								channel_number = 23;
								Send_String_Poll("2");
						    Send_String_Poll("3");
							}
							else if (buffer[2] == 0x36) {
								channel_number = 26;
								Send_String_Poll("2");
						    Send_String_Poll("6");
							}
							else if (buffer[2] == 0x37) {
								channel_number = 27;
								Send_String_Poll("2");
						    Send_String_Poll("7");
							}
												
							Send_String_Poll(" Selected.\n\r");
						}
						else {
							Send_String_Poll("\n\rInvalid Input.\n\r");
						}
					}
					
					else {
						Send_String_Poll("\n\rInvalid Input.\n\r");
					}
					
					while (bp >= temp_bp) {						// while temp pointer and real pointer are different
					//UART0_Transmit_Poll(*temp_bp);	// transmit current character
						*temp_bp = NULL;								// sets data at specific address to null
						temp_bp++;											// increments temp pointer
					}
					Send_String_Poll("\n\r");					// prints new line and carriage return to terminal
					bp = &buffer[0];									// resets main pointer
					temp_bp = &buffer[0];							// resets temp pointer
					c = NULL;													// sets current incoming character to NULL
				}
				break;
		
			// ------------------------------------------------------------------------------------
			case AS:	// awaiting samples
				Control_RGB_LEDs(0, 1, 0); 						// green
				Toggle_Debug_Ports(0, 1, 0, 0);				// PTB1 high, others low
				
				ADC_start(channel_number);						// begins the ADC
					while (g_numSamplesRemaining > 0) {
						Control_RGB_LEDs(0, 1, 0); 				// green
						Toggle_Debug_Ports(0, 1, 0, 0);		// PTB1 high, others low
					}
				ADC_halt();														// stops the ADC
				g_numSamplesRemaining = 1000;					// resets sample number
				next_state = CLI;
			
				break;
		
			/*
			// ------------------------------------------------------------------------------------
			case SS:	// saving samples (via ADC ISR)
				Control_RGB_LEDs(0, 1, 1); 			// yellow
				Toggle_Debug_Ports(0, 0, 1, 0);	// PTB2 high, others low
				break;
			*/
			// ------------------------------------------------------------------------------------
			case SD:	// sending data
				Control_RGB_LEDs(1, 0, 1); 			// magenta
				Toggle_Debug_Ports(0, 0, 0, 1);	// PTB3 high, others low		
				
				while(i < 1001) {
					
					if (channel_number == 26) {
						
						if (g_Sample[i-1] >= Vtemp25) {
							voltage = (g_Sample[i-1]*3.3) / 4096;
							temperature = 25 - ((g_Sample[i-1] - Vtemp25) / m_rising);
							sprintf(volts, "Sample # %d : %f V ... @ %f C\n\r", i, voltage, temperature);		// stores the voltage value in a char buffer that can be printed via UART
							Send_String_Poll(volts);
						}
						else {
							voltage = (g_Sample[i-1]*3.3) / 4096;
							temperature = 25 - ((g_Sample[i-1] - Vtemp25) / m_falling);
							sprintf(volts, "Sample # %d : %f V ... @ %f C\n\r", i, voltage, temperature);		// stores the voltage value in a char buffer that can be printed via UART
							Send_String_Poll(volts);
						}
				
					}
					else {
						voltage = (g_Sample[i-1]*3.3) / 4096;
						sprintf(volts, "Sample # %d : %f V\n\r", i, voltage);		// stores the voltage value in a char buffer that can be printed via UART
						Send_String_Poll(volts);
					}
					i++;
				}
				i = 1;
					
				next_state = CLI;
			break;
		
			// ------------------------------------------------------------------------------------
			default:
				next_state = CLI;
				break;
		} // end case
	
	} // end main while
	
#elif USE_UART_INTERRUPTS==1 // Interrupt version of code
	Send_String("\n\rHello, World!\n\r");

	// Code listing 8.10, p. 234
	while (1) {
		// Blocking receive
		while (Q_Size(&RxQ) == 0)
			; // wait for character to arrive
		c = Q_Dequeue(&RxQ);
		
		// Blocking transmit
		sprintf((char *) buffer, "You pressed %c\n\r", c);
		// enqueue string
		bp = buffer;
		while (*bp != '\0') { 
			// copy characters up to null terminator
			while (Q_Full(&TxQ))
				; // wait for space to open up
			Q_Enqueue(&TxQ, *bp);
			bp++;
		}
		// start transmitter if it isn't already running
		if (!(UART0->C2 & UART0_C2_TIE_MASK)) {
			UART0->C2 |= UART0_C2_TIE(1);
		}
	}
#endif	

} // end main

// *******************************ARM University Program Copyright © ARM Ltd 2013*************************************   
