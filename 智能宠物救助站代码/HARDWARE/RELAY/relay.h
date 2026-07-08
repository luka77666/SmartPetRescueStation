#ifndef __RELAY_H
#define	__RELAY_H
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	5V继电器h文件
 * 版本			: V1.0
 * 日期			: 2024.9.18
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

/***************根据自己需求更改****************/
// 继电器 GPIO宏定义

#define	RELAY_CLK							RCC_APB2Periph_GPIOB // 继电器GPIO时钟

#define RELAY_GPIO_PIN 				GPIO_Pin_1 // 继电器控制引脚号

#define RELAY_GPIO_PROT 			GPIOB // 继电器控制引脚端口

#define RELAY_ON 		GPIO_SetBits(RELAY_GPIO_PROT,RELAY_GPIO_PIN) // 继电器吸合
#define RELAY_OFF 	GPIO_ResetBits(RELAY_GPIO_PROT,RELAY_GPIO_PIN) // 继电器断开


/*********************END**********************/

void RELAY_Init(void); // 继电器初始化

#endif



