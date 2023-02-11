//////////////////////////////////////////////////////////////////////////////////	 
//��ʶʵϰ����
//2021.6.17 ��adת���õ���ֵ��ʾ����Ļ��
//
//SPI
//sbit OLED_SCL=P2^7;//ʱ�� D0��SCLK?
//sbit OLED_SDIN=P2^6;//D1��MOSI�� ����
//sbit OLED_RST =P2^5;//��λ
//sbit OLED_DC =P2^4;//����/�������
//sbit OLED_CS=P2^3; //Ƭѡ

//IIC
//P10 SCL
//P11 SDA

//AD vref = 5v
//DHT11�������Ž�p3.6
//              ----------------------------------------------------------------

#include "REG52.h"
#include "oled.h"
//#include "bmp.h"
#include "I2C.H"
#include "dht11.h"
#include "esp8266.h"
#define uchar unsigned char
#define  PCF8591 0x90    //PCF8591 ��ַ

//��Ŷ���
sbit KEY = P3^2;
sbit pump1 = P0^0;//ˮ������ȿ��Ʊ�־
sbit pump2 = P0^1;
sbit fan1 = P0^2;
sbit fan2 = P0^3;


// ��������
unsigned char AD_CHANNEL;
//unsigned long xdata  LedOut[8];
unsigned int  D[3];//�洢adֵ��255��
int	ad_num1;
float ad_num2;//ת���м����
unsigned int	N[4];//�洢ת�����ADֵ
//1:����ʪ��(�ٷ�)  2:����ǿ��ֵ(�ٷ�)
unsigned int i,j;//ת��ѭ������
unsigned int flag = 0;//��Ļ��ʾѭ����־
extern char rec_dat[9];//dht11��ý��
unsigned char dht11;//dht11ת�����
unsigned char receive[8];
unsigned char k;//��esp8266���յ�������Ϊ��־λ
unsigned char buf[3];//���ֻ�����ʱ������
extern bit uart_busy;//������æ��־
bit sented;//���ֻ��Ѿ����ͱ�־



 //��������
		bit DACconversion(unsigned char sla,unsigned char c,unsigned char Val);
		bit ISendByte(unsigned char sla,unsigned char c);
		unsigned char IRcvByte(unsigned char sla);
		void int2str(unsigned int msg);
		void fanmove(void);
		void fanstop(void);
		void pumpmove(void);
		void pumpstop(void);
 int main(void)
 {	
		
	 /**********���¸�ģ���ʼ��**************/
	 fanstop();
	 pumpstop();
	 Init_uart();			//���ڳ�ʼ�� baud=115200
	 Init_ESP01S();    //esp��ʼ�������趨�˿ں� //Init_ESP01S �� Init_uart���õ�˳���ܸı�
	 //delay_init();	  //��ʱ������ʼ��	��oled����  
		OLED_Init();			//��ʼ��OLED  
    OLED_Clear();
		EX0 = 1;					//�ⲿ�ж�0��	 
	  IT0 = 0;					//�͵�ƽ����
		EA = 1;						//���жϿ�
	 
	while(1) 
	{		
		
		
		
			/********����AD-DA����*************/  
	
		
		for(AD_CHANNEL=0;AD_CHANNEL<4;AD_CHANNEL++)
		{
				switch(AD_CHANNEL)
			{
				case 0: ISendByte(PCF8591,0x41);
						D[0]=IRcvByte(PCF8591);  //ADC0 ģ��ת��1    ��������
					break;  
			
				case 1: ISendByte(PCF8591,0x42);
						D[1]=IRcvByte(PCF8591);  //ADC1 ģ��ת��2	  ��������
					break;  
			
				case 2: ISendByte(PCF8591,0x43);
						D[2]=IRcvByte(PCF8591);  //ADC2	ģ��ת��3	   ����ʪ�ȼ�
					break;  
			
				case 3: ISendByte(PCF8591,0x40);
						D[3]=IRcvByte(PCF8591);  //ADC3 ģ��ת��4	   ����������
					break;  
			
				//case 4: DACconversion(PCF8591,0x40, D[4]); //DAC	  ��ģת��
				//     break;
		 
			}
    	//D[4]=D[3];  //��ģ������������ź� ͨ����ģת�����			
		}
		
		
		/************����dht11��ʪ�ȼ��**************/
		DHT11_receive();
		
		
		
		/*********��������ת��Ϊ��ѹ**********/
/*
			ad_num2 = D[0];
			ad_num2 = (ad_num2/255)*5;
			ad_num1 = ad_num2;
			N[0] = ad_num1;
			ad_num2 = ((ad_num2-ad_num1)*100);
			N[1] = (int)ad_num2;
*/
		
		/**********����Ϊ����ת��Ϊ����ʪ��ֵ*********/
			ad_num2 = D[2];
			ad_num2 = (ad_num2/169)*100;
			ad_num1 = ad_num2;
			N[1] = ad_num1;
		/**********����Ϊ����ת��Ϊ����ǿ��ֵ*********/
			ad_num2 = 255-D[0];
			ad_num2 = (ad_num2/255)*100;
			ad_num1 = ad_num2;
			N[2] = ad_num1;
		
		
		/**********������Ļ��ʾ*************/
		EX0  = 0;//��ʾ���̹��ж�
		switch(flag)
		{
			case 0:
				OLED_ShowString(0,0,"Current_Hum_vol:"); 
				OLED_ShowNum(15,2,N[1],3,16);
				OLED_ShowString(40,2,"%"); 
				break;
			case 1:
				OLED_ShowString(0,0,"Light_Intensity"); 
				OLED_ShowNum(15,2,N[2],3,16);
				OLED_ShowString(40,2,"%");
				break;
			case 2:
				OLED_ShowString(0,0,"Humi&Temp");
				for(i=0;i<9;i++)				
				OLED_ShowChar(10*i,2,rec_dat[i]);
				i = 0;
				break;
			case 3:
				OLED_ShowString(0,0,"Air_Quality:");
				OLED_ShowNum(15,2,D[3],3,16);
				break;
			case 4:
				OLED_ShowString(0,0,"esp8266test");
				for(i=0;i<8;i++)				
				OLED_ShowChar(10*i,3,receive[i]);
				i = 0;
				break;
		}
		
		
		/***********���½���������ʪ���뻷���¶Ƚ��е�ˮ������ȿ���**************/
		if(N[1]<90)
		{
			pumpmove();
		}
		else
		{
			pumpstop();
		}
		
		//dht11
		dht11 =  rec_dat[0]*10+rec_dat[1];
		dht11 = (int)dht11-16;
		if(dht11>80)
		{
			fanmove();
		}
		else
		{
			fanstop();
		}
		
		EX0  = 1;//��ʾ�꿪�ж�
		
		
		/***********���½��д���ͨ��WiFi���ֻ���������**************/
		//���ݽ��յ������־���ִ�еĲ���
			if(sented)
			{
				k = receive[1];
				
				switch(k)
				{
					case '0'://���͵�ǰ������ʪ�ȡ�����ǿ�ȡ�������ʪ�ȡ���������
					Uart_SendStrData("AT+CIPSEND=0,70\r\n");//�������ݳ�ʼ��
						delay_ms(20);//�ȴ�20ms��Ӧ
					Uart_SendStrData("����ʪ��:");//��������							
						int2str(N[1]);
						Uart_SendStrData(buf);
					Uart_SendStrData("%  ����ǿ��:");
						int2str(N[2]);
						Uart_SendStrData(buf);
					Uart_SendStrData("%  ������ʪ��:");	
						for(i=0;i<9;i++)				
						Uart_SendByteData(rec_dat[i]);
						i = 0;
						//Uart_SendStrData(rec_dat);	
						//Uart_SendStrData("  ");
					Uart_SendStrData(" ��������AQI=:");
						int2str(D[3]);
						Uart_SendStrData(buf);
					Uart_SendStrData("\r\n");
						
						sented = 0;//�������
						break;
					
					case '1':
						fanmove();
						delay_ms(1000);
						delay_ms(1000);
						delay_ms(1000);
						fanstop();
						sented = 0;//�������
					
						break;
							
					case '2':						
						pumpmove();
						delay_ms(1000);
						delay_ms(1000);
						delay_ms(1000);
						pumpstop();
						sented = 0;//�������
						break;
				}
		
			}
	
	
	}	//while(1)

}//main



