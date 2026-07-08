# include "Modules.h"           // 传感器/驱动模块结构体头文件


/**
  * @brief  传感器数据扫描函数（主循环每10ms调用一次）
  * @param  无
  * @retval 无
  * 功能：读取温湿度/重量/水位/红外/RTC时间，并做滤波处理
  */

extern uint8_t dht11_read_tick;  // DHT11读取计时（main.c中定义）
extern float Pi_weight;           // HX711去皮重量（main.c中定义）

// 重量滑动平均滤波缓冲区（5点窗口，消除HX711瞬时波动）
#define WEIGHT_AVG_LEN 5         // 滤波窗口长度
static float weight_buf[WEIGHT_AVG_LEN] = {0};  // 滑动平均缓冲区
static uint8_t weight_buf_idx = 0;               // 缓冲区写入索引
static uint8_t weight_buf_cnt = 0;               // 已填充数据个数
static float last_valid_weight = 0;              // 上一次有效重量（用于异常值剔除）

void SensorScan(void)
{
	 float weight;                // 当前瞬时重量
	 float avg = 0;               // 滑动平均值
	 uint8_t i;

	 // DHT11采样周期至少1.5秒，每2秒读一次，其余时间用缓存值
	 dht11_read_tick++;           // 计数器+1
	 if (dht11_read_tick == 1 || dht11_read_tick >= 200)  // 第一次立即读，之后每200次≈2秒
	 {
		 dht11_read_tick = 0;    // 重置计数器
		 DHT11_Read_Data(&sensorData.temp, &sensorData.humi);  // 读取温湿度
	 }

	 driveData.Humin_Flag = HW_GetData();   // 读取红外传感器（宠物检测，1=有宠物）
    weight = 1.73 * Get_Weight(Pi_weight);  // 读取HX711重量并乘以标定系数1.73

    // 异常值剔除：与上一次有效值偏差超过500g视为干扰尖峰，丢弃不用
    // 解决继电器/水泵启动时的电磁干扰导致HX711读数跳变到几千的问题
    if (weight_buf_cnt > 0 && (weight - last_valid_weight > 500 || weight - last_valid_weight < -500))
    {
        // 本次读数异常，用上一次有效值代替，不存入缓冲区
        weight = last_valid_weight;
    }

    // 滑动平均滤波：消除跨周期跳变
    weight_buf[weight_buf_idx] = weight;     // 存入缓冲区
    weight_buf_idx = (weight_buf_idx + 1) % WEIGHT_AVG_LEN;  // 循环索引
    if (weight_buf_cnt < WEIGHT_AVG_LEN) weight_buf_cnt++;    // 填充阶段递增
    for (i = 0; i < weight_buf_cnt; i++) avg += weight_buf[i];  // 求和
    avg /= weight_buf_cnt;        // 计算平均值

    // 去除干扰：绝对值小于5g视为0（传感器噪声范围）
    if (avg < 5 && avg > -5) sensorData.weight = 0;
    else sensorData.weight = avg; // 有效重量赋值
    last_valid_weight = sensorData.weight;  // 记录本次有效重量，供下次异常值剔除参考

	 sensorData.calendarData = calendar;  // 从RTC全局变量获取当前时间
	 sensorData.water = WATER_GetData() * 100 / 4095;  // ADC值(0-4095)映射到水位百分比(0-100)
}
