#include "usart2.h"	// 串口2驱动头文件

unsigned int RxCounter = 0;   //串口2接收数据计数器

//串口1中断服务程序
	//初始化 IO 串口 3
	//pclk1:PCLK1 时钟频率(Mhz)
	//bound:波特率
void USART2_Config() // 串口2初始化配置函数
{
			NVIC_InitTypeDef NVIC_InitStructure; // 定义中断优先级结构体
			GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO初始化结构体
			USART_InitTypeDef USART_InitStructure; // 定义串口初始化结构体
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//GPIOB 时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口 3 时钟使 USART_DeInit(USART3); //复位串口 3
			//USART3_TX PB10
			GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2; //PB10
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度50MHz
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
			GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化 PB10

			//USART3_RX PB11
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; // 选择PA3引脚（USART2_RX）
			GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING; //浮空输入
			GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化 PB11

			USART_InitStructure.USART_BaudRate= 115200; //波特率设
			USART_InitStructure.USART_WordLength=USART_WordLength_8b; //8 位数据格
			USART_InitStructure.USART_StopBits = USART_StopBits_1; //一个停止
			USART_InitStructure.USART_Parity = USART_Parity_No; //无奇偶校验
			USART_InitStructure.USART_HardwareFlowControl=
			USART_HardwareFlowControl_None;
			//无硬件数据流
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  //收发模式
  USART_Init(USART2, &USART_InitStructure); //初始化串口 3
  USART_Cmd(USART2, ENABLE);//使能串口
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//使能接收中断
  //设置中断优先级
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; // 选择USART2中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级 2
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =0; //子优先级 3
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ 通道使能
  NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化 NVIC 寄存器
}
u8 Res[USART2_RXBUFF_SIZE]; // 串口2接收数据缓冲区
void USART2_IRQHandler(void) // 串口2中断服务函数
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
    {
        USART_ClearITPendingBit(USART2, USART_IT_RXNE); // 清除接收中断标志
        if(RxCounter >= USART2_RXBUFF_SIZE) // 缓冲区已满
        {
            memset(Res, 0, USART2_RXBUFF_SIZE); // 归零时清空缓冲区，防止旧数据残留
            RxCounter = 0; // 重置接收计数器
        }
            Res[RxCounter++] = USART_ReceiveData(USART2);//接收模块的数据
    }
}
void Usart_SendString(unsigned char *str, unsigned short len) // 串口2发送字符串函数
{
	unsigned short count = 0; // 发送字节计数器

	for(; count < len; count++) // 循环发送len个字节
	{
		USART_SendData(USART2, *str++);										//发送数据
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		//等待发送完成
	}

}
