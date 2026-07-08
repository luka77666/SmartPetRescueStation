
#include "tftlcd.h"
#include "delay.h"
#include "usart.h"

// LCD的画笔颜色和背景色
u16 POINT_COLOR = RED;   // 画笔颜色
u16 BACK_COLOR  = WHITE; // 背景色

_lcd_dev lcddev;

// 写寄存器函数
// regval:寄存器值
void LCD_WR_REG(u16 regval)
{
    LCD_RS = 0;
    LCD_CS = 0;
    DATAOUT(regval);
    LCD_WR = 0;
    LCD_WR = 1;
    LCD_CS = 1;
}

// 写LCD数据
// data:要写入的值
void LCD_WR_DATA(u16 data)
{
    LCD_RS = 1;
    LCD_CS = 0;
    DATAOUT(data);
    LCD_WR = 0;
    LCD_WR = 1;
    LCD_CS = 1;
}

// 读LCD数据
// 返回值:读到的值
u16 LCD_RD_DATA(void)
{
    u16 t;
    LCD_RS = 1;
    LCD_CS = 0;
    LCD_RD = 0;
    t = DATAIN;
    LCD_RD = 1;
    LCD_CS = 1;
    return t;
}

// 写寄存器
// LCD_Reg:寄存器地址
// LCD_RegValue:要写入的数据
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{
    LCD_WR_REG(LCD_Reg);
    LCD_WR_DATA(LCD_RegValue);
}

// 读寄存器
// LCD_Reg:寄存器地址
// 返回值:读到的数据
u16 LCD_ReadReg(u16 LCD_Reg)
{
    LCD_WR_REG(LCD_Reg);
    return LCD_RD_DATA();
}

// 开始写GRAM
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
}

// 写LCD数据
// RGB_Code:颜色值
void LCD_WriteRAM(u16 RGB_Code)
{
    LCD_WR_DATA(RGB_Code);
}

// 读LCD数据
// 返回值:读到的数据
u16 LCD_ReadRAM(void)
{
    LCD_RD_DATA(); // dummy read
    return LCD_RD_DATA();
}

// 设置光标位置
// Xpos:横坐标
// Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    if (lcddev.id == 0x9341 || lcddev.id == 0x5310 || lcddev.id == 0x5510)
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_DATA(Xpos & 0xFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0xFF);
    }
}

// 画点
// x,y:坐标
// point_color:点的颜色
void LCD_DrawPoint(u16 x, u16 y)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    LCD_WR_DATA(POINT_COLOR);
}

// 快速画点
// x,y:坐标
// color:颜色
void LCD_Fast_DrawPoint(u16 x, u16 y, u16 color)
{
    LCD_SetCursor(x, y);
    LCD_WriteRAM_Prepare();
    LCD_WR_DATA(color);
}

// 设置LCD显示方向
// dir:0,竖屏；1,横屏
void LCD_Display_Dir(u8 dir)
{
    if (dir == 0) // 竖屏
    {
        lcddev.width = 240;
        lcddev.height = 320;
        lcddev.wramcmd = 0x2C;
        lcddev.setxcmd = 0x2A;
        lcddev.setycmd = 0x2B;
    }
    else // 横屏
    {
        lcddev.width = 320;
        lcddev.height = 240;
        lcddev.wramcmd = 0x2C;
        lcddev.setxcmd = 0x2A;
        lcddev.setycmd = 0x2B;
    }
}

// 开启显示
void LCD_DisplayOn(void)
{
    if (lcddev.id == 0x9341 || lcddev.id == 0x5310 || lcddev.id == 0x5510)
    {
        LCD_WR_REG(0x29);
    }
}

// 关闭显示
void LCD_DisplayOff(void)
{
    if (lcddev.id == 0x9341 || lcddev.id == 0x5310 || lcddev.id == 0x5510)
    {
        LCD_WR_REG(0x28);
    }
}

// 设置光标位置
// Xpos:横坐标
// Ypos:纵坐标
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    if (lcddev.id == 0x9341 || lcddev.id == 0x5310 || lcddev.id == 0x5510)
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(Xpos >> 8);
        LCD_WR_DATA(Xpos & 0xFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(Ypos >> 8);
        LCD_WR_DATA(Ypos & 0xFF);
    }
}

// 清屏函数
// color:要清屏的填充色
void LCD_Clear(u16 color)
{
    u32 index = 0;
    u32 totalpoint = lcddev.width * lcddev.height;

    LCD_SetCursor(0x00, 0x0000);
    LCD_WriteRAM_Prepare();

    for (index = 0; index < totalpoint; index++)
    {
        LCD_WR_DATA(color);
    }
}

