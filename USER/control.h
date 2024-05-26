#ifndef __CONTROL_H__
#define __CONTROL_H__
#include "stm32f4xx.h"

uint8_t cur_state;
uint8_t prev_state;

enum state {
    IDLE = 0,
    SINGLEOUT, // ��Ƶ���
    CONTINUOUS_OUTPUT, // ɨ�����
    SPECTRUM_WAIT_SELECT, // Ƶ�ײ����ȴ�ѡ��ֱ���
    SPECTRUM_WAIT_START, // Ƶ�ײ����ȴ���ʼ
    SPECTRUM_START, // Ƶ�ײ�����ʼ
    SPECTRUM_FINISHED, // ɨ����� ����״̬
};

enum state machine_state;

#define STA_X 100
#define STA_Y 20
#endif