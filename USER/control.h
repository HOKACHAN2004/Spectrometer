#ifndef __CONTROL_H__
#define __CONTROL_H__
#include "stm32f4xx.h"

uint8_t cur_state;
uint8_t prev_state;

enum state {
    IDLE = 0,
    SINGLEOUT, // 单频输出
    CONTINUOUS_OUTPUT, // 扫描输出
    SPECTRUM_WAIT_SELECT, // 频谱测量等待选择分辨率
    SPECTRUM_WAIT_START, // 频谱测量等待开始
    SPECTRUM_START, // 频谱测量开始
    SPECTRUM_FINISHED, // 扫描结束 冗余状态
};

enum state machine_state;

#define STA_X 100
#define STA_Y 20
#endif