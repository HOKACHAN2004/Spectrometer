#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "ADF4351.h"
#include "PE4302.h"
#include "control.h"
#include "math.h"
#include "KeyBoard.h"
#include "adc.h"
#include <stdlib.h>
#include "rng.h"

#define LEN 201
float preset_freq;
float Vpp;
float scan_time;
float data[LEN]; // �������������
float data_polyed_v[LEN]; // ���ΪV������
float data_polyed_dbm[LEN]; // ���Ϊdbm������
u8 spectrum_mode=0; // ��Ļ������ģʽ��Ĭ��Ϊ0 (100kHz), 1(10K)
float start_freq = 0; // ����Ƶ��ʱ����ʼƵ��
int freq_step = 100; // ����
/*��ӡ�����˵�*/
void display_main_menu()
{
	POINT_COLOR = RED; // ��������Ϊ��ɫ
	LCD_ShowString(100, 50, 200, 16, 16, "Single Output(1)");
	LCD_ShowString(100, 100, 200, 16, 16, "Continuous Output(2)");
	LCD_ShowString(100, 150, 200, 16, 16, "Spectrum Analyzer(3)");
	LCD_ShowString(100, 200, 200, 16, 16, "Spectrum Analyzer(AM)(4)");
	return;
}

/*����˥��ֵ*/
// VALUEΪ˥����˥����db��
#define VALUE (a * 16.0 + b * 8.0f + c * 4.0f + d * 2.0f + e * 1.0f + f * 0.5f)
#define INPUT_VALUE (a * 32 + b * 16 + c * 8 + d * 4 + e * 2 + f * 1)
float cal_decay_value(float Vpp)
{
	const float jizhun = -1.8;
	float delta = jizhun - 10 - 20 * log10(0.5 * Vpp); // Ҫ˥����dbֵ
	float delta_min = 1000.0f;
	unsigned char final_setvalue = 0;
	// 16 8 4 2 1 0.5
	for (int a = 0; a < 2; a++)
		for (int b = 0; b < 2; b++)
			for (int c = 0; c < 2; c++)
				for (int d = 0; d < 2; d++)
					for (int e = 0; e < 2; e++)
						for (int f = 0; f < 2; f++)
						{
							if (fabs(VALUE - delta) < delta_min)
							{
								delta_min = fabs(VALUE - delta);
								final_setvalue = INPUT_VALUE;
							}
						}
	// printf("delta:%f\n",delta);
	return final_setvalue;
}

/* ��Ƶ���ģʽ */
#define STATE_FONTSIZE 24
#define NOTE_FONTSIZE 12
#define STA_X 80
#define STA_Y 20
#define NOT_X 10
#define NOT_Y 50
#define TXT_X 100
#define TXT_Y 150
#define TAG_X 200
#define TAG_Y 20


int pos1; // ���ֵλ��
int pos2; // �ڶ���ֵλ��
int pos3; // ������ֵλ��
int scatter_pos[201]; // ��ɢ��λ�÷ֲ�
int scatter_num;
float MA; // ����ָ��

/*Ѱ�������е���������ֵ*/
void find_top_three(float * array, int len)
{
	pos1 = -1; pos2 = -1; pos3 = -1;
	float max1 = -1;
	float max2 = -1;
	float max3 = -1;

	for (int i = 0;i < len; i++)
	{
		if (array[i] > max1)
		{
			max3 = max2;
			max2 = max1;
			max1 = array[i];

			pos3 = pos2;
			pos2 = pos1;
			pos1 = i;
		} else if (array[i] > max2 && array[i] < max1)
		{
			max3 = max2;
			max2 = array[i];

			pos3 = pos2;
			pos2 = i;
		} else if (array[i] > max3 && array[i] < max2)
		{
			pos3 = i;
			max3 = array[i];
		}
	}
}
/* Ѱ����ɢֵ */
void find_scatter()
{
	// �����ֵ����2% Vm��Ϊ��ɢ��
	scatter_num = 0;
	float threshold = data_polyed_v[pos1] * 0.02f;

	for (int i = 0; i < LEN; i++)
	{
		if (data_polyed_v[i] > threshold && i != pos1 && i != pos2 && i != pos3)
			scatter_pos[scatter_num++] = i;
	}
}
/* ������ϵ���ѹֵ */
// ��dbm����ֵ
float poly_v(float origin)
{
	float res = 2 * pow(10,((origin-10.0f)/20.0f));
	return res;
}

