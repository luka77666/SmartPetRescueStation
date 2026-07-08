#ifndef __BUMP_H
#define	__BUMP_H
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	5V水泵模块h文件
 * 版本			: V1.0
 * 日期			: 2024.9.22
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

/***************根据自己需求更改****************/
// 水泵模块 GPIO宏定义

#define	BUMP_CLK							RCC_APB2Periph_GPIOA // 水泵GPIO时钟

#define BUMP_GPIO_PIN 				GPIO_Pin_0 // 水泵控制引脚号

#define BUMP_GPIO_PROT 				GPIOA // 水泵控制引脚端口

#define BUMP_ON 		GPIO_SetBits(BUMP_GPIO_PROT,BUMP_GPIO_PIN) // 水泵开启
#define BUMP_OFF 	GPIO_ResetBits(BUMP_GPIO_PROT,BUMP_GPIO_PIN) // 水泵关闭

/*********************END**********************/

void BUMP_Init(void); // 水泵初始化

#endif



