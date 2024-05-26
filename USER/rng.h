#ifndef __RNG_H
#define __RNG_H
#include "sys.h"
#include "stm32f4xx_rng.h"

u8  RNG_Init(void);			//RNG初始化
int RNG_Get_RandomRange(int min,int max);//输出在一个范围之内的随机数
#endif
 