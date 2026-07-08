#include "stm32f10x.h"       // STM32标准外设库头文件
#include <string.h>           // 字符串处理函数库
#include "led.h"              // LED指示灯驱动头文件
#include "beep.h"             // 蜂鸣器驱动头文件
#include "usart.h"            // 串口1驱动头文件（调试/ESP8266通信）
#include "usart2.h"           // 串口2驱动头文件（ESP8266数据传输）
#include "usart3.h"           // 串口3驱动头文件（语音模块通信）
#include "delay.h"            // 延时函数头文件（us/ms级）
#include "dht11.h"            // DHT11温湿度传感器驱动头文件
#include "lcd.h"              // TFT LCD显示屏驱动头文件（ST7735）
#include "key.h"              // 按键输入驱动头文件
#include "Modules.h"          // 传感器/驱动模块结构体定义
#include "TIM2.h"             // 定时器2配置头文件（步进电机中断驱动）
#include "adcx.h"             // ADC采集头文件（水位传感器）
#include "flash.h"            // Flash读写头文件（掉电保存参数）
#include "stepmotor.h"        // 步进电机驱动头文件（28BYJ-48推料器）
#include "bump.h"             // 水泵头文件
#include "HW.h"               // 光电红外传感器头文件
#include "hx711.h"            // HX711称重传感器驱动头文件（24位ADC）
#include "water.h"            // 水位传感器驱动头文件
#include "esp8266.h"          // ESP8266 WiFi模块驱动头文件
#include "relay.h"            // 继电器驱动头文件（控制水泵）
#include "rtc.h"              // 实时时钟驱动头文件
#include "iwdg.h"             // 独立看门狗驱动头文件

/*********************************
											STM32

 * 项目			:	智能宠物救助站
 * 版本			: V2.0
 * 日期			: 2026.4
 * MCU			:	STM32F103C8T6
 * 作者     ：电子222林湘龙


*******************************************/

#define KEY_Long1	11          // 按键定义：KEY_长按1 = 11（进入设置页）

#define KEY_1	1               // 按键1：切换模式（自动/手动/设置）
#define KEY_2	2               // 按键2：切换光标/选项
#define KEY_3	3               // 按键3：开启/增加
#define KEY_4	4               // 按键4：关闭/减少

#define FLASH_START_ADDR	0x0800F000	// Flash写入起始地址（C8T6最后一页, 64KB范围内）
extern uint8_t rtc_get_flag;    // 外部声明：RTC时间获取标志（rtc.c中定义）

SensorModules sensorData;								// 传感器数据结构体（温度/湿度/重量/水位/时间）
SensorThresholdValue Sensorthreshold;		// 传感器阈值结构体（定时/喂食量/水位阈值）
DriveModules driveData;									// 驱动器状态结构体（喂食/水泵/检测/蜂鸣）

uint8_t mode = 1;	            // 系统模式：1=自动, 2=手动, 3=设置
int32_t reset;                  // HX711初始数据（去皮用）
uint8_t location = 1;           // 设置页面时间光标位置（1=时,2=分,3=秒,4~9=定时）
static uint8_t count_w = 1;     // 设置页面当前光标位置（1~6对应6个设置项）
uint8_t set_flag = 1;           // 进入设置页的初始化标志（首次进入时=1）
uint8_t ss_flag = 1;            // 蜂鸣器翻转标志（实现嘀嘀嘀声）
uint8_t count = 0;              // 蜂鸣器计数器（控制嘀嘀嘀次数）
uint8_t date;                   // 记录当前日期（用于跨日重置定时标志）
float Pi_weight;                // HX711去皮后的初始重量（皮重）
uint8_t food_flag_1 = 0;        // 定时1触发标志（0=未触发,1=已触发）
uint8_t food_flag_2 = 0;        // 定时2触发标志（0=未触发,1=已触发）
uint8_t food_flag_3 = 0;        // 定时3触发标志（0=未触发,1=已触发）
uint8_t bobao_flag = 1;         // 播报标志（每次开始喂食时播报一次）

uint32_t time_num = 1;          // 主循环计时变量（控制数据上报频率）
uint8_t dht11_read_tick = 0;    // DHT11读取计时（主循环计数，每200次读一次）
uint8_t last_status_state = 0;  // 上一次的状态（0=待机,1=喂食,2=加水,3=喂食加水）
uint16_t status_blink_cnt = 0;  // 状态闪烁计数器（控制工作状态动画）

// 增量称重判定：记录喂食启动瞬间的初始重量
static float start_weight = 0;

// 防卡粮自动反转：正转5圈 → 反转1圈 → 循环，无停顿
#define FEED_FORWARD_ANGLE  1800   // 正转5圈 (5 * 360度)
#define FEED_REVERSE_ANGLE  360    // 反转1圈
static uint8_t anti_jam_state = 0;  // 防卡状态：0=正转中, 1=反转中

_calendar_obj set_time;         // 设置页面的时间副本（修改后再写入RTC）


//系统静态变量
static uint8_t count_m = 1;     // 手动模式光标位置（1=水泵,2=喂食）
static uint8_t count_s = 1;     // 设置模式光标位置（1~6）

void Public(void);              // 函数声明：构造并发送数据帧给APP
void Flag_Writer(void);         // 函数声明：保存所有阈值参数到Flash

/**
  * @brief  系统模式枚举
  */
enum
{
	AUTO_MODE = 1,               // 自动模式（定时喂食+自动补水）
	MANUAL_MODE,                 // 手动模式（APP/按键直接控制）
	SETTINGS_MODE                // 设置模式（调整时间和阈值）

}MODE_PAGES;

/**
  * @brief  自动模式主页面（合并为单页显示）
  * @param  无
  * @retval 无
  * 显示内容：时间(顶部居中) + 温度 + 湿度 + 质量 + 水位
  */
void OLED_autoPage1(void)		//自动模式菜单（合并为单页）
{
	//显示固定标签
	OLED_ShowChinese(24,28,0,16,1); 	//温
	OLED_ShowChinese(40,28,2,16,1);	//度
	OLED_ShowChar(56,28,':',16,1);

	OLED_ShowChinese(24,52,1,16,1);	//湿
	OLED_ShowChinese(40,52,2,16,1);	//度
	OLED_ShowChar(56,52,':',16,1);

	OLED_ShowChinese(24,76,5,16,1);	//质
	OLED_ShowChinese(40,76,6,16,1);//量
	OLED_ShowChar(56,76,':',16,1);

	OLED_ShowChinese(24,100,9,16,1);	//水
	OLED_ShowChinese(40,100,13,16,1);//位
	OLED_ShowChar(56,100,':',16,1);

	OLED_Refresh();               // 刷新TFT屏幕显示
}

/**
  * @brief  自动模式传感器数据动态显示
  * @param  无
  * @retval 无
  * 温度/湿度=绿色, 水位低于阈值=红色警告, 质量=白色
  */
void SensorDataDisplay1(void)		//传感器数据显示
{
    char display_buf[48];
    u16 water_color;            // 水位显示颜色变量

    // 显示当前时间 HH:MM:SS
    sprintf(display_buf,"%02d:%02d:%02d",sensorData.calendarData.hour,sensorData.calendarData.min,sensorData.calendarData.sec);
    OLED_ShowString(32,0,(uint8_t*)display_buf,16,0);

	//显示温度数据（正常=绿色）
	OLED_ShowNum_Color(80,28,sensorData.temp,2,16,0,GREEN);
	//显示湿度数据（正常=绿色）
	OLED_ShowNum_Color(80,52,sensorData.humi,2,16,0,GREEN);
    //显示质量数据（正常=白色）
	OLED_ShowNum_Color(64,76,sensorData.weight,4,16,0,WHITE);

	//显示水位数据：低于阈值=红色警告，正常=绿色
	if (sensorData.water < Sensorthreshold.water_Value)
	    water_color = RED;       // 水位低于阈值，显示红色
	else
	    water_color = GREEN;     // 水位正常，显示绿色
    OLED_ShowNum_Color(72,100,sensorData.water,3,16,0,water_color);

    OLED_Refresh();             // 刷新屏幕
}

/**
  * @brief  WiFi/APP按键命令解析函数
  * @param  无
  * @retval 无
  * 将APP发来的命令转换为本地按键值，并处理设置类命令
  * 命令集：KA=KEY1(模式) KB=KEY2(光标) KC=KEY3(开) KD=KEY4(关) KE=KEY长按1(设置)
  * 设置类：SWxxx=水位阈值 SFxxxx=称重阈值 STnHHMM=定时时间 SDnXXXX=喂食量 SEn=启用
  */
