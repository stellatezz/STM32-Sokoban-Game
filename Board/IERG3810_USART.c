#include "stm32f10x.h"
#include "IERG3810_USART.h"

void IERG3810_USART2_init(u32 pclk1, u32 bound)
{
		//USART2
		float temp;
		u16 mantissa;
		u16 fraction;
		temp = (float) (pclk1*1000000)/(bound*16);
		mantissa = temp;
		fraction = (temp-mantissa)*16;
		mantissa <<= 4;
		mantissa += fraction;
		RCC->APB2ENR  |= 1<<2;
		RCC->APB1ENR  |= 1<<17;
		GPIOA->CRL 	  &= 0XFFFF00FF;
		GPIOA->CRL 	  |= 0X00008B00;
		RCC->APB1RSTR |= 1<<17;
		RCC->APB1RSTR &= ~(1<<17);
		USART2->BRR    = mantissa;
		USART2->CR1		|= 0X2008;
}	


void IERG3810_USART1_init(u32 pclk1, u32 bound)
{
		//USART2
		float temp;
		u16 mantissa;
		u16 fraction;
		temp = (float) (pclk1*1000000)/(bound*16);
		mantissa = temp;
		fraction = (temp-mantissa)*16;
		mantissa <<= 4;
		mantissa += fraction;
		RCC->APB2ENR  |= 1<<2; // set port A clock enable
		RCC->APB2ENR  |= 1<<14; // set USART1 clock enable
		GPIOA->CRH 	  &= 0XFFFFF00F; // init PA9, PA10 to all 0
		GPIOA->CRH 	  |= 0X000008B0; // set PA9 to push pull output mode, 50hz, PA10 to pull up/pull down input mode 
		RCC->APB2RSTR |= 1<<14; // USART1 reset
		RCC->APB2RSTR &= ~(1<<14); // set USART1 to 0 (no effect)
		USART1->BRR    = mantissa; // set baud rate register to calculated baud rate
		USART1->CR1		|= 0X2008; // set control register 1 to enable USART(bit 13), Transmitter(bit 3)(TE) 
}	

void USART_print(u8 USARTport, char *st)
{
		u8 i=0; 
		while (st[i]!=0x00)
		{
				if (USARTport == 1) {
					USART1->DR = st[i]; 
					while(!(USART1->SR & (1<<7)));
				}
				if (USARTport == 2) {
					USART2->DR = st[i]; 
					while(!(USART2->SR & (1<<7)));
				}
				
				if (i == 255) break;
				i++;
		}
}
