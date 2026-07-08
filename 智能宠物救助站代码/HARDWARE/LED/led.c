#include "led.h" // LED指示灯驱动头文件
#include "delay.h" // 毫秒延时函数头文件

/**
  * @brief  LED指示灯GPIO初始化，配置为推挽输出并默认点亮（低电平点亮）
  * @param  无
  * @retval 无
  */
void LED_Init(void)
{
	//开启GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOB时钟
	
	//配置LED引脚为模拟输出模式
	GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 设置为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = LED_GPIO_PIN; // 选择LED控制引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速率50MHz
	GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure); // 初始化GPIO
	GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN); // 默认输出低电平点亮LED
}

/**
  * @brief  LED电平翻转，用于闪烁效果
  * @param  无
  * @retval 无
  */
void LED_Toggle(void)
{
	GPIO_WriteBit(LED_GPIO_PORT, LED_GPIO_PIN, (BitAction)((1-GPIO_ReadOutputDataBit(LED_GPIO_PORT, LED_GPIO_PIN))));//led电平翻转 读取当前电平取反后写入
}

/**
  * @brief  打开LED（输出低电平点亮，低电平有效）
  * @param  无
  * @retval 无
  */
void LED_On()
{
	GPIO_WriteBit(LED_GPIO_PORT, LED_GPIO_PIN,(BitAction)0); // 输出低电平点亮LED
}

/**
  * @brief  关闭LED（输出高电平熄灭，低电平有效）
  * @param  无
  * @retval 无
  */
void LED_Off()
{
	GPIO_WriteBit(LED_GPIO_PORT, LED_GPIO_PIN,(BitAction)1); // 输出高电平熄灭LED
}

/**
  * @brief  LED闪烁一次（亮10ms后熄灭），用于状态提示
  * @param  无
  * @retval 无
  */
void LED_Twinkle()
{
	LED_On(); // 点亮LED
	delay_ms(10); // 亮10毫秒
	LED_Off(); // 熄灭LED
}