void WiFi_Key(void)
{
    // APP虚拟按键：将WiFi收到的命令映射为本地按键值
    if (BlueCmd("KA",20))       // 收到KA → 模拟按下KEY1（切换模式）
    {
        KeyNum = KEY_1;
    }
    if (BlueCmd("KB",20))       // 收到KB → 模拟按下KEY2（切换光标）
    {
        KeyNum = KEY_2;
    }
    if (BlueCmd("KC",20))       // 收到KC → 模拟按下KEY3（开启/增加）
    {
            KeyNum = KEY_3;
    }
		if (BlueCmd("KD",20))     // 收到KD → 模拟按下KEY4（关闭/减少）
    {
        KeyNum = KEY_4;
    }
		if (BlueCmd("KE",20))     // 收到KE → 模拟KEY长按1（进入设置，仅自动模式有效）
    {
        if (mode == AUTO_MODE)
        {
            KeyNum = KEY_Long1;
        }
    }
		// APP设置水位阈值: SW+3位数字，如SW050表示水位阈值50
		{
			char *sw = strstr((const char *)Res, "SW");   // 在接收缓冲区查找"SW"
			if (sw && sw[2]>='0' && sw[2]<='9' && sw[3]>='0' && sw[3]<='9' && sw[4]>='0' && sw[4]<='9')
			{
				int val = (sw[2]-'0')*100 + (sw[3]-'0')*10 + (sw[4]-'0');  // 解析3位数字
				if (val >= 0 && val <= 100)   // 水位阈值范围0~100%
				{
					Sensorthreshold.water_Value = val;   // 更新水位阈值
					Flag_Writer();              // 保存到Flash
				}
			}
		}
		// APP设置称重阈值: SF+4位数字，如SF0200表示称重阈值200g
		{
			char *sf = strstr((const char *)Res, "SF");   // 查找"SF"
			if (sf)
			{
				int val = (sf[2]-'0')*1000 + (sf[3]-'0')*100 + (sf[4]-'0')*10 + (sf[5]-'0');  // 解析4位
				if (val >= 0 && val <= 9999)  // 称重阈值范围0~9999g
				{
					Sensorthreshold.weight_Value = val;  // 更新称重阈值
					Flag_Writer();              // 保存到Flash
				}
			}
		}
		// APP设置定时时间: STnHHMM (n=1/2/3, HH=小时, MM=分钟)
		{
			char *st = strstr((const char *)Res, "ST");   // 查找"ST"
			if (st && st[2] >= '1' && st[2] <= '3')      // 定时编号1~3
			{
				int idx = st[2] - '1';                    // 转换为数组索引0/1/2
				int h = (st[3]-'0')*10 + (st[4]-'0');     // 解析小时
				int m = (st[5]-'0')*10 + (st[6]-'0');     // 解析分钟
				if (h >= 0 && h <= 23 && m >= 0 && m <= 59)  // 时间合法性检查
				{
					Sensorthreshold.schedule[idx].time.hour = h;   // 保存小时
					Sensorthreshold.schedule[idx].time.min = m;    // 保存分钟
					food_flag_1 = 0; food_flag_2 = 0; food_flag_3 = 0;  // 重置所有定时标志
					Flag_Writer();              // 保存到Flash
				}
			}
		}
		// APP设置各定时喂食量: SDnXXXX (n=1/2/3, XXXX=克数0001-9999)
		{
			char *sd = strstr((const char *)Res, "SD");   // 查找"SD"
			if (sd && sd[2] >= '1' && sd[2] <= '3')
			{
				int idx = sd[2] - '1';                    // 定时索引
				int val = (sd[3]-'0')*1000 + (sd[4]-'0')*100 + (sd[5]-'0')*10 + (sd[6]-'0');
				if (val >= 1 && val <= 9999)  // 喂食量范围1~9999g
				{
					Sensorthreshold.schedule[idx].feed_weight = val;  // 保存喂食量
					Flag_Writer();              // 保存到Flash
				}
			}
		}
		// APP设置各定时启用/禁用: SEn (n=1/2/3, 后跟0或1)
		{
			char *se = strstr((const char *)Res, "SE");   // 查找"SE"
			if (se && se[2] >= '1' && se[2] <= '3' && se[3] >= '0' && se[3] <= '1')
			{
				int idx = se[2] - '1';                              // 定时索引
				Sensorthreshold.schedule[idx].enabled = se[3] - '0'; // 0=禁用,1=启用
				food_flag_1 = 0; food_flag_2 = 0; food_flag_3 = 0;  // 重置定时标志
				Flag_Writer();              // 保存到Flash
			}
		}
		// APP时间同步: TIMEHHMMSS (HHMMSS格式，连接后自动发送)
		{
			char *tm = strstr((const char *)Res, "TIME");  // 查找"TIME"
			if (tm && tm[4]>='0' && tm[4]<='9')
			{
				int hh = (tm[4]-'0')*10 + (tm[5]-'0');    // 解析小时
				int mm = (tm[6]-'0')*10 + (tm[7]-'0');    // 解析分钟
				int ss = (tm[8]-'0')*10 + (tm[9]-'0');    // 解析秒
				if (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59 && ss >= 0 && ss <= 59)
				{
					set_time.hour = hh;         // 更新设置页时间副本
					set_time.min = mm;
					set_time.sec = ss;
					RTC_Set(sensorData.calendarData.w_year, sensorData.calendarData.w_month, sensorData.calendarData.w_date, hh, mm, ss);  // 写入RTC
					sensorData.calendarData.hour = hh;     // 同步到运行时数据
					sensorData.calendarData.min = mm;
					sensorData.calendarData.sec = ss;
					printf("\r\n[INFO] Time synced: %02d:%02d:%02d\r\n", hh, mm, ss);
				}
			}
		}

		// APP WiFi配网: WIFI:ssid,password 格式（STA模式用，当前默认AP模式不使用）
		{
			char *wf = strstr((const char *)Res, "WIFI:");  // 查找"WIFI:"
			if (wf)
			{
				char *ssid_start = wf + 5;  // 跳过 "WIFI:"
				char *comma = strchr(ssid_start, ',');       // 查找逗号分隔符
				if (comma)
				{
					char wifi_ssid[64] = {0};
					char wifi_pwd[64] = {0};
					int ssid_len = comma - ssid_start;
					if (ssid_len > 0 && ssid_len < 64)
					{
						strncpy(wifi_ssid, ssid_start, ssid_len);  // 提取WiFi名称
						strcpy(wifi_pwd, comma + 1);               // 提取WiFi密码

						// 屏幕显示正在连接WiFi
						LCD_Fill(0, 80, 128, 160, BLACK);
						OLED_ShowString(0, 80, (uint8_t*)"Connecting...", 16, 0);
						OLED_Refresh();

						// 发送AT+CWJAP连接目标WiFi（STA模式）
						if (ESP8266_JoinAP(wifi_ssid, wifi_pwd))
						{
							// 连接成功，获取STA IP并回传给APP
							char ip_buf[20] = {0};
							ESP8266_Cmd("AT+CIFSR", "OK", 0, 2000);  // 查询IP地址

							// 从CIFSR响应中提取STAIP
							char *staip = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STAIP,\"");
							if (staip)
							{
								staip += 7;  // 跳过 STAIP,"
								int i = 0;
								while (staip[i] != '\"' && staip[i] != '\0' && i < 19)
								{
									ip_buf[i] = staip[i];
									i++;
								}
								ip_buf[i] = '\0';
							}

							// 通过ESP8266回传WiFi连接结果给APP
							char result[80] = {0};
							sprintf(result, "WIFI_OK:%s", ip_buf);
							ESP8266_SendData((unsigned char *)result, strlen(result));

							// 屏幕显示连接成功
							LCD_Fill(0, 80, 128, 160, BLACK);
							OLED_ShowString(0, 80, (uint8_t*)"WiFi OK", 16, 0);
							OLED_ShowString(0, 96, (uint8_t*)ip_buf, 16, 0);
							OLED_Refresh();
						}
						else
						{
							// 连接失败，回传错误
							ESP8266_SendData((unsigned char *)"WIFI_FAIL", 9);

							LCD_Fill(0, 80, 128, 160, BLACK);
							OLED_ShowString(0, 80, (uint8_t*)"WiFi FAIL", 16, 0);
							OLED_Refresh();
						}
					}
				}
			}
		}
}

/**
  * @brief  手动模式界面固定标签显示
  * @param  无
  * @retval 无
  * 显示：水泵(开/关) + 喂食(开/关) + 水位(数值) + 重量(数值) + 状态文字
  */
void OLED_manualPage1(void)
{
	//显示"水泵"
	OLED_ShowChinese(16,20,9,16,0);    // 水
	OLED_ShowChinese(32,20,10,16,0);   // 泵
	OLED_ShowChar(64,20,':',16,0);

	//显示"喂食"
	OLED_ShowChinese(16,40,14,16,0);   // 喂
	OLED_ShowChinese(32,40,15,16,0);   // 食
	OLED_ShowChar(64,40,':',16,0);

	// 显示"水位:"
	OLED_ShowChinese(16,66,9,16,0);   // 水 (index 9)
	OLED_ShowChinese(32,66,13,16,0);  // 位 (index 13)
	OLED_ShowChar(48,66,':',16,0);

	// 显示"重量:"
	OLED_ShowChinese(16,90,28,16,0);  // 重 (index 28)
	OLED_ShowChinese(32,90,29,16,0);  // 量 (index 29)
	OLED_ShowChar(48,90,':',16,0);

	// 状态行：由 ManualSettingsDisplay1 动态填充（待机中/喂食中/加水中/喂食加水中）

    OLED_Refresh();               // 刷新屏幕
}

