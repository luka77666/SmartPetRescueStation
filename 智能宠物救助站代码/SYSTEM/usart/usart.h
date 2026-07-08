#ifndef __USART1_H
#define	__USART1_H

#include "stm32f10x.h"
#include <stdio.h>

void USART1_Config(void); // 串口1配置初始化
int fputc(int ch, FILE *f); // 重定向fputc函数（支持printf输出）
void USART1_printf(USART_TypeDef* USARTx, uint8_t *Data,...); // 串口1格式化打印

#endif /* __USART1_H */
