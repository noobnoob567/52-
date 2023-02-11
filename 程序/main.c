//////////////////////////////////////////////////////////////////////////////////	 
//认识实习程序
//2021.6.17 将ad转换得到的值显示在屏幕上
//
//SPI
//sbit OLED_SCL=P2^7;//时钟 D0（SCLK?
//sbit OLED_SDIN=P2^6;//D1（MOSI） 数据
//sbit OLED_RST =P2^5;//复位
//sbit OLED_DC =P2^4;//数据/命令控制
//sbit OLED_CS=P2^3; //片选

//IIC
//P10 SCL
//P11 SDA

//AD vref = 5v
//DHT11数据引脚接p3.6
//              ----------------------------------------------------------------

#include "REG52.h"
#include "oled.h"
//#include "bmp.h"
#include "I2C.H"
#include "dht11.h"
#include "esp8266.h"
#define uchar unsigned char
#define  PCF8591 0x90    //PCF8591 地址

//针脚定义
sbit KEY = P3^2;
sbit pump1 = P0^0;//水泵与风扇控制标志
sbit pump2 = P0^1;
sbit fan1 = P0^2;
sbit fan2 = P0^3;


// 变量定义
unsigned char AD_CHANNEL;
//unsigned long xdata  LedOut[8];
unsigned int  D[3];//存储ad值（255）
int	ad_num1;
float ad_num2;//转换中间变量
unsigned int	N[4];//存储转换后的AD值
//1:土壤湿度(百分)  2:光照强度值(百分)
unsigned int i,j;//转换循环计数
unsigned int flag = 0;//屏幕显示循环标志
extern char rec_dat[9];//dht11获得结果
unsigned char dht11;//dht11转换结果
unsigned char receive[8];
unsigned char k;//从esp8266接收的数字作为标志位
unsigned char buf[3];//向手机发送时的数据
extern bit uart_busy;//串口正忙标志
bit sented;//向手机已经发送标志



 //函数声明
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
		
	 /**********以下各模块初始化**************/
	 fanstop();
	 pumpstop();
	 Init_uart();			//串口初始化 baud=115200
	 Init_ESP01S();    //esp初始化发送设定端口号 //Init_ESP01S 与 Init_uart调用的顺序不能改变
	 //delay_init();	  //延时函数初始化	在oled里了  
		OLED_Init();			//初始化OLED  
    OLED_Clear();
		EX0 = 1;					//外部中断0开	 
	  IT0 = 0;					//低电平触发
		EA = 1;						//总中断开
	 
	while(1) 
	{		
		
		
		
			/********以下AD-DA处理*************/  
	
		
		for(AD_CHANNEL=0;AD_CHANNEL<4;AD_CHANNEL++)
		{
				switch(AD_CHANNEL)
			{
				case 0: ISendByte(PCF8591,0x41);
						D[0]=IRcvByte(PCF8591);  //ADC0 模数转换1    光敏电阻
					break;  
			
				case 1: ISendByte(PCF8591,0x42);
						D[1]=IRcvByte(PCF8591);  //ADC1 模数转换2	  热敏电阻
					break;  
			
				case 2: ISendByte(PCF8591,0x43);
						D[2]=IRcvByte(PCF8591);  //ADC2	模数转换3	   土壤湿度计
					break;  
			
				case 3: ISendByte(PCF8591,0x40);
						D[3]=IRcvByte(PCF8591);  //ADC3 模数转换4	   空气质量计
					break;  
			
				//case 4: DACconversion(PCF8591,0x40, D[4]); //DAC	  数模转换
				//     break;
		 
			}
    	//D[4]=D[3];  //把模拟输入采样的信号 通过数模转换输出			
		}
		
		
		/************以下dht11温湿度检测**************/
		DHT11_receive();
		
		
		
		/*********以下数字转换为电压**********/
/*
			ad_num2 = D[0];
			ad_num2 = (ad_num2/255)*5;
			ad_num1 = ad_num2;
			N[0] = ad_num1;
			ad_num2 = ((ad_num2-ad_num1)*100);
			N[1] = (int)ad_num2;
*/
		
		/**********以下为数字转换为土壤湿度值*********/
			ad_num2 = D[2];
			ad_num2 = (ad_num2/169)*100;
			ad_num1 = ad_num2;
			N[1] = ad_num1;
		/**********以下为数字转换为光照强度值*********/
			ad_num2 = 255-D[0];
			ad_num2 = (ad_num2/255)*100;
			ad_num1 = ad_num2;
			N[2] = ad_num1;
		
		
		/**********以下屏幕显示*************/
		EX0  = 0;//显示过程关中断
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
		
		
		/***********以下进行由土壤湿度与环境温度进行的水泵与风扇控制**************/
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
		
		EX0  = 1;//显示完开中断
		
		
		/***********以下进行串口通过WiFi向手机发送数据**************/
		//根据接收到的数字决定执行的操作
			if(sented)
			{
				k = receive[1];
				
				switch(k)
				{
					case '0'://发送当前的土壤湿度、光照强度、环境温湿度、空气质量
					Uart_SendStrData("AT+CIPSEND=0,70\r\n");//发送数据初始化
						delay_ms(20);//等待20ms响应
					Uart_SendStrData("土壤湿度:");//发送数据							
						int2str(N[1]);
						Uart_SendStrData(buf);
					Uart_SendStrData("%  光照强度:");
						int2str(N[2]);
						Uart_SendStrData(buf);
					Uart_SendStrData("%  环境温湿度:");	
						for(i=0;i<9;i++)				
						Uart_SendByteData(rec_dat[i]);
						i = 0;
						//Uart_SendStrData(rec_dat);	
						//Uart_SendStrData("  ");
					Uart_SendStrData(" 空气质量AQI=:");
						int2str(D[3]);
						Uart_SendStrData(buf);
					Uart_SendStrData("\r\n");
						
						sented = 0;//发送完毕
						break;
					
					case '1':
						fanmove();
						delay_ms(1000);
						delay_ms(1000);
						delay_ms(1000);
						fanstop();
						sented = 0;//发送完毕
					
						break;
							
					case '2':						
						pumpmove();
						delay_ms(1000);
						delay_ms(1000);
						delay_ms(1000);
						pumpstop();
						sented = 0;//发送完毕
						break;
				}
		
			}
	
	
	}	//while(1)

}//main



