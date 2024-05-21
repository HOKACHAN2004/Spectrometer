#ifndef __CONTROL_H__
#define __CONTROL_H__
#include "stm32f4xx.h"

uint8_t cur_state;
uint8_t prev_state;

enum state {
    IDLE = 0,
    SINGLEOUT, // µ¥ÆµÊä³ö
};

enum state machine_state;

#endif