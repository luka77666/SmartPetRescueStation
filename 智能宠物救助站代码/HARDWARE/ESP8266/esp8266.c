#include "esp8266.h"           // ESP8266 WiFi模块驱动头文件
#include "common.h"             // 通用工具函数（printf重定向等）
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "delay.h"              // 延时函数

volatile uint8_t ucTcpClosedFlag = 0;  // TCP连接关闭标志

char cStr [ 1500 ] = { 0 };    // ESP8266接收缓冲区（大数组暂未使用）
static void                   ESP8266_GPIO_Config                 ( void );   // GPIO配置函数声明
static void                   ESP8266_USART_Config                ( void );   // 串口配置函数声明
static void                   ESP8266_USART_NVIC_Configuration    ( void );   // NVIC中断配置函数声明
struct  STRUCT_USARTx_Fram strEsp8266_Fram_Record = { 0 };  // ESP8266接收帧结构体（存放收到的数据）


/**
  * @brief  ESP8266 STA模式TCP客户端初始化（旧版，当前使用AP模式）
  * @param  无
  * @retval 无
  * 流程：AT测试 → STA模式 → 连接路由器 → 开多连接 → 开TCP服务器端口5000
  */
void ESP8266_StaTcpClient ( void )
{
	printf ( "\r\n正在配置 ESP8266 ......\r\n" );

	macESP8266_CH_ENABLE();      // 使能ESP8266 CH_PD引脚

	ESP8266_AT_Test ();          // AT通信测试

	ESP8266_Net_Mode_Choose ( STA );  // 选择STA模式（连接路由器）

  while ( ! ESP8266_JoinAP ( macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd ) );  // 连接WiFi热点
	ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 1000 );  // 查询IP地址
	ESP8266_Cmd ( "AT+CIPMUX=1", "OK", 0, 1000 );  // 开启多连接模式

	ESP8266_Cmd ( "AT+CIPSERVER=1,5000", "OK", 0, 1000 );  // 开TCP服务器端口5000

	printf( "\r\n配置 ESP8266 完毕\r\n" );
}


/**
  * @brief  ESP8266初始化函数（当前版本仅配置串口通信）
  * @param  无
  * @retval 无
  */
void ESP8266_Init ( void )
{
	ESP8266_USART_Config ();     // 配置ESP8266通信串口
}


/**
  * @brief  初始化ESP8266用到的GPIO引脚（CH_PD和RST）
  * @param  无
  * @retval 无
  */
static void ESP8266_GPIO_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* 配置 CH_PD 引脚（ESP8266使能/复位引脚）*/
	macESP8266_CH_PD_APBxClock_FUN ( macESP8266_CH_PD_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = macESP8266_CH_PD_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init ( macESP8266_CH_PD_PORT, & GPIO_InitStructure );

	/* 配置 RST 引脚（ESP8266复位引脚）*/
	macESP8266_RST_APBxClock_FUN ( macESP8266_RST_CLK, ENABLE );

	GPIO_InitStructure.GPIO_Pin = macESP8266_RST_PIN;
	GPIO_Init ( macESP8266_RST_PORT, & GPIO_InitStructure );
}


/**
  * @brief  初始化ESP8266通信串口（USART2）
  * @param  无
  * @retval 无
  * 配置：8位数据位，1位停止位，无校验，使能接收和空闲中断
  */
static void ESP8266_USART_Config ( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 使能USART和GPIO时钟
	macESP8266_USART_APBxClock_FUN ( macESP8266_USART_CLK, ENABLE );
	macESP8266_USART_GPIO_APBxClock_FUN ( macESP8266_USART_GPIO_CLK, ENABLE );

	// USART TX引脚配置（复用推挽输出）
	GPIO_InitStructure.GPIO_Pin =  macESP8266_USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(macESP8266_USART_TX_PORT, &GPIO_InitStructure);

	// USART RX引脚配置（浮空输入）
	GPIO_InitStructure.GPIO_Pin = macESP8266_USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(macESP8266_USART_RX_PORT, &GPIO_InitStructure);

	// USART参数配置：波特率、8位数据、1位停止、无校验、收发模式
	USART_InitStructure.USART_BaudRate = macESP8266_USART_BAUD_RATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(macESP8266_USARTx, &USART_InitStructure);

	// 使能串口接收中断和总线空闲中断（空闲中断用于判断一帧数据接收完成）
	USART_ITConfig ( macESP8266_USARTx, USART_IT_RXNE, ENABLE );  // 接收中断
	USART_ITConfig ( macESP8266_USARTx, USART_IT_IDLE, ENABLE );  // 空闲中断

	ESP8266_USART_NVIC_Configuration ();  // 配置NVIC中断优先级

	USART_Cmd(macESP8266_USARTx, ENABLE); // 使能串口
}


