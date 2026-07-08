#include "beep.h" // 蜂鸣器驱动头文件
#include "delay.h" // 毫秒延时函数头文件

/**
  * @brief  蜂鸣器GPIO初始化，配置为推挽输出并默认关闭（低电平关闭）
  * @param  无
  * @retval 无
  */
void BEEP_Init(void)
{
	//开启GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // 使能GPIOC时钟（蜂鸣器接在GPIOC上）
	
	//配置LED引脚为模拟输出模式
	GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = BEEP_GPIO_PIN; // 选择蜂鸣器控制引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速率50MHz
	GPIO_Init(BEEP_GPIO_PROT, &GPIO_InitStructure); // 初始化GPIO
	GPIO_ResetBits(BEEP_GPIO_PROT, BEEP_GPIO_PIN); // 默认输出低电平，关闭蜂鸣器
}

/**
  * @brief  蜂鸣器电平翻转，用于产生蜂鸣效果
  * @param  无
  * @retval 无
  */
void BEEP_Toggle(void)
{
	GPIO_WriteBit(BEEP_GPIO_PROT, BEEP_GPIO_PIN, (BitAction)((1-GPIO_ReadOutputDataBit(BEEP_GPIO_PROT, BEEP_GPIO_PIN))));//led电平翻转 读取当前电平取反后写入
}

/**
  * @brief  打开蜂鸣器（输出高电平）
  * @param  无
  * @retval 无
  */
void BEEP_On()
{
	GPIO_WriteBit(BEEP_GPIO_PROT, BEEP_GPIO_PIN,(BitAction)1); // 输出高电平打开蜂鸣器
}

/**
  * @brief  关闭蜂鸣器（输出低电平）
  * @param  无
  * @retval 无
  */
void BEEP_Off()
{
	GPIO_WriteBit(BEEP_GPIO_PROT, BEEP_GPIO_PIN,(BitAction)0); // 输出低电平关闭蜂鸣器
}

/**
  * @brief  蜂鸣器短响一次（响10ms），用于按键提示音等
  * @param  无
  * @retval 无
  */
void BEEP_Twinkle()
{
	BEEP_On(); // 打开蜂鸣器
	delay_ms(10); // 响10毫秒
	BEEP_Off(); // 关闭蜂鸣器
}
