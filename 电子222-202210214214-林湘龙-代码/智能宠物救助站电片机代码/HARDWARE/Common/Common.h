#ifndef __COMMON_H
#define __COMMON_H



#include "stm32f10x.h"



/******************************* 宏定义 ***************************/
#define            macNVIC_PriorityGroup_x                     NVIC_PriorityGroup_2 // NVIC优先级分组：2组（2位抢占优先级，2位响应优先级）



/********************************** 函数声明 ***************************************/
void                     USART_printf                       ( USART_TypeDef * USARTx, char * Data, ... ); // 串口格式化打印函数



#endif /* __COMMON_H */
