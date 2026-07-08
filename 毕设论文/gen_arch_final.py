# -*- coding: utf-8 -*-
"""生成图1-1 系统总体架构图（Pillow版本）"""
from PIL import Image, ImageDraw, ImageFont
import math, os

W, H = 1800, 1100
img  = Image.new('RGB', (W, H), '#F5F9FF')
draw = ImageDraw.Draw(img)

# ---------- 字体 ----------
try:
    f32 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 32)
    f24 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 24)
    f20 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 20)
    f17 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 17)
    f14 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 14)
    fe  = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', 14)
except:
    f32=f24=f20=f17=f14=fe=ImageFont.load_default()

# ---------- 颜色 ----------
C_BLUE   = '#1A73E8'
C_GREEN  = '#1B8C4A'
C_ORANGE = '#E65100'
C_RED    = '#C62828'
C_GRAY   = '#455A64'
C_WHITE  = '#FFFFFF'
C_LBLUE  = '#E3F2FD'
C_LGREEN = '#E8F5E9'

def ct(cx, cy, text, font, fill='#000'):
    """居中写文字"""
    bb = draw.textbbox((0,0), text, font=font)
    w = bb[2]-bb[0]; h = bb[3]-bb[1]
    draw.text((cx-w//2, cy-h//2), text, font=font, fill=fill)

def rr(x1,y1,x2,y2, r=14, fill='#FFF', outline='#AAA', w=2):
    draw.rounded_rectangle([x1,y1,x2,y2], radius=r, fill=fill, outline=outline, width=w)

def arrow(x1,y1, x2,y2, color='#444', w=3):
    draw.line([(x1,y1),(x2,y2)], fill=color, width=w)
    ang = math.atan2(y2-y1, x2-x1)
    L   = 15; aa = math.radians(22)
    for s in (-1,1):
        px = x2 - L*math.cos(ang + s*aa)
        py = y2 - L*math.sin(ang + s*aa)
        draw.polygon([(x2,y2),(px,py)], fill=color)

# ========== 标题 ==========
ct(W//2, 48, '图1-1  系统总体架构图', f32, C_GRAY)

# ========== 左侧 APP端（客户端）==========
AX1, AY1 = 70, 160
AX2, AY2 = 620, 920
rr(AX1,AY1, AX2,AY2, r=20, fill=C_LBLUE, outline=C_BLUE, w=4)
# 左侧蓝色竖条
draw.rectangle([AX1+4, AY1+4, AX1+52, AY2-4], fill=C_BLUE)
ct(AX1+28, (AY1+AY2)//2, 'APP\n客\n户\n端', f20, C_WHITE)

# APP内部模块
modules = [
    (210, 290, '首页监测',   '温度/湿度/重量/水位'),
    (320, 400, '喂食控制',   '手动/自动模式切换'),
    (430, 510, '定时设置',   '3组定时任务'),
    (540, 620, '通信管理',   'TCP Socket 客户端'),
    (650, 730, '连接管理',   'WiFi热点连接/重连'),
]
for y1,y2, t, s in modules:
    rr(AX1+65, y1, AX2-20, y2, r=12, fill=C_BLUE, outline=C_BLUE, w=2)
    ct((AX1+65+AX2-20)//2, (y1+y2)//2-12, t, f17, C_WHITE)
    ct((AX1+65+AX2-20)//2, (y1+y2)//2+14, s, f14, C_WHITE)

ct((AX1+AX2)//2, AY2+45, 'Android APP  —  开发语言：Kotlin + Jetpack Compose', f14, '#555')

# ========== 中间 通信层 ==========
CX = W // 2
# 云朵
for (cx,cy,rx,ry) in [(CX,500,230,85),(CX-90,490,160,75),(CX+70,490,180,80)]:
    draw.ellipse([cx-rx,cy-ry, cx+rx,cy+ry], fill='#FFFFFF', outline='#90A4AE', width=2)
ct(CX, 500, 'WiFi 通信层', f24, C_BLUE)
ct(CX, 535, 'AP模式  |  端口 5000  |  TCP Socket', f14, '#555')

# 协议框
rr(CX-280, 620, CX+280, 760, r=12, fill='#FFF9C4', outline='#F9A825', w=2)
ct(CX, 655, '通信协议：自定义文本协议', f20, '#333')
ct(CX, 693, '数据格式：T25H60W0200L080M1F0P0...', fe, '#444')
ct(CX, 728, '帧结束标志：\\r\\n  （换行符）', f14, '#555')

# 心跳
ct(CX, 800, '心跳保活：APP 每 5s 发 PING  ←→  设备回复 PONG', f14, '#777')

# 箭头：APP→设备（指令下发）
arrow(AX2+10, 430, CX-230-10, 430, color=C_BLUE, w=3)
arrow(CX+230+10, 430, 1080-10, 430, color=C_BLUE, w=3)
ct((AX2+CX-230)//2, 408, 'TCP 指令下发', f17, C_BLUE)
ct((CX+230+1080)//2, 408, 'TCP 指令下发', f17, C_BLUE)

# 箭头：设备→APP（数据上传）
arrow(CX+230+10, 590, 1080-10, 590, color=C_GREEN, w=3)
arrow(AX2+10, 590, CX-230-10, 590, color=C_GREEN, w=3)
ct((AX2+CX-230)//2, 612, '传感器数据上传', f17, C_GREEN)
ct((CX+230+1080)//2, 612, '传感器数据上传', f17, C_GREEN)

# ========== 右侧 设备端（服务端）==========
DX1, DY1 = 1080, 160
DX2, DY2 = 1630, 920
rr(DX1,DY1, DX2,DY2, r=20, fill=C_LGREEN, outline=C_GREEN, w=4)
# 右侧绿色竖条
draw.rectangle([DX2-52, DY1+4, DX2-4, DY2-4], fill=C_GREEN)
ct(DX2-28, (DY1+DY2)//2, '设\n备\n服\n务\n端', f20, C_WHITE)

# 设备内部模块
dev_mods = [
    (210, 290, 'STM32 主控',  'F103C8T6 @72MHz', C_GREEN),
    (320, 400, 'ESP8266 AP',   'TCP Server : 5000',  C_GREEN),
    (430, 510, '传感器单元',  'DHT11/HX711/水位', '#1B4332'),
    (540, 620, '执行机构',    '步进电机 / 5V水泵', '#1B4332'),
    (650, 730, 'TFT 显示',    '1.8" 128×160 LCD', '#1B4332'),
    (760, 850, 'Flash 存储',   '参数掉电保存', C_GREEN),
]
for y1,y2,t,s,c in dev_mods:
    rr(DX1+20, y1, DX2-70, y2, r=12, fill=c, outline=c, w=2)
    ct((DX1+20+DX2-70)//2, (y1+y2)//2-14, t, f17, C_WHITE)
    ct((DX1+20+DX2-70)//2, (y1+y2)//2+12, s, f14, C_WHITE)

ct((DX1+DX2)//2, DY2+45, '设备端  —  开发环境：Keil5  |  C语言', f14, '#555')

# ========== 底部说明 ==========
note = ('架构说明：Android APP（客户端）通过TCP Socket连接设备端ESP8266 WiFi模块（AP模式，服务端）\n'
         'ESP8266开启TCP Server（端口5000），APP作为客户端主动连接；通信数据采用自定义文本协议，以"\\r\\n"作为帧结束标志。')
bb = draw.textbbox((0,0), note, font=f17)
tw = bb[2]-bb[0]
# 分行写
lines = note.split('\n')
ny = 1020
for line in lines:
    ct(W//2, ny, line, f17, '#333333')
    ny += 32

# ========== 保存 ==========
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图1-1_系统总体架构图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：', img.size)
