#ifndef __ESP8266_H
#define	__ESP8266_H

#include "stm32f10x.h"
#include "common.h"
#include <stdio.h>
#include <stdbool.h>
#include "usart2.h"


#if defined ( __CC_ARM   )
#pragma anon_unions  // 支持匿名联合体（ARM编译器）
#endif

#define  ESP8266_CNT         RxCounter // ESP8266接收计数器别名
extern u8 Res[USART2_RXBUFF_SIZE];     // 串口2接收缓冲区
extern unsigned int RxCounter;         // 接收数据计数器

/******************************* ESP8266 数据类型定义 ***************************/
typedef enum{
	STA,       // Station模式（连接路由器）
  AP,        // Access Point模式（热点模式）
  STA_AP     // 同时启用STA和AP模式
} ENUM_Net_ModeTypeDef; // 网络工作模式枚举


typedef enum{
	 enumTCP,  // TCP协议
	 enumUDP,  // UDP协议
} ENUM_NetPro_TypeDef; // 网络协议类型枚举


typedef enum{
	Multiple_ID_0 = 0, // 多连接ID 0
	Multiple_ID_1 = 1, // 多连接ID 1
	Multiple_ID_2 = 2, // 多连接ID 2
	Multiple_ID_3 = 3, // 多连接ID 3
	Multiple_ID_4 = 4, // 多连接ID 4
	Single_ID_0 = 5,   // 单连接ID
} ENUM_ID_NO_TypeDef; // 连接ID枚举


typedef enum{
	OPEN = 0,          // 开放（无加密）
	WEP = 1,           // WEP加密
	WPA_PSK = 2,       // WPA-PSK加密
	WPA2_PSK = 3,      // WPA2-PSK加密
	WPA_WPA2_PSK = 4,  // WPA/WPA2混合加密
} ENUM_AP_PsdMode_TypeDef; // AP加密模式枚举



/******************************* ESP8266 外部全局变量声明 ***************************/
#define RX_BUF_MAX_LEN     1024                                     //最大接收缓存字节数

extern struct  STRUCT_USARTx_Fram                                  //串口数据帧的处理结构体
{
	char  Data_RX_BUF [ RX_BUF_MAX_LEN ]; // 数据接收缓冲区

  union {
    __IO u16 InfAll;   // 帧信息整体（16位）
    struct {
		  __IO u16 FramLength       :15;                               // 14:0  帧长度
		  __IO u16 FramFinishFlag   :1;                                // 15   帧接收完成标志
	  } InfBit;
  };

} strEsp8266_Fram_Record; // 串口数据帧结构体实例



/******************************** ESP8266 连接引脚定义 ***********************************/
#define      macESP8266_CH_PD_APBxClock_FUN                   RCC_APB2PeriphClockCmd // CH_PD引脚时钟使能函数
#define      macESP8266_CH_PD_CLK                             RCC_APB2Periph_GPIOA  // CH_PD引脚时钟
#define      macESP8266_CH_PD_PORT                            GPIOA                 // CH_PD引脚所在端口
#define      macESP8266_CH_PD_PIN                             GPIO_Pin_5             // CH_PD引脚号

#define      macESP8266_RST_APBxClock_FUN                     RCC_APB2PeriphClockCmd // RST引脚时钟使能函数
#define      macESP8266_RST_CLK                               RCC_APB2Periph_GPIOA  // RST引脚时钟
#define      macESP8266_RST_PORT                              GPIOA                 // RST引脚所在端口
#define      macESP8266_RST_PIN                               GPIO_Pin_6             // RST引脚号


#define      macESP8266_USART_BAUD_RATE                       115200 // ESP8266串口波特率

#define      macESP8266_USARTx                                USART2 // ESP8266使用的串口
#define      macESP8266_USART_APBxClock_FUN                   RCC_APB1PeriphClockCmd // 串口APB时钟使能函数
#define      macESP8266_USART_CLK                             RCC_APB1Periph_USART2  // 串口时钟
#define      macESP8266_USART_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd // 串口GPIO时钟使能函数
#define      macESP8266_USART_GPIO_CLK                        RCC_APB2Periph_GPIOA   // 串口GPIO时钟
#define      macESP8266_USART_TX_PORT                         GPIOA                  // 串口TX引脚端口
#define      macESP8266_USART_TX_PIN                          GPIO_Pin_2              // 串口TX引脚号
#define      macESP8266_USART_RX_PORT                         GPIOA                  // 串口RX引脚端口
#define      macESP8266_USART_RX_PIN                          GPIO_Pin_3              // 串口RX引脚号
#define      macESP8266_USART_IRQ                             USART2_IRQn            // 串口中断号
#define      macESP8266_USART_INT_FUN                         USART2_IRQHandler      // 串口中断服务函数



