#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f10x.h"                  // Device header
#include "delay.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	DHT11温湿度传感器h文件
 * 版本			: V1.0
 * 日期			: 2024.8.4
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

**********************BEGIN***********************/

/***************根据自己需求更改****************/
//DHT11引脚宏定义
#define DHT11_GPIO_PORT  GPIOA // DHT11数据引脚端口
#define DHT11_GPIO_PIN   GPIO_Pin_11 // DHT11数据引脚号
#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOA // DHT11 GPIO时钟
/*********************END**********************/

//输出状态定义
#define OUT 1 // 输出模式
#define IN  0 // 输入模式

//控制DHT11引脚输出高低电平
#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN) // DHT11引脚拉低
#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN) // DHT11引脚拉高


u8 DHT11_Init(void);//初始化DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//读取温湿度数据
u8 DHT11_Read_Byte(void);//读取一个字节的数据
u8 DHT11_Read_Bit(void);//读取一位的数据
void DHT11_Mode(u8 mode);//DHT11引脚输出模式控制
u8 DHT11_Check(void);//检测DHT11
void DHT11_Rst(void);//复位DHT11

#endif
