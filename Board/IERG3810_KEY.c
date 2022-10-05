#include "stm32f10x.h"
#include "IERG3810_KEY.h"

void IERG3810_KEY_INIT()
{
	RCC->APB2ENR |= 1<<6;
	// PE3, 2 - KEY1, KEY2
	GPIOE->CRL &= 0xFFFF00FF;
	GPIOE->CRL |= 0x00008800;
	
	GPIOE->BSRR = 1<<2;
	GPIOE->BSRR = 1<<3;
	
	RCC->APB2ENR |= 1<<2;
	// PA0 - KEY_UP
	GPIOA->CRL &= 0xFFFFFFF0;
	GPIOA->CRL |= 0x00000008;
	
	GPIOA->BSRR = 0;
}
