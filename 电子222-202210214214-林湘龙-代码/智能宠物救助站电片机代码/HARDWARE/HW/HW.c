#include "HW.h" // 光电红外传感器驱动头文件

/*******************************
											STM32
 * 文件			:	光电红外传感器c文件                   
 * 版本			: V1.0
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码					
 
**********************BEGIN***********************/	

/**
  * @brief  光电红外传感器GPIO初始化，配置DO引脚为上拉输入模式
  * @param  无
  * @retval 无
  */
void HW_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
		
		RCC_APB2PeriphClockCmd (HW_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = HW_GPIO_PIN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 设置为上拉输入
		
		GPIO_Init(HW_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
	
}

/**
  * @brief  读取光电红外传感器的状态值
  * @param  无
  * @retval 1表示检测到障碍物（红外被遮挡），0表示未检测到
  */
uint16_t HW_GetData(void)
{
	uint16_t tempData; // 存储传感器读取结果
	tempData = !GPIO_ReadInputDataBit(HW_GPIO_PORT, HW_GPIO_PIN); // 读取引脚电平并取反：低电平表示检测到目标
	return tempData; // 返回0或1
}



