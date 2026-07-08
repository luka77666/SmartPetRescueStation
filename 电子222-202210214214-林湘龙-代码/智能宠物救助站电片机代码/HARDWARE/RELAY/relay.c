#include "relay.h" // 继电器驱动头文件

/*******************************
						STM32
 * 文件			:	5V继电器c文件                   
 * 版本			: V1.0
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码					
 
**********************BEGIN***********************/

/**
  * @brief  继电器GPIO初始化，配置为推挽输出并默认关闭继电器
  * @param  无
  * @retval 无
  */
void RELAY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
	RCC_APB2PeriphClockCmd(RELAY_CLK, ENABLE ); //配置时钟 使能继电器所在GPIO端口时钟
	
	GPIO_InitStructure.GPIO_Pin = RELAY_GPIO_PIN; // 选择继电器控制引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO输出速率50MHz
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置为推挽输出模式
	GPIO_Init(RELAY_GPIO_PROT,&GPIO_InitStructure); // 初始化GPIO

	RELAY_OFF; // 默认关闭继电器（水泵关闭）
}
