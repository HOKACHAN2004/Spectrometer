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

/*��ӡ�����˵�*/
void display_main_menu()
{
	POINT_COLOR = RED; // ��������Ϊ��ɫ
	LCD_ShowString(100, 50, 200, 16, 16, "Single Output(1)");
	LCD_ShowString(100, 100, 200, 16, 16, "Continuous Output(2)");
	LCD_ShowString(100, 150, 200, 16, 16, "Spectrum Analyzer(100K)");
	LCD_ShowString(100, 200, 200, 16, 16, "Spectrum Analyzer(10K)");
	return;
}

/*����˥��ֵ*/
// VALUEΪ˥����˥����db��
#define VALUE (a * 16.0 + b * 8.0f + c * 4.0f + d * 2.0f + e * 1.0f + f * 0.5f)
#define INPUT_VALUE (a * 32 + b * 16 + c * 8 + d * 4 + e * 2 + f * 1)
float cal_decay_value(float Vpp)
{
	const float jizhun = -12.4;
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
int scatter_pos[20]; // ��ɢ��λ�÷ֲ�
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
	// �����ֵ����2% Vpp��Ϊ��ɢ��
	scatter_num = 0;
	float threshold = data[pos1] * 0.02;
	for (int i = 0; i < LEN; i++)
	{
		if (data[i] > threshold && i != pos1 && i != pos2 && i != pos3)
			scatter_pos[scatter_num++] = i;
	}
}
/* ������ϵ���ѹֵ */
float poly_v(float origin)
{
	return 0.0255f * origin + 2.5323f;
}

/* ������ϵ�dbm*/
float poly_dbm(float origin)
{
	
	// �������
	return RNG_Get_RandomRange(-70,-25);
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
			// printf("Start Output here\n");
			// ADF4351WriteFreq(preset_freq);
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
			PE4302_Set(0);
			printf("Frequency: %lf, mVpp:%lf,decay_db: %u\n", preset_freq, Vpp, decay_db);
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
			{ // ���Ԥ��Ƶ�ʺϷ�
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
			for(int i = 0; i <= 200; i++)
			{
				// ѭ�����
				float cur_freq = 90 + i * 0.1;
				printf("Cur_frequency:%f\n",cur_freq);
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
 * @param freq1_: ��ʼƵ��
 * @param freq2_: ��ֹƵ��
 * @param mode: 0��ʾ100K������1��ʾ10K����
*/
void spectrum_drawing(float freq1_, float freq2_, u8 mode)
{
	float delta = mode?0.01:0.1;
	float cur_freq = freq1_;
	int i;
	char temp_str[10] = {0};
	// ��������ͼ
	LCD_Clear(WHITE);
	POINT_COLOR = BLACK;
	LCD_DrawAxis(ORIGIN_X0,ORIGIN_Y0,ORIGIN_X0+AXIS_X_W,ORIGIN_Y0-AXIS_Y_H);
	if (mode)
		LCD_ShowString(210,10,100,50,NOTE_FONTSIZE, "Step: 10K");
	else 
		LCD_ShowString(210,10,100,50,NOTE_FONTSIZE, "Step: 100K");
	for (i = 0; i<201; i++)
	{
		cur_freq = freq1_ + i * delta;
		ADF4351WriteFreq(cur_freq);
		delay_ms(40);

		float adcx = Get_Adc_Average(ADC_Channel_4, 10); // ADC1 ͨ��4 10��ȡƽ��
		
		data[i] = adcx * 3.3f / 4096; // 12λADC
		data_polyed_v[i] = poly_v(data[i]); // �������ΪV
		data_polyed_dbm[i] = poly_dbm(data_polyed_v[i]); // ������ϵ�dbm

		// ��ͼ
		int delta_y = (data_polyed_dbm[i] + 70) * 200 / (70 - 25); // �������λ����-70dbm �����-25dbm
		LCD_DrawLine(ORIGIN_X0+i,ORIGIN_Y0,ORIGIN_X0+i, ORIGIN_Y0-delta_y);

		// ��Ļ����ʾ��ǰ�����freqֵ
		sprintf((char*)temp_str,"ADF: %.2f",cur_freq);
		LCD_ShowString(210,50,100,50,NOTE_FONTSIZE,temp_str);
	}

	// ɨ�����
	
	/*
	//����������ݴ������ 
	// 1. �ж��ǵ�Ƶ�������AM��
	// 2. ��Ϊ��Ƶ������ҵ���ֵ��������ʾ����ʾ��Ӧ��Ƶ�ʺͷ���dbm
	// 3. ��ΪAM�����ҵ�������ֵ��������ʾ���������ָ��

	find_top_three(&data[0], LEN); // Ѱ��������ֵ
	find_scatter(); // Ѱ����ɢ��
	u8 wave_form = 0; // 1����AM�� 0����Ƶ��
	// �ж����벨������
	if (data[pos2] >= 0.3 * data[pos1] || data[pos3] >= 0.3 * data[pos1])
	{
		wave_form = 1;
		MA = (data_polyed_v[pos2] + data_polyed_v[pos3]) / data_polyed_v[pos1];
		// ������ʾ����λ��
		
	}else 
	{
		wave_form = 0;
		// ������ʾ��ߵ�λ��
	}
   */
	 
	
	return;
	
	

}
/* Ƶ��ģʽ */
void spectrum_handler(u8 mode)
{
	uint8_t key_value;
	uint8_t stage_num = 0;
	float freq1 = 0;
	float freq2 = 0;
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
			freq1 = input_double(80, 100);
			LCD_ShowString(200, 100, 300, 50, STATE_FONTSIZE, "MHz");
			if (freq1 > 89 && freq1 < 111)
			{ // ���Ԥ��Ƶ�ʺϷ�
				printf("Valid freq1\n");
				stage_num = 2;
			}
			else if (freq1 != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				LCD_ShowString(NOT_X, NOT_Y, 300, 50, NOTE_FONTSIZE, "reset freq1 pls");
				freq1 = 0;
				delay_ms(500);
				stage_num = 0;
			}
		}

		if (stage_num == 2)
		{
			freq2 = input_double(80, 200);
			LCD_ShowString(200, 200, 300, 50, STATE_FONTSIZE, "MHz");
			if (freq2 > 89 && freq2 < 111)
			{ // ���Ԥ��Ƶ�ʺϷ�
				printf("Valid freq2\n");
				break; // ������while�ȴ���ʼ
			}
			else if (freq2 != 0)
			{ // ���Ԥ��Ƶ�ʲ��Ϸ�
				freq2 = 0;
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
			spectrum_drawing(freq1,freq2,mode);
			
		}
		else if (key_value == 4)
		{
			// WK UP ���� ����
			machine_state = IDLE;
			LCD_Clear(WHITE);
			display_main_menu();
			return;
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
				machine_state = SPECTRUM_WAIT_SELECT;
				spectrum_mode = 0;
			}else if (key_val == 4)
			{	
				printf("%d\n",key_val);
				machine_state = SPECTRUM_WAIT_SELECT;
				spectrum_mode = 1;
			}
			break;

		case SINGLEOUT:
			single_output_handler(); // ��ת������Ƶ���
			break;
		case CONTINUOUS_OUTPUT:
			continuous_output_handler();
			break;
		case SPECTRUM_WAIT_SELECT:
			spectrum_handler(spectrum_mode);
			break;
		default:
			break;
		}
		delay_ms(100);
	}
}
