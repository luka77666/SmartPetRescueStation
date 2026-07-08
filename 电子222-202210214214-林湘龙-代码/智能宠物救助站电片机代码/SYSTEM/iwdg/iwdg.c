#include "iwdg.h" // 独立看门狗驱动头文件

/**
 * @brief  初始化独立看门狗（直接寄存器操作，不依赖stm32f10x_iwdg.c）
 * @param  prer: 预分频值 (IWDG_Prescaler_4~256)
 * @param  rlr:  重装载值 (0~0xFFF)
 *         超时计算: T = rlr * prer / 40000 (LSI约40kHz)
 *         例: prer=64, rlr=1875 → T = 1875*64/40000 = 3s
 */
void IWDG_Init(uint16_t prer, uint16_t rlr) // 初始化独立看门狗，prer为预分频值，rlr为重装载值
{
    IWDG->KR = 0x5555;          // 使能寄存器写入
    IWDG->PR = prer;            // 设置预分频
    IWDG->RLR = rlr;            // 设置重装载值
    IWDG->KR = 0xAAAA;          // 装载计数器(喂狗)
    IWDG->KR = 0xCCCC;          // 启动看门狗
}

void IWDG_Feed(void) // 喂狗函数（重装载计数器，防止系统复位）
{
    IWDG->KR = 0xAAAA;  // 喂狗，重载计数器
}
