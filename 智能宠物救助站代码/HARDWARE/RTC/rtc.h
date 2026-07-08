#ifndef __RTC_H_
#define __RTC_H_


/**********************************
包含头文件
**********************************/
//#include "sys.h"
#include "delay.h"

/**********************************
时间结构体
**********************************/
typedef struct
{
	vu8 hour;    // 时
	vu8 min;     // 分
	vu8 sec;     // 秒
	vu16 w_year; // 年
	vu8  w_month;// 月
	vu8  w_date; // 日
	vu8  week;   // 星期
}_calendar_obj; // 日历时间结构体

extern _calendar_obj calendar;					//日历结构体，供其他文件调用


/**********************************
函数声明
**********************************/
uint8_t RTC_Init(void);        					//RTC初始化函数
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec); // 设置RTC时间

#endif
