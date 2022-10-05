#include "stm32f10x.h"
#include "IERG3810_LED.h"

void IERG3810_LED_INIT()
{
	// enable port B clock
	RCC->APB2ENR |= 1<<3;
	// PB5 - DS0
	GPIOB->CRL &= 0xFF0FFFFF;
	GPIOB->CRL |= 0x00300000;
	
	RCC->APB2ENR |= 1<<6;
	// PE5 - DS0
	GPIOE->CRL &= 0xFF0FFFFF;
	GPIOE->CRL |= 0x00300000;
}
