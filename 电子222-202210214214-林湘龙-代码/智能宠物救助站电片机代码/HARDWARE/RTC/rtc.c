/**********************************
包含头文件
**********************************/
#include "rtc.h"                 // 实时时钟驱动头文件
#include "delay.h"               // 延时函数头文件

uint16_t MyRTC_Time[] = {2023, 12, 17, 20, 15, 15};  // 全局时间数组（备用，实际使用calendar结构体）
uint8_t rtc_get_flag = 1;        // RTC时间获取标志（1=正常更新时间，0=暂停更新，用于设置页）

/*
注意：PC14和PC15不能用于普通IO使用，因为这两个引脚是外部晶振的输入和输出引脚
*/


/**********************************
变量定义
**********************************/
_calendar_obj calendar;          // RTC时间结构体全局变量（年月日时分秒星期）


/****
*******RTC中断NVIC配置
*****/
static void RTC_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;          // RTC全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  // 抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;      // 子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // 使能RTC中断
	NVIC_Init(&NVIC_InitStructure);
}

/****
*******判断是否是闰年
输入: 年份
输出: 1=是闰年, 0=不是闰年
*****/
uint8_t Is_Leap_Year(uint16_t year)
{
	if(year%4==0)                  // 必须能被4整除
	{
		if(year%100==0)
		{
			if(year%400==0)
				return 1;         // 能被400整除才是闰年（如2000年）
			else
				return 0;         // 能被100整除但不能被400整除不是（如1900年）
		}
		else
			return 1;             // 能被4整除但不能被100整除是闰年
	}
	else
		return 0;                 // 不能被4整除不是闰年
}

/****
*******设置RTC时钟
把输入的年月日时分秒转换为从1970年1月1日起的总秒数
以1970年1月1日为基准（Unix时间戳）
1970~2099年为合法年份
返回值: 0=成功, 其他=错误代码
*****/
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};  // 月份修正数据表（用于计算星期几）

const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};  // 平年各月份天数表

uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;

	if(syear<1970||syear>2099)     // 年份合法性检查
		return 1;

	for(t=1970;t<syear;t++)       // 累加所有年份的秒数
	{
		if(Is_Leap_Year(t))        // 闰年366天
			seccount+=31622400;    // 366×86400
		else                       // 平年365天
			seccount+=31536000;    // 365×86400
	}
	smon-=1;                       // 月份从0开始
	for(t=0;t<smon;t++)           // 累加前面月份的秒数
	{
		seccount+=(uint32_t)mon_table[t]*86400;
		if(Is_Leap_Year(syear)&&t==1)  // 闰年2月多一天
			seccount+=86400;
	}
	seccount+=(uint32_t)(sday-1)*86400;  // 累加天的秒数
	seccount+=(uint32_t)hour*3600;       // 累加小时的秒数
	seccount+=(uint32_t)min*60;          // 累加分钟的秒数
	seccount+=sec;                       // 加上秒数

	RTC_SetCounter(seccount);     // 写入RTC计数器（从1970年起算的总秒数）

	RTC_WaitForLastTask();        // 等待RTC写入完成
	return 0;
}

/****
*******初始化RTC闹钟（当前项目未使用闹钟功能）
*****/
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)
		return 1;
	for(t=1970;t<syear;t++)
	{
		if(Is_Leap_Year(t))
			seccount+=31622400;
		else
			seccount+=31536000;
	}
	smon-=1;
	for(t=0;t<smon;t++)
	{
		seccount+=(uint32_t)mon_table[t]*86400;
		if(Is_Leap_Year(syear)&&t==1)
			seccount+=86400;
	}
	seccount+=(uint32_t)(sday-1)*86400;
	seccount+=(uint32_t)hour*3600;
	seccount+=(uint32_t)min*60;
	seccount+=sec;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  // 使能PWR和BKP时钟
	PWR_BackupAccessCmd(ENABLE);  // 使能后备寄存器访问

	RTC_SetAlarm(seccount);       // 设置闹钟时间

	RTC_WaitForLastTask();

	return 0;
}

/****
*******根据公历日期计算星期几
输入: 年月日（1901-2099年）
返回值: 星期号（0=周日, 1=周一, ..., 6=周六）
*****/
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{
	uint16_t temp2;
	uint8_t yearH,yearL;

	yearH=year/100;               // 世纪数
	yearL=year%100;               // 年份后两位

	if (yearH>19)
		yearL+=100;                // 21世纪则加100

	temp2=yearL+yearL/4;          // 基姆拉尔森算法
	temp2=temp2%7;
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)
		temp2--;
	return(temp2%7);
}