/*************************************************
�ⲿ�ж�0������
*************************************************/
void ISR_Key(void) interrupt 0 using 1
{
	EX0 = 0;
	//delay_ms(20);
	OLED_Clear();
	flag++;
	if(flag>4)
	{
		flag = 0;
	}
	
	while(!KEY)
	{
		;
	}
	
	//delay_ms(20);
	EX0 = 1;
}

/*************************************************
�����жϷ�����
*************************************************/

void UART_Interrupt(void) interrupt 4 
	{
		unsigned char res;
		if(RI)
			{
				res = SBUF;
				RI = 0;
				if(res == ':'||i>0)
				{
					receive[i] = res;
					i++;
					if(res == '\n')
					{	
						i = 0;
						//memset(receive,0,8);//��ս�����
					}
				}	
				sented = 1;
			}
	
						// ����������ж�
	if(TI)
		{
			TI = 0;
			uart_busy = 0;
		}
	
}


/***********************************************
������ת��Ϊ�ַ�������
************************************************/
void int2str(unsigned int msg)
{
	buf[0] = '0'+(msg/100);
	buf[1] = '0'+(msg%100)/10;
	buf[2] = '0'+(msg%100)%10;

}


/**********************************************
����ת����ֹͣ����
**********************************************/
void fanmove(void)
{
			fan1 = 1;
			fan2 = 0;
			OLED_ShowString(70,6,"FAN  GO");
}
	
