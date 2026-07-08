#ifndef __USART2_H
#define __USART2_H

#include "stm32f10x.h"                  // Device header
#include "usart.h"

#define USART2_RXBUFF_SIZE 128 // 串口2接收缓冲区大小

void USART2_Config(void); //串口 3 初始化

void Usart_SendString(unsigned char *str, unsigned short len); // 串口发送字符串

#endif