/****
*******从RTC计数器读取当前时间并更新calendar结构体
返回值: 0=成功
*****/
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;     // 上次读取的天数（用于检测日期变化）
	uint32_t timecount=0;
	uint32_t temp=0;
	uint16_t temp1=0;
	timecount=RTC_GetCounter();   // 读取RTC秒计数器
	temp=timecount/86400;         // 转换为天数
	if(daycnt!=temp)              // 日期发生了变化（跨日）
	{
		daycnt=temp;
		temp1=1970;                // 从1970年开始计算年份
		while(temp>=365)           // 逐年减去天数
		{
			if(Is_Leap_Year(temp1))  // 闰年
			{
				if(temp>=366)
					temp-=366;
				else
				{
					temp1++;
					break;
				}
			}
			else                   // 平年
				temp-=365;
			temp1++;
		}
		calendar.w_year=temp1;    // 得到年份
		temp1=0;
		while(temp>=28)           // 逐月减去天数
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)  // 闰年2月
			{
				if(temp>=29)
					temp-=29;
				else
					break;
			}
			else
			{
				if(temp>=mon_table[temp1])
					temp-=mon_table[temp1];
				else
					break;
			}
			temp1++;
		}
		calendar.w_month=temp1+1;  // 得到月份（0起始→+1）
		calendar.w_date=temp+1;    // 得到日期
	}
	temp=timecount%86400;         // 当天剩余秒数
	calendar.hour=temp/3600;      // 提取小时
	calendar.min=(temp%3600)/60;  // 提取分钟
	calendar.sec=(temp%3600)%60;  // 提取秒
	calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);  // 计算星期几

	return 0;
}

/****
*******RTC初始化函数
BKP->DR1用于判断是否第一次配置：
- 如果BKP_DR1 != 0xA5A5：首次上电，初始化RTC时钟+写入默认时间
- 如果BKP_DR1 == 0xA5A5：非首次上电，恢复计时（RTC由VBAT供电持续走时）
返回值：1=初始化失败(晶振问题), 0=初始化成功
*****/
uint8_t RTC_Init(void)
{
	uint8_t temp=0;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  // 使能PWR和BKP时钟
	PWR_BackupAccessCmd(ENABLE);  // 使能后备寄存器访问（才能读写BKP和RTC）

    if(BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)  // 首次上电（标志不匹配）
	{
		// 启动内部低速晶振LSI（约40kHz，精度不如LSE但无需外部晶振）
		RCC_LSICmd(ENABLE);
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET&&temp<250)  // 等待LSI就绪
		{
			temp++;
			delay_ms(10);
		}
		if(temp>=250)return 1;     // 超时，晶振启动失败
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);  // 选择LSI作为RTC时钟源
		RCC_RTCCLKCmd(ENABLE);     // 使能RTC时钟
        RTC_WaitForSynchro();      // 等待RTC寄存器同步
		RTC_WaitForLastTask();
		RTC_ITConfig(RTC_IT_SEC, ENABLE);  // 使能RTC秒中断（每秒触发一次）
		RTC_WaitForLastTask();
		RTC_EnterConfigMode();      // 进入RTC配置模式
		RTC_SetPrescaler(40000 - 1);  // 设置预分频（LSI约40kHz，分频后1秒触发一次）
		RTC_WaitForLastTask();
		RTC_Set(2025,1,1,12,00,0);  // 设置默认时间：2025年1月1日12:00:00
		RTC_ExitConfigMode();       // 退出配置模式
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);  // 写入标志（下次上电不再重复初始化）
	}
	else                           // 非首次上电（RTC一直在走时）
	{
        RCC_LSICmd(ENABLE);       // 确保LSI启动
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
        RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_SEC, ENABLE);  // 使能秒中断
		RTC_WaitForLastTask();
	}
	RTC_NVIC_Config();            // 配置RTC中断优先级
	RTC_Get();                    // 立即读取一次时间

	return 0;
}

/****
*******RTC秒中断服务函数（每秒触发一次）
在stm32f10x_it.c中调用
*****/
void RTC_IRQHandler(void)
{
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)  // 秒中断
	{
        if (rtc_get_flag)      // 如果允许更新时间（设置页修改期间暂停）
            RTC_Get();          // 更新calendar结构体（年月日时分秒）
	}
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)  // 闹钟中断
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);  // 清除闹钟中断标志
		RTC_Get();
	}
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);  // 清除秒中断和溢出中断标志
	RTC_WaitForLastTask();
}
