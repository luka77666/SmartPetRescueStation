#include "stm32f10x_it.h"       // 中断服务程序头文件

/*====== Cortex-M3 核心异常处理（系统保留，一般不修改）======*/

void NMI_Handler(void)           // 不可屏蔽中断处理（硬件错误）
{
}

void HardFault_Handler(void)      // 硬件错误处理（死循环防止跑飞）
{
  while (1)                      // 硬件错误时进入死循环
  {
  }
}

void MemManage_Handler(void)      // 内存管理错误处理
{
  while (1)
  {
  }
}

void BusFault_Handler(void)       // 总线错误处理
{
  while (1)
  {
  }
}

void UsageFault_Handler(void)     // 用法错误处理（除零/未对齐等）
{
  while (1)
  {
  }
}

void SVC_Handler(void)           // 系统服务调用处理
{
}

void DebugMon_Handler(void)      // 调试监视处理
{
}

void PendSV_Handler(void)        // 可挂起系统服务调用
{
}

void SysTick_Handler(void)       // 系统滴答定时器中断（1ms周期）
{
}
