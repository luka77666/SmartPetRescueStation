#include "sys.h" // 系统底层驱动头文件

void WFI_SET(void) // 进入睡眠模式（等待中断唤醒）
{
	__ASM volatile("wfi");
}
//关闭所有中断
void INTX_DISABLE(void) // 关闭全局中断（PRIMASK=1）
{
	__ASM volatile("cpsid i"); // 禁止IRQ中断
}
//开启所有中断
void INTX_ENABLE(void) // 开启全局中断（PRIMASK=0）
{
	__ASM volatile("cpsie i");		  // 允许IRQ中断
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) // 设置主堆栈指针MSP
{
    MSR MSP, r0 			//set Main Stack value
    BX r14 // 返回
}
