#ifndef __CONTROL_H__
#define __CONTROL_H__
#include "stm32f4xx.h"

uint8_t cur_state;
uint8_t prev_state;

#define LOWEST 120
#define HIGHEST 25
#define BIAS 0

enum state {
    IDLE = 0,
    SINGLEOUT, // 单频输出
    CONTINUOUS_OUTPUT, // 扫描输出
    SPECTRUM, // 频谱测量
    SPECTRUM_AM, // 测量MA指数

};

enum state machine_state;

#endif
