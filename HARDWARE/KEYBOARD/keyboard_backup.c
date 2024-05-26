#include "stm32f4xx.h"
#include "delay.h"
#include "keyboard.h"


//uint8_t Send_F=0;


void Keyboard_GPIO_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;
    
    KEYBOARD_GPIO_CLK_FUN(KEYBOARD_GPIO_CLK,ENABLE);
    
    //LINE    
    GPIO_InitStruct.GPIO_Pin=KEYBOARD_GPIO_PIN0|KEYBOARD_GPIO_PIN1|KEYBOARD_GPIO_PIN2|KEYBOARD_GPIO_PIN3;
    GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Speed=GPIO_Speed_25MHz;
    GPIO_Init(KEYBOARD_GPIO_PORT,&GPIO_InitStruct);

    //ROW
    GPIO_InitStruct.GPIO_Pin=KEYBOARD_GPIO_PIN4|KEYBOARD_GPIO_PIN5|KEYBOARD_GPIO_PIN6|KEYBOARD_GPIO_PIN7;
    GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_Speed=GPIO_Speed_25MHz;
    GPIO_Init(KEYBOARD_GPIO_PORT,&GPIO_InitStruct);
}

u32 ReadKeyBoard(void)
{
  u32 return_temp=100;
  int nTemp=0;
  if(ROW0==0||ROW1==0||ROW2==0||ROW3==0)//B7��B6.。。。。
  {
    delay_us(500);
    if(ROW0==0||ROW1==0||ROW2==0||ROW3==0)
    {
      nTemp =TestRow();
      ROW_L_COL_H();
      ROW_L_COL_H();
      delay_us(100);
      nTemp|=TestCol();
      switch(nTemp)
      {
      case 126:
        return_temp= 13;
        break;//'*'
      case 190:
        return_temp= 0;
        break;//'0'
      case 222:
        return_temp= 14;
        break;//'#'
      case 238:
        return_temp= 15;
        break;//'D'
      case 125:
        return_temp= 7;
        break;//'7'
      case 189:
        return_temp= 8;
        break;//'8'
      case 221:
        return_temp= 9;
        break;//'9'
      case 237:
        return_temp= 12;
        break;//'C'
      case 123:
        return_temp= 4;
        break;//'4'
      case 187:
        return_temp= 5;
        break;//'5'
      case 219:
        return_temp= 6;
        break;//'6'
      case 235:
        return_temp= 11;
        break;//'B'
      case 119:
        return_temp= 1;
        break;//'1'
      case 183:
        return_temp= 2;
        break;//'2'
      case 215:
        return_temp= 3;
        break;//'3'
      case 231:
        return_temp= 10;
        break;//'A'
      default:
        return_temp= 160;
        break;
      }
      while(COL0==0||COL1==0||COL2==0||COL3==0);
      delay_us(5);
      while(COL0==0||COL1==0||COL2==0||COL3==0);
    }
  }
  ROW_H_COL_L();
  return return_temp;
}

//uint16_t keyboard_scan(void)
//{
//    uint16_t key_val=0;
//    uint16_t temp=0;
//	
//	
//	/*************Scan  1st Line************************/

//    GPIOD->ODR=0X00;

//    GPIOD->ODR=0XF7;

//    if((GPIOD->IDR&0XF0)!=0XF0)
//    {
//        delay_ms(50);

//        if((GPIOD->IDR & 0XF0)!=0XF0)
//        {
//            temp=(GPIOD->IDR&0XF7);
//            switch(temp)
//            {
//                case 0xE7:  key_val=1;   break;

//                case 0xD7:  key_val=2;   break;

//                case 0xB7:  key_val=3;   break;

//                case 0x77:  key_val=4;   break;

//                default:    key_val=0;   break;
//            }
//        }
//    }
//    
//		/*************Scan  2st Line************************/
//    GPIOD->ODR=0X00;

//    GPIOD->ODR=0XFB;

//    if((GPIOD->IDR&0XF0)!=0XF0)
//    {
//        delay_ms(50);

//        if((GPIOD->IDR & 0XF0)!=0XF0)
//        {
//            temp=(GPIOD->IDR&0XFB);
//            switch(temp)
//            {
//                case 0xEB:  key_val=5;  break;

//                case 0xDB:  key_val=6;  break;

//                case 0xBB:  key_val=7;  break;

//                case 0x7B:  key_val=8;  break;

//                default:    key_val=0;  break;
//            }
//        }
//    }

//		/*************Scan  3st Line************************/
//    GPIOD->ODR=0X00;

//    GPIOD->ODR=0XFD;

//    if((GPIOD->IDR&0XF0)!=0XF0)
//    {
//        delay_ms(50);

//        if((GPIOD->IDR & 0XF0)!=0XF0)
//        {
//            temp=(GPIOD->IDR&0XFD);
//            switch(temp)
//            {
//                case 0xED:  key_val=9;   break;

//                case 0xDD:  key_val=10;  break;

//                case 0xBD:  key_val=11;  break;

//                case 0x7D:  key_val=12;  break;

//                default:    key_val=0;   break;
//            }
//        }
//    }

//		/*************Scan  4st Line************************/
//    GPIOD->ODR=0X00;

//    GPIOD->ODR=0XFE;

//    if((GPIOD->IDR&0XF0)!=0XF0)
//    {
//        delay_ms(50);

//        if((GPIOD->IDR & 0XF0)!=0XF0)
//        {
//            temp=(GPIOD->IDR&0XFE);
//            switch(temp)
//            {
//                case 0xEE:  key_val=13;  break;

//                case 0xDE:  key_val=14;  break;

//                case 0xBE:  key_val=15;  break;

//                case 0x7E:  key_val=16;  break;

//                default:    key_val=0;   break;
//            }
//        }
//    }

//    return key_val;

//}

