#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"                  // Device header


void FLASH_W(u32 add, u16 *dat, u8 count); // 向Flash写入多个16位数据

u16 FLASH_R(u32 add); // 从Flash读取一个16位数据

#endif
