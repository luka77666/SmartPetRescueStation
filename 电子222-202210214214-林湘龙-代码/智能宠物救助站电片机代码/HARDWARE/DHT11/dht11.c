#include "dht11.h" // DHT11温湿度传感器驱动头文件
#include "delay.h" // 微秒/毫秒延时函数头文件
      
/*******************************
						STM32
 * 文件			:	DHT11温湿度传感器c文件                   
 * 版本			: V1.0
 * MCU			:	STM32F103C8T6
 * 接口			:	见dht11.h文件		
 
**********************BEGIN***********************/	

//复位DHT11
/**
  * @brief  复位DHT11传感器，主机发送起始信号（拉低18~30ms后拉高）
  * @param  无
  * @retval 无
  */
void DHT11_Rst(void)	   
{                
	DHT11_Mode(OUT); 	//SET OUTPUT 将数据引脚设为输出模式
	DHT11_Low; 	      //拉低DQ 发送起始信号
	delay_ms(20);    	//主机拉低18~30ms 等待DHT11响应
	DHT11_High; 			//DQ=1 释放总线
	delay_us(13);    	//主机拉高10~35us 等待DHT11应答
}

//等待DHT11的回应
//返回1:未检测到DHT11的存在
//返回0:存在
/**
  * @brief  检测DHT11是否响应，等待DHT11的80us低电平+80us高电平应答信号
  * @param  无
  * @retval 0表示DHT11存在并响应，1表示未检测到DHT11
  */
u8 DHT11_Check(void)	   
{   
	u8 retry=0; // 超时重试计数器
	DHT11_Mode(IN);//SET INPUT	 切换为输入模式等待DHT11响应
    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//DHT11会拉低40~80us
	{
		retry++; // 超时计数加1
		delay_us(1); // 等待1微秒
	};	 
	if(retry>=100)return 1; // 超时未检测到低电平，返回1表示DHT11不存在
	else retry=0; // 重置计数器，继续检测高电平
    while (!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//DHT11拉低后会再次拉高40~80us
	{
		retry++; // 超时计数加1
		delay_us(1); // 等待1微秒
	};
	if(retry>=100)return 1;	     // 超时未检测到高电平，返回1表示DHT11不存在
	return 0; // 检测到完整应答信号，返回0表示DHT11正常
}

//从DHT11读取一个位
//返回值：1/0
/**
  * @brief  从DHT11读取一位数据，通过高电平持续时间判断数据是0还是1
  * @param  无
  * @retval 1或0，表示读取到的单个位
  */
u8 DHT11_Read_Bit(void)		 
{ 
	u8 retry=0; // 超时重试计数器
	while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//等待变为低电平
	{
		retry++; // 超时计数加1
		delay_us(1); // 等待1微秒
	}
	retry=0; // 重置计数器
	while(!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100)//等待变高电平
	{
		retry++; // 超时计数加1
		delay_us(1); // 等待1微秒
	}
	delay_us(40);//等待40us
	if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN))return 1; // 40us后仍为高电平表示数据位为1
	else return 0;		   // 40us后为低电平表示数据位为0
}

//从DHT11读取一个字节
//返回值：读到的数据
/**
  * @brief  从DHT11读取一个字节（8位），高位在前依次读取
  * @param  无
  * @retval 读到的8位数据
  */
u8 DHT11_Read_Byte(void)   
{       
	u8 i,dat; // i为循环计数，dat存储读取的字节
	dat=0; // 清零数据
	for (i=0;i<8;i++) // 循环读取8位
	{
		dat<<=1; // 数据左移一位，腾出最低位
		dat|=DHT11_Read_Bit(); // 读取一位并拼接到最低位
	}					    
	return dat; // 返回读取到的完整字节
}

//从DHT11读取一次数据
//temp:温度值(范围:0~50°)
//humi:湿度值(范围:20%~90%)
//返回值：0,正常;1,读取失败
/**
  * @brief  从DHT11读取一次温湿度数据，包含40位数据（8位湿度整数+8位湿度小数+8位温度整数+8位温度小数+8位校验和）
  * @param  temp 指向温度值的指针
  * @param  humi 指向湿度值的指针
  * @retval 0表示读取成功，1表示读取失败
  */
u8 DHT11_Read_Data(u8 *temp,u8 *humi)   
{       
 
	u8 buf[5]; // 接收缓冲区：湿度整数、湿度小数、温度整数、温度小数、校验和
	u8 i; // 循环计数变量
	DHT11_Rst(); // 发送复位信号
	if(DHT11_Check()==0) // 检测DHT11响应
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte(); // 逐字节读取并存入缓冲区
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4]) // 校验和验证：前4字节之和等于第5字节
		{
			*humi=buf[0]; // 取湿度整数部分
			*temp=buf[2]; // 取温度整数部分
		}
	}
	else return 1; // DHT11未响应，返回失败
	return 0;	     // 读取成功
}

//初始化DHT11的IO口 DQ 同时检测DHT11的存在
//返回1:不存在
//返回0:存在   	 
/**
  * @brief  初始化DHT11的GPIO引脚，并发送复位信号检测传感器是否存在
  * @param  无
  * @retval 0表示DHT11存在，1表示不存在
  */
u8 DHT11_Init(void)
{	 
 
	GPIO_InitTypeDef  GPIO_InitStructure;	
	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);	 //使能PA端口时钟
	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;				 //PG11端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速率50MHz
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);				 //初始化IO口
	GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);							 //PG11 输出高 释放总线
		    
	DHT11_Rst();  //复位DHT11 发送起始信号
	return DHT11_Check();//等待DHT11的回应 检测传感器是否存在
} 

/**
  * @brief  切换DHT11数据引脚的输入/输出方向模式
  * @param  mode 1表示输出模式，0表示浮空输入模式
  * @retval 无
  */
void DHT11_Mode(u8 mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	if(mode) // 输出模式
	{
		GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN; // DHT11数据引脚
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速率50MHz
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出模式
	}
	else // 输入模式
	{
		GPIO_InitStructure.GPIO_Pin =  DHT11_GPIO_PIN; // DHT11数据引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入模式
	}
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure); // 重新初始化GPIO引脚
}
