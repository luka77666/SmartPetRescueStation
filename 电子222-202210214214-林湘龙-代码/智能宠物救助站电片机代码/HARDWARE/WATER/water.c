#include "water.h" // 水位传感器驱动头文件

/*********************************
						STM32
 * 文件			:	水位传感器c文件                   
 * 版本			: V1.0
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码					

*********************************************/

/**
  * @brief  水位传感器初始化，根据MODE宏选择ADC模拟读取或DO数字读取模式
  * @param  无
  * @retval 无
  */
void WATER_Init(void)
{
	#if MODE
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (WATER_AO_GPIO_CLK, ENABLE );	// 打开 ADC IO端口时钟
		GPIO_InitStructure.GPIO_Pin = WATER_AO_GPIO_PIN;					// 配置 ADC IO 引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(WATER_AO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO
		
		ADCX_Init(); // 初始化ADC外设
	}
	#else
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (WATER_DO_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = WATER_DO_GPIO_PIN;		// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 设置为上拉输入
		
		GPIO_Init(WATER_DO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
		
	}
	#endif
	
}

#if MODE

#endif

/**
  * @brief  获取水位传感器数据，支持ADC多次采样均值或DO数字电平读取
  * @param  无
  * @retval ADC模式下返回多次采样均值(0~4095)，DO模式下返回0或1
  */
uint16_t WATER_GetData(void)
{
	
	#if MODE
	uint32_t  tempData = 0; // ADC采样累加和
	for (uint8_t i = 0; i < WATER_READ_TIMES; i++) // 循环读取多次ADC值
	{
		tempData += WATER_ADC_Read(); // 累加每次ADC读取值
		delay_ms(5); // 每次采样间隔5毫秒
	}

	tempData /= WATER_READ_TIMES; // 计算多次采样的平均值
	return tempData; // 返回平均后的ADC值
	
	#else
	uint16_t tempData; // 存储DO引脚电平状态
	tempData = GPIO_ReadInputDataBit(WATER_DO_GPIO_PORT, WATER_DO_GPIO_PIN); // 读取水位传感器DO引脚的电平
	return tempData; // 返回数字量：高电平或低电平
	#endif
}


