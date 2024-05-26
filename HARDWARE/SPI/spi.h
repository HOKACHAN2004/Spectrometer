#ifndef __LED_H
#define __LED_H
#include "sys.h"

#define mosi PFout(6)	
#define cs PFout(2)
#define sclk PFout(11) 
#define miso 		GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_8) //PE4

void spi_io_Init(void);//≥ı ºªØ		 				    
#endif
