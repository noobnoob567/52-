#include "esp8266.h"
bit uart_busy = 0;



/* ----------------------
	  延时5毫秒
---------------------- */
void delay5ms() {
    unsigned char a,b;
    for(b=101;b>0;b--)
        for(a=147;a>0;a--);
}

/* ----------------------
	  初始化串口，波特率115200
---------------------- */
void Init_uart(void) 
	{  
	   SCON=0x50; //串口工作方式1，8位UART，波特率可变  
     TH2=0xFF;           
     TL2=0xFD;    //波特率:115200 晶振=11.0592MHz 
     RCAP2H=0xFF;   
     RCAP2L=0xFD; //16位自动再装入值


/*****************/
     TCLK=1;   
     RCLK=1;   
     C_T2=0;   
     EXEN2=0; //波特率发生器工作方式


/*****************/
		TR2=1 ; //定时器2开始
		EA =1;                 //开总中断  
		ES =1;                 //开串口1中断  
	}


/* ----------------------
	串口发送数据方法
---------------------- */
	



void Uart_SendByteData(unsigned char msg){
	while(uart_busy); //等待上一个数据发送完成
	SBUF=msg;		  
	uart_busy = 1;	 //串口发送置忙
}
void Uart_SendStrData(unsigned char *msg){
	while(*msg){
		Uart_SendByteData(*msg++);
	}
}

void Uart_SendIntData(unsigned int msg)
{
	unsigned char i,j,k;
	i = msg/100;
	j = (msg%100)/10;
	k = (msg%100)%10;
	Uart_SendByteData(i);
	Uart_SendByteData(j);
	Uart_SendByteData(k);
}
/* ----------------------
	初始化WIFI模块
---------------------- */
void Init_ESP01S(){   
	Uart_SendStrData("AT+CIPMUX=1\r\n");            // 设置多连接
	delay5ms();   
	Uart_SendStrData("AT+CIPSERVER=1,8080\r\n");      // 配置服务器，监听8080端口
	delay5ms();
}


/*void main(){
	Init_uart();
	Init_ESP01S();    // Init_ESP01S 与 Init_uart调用的顺序不能改变
	
	while(1);
}


/* ----------------------
	  串口中断
---------------------- */
/*
void UART_Interrupt(void) interrupt 4 {
	unsigned char one_char;
	
	if(RI)
	{
		one_char = SBUF;
		RI = 0;
		//解析ESP-01S发送到串口的数据
	
	}
	
	// 处理发送完成中断
	if(TI)
	{
		TI = 0;
		uart_busy = 0;
	}
	
}
*/