void fanstop(void)
{
			fan1 = 0;
			fan2 = 0;
			OLED_ShowString(70,6,"FANSTOP");
}

/**********************************************
ˮ��ת����ֹͣ����
**********************************************/
void pumpmove(void)
{
			pump1 = 1;
			pump2 = 0;
			OLED_ShowString(0,6,"PUMP  GO");
}
	
void pumpstop(void)
{
			pump1 = 0;
			pump2 = 0;
			OLED_ShowString(0,6,"PUMPSTOP");
}


/*******************************************************************
DAC �任, ת������               
*******************************************************************/
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val)
{
   Start_I2c();              //��������
   SendByte(sla);            //����������ַ
   if(ack==0)return(0);
   SendByte(c);              //���Ϳ����ֽ�
   if(ack==0)return(0);
   SendByte(Val);            //����DAC����ֵ  
   if(ack==0)return(0);
   Stop_I2c();               //��������
   return(1);
}

/*******************************************************************
ADC�����ֽ�[����]���ݺ���               
*******************************************************************/
bit ISendByte(unsigned char sla,unsigned char c)
{
   Start_I2c();              //��������
   SendByte(sla);            //����������ַ
   if(ack==0)return(0);
   SendByte(c);              //��������
   if(ack==0)return(0);
   Stop_I2c();               //��������
   return(1);
}

/*******************************************************************
ADC���ֽ����ݺ���               
*******************************************************************/
unsigned char IRcvByte(unsigned char sla)
{  unsigned char c;

   Start_I2c();          //��������
   SendByte(sla+1);      //����������ַ
   if(ack==0)return(0);
   c=RcvByte();          //��ȡ����0

   Ack_I2c(1);           //���ͷǾʹ�λ
   Stop_I2c();           //��������
   return(c);
}



	/*
		��Ļ�÷�
		OLED_Clear();
		OLED_ShowCHinese(0,0,0);//��
		OLED_ShowCHinese(18,0,1);//��
		OLED_ShowCHinese(36,0,2);//԰
		OLED_ShowCHinese(54,0,3);//��
		OLED_ShowCHinese(72,0,4);//��
		OLED_ShowCHinese(90,0,5);//��
		OLED_ShowCHinese(108,0,6);//��
		OLED_ShowString(0,2,"1.3' OLED TEST");
		OLED_ShowString(8,2,"ZHONGJINGYUAN");  
	 	OLED_ShowString(20,4,"2014/05/01");  
		OLED_ShowString(0,6,"ASCII:");  
		OLED_ShowString(63,6,"CODE:");  
		OLED_ShowChar(48,6,t);//��ʾASCII�ַ�	   
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,6,t,3,16);//��ʾASCII�ַ�����ֵ 	
		delay_ms(500);
		OLED_Clear();
		delay_ms(50);
		OLED_DrawBMP(0,0,128,8,BMP1);  //ͼƬ��ʾ(ͼƬ��ʾ���ã����ɵ��ֱ�ϴ󣬻�ռ�ý϶�ռ䣬FLASH�ռ�8K��������)
		delay_ms(200);
		LED6=1;
		OLED_DrawBMP(0,0,128,8,BMP2);
		delay_ms(500);*/