/**
  * @brief  手动模式动态数据显示（开/关状态 + 重量 + 水位 + 工作状态）
  * @param  无
  * @retval 无
  * 状态文字用黄色显示，开/关用绿色/白色，水位低于阈值红色警告
  */
void ManualSettingsDisplay1(void)
{
	// 显示时间（顶部，与首页相同位置）
	char display_buf[48];
	u16 water_color;
	sprintf(display_buf,"%02d:%02d:%02d",sensorData.calendarData.hour,sensorData.calendarData.min,sensorData.calendarData.sec);
	OLED_ShowString(32, 0, (uint8_t*)display_buf, 16, 0);

	if(driveData.Water_Flag)        // 水泵状态
	{
		OLED_ShowChinese_Color(96,20,11,16,0,GREEN); 	//开(绿色)
	}
	else
	{
		OLED_ShowChinese(96,20,12,16,0); 	//关(白色)
	}

	if(driveData.Food_Flag)         // 喂食状态
	{
		OLED_ShowChinese_Color(96,40,11,16,0,GREEN); 	//开(绿色)
	}
	else
	{
		OLED_ShowChinese(96,40,12,16,0); 	//关(白色)
	}

	// 显示当前重量数值（动态更新，正常=白色）
	OLED_ShowNum_Color(88, 90, (uint16_t)sensorData.weight, 4, 16, 0, WHITE);

	// 显示当前水位数值：低于阈值=红色警告，正常=绿色
	if (sensorData.water < Sensorthreshold.water_Value)
	    water_color = RED;           // 水位低→红色
	else
	    water_color = GREEN;         // 水位正常→绿色
	OLED_ShowNum_Color(88, 66, (uint16_t)sensorData.water, 3, 16, 0, water_color);

	// 显示综合状态（4种状态），文字用黄色
	uint8_t new_state;
	if (driveData.Now_Food_Flag && driveData.Water_Flag)
		new_state = 3;  // 喂食加水中
	else if (driveData.Now_Food_Flag)
		new_state = 1;  // 喂食中
	else if (driveData.Water_Flag)
		new_state = 2;  // 加水中
	else
		new_state = 0;  // 待机中

	// 状态切换时清屏+重置闪烁计数
	if (new_state != last_status_state)
	{
		LCD_Fill(0, 122, 128, 138, BLACK);
		last_status_state = new_state;
		status_blink_cnt = 0;
	}

	// 常亮显示状态文字（工作状态=黄色，待机=白色）
	if (new_state == 3)             // 喂食加水中
	{
		OLED_ShowChinese_Color(16, 122, 14,16,0,YELLOW);  // 喂
		OLED_ShowChinese_Color(32, 122, 15,16,0,YELLOW);  // 食
		OLED_ShowChinese_Color(48, 122, 34,16,0,YELLOW);  // 加
		OLED_ShowChinese_Color(64, 122, 9,16,0,YELLOW);   // 水
		OLED_ShowChinese_Color(80, 122, 35,16,0,YELLOW);  // 中
	}
	else if (new_state == 1)        // 喂食中
	{
		OLED_ShowChinese_Color(32, 122, 14,16,0,YELLOW);  // 喂
		OLED_ShowChinese_Color(48, 122, 15,16,0,YELLOW);  // 食
		OLED_ShowChinese_Color(64, 122, 35,16,0,YELLOW);  // 中
	}
	else if (new_state == 2)        // 加水中
	{
		OLED_ShowChinese_Color(32, 122, 34,16,0,YELLOW);  // 加
		OLED_ShowChinese_Color(48, 122, 9,16,0,YELLOW);   // 水
		OLED_ShowChinese_Color(64, 122, 35,16,0,YELLOW);  // 中
	}
	else                            // 待机中
	{
		OLED_ShowChinese(32, 122, 36,16,0);  // 待
		OLED_ShowChinese(48, 122, 37,16,0);  // 机
		OLED_ShowChinese(64, 122, 35,16,0);  // 中
	}

	// 3个闪烁点（仅工作状态闪烁，待机不显示）
	if (new_state > 0)
	{
		status_blink_cnt++;
		u16 dot_x, dot_y = 127, dot_size = 6;    // 闪烁点位置和大小
		// 计算点的起始x：跟在状态文字后面
		if (new_state == 3)
			dot_x = 96;   // 5字后
		else
			dot_x = 80;   // 3字后

		if (status_blink_cnt % 5 < 3)  // 约0.5秒闪1次（亮3暗2）
		{
			LCD_Fill(dot_x, dot_y, dot_x+dot_size, dot_y+dot_size, YELLOW);      // 点1亮
			LCD_Fill(dot_x+10, dot_y, dot_x+10+dot_size, dot_y+dot_size, YELLOW); // 点2亮
			LCD_Fill(dot_x+20, dot_y, dot_x+20+dot_size, dot_y+dot_size, YELLOW); // 点3亮
		}
		else
		{
			LCD_Fill(dot_x, dot_y, dot_x+dot_size, dot_y+dot_size, BLACK);        // 点1灭
			LCD_Fill(dot_x+10, dot_y, dot_x+10+dot_size, dot_y+dot_size, BLACK);   // 点2灭
			LCD_Fill(dot_x+20, dot_y, dot_x+20+dot_size, dot_y+dot_size, BLACK);   // 点3灭
		}
	}

	OLED_Refresh();                 // 刷新屏幕
}

/**
  * @brief  设置页面固定标签显示（合并单页，6个设置项）
  * @param  无
  * @retval 无
  * 布局：时间 + 定时1/2/3 + 食物阈值 + 水位阈值
  */
void OLED_settingsPage1(void)
{
    // === 第一区域：时间设置 ===
    // 显示"时间"
	OLED_ShowChinese(16,0,18,16,1);   // 时
	OLED_ShowChinese(32,0,16,16,1);   // 间
	OLED_ShowChar(48,0,':',16,1);

    // === 第二区域：定时设置 ===
	OLED_ShowChinese(16,22,17,16,1);  // 定
	OLED_ShowChinese(32,22,18,16,1);  // 时
    OLED_ShowChar(48, 22, '1', 16, 1);  // 编号1
	OLED_ShowChar(64,22,':',16,1);

    OLED_ShowChinese(16,42,17,16,1);  // 定
	OLED_ShowChinese(32,42,18,16,1);  // 时
    OLED_ShowChar(48, 42, '2', 16, 1);  // 编号2
	OLED_ShowChar(64,42,':',16,1);

    OLED_ShowChinese(16,62,17,16,1);  // 定
	OLED_ShowChinese(32,62,18,16,1);  // 时
    OLED_ShowChar(48, 62, '3', 16, 1);  // 编号3
	OLED_ShowChar(64,62,':',16,1);

    // === 第三区域：阈值设置 ===
    // 显示"食物阈值"
    OLED_ShowChinese(16,85,15,16,1);  // 食
	OLED_ShowChinese(32,85,19,16,1);  // 物
	OLED_ShowChinese(48,85,7,16,1);   // 阈
	OLED_ShowChinese(64,85,8,16,1);   // 值
    OLED_ShowChar(80,85,':',16,1);

	// 显示"水位阈值"
    OLED_ShowChinese(16,105,9,16,1);   // 水
	OLED_ShowChinese(32,105,13,16,1);  // 位
	OLED_ShowChinese(48,105,7,16,1);   // 阈
	OLED_ShowChinese(64,105,8,16,1);   // 值
    OLED_ShowChar(80,105,':',16,1);

	OLED_Refresh();                 // 刷新屏幕
}

/**
  * @brief  设置页面动态数据显示（时间+定时时间+阈值数值）
  * @param  无
  * @retval 无
  */
void SettingsThresholdDisplay1(void)
{
	uint8_t display_buf[48];
    // 显示当前设置时间
    sprintf((char*)display_buf,"%02d:%02d:%02d",set_time.hour,set_time.min,set_time.sec);
    OLED_ShowString(64, 0, display_buf, 16, 0);
    // 显示定时1时间
    sprintf((char*)display_buf,"%02d:%02d",Sensorthreshold.schedule[0].time.hour, Sensorthreshold.schedule[0].time.min);
    OLED_ShowString(88, 22, display_buf, 16, 0);

    // 显示定时2时间
    sprintf((char*)display_buf,"%02d:%02d",Sensorthreshold.schedule[1].time.hour, Sensorthreshold.schedule[1].time.min);
    OLED_ShowString(88, 42, display_buf, 16, 0);

    // 显示定时3时间
    sprintf((char*)display_buf,"%02d:%02d",Sensorthreshold.schedule[2].time.hour, Sensorthreshold.schedule[2].time.min);
    OLED_ShowString(88, 62, display_buf, 16, 0);

    // 显示喂食阈值数值（单位g）
	OLED_ShowNum(88, 85, Sensorthreshold.weight_Value, 4, 16, 0);
	// 显示水位阈值数值（单位%）
	OLED_ShowNum(88, 105, Sensorthreshold.water_Value, 4, 16, 0);
	OLED_Refresh();
}