// 在指定位置填充矩形
// (x1,y1),(x2,y2):填充矩形的对角坐标
// area要填充的矩形区域
void LCD_Fill(u16 x1, u16 y1, u16 x2, u16 y2, u16 color)
{
    u16 i, j;

    LCD_SetCursor(x1, y1);
    LCD_WriteRAM_Prepare();

    for (i = y1; i <= y2; i++)
    {
        for (j = x1; j <= x2; j++)
        {
            LCD_WR_DATA(color);
        }
    }
}

// 画线
// x1,y1:起点坐标
// x2,y2:终点坐标
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;

    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)incx = 1;
    else if (delta_x == 0)incx = 0;
    else {incx = -1; delta_x = -delta_x;}

    if (delta_y > 0)incy = 1;
    else if (delta_y == 0)incy = 0;
    else {incy = -1; delta_y = -delta_y;}

    if (delta_x > delta_y)distance = delta_x;
    else distance = delta_y;

    for (t = 0; t <= distance + 1; t++)
    {
        LCD_DrawPoint(uRow, uCol);
        xerr += delta_x;
        yerr += delta_y;
        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }
        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

// 画矩形
// (x1,y1),(x2,y2):矩形的对角坐标
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_DrawLine(x1, y1, x2, y1);
    LCD_DrawLine(x1, y1, x1, y2);
    LCD_DrawLine(x1, y2, x2, y2);
    LCD_DrawLine(x2, y1, x2, y2);
}

// 在指定位置画一个指定大小的圆
// (x,y):中心点
// r:半径
void LCD_DrawCircle(u16 x0, u16 y0, u8 r)
{
    int a, b;
    int di;

    a = 0;
    b = r;
    di = 3 - (r << 1);

    while (a <= b)
    {
        LCD_DrawPoint(x0 - b, y0 - a);
        LCD_DrawPoint(x0 + b, y0 - a);
        LCD_DrawPoint(x0 - a, y0 + b);
        LCD_DrawPoint(x0 - b, y0 - a);
        LCD_DrawPoint(x0 - a, y0 - b);
        LCD_DrawPoint(x0 + b, y0 + a);
        LCD_DrawPoint(x0 + a, y0 - b);
        LCD_DrawPoint(x0 + a, y0 + b);
        LCD_DrawPoint(x0 - b, y0 + a);
        a++;
        if (di < 0)di += 4 * a + 6;
        else
        {
            di += 10 - 4 * (b--);
        }
    }
}

// 在指定位置显示一个字符
// x,y:起始坐标
// num:要显示的字符:" "--->"~"
// size:字体大小 12/16/24
// mode:叠加方式(1)还是非叠加方式(0)
void LCD_ShowChar(u16 x, u16 y, u8 num, u8 size, u8 mode)
{
    u8 temp, t1, t;
    u16 y0 = y;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size / 2); // 得到字体一个字符对应点阵集所占的字节数
    num = num - ' '; // 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库）

    for (t = 0; t < csize; t++)
    {
        if (size == 12)temp = asc2_1206[num][t];    // 调用1206字体
        else if (size == 16)temp = asc2_1608[num][t]; // 调用1608字体
        else if (size == 24)temp = asc2_2412[num][t]; // 调用2412字体
        else return; // 没有的字库

        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)LCD_Fast_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)LCD_Fast_DrawPoint(x, y, BACK_COLOR);

            temp <<= 1;
            y++;
            if (y >= lcddev.height)return; // 超区域了

            if ((y - y0) == size)
            {
                y = y0;
                x++;
                if (x >= lcddev.width)return; // 超区域了
                break;
            }
        }
    }
}

// m^n函数
// 返回值:m^n次方.
u32 LCD_Pow(u8 m, u8 n)
{
    u32 result = 1;

    while (n--)result *= m;

    return result;
}

// 显示2个数字
// x,y :起点坐标
// num :要显示的数值
// len :数字的位数
// size:字体大小
// mode:模式  0,填充模式;1,叠加模式
void LCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size)
{
    u8 t, temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                LCD_ShowChar(x + (size / 2)*t, y, ' ', size, 0);
                continue;
            }
            else enshow = 1;
        }

        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, 0);
    }
}

