# -*- coding: utf-8 -*-
"""生成图1-1 系统总体架构图"""

from PIL import Image, ImageDraw, ImageFont
import math, os

W, H = 1800, 1200
img = Image.new('RGB', (W, H), color='#F5F9FF')
draw = ImageDraw.Draw(img)

# 字体
try:
    f28 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 28)
    f22 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 22)
    f18 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 18)
    f15 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 15)
    f_en = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', 14)
except:
    f28=f22=f18=f15=f_en=ImageFont.load_default()

# 颜色
C_BLUE   = '#2A5CAA'
C_GREEN  = '#2E7D32'
C_ORANGE = '#E65100'
C_BG1    = '#E3F2FD'
C_BG2    = '#E8F5E9'
C_LINE   = '#444444'
C_WHITE  = '#FFFFFF'

def rr(x1,y1,x2,y2, r=12, fill='#FFF', outline='#AAA', w=2):
    draw.rounded_rectangle([x1,y1,x2,y2], radius=r, fill=fill, outline=outline, width=w)

def arr(x1,y1, x2,y2, color='#444', w=3):
    draw.line([(x1,y1),(x2,y2)], fill=color, width=w)
    ang = math.atan2(y2-y1, x2-x1)
    al = 14; aa = math.radians(20)
    for s in [-1,1]:
        px = x2 - al*math.cos(ang + s*aa)
        py = y2 - al*math.sin(ang + s*aa)
        draw.polygon([(x2,y2),(px,py)], fill=color)

def ct(cx, cy, text, font, fill='#000'):
    bb = draw.textbbox((0,0), text, font=font)
    tw = bb[2]-bb[0]; th = bb[3]-bb[1]
    draw.text((cx-tw//2, cy-th//2), text, font=font, fill=fill)

def box(x1,y1,x2,y2, title, subtitle='', fc='#2A5CAA', wc=3):
    rr(x1,y1,x2,y2, r=14, fill='#FFFFFF', outline=fc, w=wc)
    draw.rectangle([x1+wc//2, y1+wc//2, x2-wc//2, y1+42], fill=fc)
    ct((x1+x2)//2, y1+21, title, f18, '#FFFFFF')
    if subtitle:
        ct((x1+x2)//2, y1+60, subtitle, f15, '#333333')

# ========== 标题 ==========
ct(W//2, 45, '图1-1  系统总体架构图', f28, '#1A237E')

# ========== 左侧 APP端（客户端）==========
AX1,AY1 = 80, 150
AX2,AY2 = 680, 1120
rr(AX1,AY1,AX2,AY2, r=20, fill=C_BG1, outline=C_BLUE, w=4)
# 左侧竖条标签
draw.rectangle([AX1+4, AY1+4, AX1+48, AY2-4], fill=C_BLUE)
ct(AX1+24, (AY1+AY2)//2, 'APP\n端', f18, '#FFFFFF')

# APP内模块
app = [
    (200, 290, '首页监测',   '温度/湿度/重量/水位'),
    (310, 400, '喂食控制',   '手动/自动模式切换'),
    (420, 510, '定时设置',   '3组定时任务管理'),
    (530, 620, '通信管理',   'TCP Socket 客户端'),
    (640, 730, '连接管理',   'WiFi热点连接/重连'),
]
for y1,y2,t,s in app:
    box(AX1+60, y1, AX2-20, y2, t, s, fc=C_BLUE, wc=2)

ct(W//2, 1170, 'Android APP（客户端）— 开发语言：Kotlin + Jetpack Compose', f15, '#555555')

# ========== 中间 通信层 ==========
CX = W // 2
# 云朵
for cx,cy,rx,ry in [(CX,560,240,90),(CX-80,540,150,80),(CX+60,550,170,85)]:
    draw.ellipse([cx-rx,cy-ry,cx+rx,cy+ry], fill='#FFFFFF', outline='#90A4AE', width=2)
ct(CX, 560, 'WiFi 通信层', f22, C_BLUE)

# 协议框
rr(CX-300, 700, CX+300, 830, r=12, fill='#FFF9C4', outline='#F9A825', w=2)
ct(CX, 730, '通信协议', f18, '#333')
ct(CX, 762, 'TCP Socket  |  AP模式  |  端口 5000', f_en, '#444')
ct(CX, 795, '数据格式：T25H60W0200L080M1F0P0...（\\r\\n 帧结束）', f_en, '#555')

# 箭头 APP→设备（指令下发）
arr(AX2+10, 420, CX-240-10, 420, color=C_BLUE, w=3)
arr(CX+240+10, 420, 1080-10, 420, color=C_BLUE, w=3)
ct((AX2+CX-240)//2, 398, 'TCP 指令下发', f15, C_BLUE)
ct((CX+240+1080)//2, 398, 'TCP 指令下发', f15, C_BLUE)

# 箭头 设备→APP（数据上传）
arr(CX+240+10, 620, 1080-10, 620, color=C_GREEN, w=3)
arr(AX2+10, 620, CX-240-10, 620, color=C_GREEN, w=3)
ct((AX2+CX-240)//2, 642, '传感器数据上传', f15, C_GREEN)
ct((CX+240+1080)//2, 642, '传感器数据上传', f15, C_GREEN)

# 心跳
ct(CX, 860, '心跳保活：APP每5s发 PING ←→ 设备回复 PONG', f_en, '#777777')

# ========== 右侧 设备端（服务端）==========
DX1,DY1 = 1080, 150
DX2,DY2 = 1680, 1120
rr(DX1,DY1,DX2,DY2, r=20, fill=C_BG2, outline=C_GREEN, w=4)
# 右侧竖条标签
draw.rectangle([DX2-48, DY1+4, DX2-4, DY2-4], fill=C_GREEN)
ct(DX2-24, (DY1+DY2)//2, '设备\n端', f18, '#FFFFFF')

# 设备内模块
dev = [
    (200, 290, 'STM32 主控',  'F103C8T6 @72MHz'),
    (310, 400, 'ESP8266 AP',  'TCP Server:5000\nSSID: ZNCWJZZ'),
    (420, 510, '传感器单元',  'DHT11/HX711/水位'),
    (530, 620, '执行机构',   '步进电机/5V水泵'),
    (640, 730, 'TFT 显示',   '1.8" 128×160'),
    (750, 850, 'Flash 存储',  '参数掉电保存'),
]
for y1,y2,t,s in dev:
    box(DX1+20, y1, DX2-60, y2, t, s, fc=C_GREEN, wc=2)

# 传感器小图标
sen_y = 900
rr(DX1+20, sen_y, DX2-60, sen_y+60, r=8, fill='#FFFFFF', outline='#81C784', w=1)
for i,s in enumerate(['DHT11','HX711','水位','红外']):
    sx = DX1+35 + i*(DX2-DX1-90)//4
    ct(sx, sen_y+30, s, f15, '#1B5E20')

# ========== 底部说明 ==========
note = ('架构说明：Android APP（客户端）通过TCP Socket连接设备端ESP8266 WiFi模块（AP模式，服务端），'
        'ESP8266开启TCP Server（端口5000），APP作为客户端主动连接。'
        '通信数据采用自定义文本协议，以"\\r\\n"作为帧结束标志。')
ct(W//2, 1100, note, f15, '#333333')

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图1-1_系统总体架构图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：', img.size)
