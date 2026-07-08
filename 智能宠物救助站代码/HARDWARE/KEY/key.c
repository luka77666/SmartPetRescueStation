#include "key.h" // 按键驱动头文件

u8 key1_long_flag,key2_long_flag,key3_long_flag,key4_long_flag; // 按键1~4的长按标志位，1表示已触发长按
u8 key1_lock_flag,key2_lock_flag,key3_lock_flag,key4_lock_flag; // 按键1~4的自锁标志位，防止按下后重复触发
u16 key1_cnt,key2_cnt,key3_cnt,key4_cnt; // 按键1~4的消抖/长按计时计数器
u16 key1_cnt2,key2_cnt2,key3_cnt2,key4_cnt2; // 按键1~4的连击功能计时计数器
u8 key1_lock_flag,key2_lock_flag,key3_lock_flag,key4_lock_flag; // 按键1~4的自锁标志位（重复定义）

u8 key1_short_flag = 0; // 按键1短按标志位，1表示检测到短按
u8 key2_short_flag = 0; // 按键2短按标志位，1表示检测到短按
u8 key3_short_flag = 0; // 按键3短按标志位，按键3无短按返回
u8 key4_short_flag = 0; // 按键4短按标志位，按键4无短按返回


u8 KeyNum = 0; // 当前扫描到的按键键值，0表示无按键，1~4为短按，11/22/33/44为长按
/**
  * @brief  按键GPIO初始化，配置KEY1~KEY4为上拉输入模式
  * @param  无
  * @retval 无
  */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能GPIOB时钟
	GPIO_InitTypeDef GPIO_InitStructure; // GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 设置为上拉输入模式，按键按下时接地为低电平
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN | KEY2_GPIO_PIN | KEY3_GPIO_PIN | KEY4_GPIO_PIN ; // 配置所有按键引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速率50MHz
	GPIO_Init(KEY_PORT, &GPIO_InitStructure); // 初始化GPIO
}

/**
  * @brief  按键扫描函数，需在主循环中轮询调用。支持短按、长按、连击三种触发模式。
  *         按键1/2支持短按和长按，按键3/4支持连击和长按。
  * @param  无
  * @retval 无，结果保存在全局变量KeyNum中（松开时赋值）
  */
