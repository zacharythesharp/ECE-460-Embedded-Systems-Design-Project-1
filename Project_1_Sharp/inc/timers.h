#ifndef TIMERS_H
#define TIMERS_H
#include "MKL25Z4.h"
#define BASE_CLK (48000000)
#define DESIRED_CLK (100000)
#define PWM_PERIOD (48000)
#define FULL_ON (PWM_PERIOD-1)
#define FULL_OFF (0)

void Init_Timer_Output(void);

#endif

