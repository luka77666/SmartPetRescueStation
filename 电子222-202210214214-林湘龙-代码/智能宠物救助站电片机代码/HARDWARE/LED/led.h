#ifndef	__LED_H
#define	__LED_H

#include "stm32f10x.h"                  // Device header

#define LED_GPIO_PORT		GPIOB // LED控制引脚端口
#define LED_GPIO_PIN	  GPIO_Pin_0 // LED控制引脚号

void LED_Init(void); // LED初始化
void LED_Toggle(void); // LED状态翻转
void LED_On(void); // LED开启
void LED_Off(void); // LED关闭
void LED_Twinkle(void); // LED闪烁
#endif