void Key_scan(void)
{
	if(KEY1) //如果没有按键按下
	{
		key1_lock_flag = 0; //清零自锁标志
		key1_cnt = 0; //清零计数标志
		if(key1_short_flag)
		{
			key1_short_flag = 0; //清零短按标志位
			KeyNum = 1; //赋键值编码 短按键值编码
		}
		if(key1_long_flag)
		{
			key1_long_flag = 0;	//清零长按标志位
			KeyNum = 11; //赋键值编码 长按键值编码
		}
	}
	else if(!key1_lock_flag)
	{
		key1_cnt++; //累计按键消抖延时次数
		if(key1_cnt > KEY_DELAY_TIME) // 超过消抖时间阈值
		{
			key1_cnt = 0; //清零计数标志
			key1_short_flag = 1; // 标记短按已触发
			key1_long_flag = 0;	//赋长按标志位 确保为0
			key1_lock_flag = 1; //自锁标志置1 防止多次触发
		}
	}
		else
	{
		key1_cnt++;	//累计按键长按功能延时时间

		if(key1_cnt > KEY1_LONG_TIME) // 超过长按时间阈值
		{
			key1_short_flag = 0; // 清除短按标志
			key1_long_flag = 1;	//赋长按标志位 标记长按已触发
		}
	}

	
	if(KEY2) //如果没有按键按下
	{
		key2_lock_flag = 0; //清零自锁标志
		key2_cnt = 0; //清零计数标志
		if(key2_short_flag)
		{
			key2_short_flag = 0; //清零短按标志位
			KeyNum = 2; //赋键值编码 短按键值编码
		}
		if(key2_long_flag)
		{
			key2_long_flag = 0;	//清零长按标志位
			KeyNum = 22; //赋键值编码 长按键值编码
		}
	}
	else if(!key2_lock_flag)
	{
		key2_cnt++; //累计按键消抖延时次数
		if(key2_cnt > KEY_DELAY_TIME) // 超过消抖时间阈值
		{
			key2_cnt = 0; //清零计数标志
			key2_short_flag = 1; // 标记短按已触发
			key2_long_flag = 0;	//赋长按标志位 确保为0
			key2_lock_flag = 1; //自锁标志置1 防止多次触发
		}
	}
		else
	{
		key2_cnt++;	//累计按键长按功能延时时间

		if(key2_cnt > KEY_LONG_TIME) // 超过长按时间阈值
		{
			key2_short_flag = 0; // 清除短按标志
			key2_long_flag = 1;	//赋长按标志位 标记长按已触发
		}
	}
	
	if(KEY3) //如果没有按键按下
	{
		key3_lock_flag = 0; //清零自锁标志
		key3_cnt = 0; //清零计数标志
		if(key3_long_flag)
		{
			key3_long_flag = 0;	//清零长按标志位
			KeyNum = 33; //赋键值编码 长按键值编码
		}
	}
	else if(!key3_lock_flag)
	{
		key3_cnt++; //累计按键消抖延时次数
		if(key3_cnt > KEY_DELAY_TIME) // 超过消抖时间阈值
		{
			key3_cnt = 0; //清零计数标志
			KeyNum = 3; //赋键值编码 短按键值编码
			key3_lock_flag = 1; //自锁标志置1 防止多次触发
		}
	}
	else if(key3_cnt < KEY_Continue_TIME) // 在消抖后到连击触发前的等待阶段
	{
		key3_cnt++;	//累计按键连击功能触发时间
	}
	else // 进入连击触发阶段
	{
		key3_cnt++;	//累计按键长按功能延时时间
		key3_cnt2++;	//累计按键连击功能延迟时间
		if(key3_cnt2 > KEY_Continue_Trigger_TIME) // 超过连击触发间隔
		{
			key3_cnt2 = 0;	//清零连击延迟时间
			KeyNum = 3;	//赋连击编码键值 持续按下时反复触发
		}
		if(key3_cnt > KEY_LONG_TIME) // 超过长按时间阈值
		{
			key3_long_flag = 1;	//赋长按标志位 标记长按已触发
		}
	
	}
	
	if(KEY4) //如果没有按键按下
	{
		key4_lock_flag = 0; //清零自锁标志
		key4_cnt = 0; //清零计数标志
		if(key4_long_flag)
		{
			key4_long_flag = 0;	//清零长按标志位
			KeyNum = 44; //赋键值编码 长按键值编码
		}
	}
	else if(!key4_lock_flag)
	{
		key4_cnt++; //累计按键消抖延时次数
		if(key4_cnt > KEY_DELAY_TIME) // 超过消抖时间阈值
		{
			key4_cnt = 0; //清零计数标志
			KeyNum = 4; //赋键值编码 短按键值编码
			key4_lock_flag = 1; //自锁标志置1 防止多次触发
		}
	}
	else if(key4_cnt < KEY_Continue_TIME) // 在消抖后到连击触发前的等待阶段
	{
		key4_cnt++;	//累计按键连击功能触发时间
	}
	else // 进入连击触发阶段
	{
		key4_cnt++;	//累计按键长按功能延时时间
		key4_cnt2++;	//累计按键连击功能延迟时间
		if(key4_cnt2 > KEY_Continue_Trigger_TIME) // 超过连击触发间隔
		{
			key4_cnt2 = 0;	//清零连击延迟时间
			KeyNum = 4;	//赋连击编码键值 持续按下时反复触发
		}
		if(key4_cnt > KEY_LONG_TIME) // 超过长按时间阈值
		{
			key4_long_flag = 1;	//赋长按标志位 标记长按已触发
		}
	
	}
}




