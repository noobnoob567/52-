#include <REG52.H>
#ifndef __DHT11_H
#define __DHT11_H	

void DHT11_delay_us(char n);
void DHT11_delay_ms(int z);
void DHT11_start();
char DHT11_rec_byte() ;
void DHT11_receive() ;

#endif