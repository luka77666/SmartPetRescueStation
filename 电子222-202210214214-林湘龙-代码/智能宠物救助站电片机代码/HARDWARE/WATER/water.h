#ifndef __WATER_H
#define	__WATER_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	5V水泵模块h文件
 * 版本			: V1.0
 * 日期			: 2024.9.22
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

#define WATER_READ_TIMES	10  //WATER传感器ADC循环读取次数

//模式选择
//模拟AO:	1
//数字DO:	0
#define	MODE 	1 // 传感器模式选择：1=模拟量(AO), 0=数字量(DO)

/***************根据自己需求更改****************/
// WATER GPIO宏定义
#if MODE
#define		WATER_AO_GPIO_CLK								RCC_APB2Periph_GPIOA // 水位传感器AO引脚时钟
#define 	WATER_AO_GPIO_PORT							GPIOA // 水位传感器AO引脚端口
#define		WATER_AO_GPIO_PIN								GPIO_Pin_1 // 水位传感器AO引脚号
#define   ADC_CHANNEL               			ADC_Channel_1	// ADC 通道宏定义

#else
#define		WATER_DO_GPIO_CLK								RCC_APB2Periph_GPIOA // 水位传感器DO引脚时钟
#define 	WATER_DO_GPIO_PORT							GPIOA // 水位传感器DO引脚端口
#define		WATER_DO_GPIO_PIN								GPIO_Pin_1 // 水位传感器DO引脚号

#endif
/*********************END**********************/

void WATER_Init(void); // 水位传感器初始化
uint16_t WATER_GetData(void); // 获取水位ADC数据

#endif /* __WATER_H */
