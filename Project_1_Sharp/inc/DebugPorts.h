#include <MKL25Z4.h> 

#define PIN_ZERO 	(0)
#define PIN_ONE	  (1)
#define PIN_TWO		(2)
#define PIN_THREE	(3)

/*
header file for debug pin settings
*/

void Toggle_Debug_Ports(unsigned int zero, unsigned int one, unsigned int two, unsigned int three);
void Init_Debug_Ports(void);
