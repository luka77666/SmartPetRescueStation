#ifndef	__BEEP_H
#define	__BEEP_H

#include "stm32f10x.h"                  // Device header

#define BEEP_GPIO_PROT		GPIOC // 蜂鸣器控制引脚端口
#define BEEP_GPIO_PIN	  	GPIO_Pin_13 // 蜂鸣器控制引脚号

void BEEP_Init(void); // 蜂鸣器初始化
void BEEP_Toggle(void); // 蜂鸣器状态翻转
void BEEP_On(void); // 蜂鸣器开启
void BEEP_Off(void); // 蜂鸣器关闭
void BEEP_Twinkle(void); // 蜂鸣器闪烁鸣叫
#endif
