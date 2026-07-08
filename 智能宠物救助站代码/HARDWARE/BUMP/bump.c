#include "bump.h" // 5V水泵模块驱动头文件

/*******************************
						STM32
 * 文件			:	5V水泵模块c文件                   
 * 版本			: V1.0
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码						
 
**********************BEGIN***********************/

/**
  * @brief  5V水泵模块GPIO初始化，配置为推挽输出并默认关闭
  * @param  无
  * @retval 无
  */
void BUMP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
	RCC_APB2PeriphClockCmd(BUMP_CLK, ENABLE ); // 使能水泵模块所在GPIO端口时钟
	
	GPIO_InitStructure.GPIO_Pin = BUMP_GPIO_PIN; // 选择水泵控制引脚（PA0）
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO输出速率50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置为推挽输出模式
	GPIO_Init(BUMP_GPIO_PROT,&GPIO_InitStructure); // 初始化GPIO

	BUMP_OFF; // 默认关闭水泵
}