/*************************************************
外部中断0服务函数
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
串口中断服务函数
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
						//memset(receive,0,8);//清空接收区
					}
				}	
				sented = 1;
			}
	
						// 处理发送完成中断
	if(TI)
		{
			TI = 0;
			uart_busy = 0;
		}
	
}


/***********************************************
将数字转化为字符串函数
************************************************/
void int2str(unsigned int msg)
{
	buf[0] = '0'+(msg/100);
	buf[1] = '0'+(msg%100)/10;
	buf[2] = '0'+(msg%100)%10;

}


/**********************************************
风扇转动与停止函数
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
水泵转动与停止函数
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
DAC 变换, 转化函数               
*******************************************************************/
bit DACconversion(unsigned char sla,unsigned char c,  unsigned char Val)
{
   Start_I2c();              //启动总线
   SendByte(sla);            //发送器件地址
   if(ack==0)return(0);
   SendByte(c);              //发送控制字节
   if(ack==0)return(0);
   SendByte(Val);            //发送DAC的数值  
   if(ack==0)return(0);
   Stop_I2c();               //结束总线
   return(1);
}

/*******************************************************************
ADC发送字节[命令]数据函数               
*******************************************************************/
bit ISendByte(unsigned char sla,unsigned char c)
{
   Start_I2c();              //启动总线
   SendByte(sla);            //发送器件地址
   if(ack==0)return(0);
   SendByte(c);              //发送数据
   if(ack==0)return(0);
   Stop_I2c();               //结束总线
   return(1);
}

/*******************************************************************
ADC读字节数据函数               
*******************************************************************/
unsigned char IRcvByte(unsigned char sla)
{  unsigned char c;

   Start_I2c();          //启动总线
   SendByte(sla+1);      //发送器件地址
   if(ack==0)return(0);
   c=RcvByte();          //读取数据0

   Ack_I2c(1);           //发送非就答位
   Stop_I2c();           //结束总线
   return(c);
}



	/*
		屏幕用法
		OLED_Clear();
		OLED_ShowCHinese(0,0,0);//中
		OLED_ShowCHinese(18,0,1);//景
		OLED_ShowCHinese(36,0,2);//园
		OLED_ShowCHinese(54,0,3);//电
		OLED_ShowCHinese(72,0,4);//子
		OLED_ShowCHinese(90,0,5);//科
		OLED_ShowCHinese(108,0,6);//技
		OLED_ShowString(0,2,"1.3' OLED TEST");
		OLED_ShowString(8,2,"ZHONGJINGYUAN");  
	 	OLED_ShowString(20,4,"2014/05/01");  
		OLED_ShowString(0,6,"ASCII:");  
		OLED_ShowString(63,6,"CODE:");  
		OLED_ShowChar(48,6,t);//显示ASCII字符	   
		t++;
		if(t>'~')t=' ';
		OLED_ShowNum(103,6,t,3,16);//显示ASCII字符的码值 	
		delay_ms(500);
		OLED_Clear();
		delay_ms(50);
		OLED_DrawBMP(0,0,128,8,BMP1);  //图片显示(图片显示慎用，生成的字表较大，会占用较多空间，FLASH空间8K以下慎用)
		delay_ms(200);
		LED6=1;
		OLED_DrawBMP(0,0,128,8,BMP2);
		delay_ms(500);*/