/* ������ϵ�dbm*/
// �ɲ�����ֱ����ƽ�����dbm
float poly_dbm(float origin)
{
	
	float res = 39.75f * origin - 124.8f + BIAS;
	//return RNG_Get_RandomRange(-60,-30);
	return res;
}
void single_output_handler()
{
	uint8_t stage_num = 0;
	POINT_COLOR = RED;
	BACK_COLOR = WHITE;
	uint8_t key_value;

	while (1)
	{
		// �������Ƿ����˳�
		key_value = KEY_Scan(0);
		if (key_value == 4)
		{
			// KEY UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
		}

		if (stage_num == 0)
		{
			LCD_Clear(WHITE);
			LCD_ShowString(STA_X, STA_Y, 300, 50, STATE_FONTSIZE, "SINGLEOUT");
			stage_num = 1; // �ȴ�����Freq
		}
		if (stage_num == 1)
		{
			preset_freq = input_double(80, 100);
			LCD_ShowString(200, 100, 300, 50, STATE_FONTSIZE, "MHz");
			if (preset_freq > 89 && preset_freq < 111)
			{ // ���Ԥ��Ƶ�ʺϷ�
				printf("Valid freq\n");
				stage_num = 2;
			}
			else if (preset_freq != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				LCD_ShowString(NOT_X, NOT_Y, 300, 50, NOTE_FONTSIZE, "reset freq pls");
				preset_freq = 0;
				delay_ms(500);
				stage_num = 0;
			}
		}

		if (stage_num == 2)
		{
			//  �������
			Vpp = input_double(80, 200);
			LCD_ShowString(200, 200, 300, 50, STATE_FONTSIZE, "mVpp");
			// ���õķ��ȺϷ�
			if (Vpp > 9 && Vpp < 101)
			{
				printf("Valid Vpp\n");
				break;
			}
			else if (Vpp != 0)
			{
				Vpp = 0;
				delay_ms(500);
				stage_num = 2;
			}
		}
	}

	
	while (1)
	{
		key_value = KEY_Scan(0);
		if (key_value == 1)
		{
			// KEY0 ���� ��ʼ���
			ADF4351WriteFreq(preset_freq);
			unsigned char decay_db = cal_decay_value(0.001 * Vpp);
			delay_ms(100);
			PE4302_Set(decay_db);
			printf("Frequency: %lf, mVpp:%lf,decay_db: %u\n", preset_freq, Vpp, decay_db);
		}
		else if (key_value == 4)
		{
			// KEY UP ���� ����
			
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
			
			// debug here
			//float adcx = Get_Adc_Average(ADC_Channel_4, 20); // ADC1 ͨ��4 20��ȡƽ��
			//float data_converted = adcx * 3.3f / 4096; // 12λADC�ɼ���ԭʼ����
			//printf("%lf\n",data_converted);
		}
	}
}
/* ɨƵ���ģʽ */
void continuous_output_handler()
{
	uint8_t stage_num = 0;
	POINT_COLOR = RED;
	BACK_COLOR = WHITE;
	uint8_t key_value;

	while (1)
	{
		// �������Ƿ����˳�
		key_value = KEY_Scan(0);
		if (key_value == 4)
		{
			// KEY UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
		}
		if (stage_num == 0)
		{
			LCD_Clear(WHITE);
			LCD_ShowString(STA_X, STA_Y, 300, 50, STATE_FONTSIZE, "CONTINUOUS OUT");
			stage_num = 1; // �ȴ�����Vpp
		}

		if (stage_num == 1)
		{
			Vpp = input_double(80, 100);
			LCD_ShowString(200, 100, 300, 50, STATE_FONTSIZE, "mVpp");
			if (Vpp > 9 && Vpp < 101)
			{ // ���Ԥ�÷�ֵ�Ϸ�
				printf("Valid Vpp\n");
				stage_num = 2;
			}
			else if (Vpp != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				LCD_ShowString(NOT_X, NOT_Y, 300, 50, NOTE_FONTSIZE, "reset vpp pls");
				Vpp = 0;
				delay_ms(500);
				stage_num = 0;
			}
		}

		if (stage_num == 2)
		{
			//  ����ɨ��ʱ��
			scan_time = input_double(80, 200);
			LCD_ShowString(200, 200, 300, 50, STATE_FONTSIZE, "s");
			// ���õ�ɨ��ʱ��Ϸ�
			if (scan_time > 9 && scan_time < 31)
			{
				printf("Valid Scan Time\n");
				break;
			}
			else if (scan_time != 0)
			{
				scan_time = 0;
				delay_ms(500);
				stage_num = 2;
			}
		}
	}

	
	while (1)
	{
		key_value = KEY_Scan(0);
		if (key_value == 1)
		{
			// KEY0 ���� ��ʼɨ�����
			// TODO �޸ķ���
			float delta_time = scan_time / 200.0f;
			printf("Start continuous output.\n");
			printf("Scan time:%f\n",scan_time);
			unsigned char decay_value = cal_decay_value(0.001*Vpp);
			PE4302_Set(decay_value);
			for(int i = 0; i <= 200; i++)
			{
				// ѭ�����
				float cur_freq = 90 + i * 0.1;
				if (i % 20 == 0) printf("Cur_frequency:%f\n",cur_freq);
				ADF4351WriteFreq(cur_freq);
				delay_ms(delta_time * 1000);
				
			}
			printf("Finished continuous output.\n");
		}
		else if (key_value == 4)
		{
			// KEY UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
		}
	}
}
/**
 * ɨƵ���������ͼ��
 * @param mode: 0��ʾһ��Ƶ�ײ��� 1��ʾAM����MAָ������ģʽ
*/

