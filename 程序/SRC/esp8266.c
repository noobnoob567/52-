#include "esp8266.h"
bit uart_busy = 0;



/* ----------------------
	  ��ʱ5����
---------------------- */
void delay5ms() {
    unsigned char a,b;
    for(b=101;b>0;b--)
        for(a=147;a>0;a--);
}

/* ----------------------
	  ��ʼ�����ڣ�������115200
---------------------- */
void Init_uart(void) 
	{  
	   SCON=0x50; //���ڹ�����ʽ1��8λUART�������ʿɱ�  
     TH2=0xFF;           
     TL2=0xFD;    //������:115200 ����=11.0592MHz 
     RCAP2H=0xFF;   
     RCAP2L=0xFD; //16λ�Զ���װ��ֵ


/*****************/
     TCLK=1;   
     RCLK=1;   
     C_T2=0;   
     EXEN2=0; //�����ʷ�����������ʽ


/*****************/
		TR2=1 ; //��ʱ��2��ʼ
		EA =1;                 //�����ж�  
		ES =1;                 //������1�ж�  
	}


/* ----------------------
	���ڷ������ݷ���
---------------------- */
	



void Uart_SendByteData(unsigned char msg){
	while(uart_busy); //�ȴ���һ�����ݷ������
	SBUF=msg;		  
	uart_busy = 1;	 //���ڷ�����æ
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
	��ʼ��WIFIģ��
---------------------- */
void Init_ESP01S(){   
	Uart_SendStrData("AT+CIPMUX=1\r\n");            // ���ö�����
	delay5ms();   
	Uart_SendStrData("AT+CIPSERVER=1,8080\r\n");      // ���÷�����������8080�˿�
	delay5ms();
}


/*void main(){
	Init_uart();
	Init_ESP01S();    // Init_ESP01S �� Init_uart���õ�˳���ܸı�
	
	while(1);
}


/* ----------------------
	  �����ж�
---------------------- */
/*
void UART_Interrupt(void) interrupt 4 {
	unsigned char one_char;
	
	if(RI)
	{
		one_char = SBUF;
		RI = 0;
		//����ESP-01S���͵����ڵ�����
	
	}
	
	// ����������ж�
	if(TI)
	{
		TI = 0;
		uart_busy = 0;
	}
	
}
*/