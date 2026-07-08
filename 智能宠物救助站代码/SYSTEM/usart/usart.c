/***************STM32F103C8T6**********************
 * 文件名  ：usart1.c
 * 描述    ：将printf函数重定向到USART1。
 * 硬件连接：------------------------
 *          | PA9  - USART1(Tx)      |
 *          | PA10 - USART1(Rx)      |
 *           ------------------------
 * 库版本  ：ST3.0.0  *

********************LIGEN*************************/

#include "usart.h" // 串口1驱动头文件
#include <stdarg.h> // 可变参数函数头文件（printf重定向需要）


void USART1_Config(void) // 串口1初始化配置函数
{
	GPIO_InitTypeDef GPIO_InitStructure; // 定义GPIO初始化结构体
	USART_InitTypeDef USART_InitStructure; // 定义串口初始化结构体

	/* 使能 USART1 时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // 使能USART1和GPIOA时钟

	/* USART1 使用IO端口配置 */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // 选择PA9引脚（USART1_TX）
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // GPIO速度50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // 选择PA10引脚（USART1_RX）
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);   //初始化GPIOA


	/* USART1 工作模式配置 */
	USART_InitStructure.USART_BaudRate = 115200;	//波特率设置：115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//数据位数设置：8位
	USART_InitStructure.USART_StopBits = USART_StopBits_1; 	//停止位设置：1位
	USART_InitStructure.USART_Parity = USART_Parity_No ;  //是否奇偶校验：无
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制模式设置：没有使能
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//接收与发送都使能
	USART_Init(USART1, &USART_InitStructure);  //初始化USART1
	USART_Cmd(USART1, ENABLE);// USART1使能
}


 /* 描述  ：重定向c库函数printf到USART1*/
int fputc(int ch, FILE *f)
{
/* 将Printf内容发往串口 */
  USART_SendData(USART1, (unsigned char) ch); // 发送一个字节数据到串口1
  while (!(USART1->SR & USART_FLAG_TXE)); // 等待发送数据寄存器为空

  return (ch); // 返回发送的字符
}


