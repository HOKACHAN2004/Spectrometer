#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "touch.h" 
#include "ADF4351.h"
#include "PE4302.h"
#include "control.h"
#include "math.h"


//ALIENTEK ̽����STM32F407������ ʵ��28
//������ʵ��   --�⺯���汾
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

//�����Ļ�������Ͻ���ʾ"RST"
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//����   
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
  POINT_COLOR=RED;//���û��ʺ�ɫ
}
////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   
////////////////////////////////////////////////////////////////////////////////
 //5�����ص����ɫ(���ݴ�������)												 
const u16 POINT_COLOR_TBL[OTT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//���败�������Ժ���
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	while(1)
	{
	 	key=KEY_Scan(1);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//���
				else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//��ͼ	  			   
			}
		}else delay_ms(10);	//û�а������µ�ʱ�� 	    
		if(key==KEY0_PRES)	//KEY0����,��ִ��У׼����
		{
			LCD_Clear(WHITE);	//����
		    TP_Adjust();  		//��ĻУ׼ 
			TP_Save_Adjdata();	 
			Load_Drow_Dialog();
		}
		//i++;
		//if(i%20==0)LED0=!LED0;
	}
}
//���ݴ��������Ժ���
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//���һ�ε����� 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<OTT_MAX_TOUCH;t++)
		{
			if((tp_dev.sta)&(1<<t))
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<20)
					{
						Load_Drow_Dialog();//���
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}
/*��������*/
void display_main_menu();
void wait_for_press(int stage);
int main(void)
{ 

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
	
	LED_Init();					//��ʼ��LED 
 	LCD_Init();					//LCD��ʼ�� 
	lcddev.dir = 1;
	KEY_Init(); 				//������ʼ��  
	tp_dev.init();				//��������ʼ��
	
  	ADF4351Init();        // ��ʼ��ADF4351
	ADF4351WriteFreq(400);
	PE_GPIO_Init();   // ��ʼ��PE4302
	PE4302_0_Set(60);
 	POINT_COLOR=RED;//��������Ϊ��ɫ 
 	
	/*
	if(tp_dev.touchtype&0X80)ctp_test();//����������
	else rtp_test(); 					//����������
	*/
	
	/* ����У׼ */
	
	LCD_Clear(WHITE);	//����
	//TP_Adjust();  		//��ĻУ׼ 
	//TP_Save_Adjdata();	 
	//Load_Drow_Dialog(); // ˢ��������ʾRST
	
	
	display_main_menu();
	machine_state = IDLE;

	while(1){
		// ���ȵȴ�����
		
		delay_ms(100);
	}

	
}
/*
	��ӡ�����˵�
*/
void display_main_menu()
{
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(100,50,100,16,16,"Wave Output(1)");
	LCD_DrawRectangle(80,40,200,80);
	
	LCD_ShowString(100,100,100,16,16,"Scan Mode(2)");
	LCD_DrawRectangle(80,90,200,130);
	
	LCD_ShowString(100,150,100,16,16,"AM(3)");
	LCD_DrawRectangle(80,140,200,180);
	return;
}

// stage = 0: ��ʾ�ڳ�ʼ�˵��ȴ� 
// stage = 1����ʾ����������˵��ȴ�
#define BOUND(x1,x2,y1,y2) (tp_dev.x[0] >= (x1) && tp_dev.x[0] <= (x2) && tp_dev.y[0] >= (y1) && tp_dev.y[0] <= (y2))

void wait_for_press(int stage)
{
	
	
	u8 key;
	while(1){
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
			if(stage==0)
			{
				// �ڳ�ʼ�˵��ȴ�
				if(tp_dev.x[0] >= 80 && tp_dev.x[0] <= 200 && tp_dev.y[0] >= 40 && tp_dev.y[0] <= 80)
				{
					// Scan 10KHZ
					cur_state = MODE10K;
					printf("%d,%d\n",tp_dev.x[0],tp_dev.y[0]);
					LED0 = !LED0;
					return;
				}
				else if (BOUND(80,200,90,130))
				{
					cur_state = MODE100K;
					printf("100K\n");
					LED0 = !LED0;
					return;
				}else if (BOUND(80,200,140,180))
				{
					cur_state = AMMODE;
					printf("AM\n");
					printf("%d,%d\n",tp_dev.x[0],tp_dev.y[0]);
					LED0 = !LED0;
					return;
				}
			}
		}else delay_ms(10);	//û�а������µ�ʱ��
		
	}
}




/*����˥��ֵ*/
#define VALUE (a*16.0f+b*8.0f+c*4.0f+d*2.0f+e*1.0f+f*0.5f)
float cal_decay_value(float Vpp)
{
	const float jizhun = -13.1;
	float delta = jizhun - 10 - 20 * log10(0.5*Vpp); // Ҫ˥����ֵ
	float delta_min = 1000.0f;
	float final_setvalue = 0.0f;
	// 16 8 4 2 1 0.5
	for (int a = 0; a < 2; a++)
		for (int b = 0; b < 2; b++)
			for (int c = 0; c < 2; c++)
				for (int d = 0; d < 2; d++)
					for (int e = 0; e < 2; e++)
						for (int f = 0; f < 2; f++)
						{
							if (fabs(VALUE-delta) < delta_min)
							{
								delta_min = fabs(VALUE - delta);
								final_setvalue = VALUE;
							}
						}
						//printf("delta:%f",delta);
	return final_setvalue;
}
