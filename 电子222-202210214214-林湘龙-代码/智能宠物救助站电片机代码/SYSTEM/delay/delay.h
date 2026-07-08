#ifndef __DELAY_H
#define __DELAY_H
#include "stm32f10x.h"

void delay_init(u8 SYSCLK); // 延时初始化（参数：系统时钟频率）
void delay_ms(u16 nms); // 毫秒级延时
void delay_us(u32 nus); // 微秒级延时

#endif
