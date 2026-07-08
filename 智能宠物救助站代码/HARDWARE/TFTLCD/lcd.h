#ifndef __LCD_H
#define __LCD_H
#include "sys.h"
#include "stdlib.h"

/***********************************
											STM32
 * 文件			:	TFT-LCD显示屏(1.8寸)h文件
 * 版本			: V1.0
 * 日期			: 2024.9.13
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码

*********************************************/

//----------------OLED端口定义-----------------
/***************根据自己需求更改****************/
#define LCD_SCL_GPIO_PORT				GPIOB // LCD时钟(SCL)引脚端口
#define LCD_SCL_GPIO_PIN				GPIO_Pin_7 // LCD时钟(SCL)引脚号

#define LCD_SDA_GPIO_PORT				GPIOB // LCD数据(SDA)引脚端口
#define LCD_SDA_GPIO_PIN				GPIO_Pin_6 // LCD数据(SDA)引脚号

#define LCD_RST_GPIO_PORT				GPIOB // LCD复位(RST)引脚端口
#define LCD_RST_GPIO_PIN				GPIO_Pin_5 // LCD复位(RST)引脚号

#define LCD_DC_GPIO_PORT				GPIOB // LCD数据/命令(DC)引脚端口
#define LCD_DC_GPIO_PIN					GPIO_Pin_4 // LCD数据/命令(DC)引脚号

#define LCD_CS_GPIO_PORT				GPIOB // LCD片选(CS)引脚端口
#define LCD_CS_GPIO_PIN					GPIO_Pin_3 // LCD片选(CS)引脚号

#define LCD_BLK_GPIO_PORT				GPIOA // LCD背光(BLK)引脚端口
#define LCD_BLK_GPIO_PIN				GPIO_Pin_8 // LCD背光(BLK)引脚号

/*********************END**********************/


#define LCD_SCLK_Clr() GPIO_ResetBits(LCD_SCL_GPIO_PORT,LCD_SCL_GPIO_PIN)//SCL=SCLK 时钟拉低
#define LCD_SCLK_Set() GPIO_SetBits(LCD_SCL_GPIO_PORT,LCD_SCL_GPIO_PIN) // 时钟拉高

#define LCD_MOSI_Clr() GPIO_ResetBits(LCD_SDA_GPIO_PORT,LCD_SDA_GPIO_PIN)//SDA=MOSI 数据拉低
#define LCD_MOSI_Set() GPIO_SetBits(LCD_SDA_GPIO_PORT,LCD_SDA_GPIO_PIN) // 数据拉高

#define LCD_RES_Clr()  GPIO_ResetBits(LCD_RST_GPIO_PORT,LCD_RST_GPIO_PIN)//RES 复位拉低
#define LCD_RES_Set()  GPIO_SetBits(LCD_RST_GPIO_PORT,LCD_RST_GPIO_PIN) // 复位拉高

#define LCD_DC_Clr()   GPIO_ResetBits(LCD_DC_GPIO_PORT,LCD_DC_GPIO_PIN)//DC 命令模式拉低
#define LCD_DC_Set()   GPIO_SetBits(LCD_DC_GPIO_PORT,LCD_DC_GPIO_PIN) // DC 数据模式拉高

#define LCD_CS_Clr()   GPIO_ResetBits(LCD_CS_GPIO_PORT,LCD_CS_GPIO_PIN)//CS 片选拉低（选中）
#define LCD_CS_Set()   GPIO_SetBits(LCD_CS_GPIO_PORT,LCD_CS_GPIO_PIN) // 片选拉高（取消选中）

#define LCD_BLK_Clr()  GPIO_ResetBits(LCD_BLK_GPIO_PORT,LCD_BLK_GPIO_PIN)//BLK 背光关闭
#define LCD_BLK_Set()  GPIO_SetBits(LCD_BLK_GPIO_PORT,LCD_BLK_GPIO_PIN) // 背光打开


#define USE_HORIZONTAL 1  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128 // 竖屏宽度
#define LCD_H 160 // 竖屏高度

#else
#define LCD_W 160 // 横屏宽度
#define LCD_H 128 // 横屏高度
#endif

//画笔颜色
#define WHITE         	 0xFFFF // 白色
#define BLACK         	 0x0000 // 黑色
#define BLUE           	 0x001F // 蓝色
#define BRED             0XF81F // 蓝红色
#define GRED 			       0XFFE0 // 绿红色
#define GBLUE			       0X07FF // 绿蓝色
#define RED           	 0xF800 // 红色
#define MAGENTA       	 0xF81F // 品红色
#define GREEN         	 0x07E0 // 绿色
#define CYAN          	 0x7FFF // 青色
#define YELLOW        	 0xFFE0 // 黄色
#define BROWN 			     0XBC40 //棕色
#define BRRED 			     0XFC07 //棕红色
#define GRAY  			     0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			     0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void LCD_Init(void);//LCD初始化


void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color);//指定区域填充颜色
void LCD_DrawPoint(u16 x,u16 y,u16 color);//在指定位置画一个点
void LCD_DrawLine(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);//在指定位置画一条线
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2,u16 color);//在指定位置画一个矩形
void Draw_Circle(u16 x0,u16 y0,u8 r,u16 color);//在指定位置画一个圆

void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//显示汉字串
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//显示单个12x12汉字
void LCD_ShowChinese16x16(u16 x,u16 y,const u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//显示单个16x16汉字
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//显示单个24x24汉字
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);//显示单个32x32汉字

void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode);//显示一个字符
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode);//显示字符串
u32 mypow(u8 m,u8 n);//求幂
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey);//显示整数变量
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey);//显示两位小数变量

void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]);//显示图片

//--------OLED兼容层函数声明(供main.c调用)--------
void OLED_Clear(void); // 清屏函数
void OLED_Refresh(void); // 刷新显示函数
void OLED_ShowChinese(u16 x, u16 y, u8 num, u8 size, u8 mode); // 显示汉字
void OLED_ShowChar(u16 x, u16 y, u8 chr, u8 size, u8 mode); // 显示字符
void OLED_ShowString(u16 x, u16 y, u8 *str, u8 size, u8 mode); // 显示字符串
void OLED_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode); // 显示数字

// 带颜色参数的OLED兼容函数
void OLED_ShowChinese_Color(u16 x, u16 y, u8 num, u8 size, u8 mode, u16 fc); // 带颜色显示汉字
void OLED_ShowChar_Color(u16 x, u16 y, u8 chr, u8 size, u8 mode, u16 fc); // 带颜色显示字符
void OLED_ShowString_Color(u16 x, u16 y, u8 *str, u8 size, u8 mode, u16 fc); // 带颜色显示字符串
void OLED_ShowNum_Color(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode, u16 fc); // 带颜色显示数字

#endif
