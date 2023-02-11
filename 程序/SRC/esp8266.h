#include <REG52.H>
#ifndef __ESP8266_H
#define __ESP8266_H	
void delay5ms();
void Init_uart(void) ;
void Uart_SendIntData(unsigned int msg);
void Uart_SendByteData(unsigned char msg);
void Uart_SendStrData(unsigned char *msg);
void Init_ESP01S();


#endif