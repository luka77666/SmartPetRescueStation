#include "hx711.h" // HX711称重传感器驱动头文件
#include "delay.h" // 微秒延时函数头文件

#define MEDIAN_LEN  5  			//中值滤波的滤波长度,一般取奇数
#define MEDIAN      3  			//中值在滤波数组中的位置

/*******************************
						STM32
 * 文件			:	HX711电子秤模块c文件                   
 * 版本			: V1.0
 * 日期			: 2024.9.11
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码					
 
**********************BEGIN***********************/

uint32_t   buffer[MEDIAN_LEN];   	//中值滤波的数据缓存
int   medleng = 0;          		//一组中值滤波数据中,进入滤波缓存的数据个数
uint32_t   xd,xd1;				//数据对比大小中间变量

float	pi_weight;			        //皮重
float	hx711_xishu=12.37363426011;	//这是一个修正系数，例如1000g砝码称出来是934g，则HX711_xishu=原数据*1000/934;

/**
  * @brief  HX711初始化，配置SCK为推挽输出，DT为上拉输入
  * @param  无
  * @retval 无
  */

void HX711_Init(void)
{
		/*定义一个HX711_InitTypeDef类型的结构体*/
		GPIO_InitTypeDef GPIO_InitStructure;

		/*开启相关的GPIO外设时钟*/
		RCC_APB2PeriphClockCmd(HX711_GPIO_CLK , ENABLE); // 使能HX711所在GPIO端口时钟
		/*选择要控制的GPIO引脚*/

		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // SCK引脚设为推挽输出，用于时钟信号
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // GPIO输出速率50MHz
		GPIO_InitStructure.GPIO_Pin = HX711_SCK_GPIO_PIN; // 选择HX711的SCK时钟引脚
		GPIO_Init(HX711_SCK_GPIO_PORT, &GPIO_InitStructure); // 初始化SCK引脚
	
		GPIO_InitStructure.GPIO_Pin = HX711_DT_GPIO_PIN;	
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   	// DT引脚设为上拉输入，用于读取数据

		GPIO_Init(HX711_DT_GPIO_PORT, &GPIO_InitStructure);	
	
	
}
	
unsigned long HX711_GetData(void) // 从HX711读取24位ADC原始数据
{
		unsigned long Count; // 存储24位移位累加的ADC数据
		unsigned char i; // 循环计数变量，用于24次移位读取
		volatile uint32_t timeout = 0; // 超时计数器，防止传感器未连接时死循环
		HX711_SCK_L; // SCK拉低，准备通信
	  delay_us(1); // 等待1微秒稳定
		Count=0; // 清零数据计数器
		while(HX711_DT) { // 等待DT引脚拉低，表示HX711数据准备就绪
			if(++timeout > 200000) return 0; // 超时保护，防止传感器断线死锁
		}
		for (i=0;i<24;i++) // 循环24次，逐位读取ADC数据
 {
				HX711_SCK_H; // SCK拉高，上升沿触发HX711输出一位数据
				delay_us(1); // 等待1微秒让数据稳定
				Count=Count<<1; // 数据左移一位，为新数据腾出最低位
				HX711_SCK_L; // SCK拉低，准备读取下一位
				delay_us(1); // 等待1微秒
				if(HX711_DT) Count++; // 如果DT为高电平，最低位置1；否则保持0
		}
		HX711_SCK_H; // 额外一个时钟脉冲，选择通道A和增益128
		delay_us(1); // 等待1微秒
		Count=Count^0x800000; // 将第24位取反，将24位有符号数转换为无符号数
		HX711_SCK_L; // SCK拉低结束通信
		delay_us(1); // 等待1微秒
		
		return(Count); // 返回24位ADC原始数据
}

float Get_Weight(float pi_weight)	  //获取被测物体重量
{
	uint32_t hx711_data; // HX711读取的原始ADC数据
	float	weight;	      				//校准前的重量值
	float	later_weight;	      //校准后的重量值
	float get; // 单次采样计算的重量值
	float sum = 0; // 多次采样重量累加和
	uint8_t i; // 循环计数变量
	for(i = 0; i < 5; i++)				//采样5次取均值，消除抖动
	{
		hx711_data = HX711_GetData(); // 读取一次HX711的24位ADC数据
		get = ((float)hx711_data / 8388607.0) * 1000 * hx711_xishu; // 将ADC值转换为克数：除以2^23归一化，乘1000换算，再乘修正系数
		sum += get; // 累加每次采样结果
	}
	get = sum / 5.0f; // 计算5次采样的平均值

	// 直接使用5次均值计算重量，不再丢弃均值重新采样
	weight = get - pi_weight; // 减去皮重得到净重
	later_weight = weight; // 赋值给最终返回变量

	return later_weight; // 返回校准后的净重量值（单位：克）
}

float Get_Tare(void)//获取皮重
{
	uint32_t hx711_dat; // HX711读取的原始ADC数据
	uint8_t i, j; // 循环计数变量，i为采样循环，j为插入排序循环
	for(i = 0; i < MEDIAN_LEN; i++) // 采集MEDIAN_LEN次数据用于中值滤波
	{
		hx711_dat = HX711_GetData();	       	//HX711AD转换数据处理
		if(medleng == 0)                    //缓存的第1个元素,直接放入,不需要排序
		{ 
			buffer[0] = hx711_dat; medleng = 1; // 第一个数据直接放入缓存首位，数据个数置1
		}
		else                           		//插入排序算法,按从小到大的顺序排列 
		{  
			for(j = 0; j < medleng; j ++)  
			{
				if( buffer[j] > hx711_dat) 	// 轮询到的当前元素>AD值,则交换它们的值，xd为中间变量存放位置
				{ 
					xd = hx711_dat; hx711_dat = buffer[j]; buffer[j] = xd; // 交换当前缓存元素和新数据
				}
			}
			buffer[medleng] = hx711_dat; 	//把轮询出较大的数放入缓存的后面.
			medleng++; // 缓存数据个数加1
		}	
		if(medleng >= MEDIAN_LEN) 		    //ADC采样的数据个数达到中值滤波要求的数据个数
		{
			hx711_dat = buffer[MEDIAN];	    //最终重量取中值滤波数组的中间值
			medleng = 0; // 重置缓存数据个数，准备下一轮中值滤波
		}
	}
	pi_weight=((float)hx711_dat/8388607.0)*1000*hx711_xishu; // 将中值滤波后的ADC数据转换为克数作为皮重
	return pi_weight; // 返回计算得到的皮重值（单位：克）
}
