#ifndef	__MODULES_H_
#define __MODULES_H_

#include "stm32f10x.h"          // STM32标准外设库
#include "dht11.h"              // DHT11温湿度传感器
#include "adcx.h"               // ADC采集
#include "rtc.h"                // 实时时钟
#include "hx711.h"              // HX711称重传感器
#include "HW.h"                 // 硬件GPIO抽象层
#include "water.h"              // 水位传感器

/**
  * @brief  传感器数据结构体（存储所有传感器实时读数）
  */
typedef struct
{
	uint8_t humi;                // 环境湿度（单位%，DHT11量程20-90%）
	uint8_t temp;                // 环境温度（单位℃，DHT11量程0-50℃）
	float weight;                // 食物重量（单位g，HX711测量值×标定系数）
    _calendar_obj calendarData;  // RTC时间数据（年月日时分秒星期）
    uint16_t water;              // 水位百分比（0-100%，ADC映射值）
}SensorModules;

/**
  * @brief  定时喂食项结构体（一个定时任务的配置）
  */
typedef struct
{
	_calendar_obj time;           // 定时触发时间（时分）
	uint16_t feed_weight;         // 该定时的喂食量（单位g）
	uint8_t enabled;              // 是否启用: 1=启用, 0=禁用
}ScheduleItem;

/**
  * @brief  传感器阈值结构体（所有可配置的参数）
  */
typedef struct
{
	float weight_Value;           // 手动喂食称重阈值（单位g，达到此重量停止喂食）
	uint16_t water_Value;         // 水位阈值（百分比0-100，低于此值自动补水）
	ScheduleItem schedule[3];     // 3组定时喂食配置（定时1/2/3）
}SensorThresholdValue;

/**
  * @brief  驱动器状态结构体（所有执行器/输出的当前状态）
  */
typedef struct
{
	uint8_t Food_Flag;            // 喂食请求标志（1=需要喂食，由定时或手动设置）
	uint8_t Now_Food_Flag;        // 正在喂食标志（1=电机正在转动）
    uint8_t Time_Food_Flag;      // 当前触发的定时编号(1-3)，0表示非定时触发
    uint8_t Time_Feed_Target;    // 当前定时触发的喂食量目标(g)
	uint8_t Water_Flag;           // 水泵开启标志（1=水泵开启）
	uint8_t Humin_Flag;           // 宠物检测标志（1=红外检测到宠物在场）
    uint8_t Beep_Flag;           // 蜂鸣器标志（1=嘀嘀嘀提醒）
}DriveModules;

extern SensorModules sensorData;           // 声明传感器数据全局变量
extern SensorThresholdValue Sensorthreshold; // 声明阈值参数全局变量
extern DriveModules driveData;             // 声明驱动器状态全局变量
void SensorScan(void);                     // 传感器扫描函数声明

#endif
