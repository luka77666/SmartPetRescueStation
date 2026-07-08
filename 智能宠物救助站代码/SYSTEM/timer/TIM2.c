#include "TIM2.h" // 定时器2驱动头文件
#include "stepmotor.h" // 步进电机驱动头文件

volatile uint32_t sys_tick_ms = 0;  // 系统毫秒计时器，每1ms递增

void TIM2_Init(u16 Prescaler, u16 Period) // 初始化定时器2，Prescaler为预分频值，Period为自动重装载值
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // 使能TIM2时钟

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure; // 定义定时器初始化结构体
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频：不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数模式：向上计数
	TIM_TimeBaseInitStructure.TIM_Period = Period; // 自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = Prescaler; // 预分频值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0; // 重复计数器值（高级定时器用）

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure); // 初始化定时器2

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // 使能定时器2更新中断

	NVIC_InitTypeDef NVIC_InitStructure; // 定义中断优先级结构体
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; // 选择TIM2中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // 使能中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; // 响应优先级为2
	NVIC_Init(&NVIC_InitStructure); // 初始化中断优先级

	TIM_Cmd(TIM2, ENABLE); // 启动定时器2
}

void TIM2_IRQHandler(void)	//1ms定时中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) // 检查是否发生更新中断
	{
		sys_tick_ms++;            // 毫秒计数器
		Key_scan();	//按键扫描函数
		MOTOR_StepTick(); //步进电机单步推进（非阻塞）
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // 清除中断标志位
	}
}
