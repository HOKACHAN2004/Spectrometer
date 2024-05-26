#include "rng.h"
#include "delay.h"
#include "sys.h"

u8 RNG_Init(void)
{
    u16 i;
    delay_init(168);
    // 使能时钟
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY)==0)
    {
        i++;
        delay_us(100);
        if (i >= 10000)
        {
            return 1; // 超时退出
        }
    }

    return 0;
}


int RNG_Get_RandomRange(int min, int max)
{
    return min + RNG_GetRandomNumber()%(max-min+1);
}