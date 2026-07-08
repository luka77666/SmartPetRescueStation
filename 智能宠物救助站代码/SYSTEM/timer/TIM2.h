#ifndef __TIM2_H_
#define __TIM2_H_

#include "stm32f10x.h"                  // Device header
#include "key.h"

void TIM2_Init(u16 Prescaler, u16 Period); // 定时器2初始化（参数：预分频值、自动重装载值）
extern volatile uint32_t sys_tick_ms;  // 系统毫秒计时器，每1ms在TIM2中断中递增


#endif