// 显示数字,不显示高位的0
// x,y:起点坐标
// num:数值(0~999999999);
// len:长度(即要显示的位数)
// size:字体大小
// mode:
// [7]:0,不填充;1,填充0.
// [6:1]:保留
// [0]:0,非叠加显示;1,叠加显示.
void LCD_ShowxNum(u16 x, u16 y, u32 num, u8 len, u8 size, u8 mode)
{
    u8 t, temp;
    u8 enshow = 0;

    for (t = 0; t < len; t++)
    {
        temp = (num / LCD_Pow(10, len - t - 1)) % 10;

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                if (mode & 0x80)LCD_ShowChar(x + (size / 2)*t, y, '0', size, mode & 0x01);
                else LCD_ShowChar(x + (size / 2)*t, y, ' ', size, mode & 0x01);
                continue;
            }
            else enshow = 1;
        }
        LCD_ShowChar(x + (size / 2)*t, y, temp + '0', size, mode & 0x01);
    }
}

// 显示字符串
// x,y:起点坐标
// width,height:区域大小
// str:字符串
// mode:0,非叠加模式;1,叠加模式
void LCD_ShowString(u16 x, u16 y, u16 width, u16 height, u8 *str)
{
    u16 x0 = x;
    u16 y0 = y;
    u8 bhz = 0; // 字符高度

    while (*str != 0) // 数据未结束
    {
        if (x > x0 + width || y > y0 + height) // 超出区域
        {
            return;
        }

        if (*str > 0x80) // 中文
        {
            bhz = 1;
        }
        else // ASCII
        {
            bhz = 0;
        }

        if (bhz == 0) // ASCII字符
        {
            LCD_ShowChar(x, y, *str, 16, 0);
            x += 8;
        }
        str++;
    }
}

// 兼容OLED接口的初始化函数
void TFTLCD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE, ENABLE);

    // 配置LCD控制引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置LCD控制引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LCD_LED_ON; // 打开背光
    LCD_RST_OFF; // 复位
    delay_ms(100);
    LCD_RST_ON;
    delay_ms(50);

    // 这里应该是LCD初始化序列，根据具体LCD型号配置
    // 示例使用ILI9341初始化代码
    LCD_WR_REG(0xCF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0X30);

    LCD_WR_REG(0xED);
    LCD_WR_DATA(0x64);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0X12);
    LCD_WR_DATA(0X81);

    LCD_WR_REG(0xE8);
    LCD_WR_DATA(0x85);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x7A);

    LCD_WR_REG(0xCB);
    LCD_WR_DATA(0x39);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x34);
    LCD_WR_DATA(0x02);

    LCD_WR_REG(0xF7);
    LCD_WR_DATA(0x20);

    LCD_WR_REG(0xEA);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xC0); // Power control
    LCD_WR_DATA(0x23); // VRH[5:0]

    LCD_WR_REG(0xC1); // Power control
    LCD_WR_DATA(0x10); // SAP[2:0];BT[3:0]

    LCD_WR_REG(0xC5); // VCM control
    LCD_WR_DATA(0x3E); // Contrast
    LCD_WR_DATA(0x28);

    LCD_WR_REG(0xC7); // VCM control2
    LCD_WR_DATA(0x86); // --

    LCD_WR_REG(0x36); // Memory Access Control
    LCD_WR_DATA(0x48); // C8

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA(0x00);
   _WR_DATA(0x18);

    LCD_WR_REG(0xB6); // Display Function Control
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x82);
    LCD_WR_DATA(0x27);

    LCD_WR_REG(0xF2); // 3Gamma Function Disable
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x26); // Gamma curve selected
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xE0); // Set Gamma
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x4E);
    LCD_WR_DATA(0xF1);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0XE1); // Set Gamma
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0xC1);
    LCD_WR_DATA(0x48);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x31);
    LCD_WR_DATA(0x36);
    LCD_WR_DATA(0x0F);

    LCD_WR_REG(0x11); // Exit Sleep
    delay_ms(120);

    LCD_WR_REG(0x29); // Display on
    LCD_WR_REG(0x2C);

    lcddev.id = 0x9341; // 假设是ILI9341
    LCD_Display_Dir(0); // 默认为竖屏
    LCD_Clear(WHITE); // 清屏
}

// 兼容OLED接口的显示字符串函数
void TFTLCD_ShowString(u16 x, u16 y, u8 *str, u8 size1, u8 mode)
{
    LCD_ShowString(x, y, 240, 320, str);
}

// 兼容OLED接口的显示数字函数
void TFTLCD_ShowNum(u16 x, u16 y, u32 num, u8 len, u8 size1, u8 mode)
{
    LCD_ShowNum(x, y, num, len, size1);
}

// 兼容OLED接口的显示字符函数
void TFTLCD_ShowChar(u16 x, u16 y, u8 chr, u8 size1, u8 mode)
{
    LCD_ShowChar(x, y, chr, size1, mode);
}

// 兼容OLED接口的清屏函数
void TFTLCD_Clear(u16 color)
{
    LCD_Clear(color);
}
