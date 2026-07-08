#ifndef __ADCX_H
#define __ADCX_H

#include "stm32f10x.h"                  // Device header

extern uint16_t ADC_Value[1]; // ADC转换结果缓存数组

/**
  * 函    数：ADC初始化
  * 参    数：无
  * 返 回 值：无
  */
void ADCX_Init(void); // ADC初始化
uint16_t WATER_ADC_Read(void); // 读取水位ADC值

#endif
