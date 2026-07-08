#ifndef __IWDG_H
#define __IWDG_H

#include "stm32f10x.h"

void IWDG_Init(uint16_t prer, uint16_t rlr); // 独立看门狗初始化（参数：预分频值、重装载值）
void IWDG_Feed(void); // 喂狗（重置看门狗计数器）

#endif