/**
  * @brief  配置ESP8266串口的NVIC中断优先级
  * @param  无
  * @retval 无
  */
static void ESP8266_USART_NVIC_Configuration ( void )
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig ( macNVIC_PriorityGroup_x );  // 设置中断优先级分组

	// 使能ESP8266串口中断
	NVIC_InitStructure.NVIC_IRQChannel = macESP8266_USART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  // 抢占优先级0（最高）
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        // 子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  硬件复位ESP8266模块
  * @param  无
  * @retval 无
  * 通过RST引脚低电平500ms然后拉高实现复位
  */
void ESP8266_Rst ( void )
{
	#if 0
	 ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );    // AT指令复位（备用方案）
	#else
	 macESP8266_RST_LOW_LEVEL();   // RST拉低
	 delay_ms ( 500 );             // 保持500ms
	 macESP8266_RST_HIGH_LEVEL();  // RST拉高（复位完成）
	#endif
}


/**
  * @brief  发送AT指令并等待响应
  * @param  cmd: 要发送的AT指令字符串
  * @param  reply1: 期望的响应1（与reply2为或关系）
  * @param  reply2: 期望的响应2（NULL表示不需要此响应）
  * @param  waittime: 等待响应的超时时间(ms)
  * @retval 1=发送成功(收到期望响应), 0=发送失败
  */
bool ESP8266_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime )
{
	strEsp8266_Fram_Record .InfBit .FramLength = 0;  // 清零接收长度（准备接收新数据）

	macESP8266_Usart ( "%s\r\n", cmd );  // 通过串口发送AT指令

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )  // 不需要接收响应
		return true;

	delay_ms( waittime );          // 等待ESP8266响应

	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ]  = '\0';  // 字符串结尾

	macPC_Usart ( "%s", strEsp8266_Fram_Record .Data_RX_BUF );  // 回显响应到调试串口

	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )  // 两个响应都要检查（或逻辑）
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) ||
						 ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );

	else if ( reply1 != 0 )       // 只检查reply1
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) );

	else                          // 只检查reply2
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );
}


/**
  * @brief  AT通信测试（确认ESP8266模块正常响应）
  * @param  无
  * @retval 无
  * 最多重试10次，每次发送"AT"等待500ms
  */
void ESP8266_AT_Test ( void )
{
	char count=0;
	macESP8266_RST_HIGH_LEVEL();  // 确保RST为高电平
	delay_ms( 1000 );             // 等待模块启动
	while ( count < 10 )          // 最多尝试10次
	{
		if( ESP8266_Cmd ( "AT", "OK", NULL, 500 ) ) return;  // 收到OK则测试通过
		++ count;
	}
}


/**
  * @brief  选择ESP8266工作模式
  * @param  enumMode: STA=连接路由器, AP=开热点, STA_AP=混合模式
  * @retval 1=成功, 0=失败
  */
bool ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return ESP8266_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 );  // Station模式

	  case AP:
		  return ESP8266_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 );  // AP热点模式

		case STA_AP:
		  return ESP8266_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 );  // AP+STA混合模式

	  default:
		  return false;
	}
}


/**
  * @brief  ESP8266连接外部WiFi路由器（STA模式用）
  * @param  pSSID: WiFi名称
  * @param  pPassWord: WiFi密码
  * @retval 1=连接成功, 0=连接失败
  */
bool ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];
	bool result;

	sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );  // 拼接连接指令

	printf("\r\n[DEBUG] JoinAP cmd: %s\r\n", cCmd);
	result = ESP8266_Cmd ( cCmd, "OK", NULL, 20000 );  // 发送，超时20秒
	printf("[DEBUG] JoinAP result: %d, resp: %s\r\n", result, strEsp8266_Fram_Record.Data_RX_BUF);

	return result;
}


