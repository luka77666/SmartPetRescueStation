#include "stm32f10x.h"                  // STM32标准库头文件
#include <math.h> // 数学函数库
#include "delay.h" // 延时函数头文件

static uint16_t ADC_Value[1];				//存放AD转换结果


#define WATER				GPIO_Pin_1 // 水位传感器引脚定义（PA1）



/**
  * 函    数：ADC初始化
  * 参    数：无
  * 返 回 值：无
  */
void ADCX_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	//开启ADC1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		//开启DMA1的时钟

	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择6分频

	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 模拟输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1; // 选择PA1引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//将PB0、PB1初始化为模拟输入

	/*规则组通道配置*/
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5); // 配置ADC1通道1，规则组第1个，采样时间55.5周期

	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;											//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//独立模式
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//软件触发
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;								//扫描模式
	ADC_InitStructure.ADC_NbrOfChannel = 1;										//扫描规则组的前1个通道
	ADC_Init(ADC1, &ADC_InitStructure);											//配置ADC1

	/*DMA初始化*/
	DMA_InitTypeDef DMA_InitStructure;											//定义结构体变量
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;				//外设基地址
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//外设数据宽度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址自增，选择失能
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_Value;					//存储器基地址
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//存储器数据宽度
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//存储器地址自增，选择使能
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//数据传输方向
	DMA_InitStructure.DMA_BufferSize = 1;											//转运的转运次数
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//循环模式
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;									//存储器到存储器，选择失能
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//优先级，选择中等
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);									//配置DMA1的通道1

	/*DMA和ADC使能*/
	DMA_Cmd(DMA1_Channel1, ENABLE);												//DMA1的通道1使能
	ADC_DMACmd(ADC1, ENABLE);													//ADC1触发DMA1的信号使能
	ADC_Cmd(ADC1, ENABLE);														//ADC1使能

	/*ADC校准*/
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET); // 等待复位校准完成
	ADC_StartCalibration(ADC1); // 开始ADC校准
	while (ADC_GetCalibrationStatus(ADC1) == SET); // 等待校准完成

	/*ADC触发*/
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);	//软件触发ADC开始工作
}

uint16_t WATER_ADC_Read(void) // 读取水位传感器ADC值
{
	//设置指定ADC的规则组通道，采样时间
	return ADC_Value[0]; // 返回DMA搬运到内存中的ADC转换结果
}

