#ifndef	__KEY_H
#define __KEY_H

#include "stm32f10x.h"                  // Device header

#define KEY1_GPIO_PIN	GPIO_Pin_12 // 按键1引脚号
#define KEY2_GPIO_PIN	GPIO_Pin_13 // 按键2引脚号
#define KEY3_GPIO_PIN	GPIO_Pin_14 // 按键3引脚号
#define KEY4_GPIO_PIN	GPIO_Pin_15 // 按键4引脚号

#define KEY_PORT	GPIOB // 按键所在GPIO端口

#define KEY1	GPIO_ReadInputDataBit(GPIOB, KEY1_GPIO_PIN) // 读取按键0
#define KEY2	GPIO_ReadInputDataBit(GPIOB, KEY2_GPIO_PIN) // 读取按键1
#define KEY3	GPIO_ReadInputDataBit(GPIOB, KEY3_GPIO_PIN) // 读取按键2
#define KEY4	GPIO_ReadInputDataBit(GPIOB, KEY4_GPIO_PIN) // 读取按键2

#define KEY_DELAY_TIME							10 // 按键消抖延时时间(ms)
#define KEY_LONG_TIME								2000 // 长按判定时间(ms)
#define KEY1_LONG_TIME							800 // 按键1长按判定时间(ms)

#define KEY_Continue_TIME						500 // 连续触发间隔时间(ms)
#define KEY_Continue_Trigger_TIME		5 // 连续触发次数阈值

extern u8 KeyNum; // 按键键值变量

void Key_Init(void); // 按键GPIO初始化
void Key_scan(void); // 按键扫描函数

#endif
