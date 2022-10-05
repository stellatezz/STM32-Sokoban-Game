#include "stm32f10x.h"
#include "IERG3810_Buzzer.h"

void IERG3810_Buzzer_INIT()
{
	// enable port B clock
	RCC->APB2ENR |= 1<<3;
	// PB8 - buzzer
	GPIOB->CRH &= 0xFFFFFFF0;
	GPIOB->CRH |= 0x00000003;
	
	// reset pin 8 bit
	GPIOB->BSRR = 0<<8;
}
