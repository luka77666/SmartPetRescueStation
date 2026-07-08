
#ifndef __TFTLCD_H
#define __TFTLCD_H

#include "sys.h"
#include "stdlib.h"

// LCD重要参数集
typedef struct
{
    u16 width;         // LCD 宽度
    u16 height;        // LCD 高度
    u16 id;            // LCD ID
    u8  dir;           // 横屏还是竖屏控制：0，竖屏；1，横屏
    u16 wramcmd;       // 开始写gram指令
    u16 setxcmd;       // 设置x坐标指令
    u16 setycmd;       // 设置y坐标指令
} _lcd_dev; // LCD设备参数结构体

// LCD参数
extern _lcd_dev lcddev; // LCD设备参数全局变量

// 扫描方向定义
#define L2R_U2D  0 // 从左到右,从上到下
#define L2R_D2U  1 // 从左到右,从下到上
#define R2L_U2D  2 // 从右到左,从上到下
#define R2L_D2U  3 // 从右到左,从下到上

#define U2D_L2R  4 // 从上到下,从左到右
#define U2D_R2L  5 // 从上到下,从右到左
#define D2U_L2R  6 // 从下到上,从左到右
#define D2U_R2L  7 // 从下到上,从右到左

// 画笔颜色
#define WHITE          0xFFFF // 白色
#define BLACK          0x0000 // 黑色
#define BLUE           0x001F // 蓝色
#define RED            0xF800 // 红色
#define GREEN          0x07E0 // 绿色
#define YELLOW         0xFFE0 // 黄色
#define CYAN           0x07FF // 青色
#define MAGENTA        0xF81F // 品红色
#define LIGHTBLUE      0x7FFF // 浅蓝色
#define LIGHTGREEN     0xAFE0 // 浅绿色
#define LIGHTRED       0xFC10 // 浅红色
#define GRAY           0x8430 // 灰色
#define GRAY175        0xAD75 // 175级灰度
#define GRAY187        0xBDF7 // 187级灰度
#define GRAY205        0xF7DE // 205级灰度

// LCD背光控制
#define LCD_LED_ON     GPIO_SetBits(GPIOA,GPIO_Pin_8) // LCD背光开启
#define LCD_LED_OFF    GPIO_ResetBits(GPIOA,GPIO_Pin_8) // LCD背光关闭

// LCD复位
#define LCD_RST_ON     GPIO_SetBits(GPIOD,GPIO_Pin_3) // LCD复位拉高
#define LCD_RST_OFF    GPIO_ResetBits(GPIOD,GPIO_Pin_3) // LCD复位拉低

// 函数声明
void LCD_Init(void); // LCD初始化
void LCD_Clear(u16 Color); // 全屏清除（填充指定颜色）
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 color); // 指定区域填充颜色
void LCD_DrawPoint(u16 x,u16 y); // 画一个点
void LCD_DrawPoint_big(u16 x,u16 y); // 画一个大点
void LCD_ShowChar(u16 x,u16 y,u8 chr,u8 size,u8 mode); // 显示一个字符
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 *str); // 显示字符串
u32 LCD_Pow(u8 m,u8 n); // 求幂函数
void LCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size); // 显示数字
void LCD_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode); // 显示指定进制数字
void LCD_ShowString(u16 x,u16 y,u16 width,u16 height,u8 *str); // 显示字符串
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue); // 写寄存器
u16 LCD_ReadReg(u16 LCD_Reg); // 读寄存器
void LCD_WriteRAM_Prepare(void); // 准备写入GRAM
void LCD_WriteRAM(u16 RGB_Code); // 写入GRAM数据
u16 LCD_ReadRAM(void); // 读取GRAM数据
void LCD_SetCursor(u16 Xpos, u16 Ypos); // 设置光标位置
void LCD_Display_Dir(u8 dir); // 设置显示方向

// 兼容OLED接口的函数
void TFTLCD_Init(void); // TFTLCD初始化（兼容OLED接口）
void TFTLCD_ShowString(u16 x,u16 y,u8 *str,u8 size1,u8 mode); // 显示字符串（兼容OLED接口）
void TFTLCD_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size1,u8 mode); // 显示数字（兼容OLED接口）
void TFTLCD_ShowChar(u16 x,u16 y,u8 chr,u8 size1,u8 mode); // 显示字符（兼容OLED接口）
void TFTLCD_Clear(u16 color); // 清屏（兼容OLED接口）

#endif
