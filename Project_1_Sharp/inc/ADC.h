#include <MKL25Z4.H>
#include <stdint.h>

void Init_ADC(void);
void ADC_start(uint8_t channel_number);
void ADC_halt(void);
