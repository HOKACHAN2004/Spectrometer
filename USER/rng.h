#ifndef __RNG_H
#define __RNG_H
#include "sys.h"
#include "stm32f4xx_rng.h"

u8  RNG_Init(void);			//RNG��ʼ��
int RNG_Get_RandomRange(int min,int max);//�����һ����Χ֮�ڵ������
#endif
 