/**
  * @brief  ESP8266创建WiFi热点（AP模式用）
  * @param  pSSID: 热点名称
  * @param  pPassWord: 热点密码
  * @param  enunPsdMode: 加密方式（0=OPEN, 2=WPA2_PSK, 3=WPA2_WPA_PSK）
  * @retval 1=成功, 0=失败
  */
bool ESP8266_BuildAP ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWSAP=\"%s\",\"%s\",5,%d", pSSID, pPassWord, enunPsdMode );  // 通道5

	return ESP8266_Cmd ( cCmd, "OK", 0, 1000 );
}


/**
  * @brief  开启/关闭ESP8266多连接模式
  * @param  enumEnUnvarnishTx: ENABLE=多连接, DISABLE=单连接
  * @retval 1=成功, 0=失败
  */
bool ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	return ESP8266_Cmd ( "AT+CIPMUX=%d", "OK", 0, 500 );
}


/**
  * @brief  ESP8266作为客户端连接外部TCP/UDP服务器
  * @param  enumE: TCP或UDP协议
  * @param  ip: 服务器IP地址
  * @param  ComNum: 服务器端口号
  * @param  id: 连接ID号（多连接模式下使用）
  * @retval 1=连接成功, 0=失败
  */
bool ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

  switch (  enumE )
  {
		case enumTCP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
		  break;

		case enumUDP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
		  break;

		default:
			break;
  }

  if ( id < 5 )                  // 多连接模式：AT+CIPSTART=id,...
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

  else                          // 单连接模式：AT+CIPSTART=...
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );

	return ESP8266_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );
}


/**
  * @brief  开启或关闭ESP8266的TCP服务器
  * @param  enumMode: ENABLE=开启, DISABLE=关闭
  * @param  pPortNum: 端口号字符串（如"5000"）
  * @param  pTimeOver: 服务器超时时间（秒）
  * @retval 1=成功, 0=失败
  */
bool ESP8266_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
	char cCmd1 [120], cCmd2 [120];

	if ( enumMode )
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );  // 开服务器
		sprintf ( cCmd2, "AT+CIPSTO=%s", pTimeOver );           // 设置超时

		return ( ESP8266_Cmd ( cCmd1, "OK", 0, 500 ) &&
						 ESP8266_Cmd ( cCmd2, "OK", 0, 500 ) );
	}
	else
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );  // 关服务器

		return ESP8266_Cmd ( cCmd1, "OK", 0, 500 );
	}
}


/**
  * @brief  获取ESP8266的WiFi连接状态（单端口模式）
  * @param  无
  * @retval 2=获得IP(已连接), 3=已建立连接, 4=断开连接, 0=获取失败
  */
uint8_t ESP8266_Get_LinkStatus ( void )
{
	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:2\r\n" ) )
			return 2;              // 获得IP
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:3\r\n" ) )
			return 3;              // 已连接
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:4\r\n" ) )
			return 4;              // 断开连接
	}

	return 0;
}


/**
  * @brief  获取ESP8266多端口连接状态（低5位对应ID0~4）
  * @param  无
  * @retval 低5位为有效位，某位为1表示该ID已建立连接
  */
uint8_t ESP8266_Get_IdLinkStatus ( void )
{
	uint8_t ucIdLinkStatus = 0x00;

	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:0," ) )
			ucIdLinkStatus |= 0x01;   // ID0已连接
		else
			ucIdLinkStatus &= ~ 0x01;

		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:1," ) )
			ucIdLinkStatus |= 0x02;   // ID1已连接
		else
			ucIdLinkStatus &= ~ 0x02;

		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:2," ) )
			ucIdLinkStatus |= 0x04;   // ID2已连接
		else
			ucIdLinkStatus &= ~ 0x04;

		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:3," ) )
			ucIdLinkStatus |= 0x08;   // ID3已连接
		else
			ucIdLinkStatus &= ~ 0x08;

		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:4," ) )
			ucIdLinkStatus |= 0x10;   // ID4已连接
		else
			ucIdLinkStatus &= ~ 0x10;
	}

	return ucIdLinkStatus;
}


/**
  * @brief  获取ESP8266的AP模式IP地址
  * @param  pApIp: 存放AP IP的数组
  * @param  ucArrayLength: 数组长度
  * @retval 0=获取失败, 1=获取成功
  */