void spectrum_drawing(u8 mode)
{
	float delta = freq_step*0.001;
	float cur_freq = start_freq + 10.7f; // Ҫ��һ����Ƶ
	int i;
	char temp_str[20] = {0};
	int delta_y = 0;
	// ��������ͼ
	LCD_Clear(WHITE);
	POINT_COLOR = BLACK;
	LCD_DrawAxis(ORIGIN_X0,ORIGIN_Y0,ORIGIN_X0+AXIS_X_W,ORIGIN_Y0-AXIS_Y_H);
	PE4302_Set(0); // ��˥��
	if (freq_step==10)
		LCD_ShowString(210,10,100,50,NOTE_FONTSIZE, "Step: 10K");
	else 
		LCD_ShowString(210,10,100,50,NOTE_FONTSIZE, "Step: 100K");
	
	for (i = 0; i<201; i++)
	{
		cur_freq = start_freq + 10.7f + i * delta;
		ADF4351WriteFreq(cur_freq);
		delay_ms(40);
		float adcx = Get_Adc_Average(ADC_Channel_4, 20); // ADC1 ͨ��4 20��ȡƽ��
		data[i] = adcx * 3.3f / 4096; // 12λADC�ɼ���ԭʼ����
		data_polyed_dbm[i] = poly_dbm(data[i]); // ������ϵ�dbm
		printf("adcx: %.4f, dbm:%.2f, at %.2f\n",data[i],data_polyed_dbm[i],cur_freq);
		data_polyed_v[i] = poly_v(data_polyed_dbm[i]); // �������ΪV����ֵ��
		// ��ͼ
		delta_y = (data_polyed_dbm[i] + LOWEST) * 200 / (LOWEST - HIGHEST);
		
		LCD_DrawLine(ORIGIN_X0+i,ORIGIN_Y0,ORIGIN_X0+i, ORIGIN_Y0-delta_y);

		// ��Ļ����ʾ��ǰ�����freqֵ
		sprintf((char*)temp_str,"ADF: %.2f",cur_freq);
		LCD_ShowString(210,30,100,50,NOTE_FONTSIZE,temp_str);

		// ÿ��40������ʾ������
		if (i % 40 == 0)
		{
			printf("Run in axis\n");
			sprintf((char*)temp_str,"%.2f",cur_freq-10.7f);
			LCD_ShowString(ORIGIN_X0+i, ORIGIN_Y0+2,100,15,12,temp_str);
		}

	}

	// ɨ�����
	
	
	//����������ݴ������ 
	// 1. �ж��ǵ�Ƶ�������AM��
	// 2. ��Ϊ��Ƶ������ҵ���ֵ��������ʾ����ʾ��Ӧ��Ƶ�ʺͷ���dbm
	// 3. ��ΪAM�����ҵ�������ֵ��������ʾ���������ָ��

	find_top_three(&data[0], 211); // Ѱ��������ֵ
	//find_scatter(); // Ѱ����ɢ��
	float max_freq = 0;

		// AM�� ֻ������100K���������
		// �����ֵ����������� 
		MA = (data_polyed_v[pos1-1] + data_polyed_v[pos1+1]) / data_polyed_v[pos1];
		
		// ������ʾ����λ��
		POINT_COLOR = RED;
		delta_y = (data_polyed_dbm[pos1] + 105) * 200 / (105 - 25); 
		LCD_DrawLine(ORIGIN_X0+pos1,ORIGIN_Y0-delta_y,ORIGIN_X0+pos1, ORIGIN_Y0);
		delta_y = (data_polyed_dbm[pos2] + 105) * 200 / (105 - 25); 
		LCD_DrawLine(ORIGIN_X0+pos2,ORIGIN_Y0-delta_y,ORIGIN_X0+pos2, ORIGIN_Y0);
		delta_y = (data_polyed_dbm[pos3] + 105) * 200 / (105 - 25); 
		LCD_DrawLine(ORIGIN_X0+pos3,ORIGIN_Y0-delta_y,ORIGIN_X0+pos3, ORIGIN_Y0);
		// ��Ļ��ʾ����
		POINT_COLOR = BLACK;
		if (mode) LCD_ShowString(220,50,100,50,NOTE_FONTSIZE,"AM-WAVE");
		else LCD_ShowString(210,50,100,50,NOTE_FONTSIZE,"SINGLE-WAVE");
		// ��Ļ��ʾAMֵ
		sprintf((char*)temp_str,"MA: %.2f",MA);
		if (mode) LCD_ShowString(210,80,100,50,NOTE_FONTSIZE,temp_str);
		// ��ʾ��ֵ
		max_freq = start_freq + pos1 * delta;
		sprintf((char*)temp_str,"max at %.2f,%.1fdBm",max_freq,data_polyed_dbm[pos1]);
		LCD_ShowString(210,120,100,50,NOTE_FONTSIZE,temp_str);

		max_freq = start_freq + pos2 * delta;
		sprintf((char*)temp_str,"max2 at %.2f,%.1fdBm",max_freq,data_polyed_dbm[pos2]);
		LCD_ShowString(210,150,100,50,NOTE_FONTSIZE,temp_str);

		max_freq = start_freq + pos3 * delta;
		sprintf((char*)temp_str,"max3 at %.2f,%.1fdBm",max_freq,data_polyed_dbm[pos3]);
		LCD_ShowString(210,180,100,50,NOTE_FONTSIZE,temp_str);

		if (mode)
		{
			printf("max v1: %f\n",data_polyed_v[pos1]);
			printf("max v2: %f\n",data_polyed_v[pos1-1]);
			printf("max v3: %f\n",data_polyed_v[pos1+1]);
		}

	return;
}
/* Ƶ��ģʽ */
void spectrum_handler(u8 mode)
{
	uint8_t key_value;
	uint8_t stage_num = 0;

	while (1)
	{
		// �������Ƿ����˳�
		key_value = KEY_Scan(0);
		if (key_value == 4)
		{
			// KEY UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
		}

		if (stage_num == 0)
		{
			LCD_Clear(WHITE);
			LCD_ShowString(STA_X, STA_Y, 300, 50, STATE_FONTSIZE, "SPECTRUM-ANALYZER");
			stage_num = 1; // �ȴ�����Freq 1
		}

		if (stage_num == 1)
		{
			start_freq = input_double(80, 100);
			LCD_ShowString(200, 100, 300, 50, STATE_FONTSIZE, "MHz");
			if (start_freq > 70 && start_freq < 110)
			{ // ���Ԥ��Ƶ�ʺϷ�
				printf("Valid start_freq\n");
				stage_num = 2;
			}
			else if (start_freq != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				LCD_ShowString(NOT_X, NOT_Y, 300, 50, NOTE_FONTSIZE, "reset start_freq pls");
				start_freq = 0;
				delay_ms(500);
				stage_num = 0;
			}
		}

		if (stage_num == 2)
		{
			freq_step = input_int(80, 200);
			LCD_ShowString(200, 200, 300, 50, STATE_FONTSIZE, "KHz");
			if (freq_step == 10 || freq_step == 100)
			{ // ���Ԥ��Ƶ�ʺϷ�
				printf("Valid freq_step\n");
				break; // ������while�ȴ���ʼ
			}
			else if (freq_step != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				freq_step = 0;
				delay_ms(500);
				stage_num = 2;
			}
		}
	}

	while (1)
	{
		key_value = KEY_Scan(0);
		if (key_value == 1)
		{
			// KEY0 ���� ��ʼɨƵ
			spectrum_drawing(mode);
		}
		else if (key_value == 4)
		{
			// WK UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			break;
		}
	}
}

