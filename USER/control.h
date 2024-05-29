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
    SINGLEOUT, // ��Ƶ���
    CONTINUOUS_OUTPUT, // ɨ�����
    SPECTRUM, // Ƶ�ײ���
    SPECTRUM_AM, // ����MAָ��

};

enum state machine_state;

#endif
