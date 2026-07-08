#ifndef __HX711_H
#define	__HX711_H
#include "stm32f10x.h"
#include "delay.h"
#include "math.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	HX711电子秤模块h文件
 * 版本			: V1.0
 * 日期			: 2024.9.11
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

/**************V2.0版本优化了取重函数，使模块可以使用正方向和反方向的电子称传感器****************/



/***************根据自己需求更改****************/
// HX711 GPIO宏定义
#define		HX711_GPIO_CLK									RCC_APB2Periph_GPIOA // HX711 GPIO时钟
#define 	HX711_SCK_GPIO_PORT							GPIOA // HX711时钟引脚端口
#define 	HX711_SCK_GPIO_PIN							GPIO_Pin_5 // HX711时钟引脚号
#define 	HX711_DT_GPIO_PORT							GPIOA // HX711数据引脚端口
#define 	HX711_DT_GPIO_PIN								GPIO_Pin_4 // HX711数据引脚号
/*********************END**********************/

#define HX711_SCK_H				GPIO_SetBits(HX711_SCK_GPIO_PORT,HX711_SCK_GPIO_PIN); // SCK拉高
#define HX711_SCK_L				GPIO_ResetBits(HX711_SCK_GPIO_PORT,HX711_SCK_GPIO_PIN); // SCK拉低

#define HX711_DT					GPIO_ReadInputDataBit(HX711_DT_GPIO_PORT, HX711_DT_GPIO_PIN) // 读取DT引脚状态



void HX711_Init(void); // HX711初始化
unsigned long HX711_GetData(void); // 获取HX711原始数据
float Get_Weight(float pi_weight); // 获取重量值
float Get_Tare(void); // 去皮（获取当前重量作为零点）

#endif /* __ADC_H */