uint8_t ESP8266_Inquire_ApIp ( char * pApIp, uint8_t ucArrayLength )
{
	char uc;
	char * pCh;

  ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 500 );  // 查询IP

	pCh = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "APIP,\"" );  // 查找AP IP字段

	if ( pCh )
		pCh += 6;                  // 跳过 "APIP,""
	else
		return 0;                 // 未找到

	for ( uc = 0; uc < ucArrayLength; uc ++ )  // 逐字符复制IP地址
	{
		pApIp [ uc ] = * ( pCh + uc);

		if ( pApIp [ uc ] == '\"' )  // 遇到引号结束
		{
			pApIp [ uc ] = '\0';
			break;
		}
	}

	return 1;
}


/**
  * @brief  退出ESP8266透传模式（发送+++退出）
  * @param  无
  * @retval 无
  */
void ESP8266_ExitUnvarnishSend ( void )
{
	delay_ms( 1000 );             // 等待1秒
	macESP8266_Usart ( "+++" );  // 发送退出透传命令
	delay_ms( 500 );
}


/**
  * @brief  ESP8266发送字符串（支持透传和普通模式）
  * @param  enumEnUnvarnishTx: 是否已使能透传模式
  * @param  pStr: 要发送的字符串
  * @param  ulStrLength: 字符串长度
  * @param  ucId: 发送ID号（多连接模式）
  * @retval 1=成功, 0=失败
  */
bool ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;

	if ( enumEnUnvarnishTx )      // 透传模式：直接发数据
	{
		macESP8266_Usart ( "%s", pStr );
		bRet = true;
	}
	else                         // 普通模式：先发AT+CIPSEND指令
	{
		if ( ucId < 5 )            // 多连接模式
			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );
		else                      // 单连接模式
			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );

		ESP8266_Cmd ( cStr, "> ", 0, 200 );  // 等待>提示符
  }

	return bRet;
}


/**
  * @brief  ESP8266接收字符串（阻塞等待一帧数据）
  * @param  enumEnUnvarnishTx: 是否透传模式
  * @retval 接收到的字符串首地址
  */
char * ESP8266_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
	char * pRecStr = 0;

	strEsp8266_Fram_Record .InfBit .FramLength = 0;  // 清零长度
	strEsp8266_Fram_Record .InfBit .FramFinishFlag = 0;  // 清完成标志

	while ( ! strEsp8266_Fram_Record .InfBit .FramFinishFlag );  // 阻塞等待接收完成
	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';

	if ( enumEnUnvarnishTx )      // 透传模式：直接返回数据
		pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;
	else                         // 普通模式：检查+IPD前缀
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+IPD" ) )
			pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;
	}

	return pRecStr;
}


/**
  * @brief  通过ESP8266 TCP发送数据给APP（本项目主要使用此函数）
  * @param  data: 数据缓冲区指针
  * @param  len: 数据长度
  * @retval 无
  * 发送流程：AT+CIPSEND=0,len → 等待>提示 → 发送数据
  */
void ESP8266_SendData(unsigned char *data, unsigned short len)
{
	char cmdBuf[32];

	sprintf(cmdBuf, "AT+CIPSEND=0,%d\r\n", len);  // 拼接发送指令（ID=0）

	while(ESP8266_Cmd(cmdBuf, ">", 0, 200));      // 等待>提示符
	delay_ms(500);

	Usart_SendString(data , len);  // 发送实际数据（数据帧）
}

/**
  * @brief  清空ESP8266接收缓冲区
  * @param  无
  * @retval 无
  */
void ESP8266_Clear(void)
{
	memset(Res, 0, sizeof(Res));  // 清空接收缓冲区
	ESP8266_CNT = 0;              // 重置接收计数器
}

/**
  * @brief  在接收缓冲区中检索APP命令（非阻塞轮询）
  * @param  res: 要检索的关键词（如"KA"、"MF0"等）
  * @param  time: 轮询超时次数（每次1ms）
  * @retval 1=找到关键词, 0=超时未找到
  * 找到后自动清空缓冲区
  */
_Bool BlueCmd(char *res, u16 time)
{
	while(time--)
	{
		if(strstr((const char *)Res, res) != NULL)  // 如果检索到关键词
		{
			ESP8266_Clear();       // 清空缓存（防止重复触发）
			return 1;
		}

		delay_ms(1);               // 每1ms轮询一次
	}

	return 0;
}
