#ifndef __HW_H
#define	__HW_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	光电红外传感器h文件
 * 版本			: V1.0
 * 日期			: 2024.8.12
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

/***************根据自己需求更改****************/
// HW GPIO宏定义

#define		HW_GPIO_CLK								RCC_APB2Periph_GPIOA // 红外传感器GPIO时钟
#define 	HW_GPIO_PORT							GPIOA // 红外传感器引脚端口
#define 	HW_GPIO_PIN								GPIO_Pin_0 // 红外传感器引脚号

/*********************END**********************/


void HW_Init(void); // 红外传感器初始化
uint16_t HW_GetData(void); // 获取红外传感器ADC数据

#endif /* __ADC_H */