/**
  * @brief  手动模式光标切换（KEY2切换水泵/喂食光标）
  * @param  无
  * @retval 返回光标位置（1=水泵,2=喂食）
  */
uint8_t SetManual(void)
{
	if(KeyNum == KEY_2)             // 检测KEY2按下
	{
		KeyNum = 0;                  // 清除按键值
		count_m++;
		if (count_m > 2)             // 一共可控制2个外设（水泵+喂食）
		{
			OLED_Clear();
			count_m = 1;             // 循环回第一个
		}
	}
	return count_m;
}

/**
  * @brief  设置模式光标切换（KEY2在6个设置项间循环）
  * @param  无
  * @retval 返回光标位置（1~6）
  */
uint8_t SetSelection(void)
{
	if(KeyNum == KEY_2)             // 检测KEY2按下
	{
		KeyNum = 0;
		count_s++;
		if (count_s == 5)            // 从第4项跳到第5项时清屏
		{
			OLED_Clear();
		}
		else if (count_s > 6)        // 超过第6项则回到第1项
		{
			OLED_Clear();
			count_s = 1;
		}
	}
	return count_s;
}


/**
  * @brief  手动模式光标指示符显示（> 符号）
  * @param  num 光标位置：1=水泵,2=喂食
  * @retval 无
  */
void OLED_manualOption(uint8_t num)
{
	switch(num)
	{
		case 1:	                    // 光标在"水泵"行
			OLED_ShowChar(0, 20,'>',16,0);  // 显示>
			OLED_ShowChar(0,40,' ',16,0);   // 清除另一个
			break;
		case 2:	                    // 光标在"喂食"行
			OLED_ShowChar(0, 20,' ',16,0);   // 清除
			OLED_ShowChar(0,40,'>',16,0);   // 显示>
			break;
		default: break;
	}
}

/**
  * @brief  设置模式光标指示符显示（> 符号，6个位置）
  * @param  num 光标位置：1=时间,2=定时1,3=定时2,4=定时3,5=食物阈值,6=水位阈值
  * @retval 无
  */
void OLED_settingsOption(uint8_t num)
{
	switch(num)
	{
		case 1:	                    // 光标在"时间"行
			OLED_ShowChar(0, 0,'>',16,0);
			OLED_ShowChar(0,22,' ',16,0);
			OLED_ShowChar(0,42,' ',16,0);
			OLED_ShowChar(0,62,' ',16,0);
			OLED_ShowChar(0,85,' ',16,0);
			OLED_ShowChar(0,105,' ',16,0);
			break;
		case 2:	                    // 光标在"定时1"行
			OLED_ShowChar(0, 0,' ',16,0);
			OLED_ShowChar(0,22,'>',16,0);
			OLED_ShowChar(0,42,' ',16,0);
			OLED_ShowChar(0,62,' ',16,0);
			OLED_ShowChar(0,85,' ',16,0);
			OLED_ShowChar(0,105,' ',16,0);
			break;
		case 3:	                    // 光标在"定时2"行
			OLED_ShowChar(0, 0,' ',16,0);
			OLED_ShowChar(0,22,' ',16,0);
			OLED_ShowChar(0,42,'>',16,0);
			OLED_ShowChar(0,62,' ',16,0);
			OLED_ShowChar(0,85,' ',16,0);
			OLED_ShowChar(0,105,' ',16,0);
			break;
		case 4:	                    // 光标在"定时3"行
			OLED_ShowChar(0, 0,' ',16,0);
			OLED_ShowChar(0,22,' ',16,0);
			OLED_ShowChar(0,42,' ',16,0);
			OLED_ShowChar(0,62,'>',16,0);
			OLED_ShowChar(0,85,' ',16,0);
			OLED_ShowChar(0,105,' ',16,0);
			break;
		case 5:	                    // 光标在"食物阈值"行
			OLED_ShowChar(0, 0,' ',16,0);
			OLED_ShowChar(0,22,' ',16,0);
			OLED_ShowChar(0,42,' ',16,0);
			OLED_ShowChar(0,62,' ',16,0);
			OLED_ShowChar(0,85,'>',16,0);
			OLED_ShowChar(0,105,' ',16,0);
			break;
		case 6:	                    // 光标在"水位阈值"行
			OLED_ShowChar(0, 0,' ',16,0);
			OLED_ShowChar(0,22,' ',16,0);
			OLED_ShowChar(0,42,' ',16,0);
			OLED_ShowChar(0,62,' ',16,0);
			OLED_ShowChar(0,85,' ',16,0);
			OLED_ShowChar(0,105,'>',16,0);
			break;

		default: break;
	}
}

/**
  * @brief  自动模式控制逻辑（定时喂食 + 自动补水）
  * @param  无
  * @retval 无
  * 定时喂食：到点时置Food_Flag=1，由Control_Manager执行
  * 自动补水：水位<阈值 且 有宠物在场（红外检测）时开泵
  */