int main(void)
{

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // ����ϵͳ�ж����ȼ�����2
	delay_init(168);								// ��ʼ����ʱ����
	uart_init(115200);								// ��ʼ�����ڲ�����Ϊ115200
	RNG_Init();
	LED_Init(); // ��ʼ��LED
	LCD_Init(); // LCD��ʼ��
	LCD_Display_Dir(1);
	LCD_DisplayOn();
	POINT_COLOR = RED; // ��������Ϊ��ɫ
	BACK_COLOR = WHITE;
	KEY_Init(); // ������ʼ��

	ADF4351Init(); // ��ʼ��ADF4351
	PE_GPIO_Init(); // ��ʼ��PE4302

	/* ��ʼ��ADC */
	Adc_Init();
	/* ��ʼ������ */
	Init_KeyBoard_Port();	   // ��ʼ�����̽ӿ�
	KeyBoard_Interrupt_Init(); // ��ʼ�������ж�

	LCD_Clear(WHITE); // ����
	display_main_menu();
	machine_state = IDLE;
	uint32_t key_val; // ����ֵ
	while (1)
	{
		switch (machine_state)
		{
		case IDLE:
			// ����״̬ �ȴ���������
			key_val = ReadKeyBoard();
			// �Լ�ֵ�����ж�
			if (key_val == 1)
			{
				// ��Ƶ���ģʽ
				machine_state = SINGLEOUT;
			}
			else if (key_val == 2)
			{
				// ɨƵ���ģʽ
				machine_state = CONTINUOUS_OUTPUT;
			}
			else if (key_val == 3)
			{
				// Ƶ�ײ���
				printf("%d\n",key_val);
				machine_state = SPECTRUM;
				spectrum_mode = 0;
			}else if (key_val == 4)
			{	
				printf("%d\n",key_val);
				machine_state = SPECTRUM_AM;
				spectrum_mode = 1;
			}
			break;

		case SINGLEOUT:
			single_output_handler(); // ��ת������Ƶ���
			break;
		case CONTINUOUS_OUTPUT:
			continuous_output_handler();
			break;
		case SPECTRUM:
			spectrum_handler(0);
			LCD_Clear(WHITE);
			display_main_menu();
			machine_state = IDLE;
			break;
		case SPECTRUM_AM:
			spectrum_handler(1);
			break;
		default:
			break;
		}
		delay_ms(100);
	}
}