/*********************************************** ESP8266 函数宏定义 *******************************************/
#define     macESP8266_Usart( fmt, ... )           USART_printf ( macESP8266_USARTx, fmt, ##__VA_ARGS__ )  // ESP8266串口格式化发送
#define     macPC_Usart( fmt, ... )                printf ( fmt, ##__VA_ARGS__ )  // PC串口格式化发送
//#define     macPC_Usart( fmt, ... )

#define     macESP8266_CH_ENABLE()                 GPIO_SetBits ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN )  // 使能ESP8266芯片
#define     macESP8266_CH_DISABLE()                GPIO_ResetBits ( macESP8266_CH_PD_PORT, macESP8266_CH_PD_PIN ) // 失能ESP8266芯片

#define     macESP8266_RST_HIGH_LEVEL()            GPIO_SetBits ( macESP8266_RST_PORT, macESP8266_RST_PIN )  // ESP8266复位引脚拉高
#define     macESP8266_RST_LOW_LEVEL()             GPIO_ResetBits ( macESP8266_RST_PORT, macESP8266_RST_PIN ) // ESP8266复位引脚拉低



/****************************************** ESP8266 函数声明 ***********************************************/
void                     ESP8266_Init                        ( void ); // ESP8266初始化
void                     ESP8266_Rst                         ( void ); // ESP8266复位
bool                     ESP8266_Cmd                         ( char * cmd, char * reply1, char * reply2, u32 waittime ); // 发送AT指令并等待应答
void                     ESP8266_AT_Test                     ( void ); // AT指令测试
bool                     ESP8266_Net_Mode_Choose             ( ENUM_Net_ModeTypeDef enumMode ); // 选择网络工作模式
bool                     ESP8266_JoinAP                      ( char * pSSID, char * pPassWord ); // 连接WiFi热点
bool                     ESP8266_BuildAP                     ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode ); // 创建AP热点
bool                     ESP8266_Enable_MultipleId           ( FunctionalState enumEnUnvarnishTx ); // 使能多连接ID
bool                     ESP8266_Link_Server                 ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id); // 连接服务器
bool                     ESP8266_StartOrShutServer           ( FunctionalState enumMode, char * pPortNum, char * pTimeOver ); // 开启或关闭服务器
uint8_t                  ESP8266_Get_LinkStatus              ( void ); // 获取网络连接状态
uint8_t                  ESP8266_Get_IdLinkStatus            ( void ); // 获取指定ID的连接状态
uint8_t                  ESP8266_Inquire_ApIp                ( char * pApIp, uint8_t ucArrayLength ); // 查询AP的IP地址
bool                     ESP8266_UnvarnishSend               ( void ); // 进入透传模式
void                     ESP8266_ExitUnvarnishSend           ( void ); // 退出透传模式
bool                     ESP8266_SendString                  ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId ); // 发送字符串
char *                   ESP8266_ReceiveString               ( FunctionalState enumEnUnvarnishTx ); // 接收字符串
void                     ESP8266_SendData                    (unsigned char *data, unsigned short len); // 发送数据
_Bool                    BlueCmd                             (char *res, u16 time); // 蓝牙命令处理
void ESP8266_Clear(void); // 清除ESP8266接收缓冲区

/********************************** 用户需要设置的参数**********************************/
#define      macUser_ESP8266_ApSsid                       "LLLLLL"          //要连接的路由器WiFi名称
#define      macUser_ESP8266_ApPwd                        "zhku.7111"         //要连接的路由器WiFi密码

//#define      macUser_ESP8266_TcpServer_IP                 "192.168.0.11"      //要连接的服务器的 IP
//#define      macUser_ESP8266_TcpServer_Port               "8080"               //要连接的服务器的端口



/********************************** 外部全局变量 ***************************************/
extern volatile uint8_t ucTcpClosedFlag; // TCP连接断开标志



/********************************** 测试函数声明 ***************************************/
void                     ESP8266_StaTcpClient  ( void ); // STA模式TCP客户端测试函数


#endif /* __ESP8266_H */