void AutoControl(void)
{
	// 定时1触发条件：当前时间匹配 && 今日未触发 && 已启用
	if ((sensorData.calendarData.hour == Sensorthreshold.schedule[0].time.hour && sensorData.calendarData.min == Sensorthreshold.schedule[0].time.min)
        && (food_flag_1 == 0) && Sensorthreshold.schedule[0].enabled)
    {
        driveData.Food_Flag = 1;      // 启动喂食
        food_flag_1 = 1;              // 标记已触发（防止重复）
        driveData.Time_Food_Flag = 1; // 记录是定时1触发的
        driveData.Beep_Flag = 1;      // 蜂鸣提醒
    }
    // 定时2触发条件
    if ((sensorData.calendarData.hour == Sensorthreshold.schedule[1].time.hour && sensorData.calendarData.min == Sensorthreshold.schedule[1].time.min)
        && (food_flag_2 == 0) && Sensorthreshold.schedule[1].enabled)
    {
        
            driveData.Food_Flag = 1;
            food_flag_2 = 1;
            driveData.Time_Food_Flag = 2; // 记录是定时2触发的
            driveData.Beep_Flag = 1;
    // 定时3触发条件
    if ((sensorData.calendarData.hour == Sensorthreshold.schedule[2].time.hour && sensorData.calendarData.min == Sensorthreshold.schedule[2].time.min)
        && (food_flag_3 == 0) && Sensorthreshold.schedule[2].enabled)
    {        

            driveData.Food_Flag = 1;
            food_flag_3 = 1;
            driveData.Time_Food_Flag = 3; // 记录是定时3触发的
            driveData.Beep_Flag = 1;
        }：日期变化时清除所有定时触发标志
    if (date != sensorData.calendarData.w_date)
    {
        date = sensorData.calendarData.w_date;
        food_flag_1 = 0;
        food_flag_2 = 0;
        food_flag_3 = 0;
    }
    // 自动加水条件：水位低于阈值 且 红外检测到宠物在场
    if ((sensorData.water < Sensorthreshold.water_Value) && (driveData.Humin_Flag == 1))
    {
        driveData.Water_Flag = 1;     // 开启水泵
    }
    else
    {
        driveData.Water_Flag = 0;     // 关闭水泵
    }
}

/**
  * @brief  手动模式控制逻辑（APP直通 + 本地按键控制）
  * @param  num 光标位置：1=水泵,2=喂食
  * @retval 无
  * APP直通命令不依赖光标位置，本地按键依赖光标位置
  */
void ManualControl(uint8_t num)
{
	// APP直通命令：不依赖光标位置，直接控制
	// 水泵: MW0=开, MW1=关  |  喂食: MF0=开, MF1=关
	if (BlueCmd("MW0",20)) { driveData.Water_Flag = 1; }   // APP开泵
	if (BlueCmd("MW1",20)) { driveData.Water_Flag = 0; }   // APP关泵
	if (BlueCmd("MF0",20) && driveData.Now_Food_Flag == 0)  // APP开始喂食（且当前未在喂食）
	{
		driveData.Food_Flag = 1;
	}
	if (BlueCmd("MF1",20)) { driveData.Food_Flag = 0; driveData.Now_Food_Flag = 0; MOTOR_ABORT(); }  // APP停止喂食

	// 本地按键控制（依赖光标位置num）
	switch(num)
	{
		case 1:	                    // 光标在水泵行
            if (KeyNum == KEY_3)     // KEY3=开启水泵
            {
                driveData.Water_Flag = 1;
                KeyNum = 0;          // 清除按键值，防止重复触发
            }
            if (KeyNum == KEY_4)     // KEY4=关闭水泵
            {
                driveData.Water_Flag = 0;
                KeyNum = 0;
            }
            break;
        case 2:	                    // 光标在喂食行
            if (KeyNum == KEY_3)     // KEY3=开始喂食
            {
                driveData.Food_Flag = 1;
                KeyNum = 0;          // 清除按键值，防止重复触发
            }
            if (KeyNum == KEY_4)     // KEY4=停止喂食
            {
                driveData.Food_Flag = 0;
                driveData.Now_Food_Flag = 0;
                MOTOR_ABORT();       // 立即停止电机
                KeyNum = 0;
            }
            break;
		default: break;
	}

}

/**
  * @brief  控制管理器（核心执行函数：蜂鸣+增量称重+防卡粮+水泵控制）
  * @param  无
  * @retval 无
  * 功能：1.蜂鸣器嘀嘀嘀提醒 2.增量称重达标停止 3.防卡粮正反转循环 4.继电器水泵控制
  */
void Control_Manager(void)
{
    // ====== 1.蜂鸣器控制：Beep_Flag=1时嘀嘀嘀响3次后自动关闭 ======
    if (driveData.Beep_Flag)
    {
        if (count < 3)              // 响3次
        {
            if (ss_flag)
            {
                ss_flag = 0;
                BEEP_On();          // 蜂鸣器开
            }
            else
            {
                ss_flag = 1;
                BEEP_Off();         // 蜂鸣器关（翻转实现嘀嘀声）
            }
            count++;
        }
        else
        {
            count = 0;
            driveData.Beep_Flag = 0;  // 响完3次后自动清除标志
        }
    }
    else
	{
		BEEP_Off();                 // 无蜂鸣标志时确保关闭
	}
    // ====== 2.增量称重判定：当前重量 - 初始重量 >= 目标喂食量 → 停止 ======
    // 定时触发时用对应定时的喂食量，手动时用全局weight_Value
    // 增量判定：sensorData.weight - start_weight >= target（解决余粮误判）
    // 加Now_Food_Flag==1条件：等电机启动并更新start_weight后再判定，避免盘有余粮时误触发
    if (driveData.Food_Flag && driveData.Now_Food_Flag == 1)
    {
        float target;
        if (driveData.Time_Food_Flag >= 1 && driveData.Time_Food_Flag <= 3)
            target = Sensorthreshold.schedule[driveData.Time_Food_Flag - 1].feed_weight;  // 定时喂食量
        else
            target = Sensorthreshold.weight_Value;  // 手动喂食量（称重阈值）
        if (sensorData.weight >= start_weight + target)  // 增量达标
        {
            driveData.Food_Flag = 0;         // 清除喂食标志
            driveData.Time_Food_Flag = 0;    // 清除定时编号
            driveData.Now_Food_Flag = 0;     // 清除正在喂食标志
            MOTOR_ABORT();                   // 阈值达到，立即停止电机
        }
    }
    // ====== 3.喂食电机控制（含防卡粮正反转） ======
    if (driveData.Food_Flag)
    {
        if (driveData.Now_Food_Flag == 0)   // 喂食刚开始
        {
            if (bobao_flag)
            {
								USART3_SendString("A7:00002");  // 语音模块：播报"开始喂食"
                bobao_flag = 0;
            }
            driveData.Now_Food_Flag = 1;    // 标记正在喂食
            start_weight = sensorData.weight;   // 记录喂食启动瞬间的初始重量
            anti_jam_state = 0;             // 重置防卡状态为正转
            MOTOR_START(0, FEED_FORWARD_ANGLE); // 正转5圈启动推料
        }
        else                            // 正在喂食中
        {
            // ===== 防卡粮：正转完自动反转，反转完自动正转，无停顿 =====
            if (motor_running == 0)      // 当前电机转完一圈
            {
                if (anti_jam_state == 0) // 刚才是正转
                {
                    anti_jam_state = 1;
                    MOTOR_START(1, FEED_REVERSE_ANGLE);  // 正转完 → 反转1圈（防卡粮）
                }
                else                    // 刚才是反转
                {
                    anti_jam_state = 0;
                    MOTOR_START(0, FEED_FORWARD_ANGLE);  // 反转完 → 正转5圈（继续推料）
                }
            }
        }
    }
    else                            // Food_Flag=0，停止喂食
    {
        bobao_flag = 1;             // 重置播报标志
        anti_jam_state = 0;         // 清除防卡状态
        if (driveData.Now_Food_Flag == 1)  // 如果之前在喂食
        {
            driveData.Now_Food_Flag = 0;
            MOTOR_ABORT();           // 立即停止电机
        }
    }
    // ====== 4.水泵继电器控制 ======
    if (driveData.Water_Flag)
    {
        RELAY_ON;                   // 开启继电器→水泵通电
    }
    else
    {
        RELAY_OFF;                  // 关闭继电器→水泵断电
    }
}

/**
  * @brief  设置页面时间光标闪烁（用空格覆盖模拟光标闪烁）
  * @param  无
  * @retval 无
  * location 1~9分别对应时间时/分/秒、定时1分秒、定时2分秒、定时3分秒
  */
void OLED_Show_SettingTime()
{
    if (location == 1)              // 时间-小时位光标
    {
        OLED_ShowString(64, 0, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 2)         // 时间-分钟位光标
    {
        OLED_ShowString(88, 0, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 3)         // 时间-秒位光标
    {
        OLED_ShowString(112, 0, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 4)         // 定时1-小时位光标
    {
        OLED_ShowString(88, 22, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 5)         // 定时1-分钟位光标
    {
        OLED_ShowString(112, 22, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 6)         // 定时2-小时位光标
    {
        OLED_ShowString(88, 42, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 7)         // 定时2-分钟位光标
    {
        OLED_ShowString(112, 42, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 8)         // 定时3-小时位光标
    {
        OLED_ShowString(88, 62, (uint8_t*)"  ", 16, 1);
    }
    else if (location == 9)         // 定时3-分钟位光标
    {
        OLED_ShowString(112, 62, (uint8_t*)"  ", 16, 1);
    }
}

/**
  * @brief  阈值设置函数（设置页面核心逻辑）
  * @param  num 当前光标位置（1=时间,2=定时1,3=定时2,4=定时3,5=食物阈值,6=水位阈值）
  * @retval 无
  * KEY3=增加, KEY4=减少, KEY2=切换光标, KEY1=保存并退出
  */
void ThresholdSettings(uint8_t num)
{
    OLED_settingsPage1();		//显示合并后的设置界面固定信息
    SettingsThresholdDisplay1();	//显示所有设置数据
    OLED_settingsOption(num);	//实现设置页面的光标选择
	switch (num)
	{
		case 1:	                    // ===== 设置项1：调整当前时间 =====
			OLED_settingsPage1();
                IWDG_Feed();          // 喂看门狗（设置页while循环较长）
                OLED_Show_SettingTime();  // 显示光标闪烁
                if (KeyNum == KEY_3 || BlueCmd("KC",20))  // KEY3/APP_KC=增加
                {
                    KeyNum = 0;
                    if (location == 1)    // 小时+1
                    {
                        set_time.hour += 1;
                        if (set_time.hour > 23)
                        {
                            set_time.hour = 0;
                        }
                    }
                    if (location == 2)    // 分钟+1
                    {
                        set_time.min += 1;
                        if (set_time.min > 59)
                        {
                            set_time.min = 0;
                        }
                    }
                    if (location == 3)    // 秒+1
                    {
                        set_time.sec += 1;
                        if (set_time.sec > 59)
                        {
                            set_time.sec = 0;
                        }
                    }
                }
                if (KeyNum == KEY_4 || BlueCmd("KD",20))  // KEY4/APP_KD=减少
                {
                    KeyNum = 0;
                    if (location == 1)    // 小时-1
                    {
                        set_time.hour -= 1;
                        if (set_time.hour > 23)  // uint8_t下溢检查
                        {
                            set_time.hour = 0;
                        }
                    }
                    if (location == 2)    // 分钟-1
                    {
                        set_time.min -= 1;
                        if (set_time.min > 59)
                        {
                            set_time.min = 59;
                        }
                    }
                    if (location == 3)    // 秒-1
                    {
                        set_time.sec -= 1;
                        if (set_time.sec > 59)
                        {
                            set_time.sec = 59;
                        }
                    }
                }
                if (KeyNum == KEY_2 || BlueCmd("KB",20))  // KEY2/APP_KB=切换光标
                {
                    KeyNum = 0;
                    location++;
                    if (location > 3)    // 时间3个位置调完→跳到定时1
                    {
                        location = 4;
                        ++count_w;        // 光标跳到下一设置项
                        OLED_settingsPage1();
                        OLED_settingsOption(count_w);
                    }
                }
                if (BlueCmd("KA",20))
                    KeyNum = KEY_1;      // APP_KA=模式切换
                if (KeyNum == KEY_1)     // KEY1=保存退出设置页
                {
                    Flag_Writer();       // 保存参数到Flash
                    break;
                }
                SettingsThresholdDisplay1();  // 刷新数据显示
                OLED_settingsOption(count_w);  // 刷新光标位置
            break;

		case 2:	                    // ===== 设置项2：调整定时1时间 =====
                IWDG_Feed();
                OLED_Show_SettingTime();

                if (KeyNum == KEY_3 || BlueCmd("KC",20))
                {
                    KeyNum = 0;
                    if (location == 4)    // 定时1小时+1
                    {
                        Sensorthreshold.schedule[0].time.hour += 1;
                        food_flag_1 = 0;  // 重置触发标志
                        if (Sensorthreshold.schedule[0].time.hour > 23)
                        {
                            Sensorthreshold.schedule[0].time.hour = 0;
                        }
                    }
                    if (location == 5)    // 定时1分钟+1
                    {
                        Sensorthreshold.schedule[0].time.min += 1;
                        food_flag_1 = 0;
                        if (Sensorthreshold.schedule[0].time.min > 59)
                        {
                            Sensorthreshold.schedule[0].time.min = 0;
                        }
                    }
                }
                else if (KeyNum == KEY_4 || BlueCmd("KD",20))
                {
                    KeyNum = 0;
                    if (location == 4)    // 定时1小时-1
                    {
                        Sensorthreshold.schedule[0].time.hour -= 1;
                        food_flag_1 = 0;
                        if (Sensorthreshold.schedule[0].time.hour > 23)
                        {
                            Sensorthreshold.schedule[0].time.hour = 23;
                        }
                    }
                    if (location == 5)    // 定时1分钟-1
                    {
                        Sensorthreshold.schedule[0].time.min -= 1;
                        food_flag_1 = 0;
                        if (Sensorthreshold.schedule[0].time.min > 59)
                        {
                            Sensorthreshold.schedule[0].time.min = 59;
                        }
                    }
                }
                else if (KeyNum == KEY_2 || BlueCmd("KB",20))
                {
                    KeyNum = 0;
                    location++;
                    if (location > 5)    // 定时1调完→跳到定时2
                    {
                        location = 6;
                        ++count_w;
                        OLED_settingsPage1();
                        OLED_settingsOption(count_w);
                    }
                }
                if (BlueCmd("KA",20))
                    KeyNum = KEY_1;
                if (KeyNum == KEY_1)     // 保存退出
                {
                    set_flag = 1;
                    Flag_Writer();
                    break;
                }
                SettingsThresholdDisplay1();
                OLED_settingsOption(count_w);
            break;
            case 3:	                    // ===== 设置项3：调整定时2时间 =====
                IWDG_Feed();
                    OLED_Show_SettingTime();

                    if (KeyNum == KEY_3 || BlueCmd("KC",20))
                    {
                        KeyNum = 0;
                        if (location == 6)    // 定时2小时+1
                        {
                            Sensorthreshold.schedule[1].time.hour += 1;
                            food_flag_2 = 0;
                            if (Sensorthreshold.schedule[1].time.hour > 23)
                            {
                                Sensorthreshold.schedule[1].time.hour = 0;
                            }
                        }
                        if (location == 7)    // 定时2分钟+1
                        {
                            Sensorthreshold.schedule[1].time.min += 1;
                            food_flag_2 = 0;
                            if (Sensorthreshold.schedule[1].time.min > 59)
                            {
                                Sensorthreshold.schedule[1].time.min = 0;
                            }
                        }
                    }
                    else if (KeyNum == KEY_4 || BlueCmd("KD",20))
                    {
                        KeyNum = 0;
                        if (location == 6)    // 定时2小时-1
                        {
                            Sensorthreshold.schedule[1].time.hour -= 1;
                            food_flag_2 = 0;
                            if (Sensorthreshold.schedule[1].time.hour > 23)
                            {
                                Sensorthreshold.schedule[1].time.hour = 23;
                            }
                        }
                        if (location == 7)    // 定时2分钟-1
                        {
                            Sensorthreshold.schedule[1].time.min -= 1;
                            food_flag_2 = 0;
                            if (Sensorthreshold.schedule[1].time.min > 59)
                            {
                                Sensorthreshold.schedule[1].time.min = 59;
                            }
                        }
                    }
                    else if (KeyNum == KEY_2 || BlueCmd("KB",20))
                    {
                        KeyNum = 0;
                        location++;
                        if (location > 7)    // 定时2调完→跳到定时3
                        {
                            location = 8;
                            ++count_w;
                            OLED_settingsPage1();
                            OLED_settingsOption(count_w);
                        }
                    }
                    if (BlueCmd("KA",20))
                        KeyNum = KEY_1;
                    if (KeyNum == KEY_1)     // 保存退出
                    {
                        set_flag = 1;
                        Flag_Writer();
                        break;
                    }
                    SettingsThresholdDisplay1();
                    OLED_settingsOption(count_w);
                break;
            case 4:	                    // ===== 设置项4：调整定时3时间 =====
                    IWDG_Feed();
                    OLED_Show_SettingTime();

                    if (KeyNum == KEY_3 || BlueCmd("KC",20))
                    {
                        KeyNum = 0;
                        if (location == 8)    // 定时3小时+1
                        {
                            Sensorthreshold.schedule[2].time.hour += 1;
                            food_flag_3 = 0;
                            if (Sensorthreshold.schedule[2].time.hour > 23)
                            {
                                Sensorthreshold.schedule[2].time.hour = 0;
                            }
                        }
                        if (location == 9)    // 定时3分钟+1
                        {
                            Sensorthreshold.schedule[2].time.min += 1;
                            food_flag_3 = 0;
                            if (Sensorthreshold.schedule[2].time.min > 59)
                            {
                                Sensorthreshold.schedule[2].time.min = 0;
                            }
                        }
                    }
                    else if (KeyNum == KEY_4 || BlueCmd("KD",20))
                    {
                        KeyNum = 0;
                        if (location == 8)    // 定时3小时-1
                        {
                            Sensorthreshold.schedule[2].time.hour -= 1;
                            food_flag_3 = 0;
                            if (Sensorthreshold.schedule[2].time.hour > 23)
                            {
                                Sensorthreshold.schedule[2].time.hour = 23;
                            }
                        }
                        if (location == 9)    // 定时3分钟-1
                        {
                            Sensorthreshold.schedule[2].time.min -= 1;
                            food_flag_3 = 0;
                            if (Sensorthreshold.schedule[2].time.min > 59)
                            {
                                Sensorthreshold.schedule[2].time.min = 59;
                            }
                        }
                    }
                    else if (KeyNum == KEY_2 || BlueCmd("KB",20))
                    {
                        KeyNum = 0;
                        location++;
                        if (location > 9)    // 定时3调完→跳到食物阈值
                        {
                            count_w = 5;
                            location = 1;
                            break;
                        }
                    }
                    if (BlueCmd("KA",20))
                        KeyNum = KEY_1;
                    if (KeyNum == KEY_1)     // 保存退出
                    {
                        set_flag = 1;
                        Flag_Writer();
                        break;
                    }
                    SettingsThresholdDisplay1();
                    OLED_settingsOption(count_w);
                break;
            case 5:	                    // ===== 设置项5：调整食物阈值（每次±10g）=====
                if (KeyNum == KEY_3 || BlueCmd("KC",20))
                {
                    KeyNum = 0;
                    Sensorthreshold.weight_Value += 10;     // 食物阈值+10g
                    if (Sensorthreshold.weight_Value >= 9000)  // 上限循环
                    {
                        Sensorthreshold.weight_Value = 0;
                    }
                }
                else if (KeyNum == KEY_4 || BlueCmd("KD",20))
                {
                    KeyNum = 0;
                    Sensorthreshold.weight_Value -= 10;     // 食物阈值-10g
                    if (Sensorthreshold.weight_Value >= 9000)  // 下限循环（uint16_t下溢）
                    {
                        Sensorthreshold.weight_Value = 9000;
                    }
                }
                else if (KeyNum == KEY_2 || BlueCmd("KB",20))
                {
                    KeyNum = 0;
                    ++count_w;               // 光标跳到水位阈值
                }
                if (BlueCmd("KA",20))
                    KeyNum = KEY_1;
                if (KeyNum == KEY_1)         // 保存退出
                {
                    set_flag = 1;
                    Flag_Writer();
                }
                break;
            case 6:	                    // ===== 设置项6：调整水位阈值（每次±1%）=====
                if (KeyNum == KEY_3 || BlueCmd("KC",20))
                {
                    KeyNum = 0;
                    Sensorthreshold.water_Value += 1;       // 水位阈值+1%
                    if (Sensorthreshold.water_Value >= 100)   // 上限循环
                    {
                        Sensorthreshold.water_Value = 0;
                    }
                }
                else if (KeyNum == KEY_4 || BlueCmd("KD",20))
                {
                    KeyNum = 0;
                    Sensorthreshold.water_Value -= 1;       // 水位阈值-1%
                    if (Sensorthreshold.water_Value >= 100)   // 下限循环
                    {
                        Sensorthreshold.water_Value = 100;
                    }
                }
                else if (KeyNum == KEY_2 || BlueCmd("KB",20))
                {
                    KeyNum = 0;
                    count_w = 1;              // 光标循环回第一个设置项（时间）
                    location = 1;
                }
                if (BlueCmd("KA",20))
                    KeyNum = KEY_1;
                if (KeyNum == KEY_1)         // 保存退出
                {
                    set_flag = 1;
                    Flag_Writer();
                }
                break;
		default: break;
	}
}

/**
  * @brief  构造并发送数据帧给APP（通过ESP8266 TCP）
  * @param  无
  * @retval 无
  * 数据帧格式：T25H60W0200L080M1F0P0WT080FT0200T10700F10050E1T21230F20100E1T31900F30150E1
  * T=温度 H=湿度 W=重量 L=水位 M=模式 F=喂食 P=水泵
  * WT=水位阈值 FT=称重阈值
  * T1/F1/E1=定时1时间/喂食量/启用
  */
void Public(void)
{
    uint8_t buf[90];              // 数据帧缓冲区
    int len = sprintf((char*)buf, "T%02dH%02dW%04dL%03dM%dF%dP%dWT%03dFT%04dT1%02d%02dF1%04dE%dT2%02d%02dF2%04dE%dT3%02d%02dF3%04dE%d\r\n",
        sensorData.temp,          // 温度（2位）
        sensorData.humi,          // 湿度（2位）
        abs((int)sensorData.weight),  // 重量绝对值（4位）
        sensorData.water,         // 水位百分比（3位）
        (mode == 1) ? 1 : 0,     // M=模式(1=自动,0=手动)
        driveData.Food_Flag ? 1 : 0,  // F=喂食中(1=是,0=否)
        driveData.Water_Flag ? 1 : 0,  // P=水泵开(1=开,0=关)
        Sensorthreshold.water_Value,   // WT=水位阈值
        (int)Sensorthreshold.weight_Value,  // FT=称重阈值
        Sensorthreshold.schedule[0].time.hour, Sensorthreshold.schedule[0].time.min, (int)Sensorthreshold.schedule[0].feed_weight, Sensorthreshold.schedule[0].enabled,  // 定时1
        Sensorthreshold.schedule[1].time.hour, Sensorthreshold.schedule[1].time.min, (int)Sensorthreshold.schedule[1].feed_weight, Sensorthreshold.schedule[1].enabled,  // 定时2
        Sensorthreshold.schedule[2].time.hour, Sensorthreshold.schedule[2].time.min, (int)Sensorthreshold.schedule[2].feed_weight, Sensorthreshold.schedule[2].enabled); // 定时3
    ESP8266_SendData(buf, len);   // 通过ESP8266发送数据帧
}

/**
  * @brief  保存所有阈值参数到Flash + 同步RTC时间
  * @param  无
  * @retval 无
  * Flash布局(14个u16)：[定时1时,定时1分, 定时2时,定时2分, 定时3时,定时3分, 称重阈值, 水位阈值, 喂食量1,2,3, 启用1,2,3]
  */
void Flag_Writer(void)
{
    u16 dat[14];
    // 索引0-5: 3个定时的小时和分钟
    dat[0] = Sensorthreshold.schedule[0].time.hour;    // 定时1-小时
    dat[1] = Sensorthreshold.schedule[0].time.min;     // 定时1-分钟
    dat[2] = Sensorthreshold.schedule[1].time.hour;    // 定时2-小时
    dat[3] = Sensorthreshold.schedule[1].time.min;     // 定时2-分钟
    dat[4] = Sensorthreshold.schedule[2].time.hour;    // 定时3-小时
    dat[5] = Sensorthreshold.schedule[2].time.min;     // 定时3-分钟
    // 索引6-7: 称重阈值 + 水位阈值
    dat[6] = (u16)Sensorthreshold.weight_Value;        // 称重阈值(g)
    dat[7] = Sensorthreshold.water_Value;              // 水位阈值(%)
    // 索引8-10: 3个定时的喂食量
    dat[8] = (u16)Sensorthreshold.schedule[0].feed_weight;  // 定时1喂食量
    dat[9] = (u16)Sensorthreshold.schedule[1].feed_weight;  // 定时2喂食量
    dat[10] = (u16)Sensorthreshold.schedule[2].feed_weight; // 定时3喂食量
    // 索引11-13: 3个定时的启用状态
    dat[11] = Sensorthreshold.schedule[0].enabled;     // 定时1启用(0/1)
    dat[12] = Sensorthreshold.schedule[1].enabled;     // 定时2启用(0/1)
    dat[13] = Sensorthreshold.schedule[2].enabled;     // 定时3启用(0/1)
    FLASH_W(FLASH_START_ADDR, dat, 14);                // 写入14个u16到Flash
    RTC_Set(sensorData.calendarData.w_year, sensorData.calendarData.w_month, sensorData.calendarData.w_date, set_time.hour, set_time.min, set_time.sec);  // 同步RTC
    sensorData.calendarData.hour = set_time.hour;      // 同步运行时时间
    sensorData.calendarData.min = set_time.min;
    sensorData.calendarData.sec = set_time.sec;
    rtc_get_flag = 1;               // 标记RTC已更新
}

/**
  * @brief  从Flash读取并初始化所有阈值参数
  * @param  无
  * @retval 无
  * 先读Flash检查合法性，非法则写入默认值再重读
  * 默认值：定时12:01/12:02/12:03, 称重200g, 水位80%, 喂食量50/100/150g, 全部启用
  */
void Flash_Time_Init(void)
{
    // 先读Flash，检查数据是否合法
    Sensorthreshold.schedule[0].time.hour = FLASH_R(FLASH_START_ADDR);       // 读定时1小时
    Sensorthreshold.schedule[0].time.min = FLASH_R(FLASH_START_ADDR + 2);    // 读定时1分钟
    Sensorthreshold.schedule[1].time.hour = FLASH_R(FLASH_START_ADDR + 4);   // 读定时2小时
    Sensorthreshold.schedule[1].time.min = FLASH_R(FLASH_START_ADDR + 6);    // 读定时2分钟
    Sensorthreshold.schedule[2].time.hour = FLASH_R(FLASH_START_ADDR + 8);   // 读定时3小时
    Sensorthreshold.schedule[2].time.min = FLASH_R(FLASH_START_ADDR + 10);   // 读定时3分钟
    Sensorthreshold.weight_Value = FLASH_R(FLASH_START_ADDR + 12);           // 读称重阈值
    Sensorthreshold.water_Value = FLASH_R(FLASH_START_ADDR + 14);            // 读水位阈值
    Sensorthreshold.schedule[0].feed_weight = FLASH_R(FLASH_START_ADDR + 16); // 读定时1喂食量
    Sensorthreshold.schedule[1].feed_weight = FLASH_R(FLASH_START_ADDR + 18); // 读定时2喂食量
    Sensorthreshold.schedule[2].feed_weight = FLASH_R(FLASH_START_ADDR + 20); // 读定时3喂食量
    Sensorthreshold.schedule[0].enabled = FLASH_R(FLASH_START_ADDR + 22);    // 读定时1启用
    Sensorthreshold.schedule[1].enabled = FLASH_R(FLASH_START_ADDR + 24);    // 读定时2启用
    Sensorthreshold.schedule[2].enabled = FLASH_R(FLASH_START_ADDR + 26);    // 读定时3启用

    // 合法性检查：任何值超出范围则判定为首次使用，写入默认值
    if (Sensorthreshold.schedule[0].time.hour > 24 || Sensorthreshold.schedule[0].time.min > 60
        || Sensorthreshold.schedule[1].time.hour > 24 || Sensorthreshold.schedule[1].time.min > 60
        || Sensorthreshold.schedule[2].time.hour > 24 || Sensorthreshold.schedule[2].time.min > 60
        || Sensorthreshold.weight_Value == 0
        || Sensorthreshold.weight_Value == 0xFFFF)
    {
        // 初始化默认值
        u16 def[14] = {12,1, 12,2, 12,3, 200, 80, 50, 100, 150, 1, 1, 1};
        FLASH_W(FLASH_START_ADDR, def, 14);  // 写入默认值
    }

    // 重新读取（确保使用的是合法数据）
    Sensorthreshold.schedule[0].time.hour = FLASH_R(FLASH_START_ADDR);
    Sensorthreshold.schedule[0].time.min = FLASH_R(FLASH_START_ADDR + 2);
    Sensorthreshold.schedule[1].time.hour = FLASH_R(FLASH_START_ADDR + 4);
    Sensorthreshold.schedule[1].time.min = FLASH_R(FLASH_START_ADDR + 6);
    Sensorthreshold.schedule[2].time.hour = FLASH_R(FLASH_START_ADDR + 8);
    Sensorthreshold.schedule[2].time.min = FLASH_R(FLASH_START_ADDR + 10);
    Sensorthreshold.weight_Value = FLASH_R(FLASH_START_ADDR + 12);
    Sensorthreshold.water_Value = FLASH_R(FLASH_START_ADDR + 14);
    Sensorthreshold.schedule[0].feed_weight = FLASH_R(FLASH_START_ADDR + 16);
    Sensorthreshold.schedule[1].feed_weight = FLASH_R(FLASH_START_ADDR + 18);
    Sensorthreshold.schedule[2].feed_weight = FLASH_R(FLASH_START_ADDR + 20);
    Sensorthreshold.schedule[0].enabled = FLASH_R(FLASH_START_ADDR + 22);
    Sensorthreshold.schedule[1].enabled = FLASH_R(FLASH_START_ADDR + 24);
    Sensorthreshold.schedule[2].enabled = FLASH_R(FLASH_START_ADDR + 26);

    // 确保 feed_weight 至少为 1（防止除零或误停）
    if (Sensorthreshold.schedule[0].feed_weight == 0) Sensorthreshold.schedule[0].feed_weight = 50;
    if (Sensorthreshold.schedule[1].feed_weight == 0) Sensorthreshold.schedule[1].feed_weight = 100;
    if (Sensorthreshold.schedule[2].feed_weight == 0) Sensorthreshold.schedule[2].feed_weight = 150;
}

/**
  * @brief  ESP8266 WiFi初始化（AP模式热点）
  * @param  无
  * @retval 无
  * 流程：AT测试 → 复位 → AP模式 → 配置热点 → 多连接 → 开TCP服务器端口5000 → 查IP
  * 热点名称: ZNCWJZZ, 密码: 12345678, IP: 192.168.4.1, 端口: 5000
  */
void ESP8266_Server(void)
{
    // AP模式：ESP8266自己开热点，手机直接连接，最稳定方案
    // 1. AT测试（确认模块正常通信）
    while(ESP8266_Cmd("AT\r\n", "OK", 0, 500))
        delay_ms(300);            // 失败则每300ms重试
    delay_ms(500);
    OLED_ShowString(0, 16, (uint8_t*)"AT OK", 16, 1);   // 屏幕显示AT OK
    OLED_Refresh();

    // 2. 设置AP模式
    ESP8266_Cmd("AT+RST\r\n", "ready", 0, 3000);         // 复位模块
    delay_ms(1000);
    while(ESP8266_Cmd("AT\r\n", "OK", 0, 500))
        delay_ms(300);

    while(ESP8266_Cmd("AT+CWMODE=2\r\n", "OK", "no change", 2000))  // 设为AP模式
        delay_ms(500);
    OLED_ShowString(64, 16, (uint8_t*)"AP OK", 16, 1);
    OLED_Refresh();

    // 3. 配置热点: ZNCWJZZ / 12345678 / 通道5 / WPA2加密
    while(ESP8266_Cmd("AT+CWSAP=\"ZNCWJZZ\",\"12345678\",5,3\r\n", "OK", 0, 3000))
        delay_ms(500);
    OLED_ShowString(0, 32, (uint8_t*)"WiFi:ZNCWJZZ", 16, 1);
    OLED_Refresh();

    // 4. 开启多连接模式（允许多个客户端连接）
    while(ESP8266_Cmd("AT+CIPMUX=1\r\n", "OK", 0, 1000))
        delay_ms(300);

    // 5. 开启TCP Server，端口5000
    while(ESP8266_Cmd("AT+CIPSERVER=1,5000\r\n", "OK", 0, 1000))
        delay_ms(300);

    // 6. 查询并显示IP地址
    ESP8266_Cmd("AT+CIFSR\r\n", "OK", 0, 1000);
    OLED_ShowString(0, 48, (uint8_t*)"IP:192.168.4.1", 16, 1);
    OLED_Refresh();
    printf("\r\n[INFO] AP ready! SSID:ZNCWJZZ PWD:12345678 IP:192.168.4.1:5000\r\n");
    delay_ms(500);
}



/**
  * @brief  主函数（程序入口）
  * @param  无
  * @retval int（嵌入式不返回）
  * 流程：硬件初始化 → ESP8266启动 → Flash读参 → 看门狗 → 主循环(扫描→控制→显示)
  */
int main(void)
{
  SystemInit();                   // 配置系统时钟为72MHz
	delay_init(72);                 // 初始化延时函数（基于72MHz）
	ADCX_Init();                    // 初始化ADC（水位传感器PA1）
	LED_Init();                     // 初始化LED（状态指示）
	LED_On();                       // 开启LED（上电指示）
	Key_Init();                     // 初始化按键（4个物理按键）
	MOTOR_Init();                   // 初始化步进电机GPIO（PB8~PB11）
	BUMP_Init();                    // 初始化红外检测传感器（PA0）
	LCD_Init();                     // 初始化TFT LCD（ST7735 SPI）
	DHT11_Init();                   // 初始化DHT11温湿度传感器
    HW_Init();                     // 初始化硬件GPIO抽象层
    RTC_Init();                    // 初始化RTC实时时钟
    BEEP_Init();                    // 初始化蜂鸣器
    HX711_Init();                   // 初始化HX711称重模块
    WATER_Init();                   // 初始化水位传感器ADC通道
    RELAY_Init();                   // 初始化继电器（水泵控制）
	OLED_Clear();                    // 清空LCD屏幕

	Pi_weight = Get_Tare();          // HX711去皮（获取空碗重量作为零点）
	TIM2_Init(9,7198);              // 初始化TIM2：预分频9,周期7198 → 1ms中断（步进电机驱动）

	USART1_Config();                // 初始化串口1（调试打印/ESP8266通信）
    USART2_Config();                // 初始化串口2（与ESP8266数据通信）
    USART3_Config();                // 初始化串口3（语音模块通信）
	printf("Start \n");              // 串口打印启动信息

 USART3_SendString("AF:30");       // 语音模块：设置音量30
	delay_ms(300);
	USART3_SendString("A7:00001");  // 语音模块：播报"欢迎使用"


    ESP8266_Server();               // 启动ESP8266 AP热点+TCP服务器
	OLED_Clear();                    // 清除初始化信息，准备进入主界面

    Flash_Time_Init();              // 从Flash读取阈值参数（首次使用则写入默认值）

    reset = HX711_GetData();        // 获取HX711初始数据
    date = sensorData.calendarData.w_date;  // 记录当前日期（跨日重置用）

    // 初始化独立看门狗：预分频64，重装载1875，超时约3秒
    IWDG_Init(IWDG_Prescaler_64, 1875);


  while (1)                        // ====== 主循环 ======
  {
		SensorScan();                  // 获取所有传感器数据（温湿度/重量/水位/红外）
		IWDG_Feed();                   // 喂看门狗（防止3秒内未喂狗导致复位）

		switch(mode)                   // 根据当前模式执行不同逻辑
		{
			case AUTO_MODE:            // ====== 自动模式 ======

				OLED_autoPage1();      // 显示自动模式固定标签（温度/湿度/质量/水位）
				SensorDataDisplay1();  // 显示传感器实时数据
				AutoControl();         // 自动控制逻辑（定时喂食+自动补水）

         WiFi_Key();               // 解析APP发来的命令

				/*按键1按下时切换模式*/
				if (KeyNum == KEY_1)   // 系统模式：1自动→2手动
				{
					KeyNum = 0;
					mode = MANUAL_MODE;
					count_m = 1;
					OLED_Clear();
				}

				if (KeyNum == KEY_Long1)  // 长按KEY1进入设置模式
				{
					KeyNum = 0;
					mode = SETTINGS_MODE;
					count_s = 1;
					OLED_Clear();
				}

				Control_Manager();      // 执行控制（蜂鸣+称重判定+电机+水泵）

				break;

            case MANUAL_MODE:       // ====== 手动模式 ======
				{
				uint8_t manual_ret = SetManual();  // 获取光标位置
				OLED_manualOption(manual_ret);     // 显示光标指示符>
				ManualControl(manual_ret);         // 手动控制逻辑

                Control_Manager();   // 执行控制（蜂鸣+称重判定+电机+水泵）

                OLED_manualPage1();  // 显示手动模式固定标签
                ManualSettingsDisplay1();  // 显示手动模式动态数据

                WiFi_Key();          // 解析APP命令

				if (KeyNum == KEY_1) // 切回自动模式
				{
					KeyNum = 0;
					mode = AUTO_MODE;
					OLED_Clear();
				}

				break;
				}

            case SETTINGS_MODE:     // ====== 设置模式 ======
                    if (set_flag)     // 首次进入设置页时初始化
                    {
                        set_time = sensorData.calendarData;  // 复制当前时间
                        set_flag = 0;
                        location = 1;
                        count_w = 1;
                        rtc_get_flag = 0;
                    }
					ThresholdSettings(count_w);  // 执行阈值调节功能


					//判断是否退出阈值设置界面
					if (KeyNum == KEY_1)  // KEY1=保存并退出
					{
						KeyNum = 0;
						mode = AUTO_MODE;      // 跳转到自动模式
						OLED_Clear();          // 清屏

						//存储修改的传感器阈值至Flash
						Flag_Writer();
					}
				break;
				default: break;
		}
		time_num++;                                               // 主循环计数+1
        if (time_num % 2 == 0)     // 每2个循环发送一次数据帧（约20ms一次）
            Public();                // 发送传感器数据给APP
		delay_ms(10);                // 主循环延时10ms（控制循环频率约100Hz）
		if(time_num >= 5000)         // 计数器溢出清零
		{
			time_num = 0;
		}


  }
}
