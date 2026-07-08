# -*- coding: utf-8 -*-
"""生成图1-1 系统总体架构图 V2 - 更专业准确"""

from PIL import Image, ImageDraw, ImageFont
import math, os

W, H = 1800, 1200
img = Image.new('RGB', (W, H), color='#F5F7FA')
draw = ImageDraw.Draw(img)

# 字体
try:
    f32 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 32)
    f26 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 26)
    f22 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 22)
    f19 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 19)
    f16 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 16)
    f14 = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 14)
    f_en = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', 15)
    f_en_s = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', 13)
except:
    f32=f26=f22=f19=f16=f14=f_en=f_en_s=ImageFont.load_default()

C_BG_APP   = '#EBF0FF'
C_BORDER_APP = '#4361EE'
C_BG_DEV  = '#E8F5E9'
C_BORDER_DEV= '#2D6A4F'
C_BG_CLOUD= '#FFFFFF'
C_ARROW   = '#333333'
C_BLUE    = '#4361EE'
C_GREEN   = '#2D6A4F'
C_RED     = '#E63946'
C_DARK    = '#1A1A2E'

def rr(draw, x1,y1,x2,y2, r=14, fill='#FFF', outline='#AAA', w=2):
    draw.rounded_rectangle([x1,y1,x2,y2], radius=r, fill=fill, outline=outline, width=w)

def arrow(draw, x1,y1, x2,y2, color='#333', w=3):
    draw.line([(x1,y1),(x2,y2)], fill=color, width=w)
    ang = math.atan2(y2-y1, x2-x1)
    al = 15; aa = math.radians(22)
    for sx in [-1,1]:
        ex = x2 - al * math.cos(ang + sx*aa)
        ey = y2 - al * math.sin(ang + sx*aa)
        draw.polygon([(x2,y2),(ex,ey)], fill=color)

def ctext(draw, cx, cy, text, font, fill='#000'):
    bb = draw.textbbox((0,0), text, font=font)
    tw = bb[2]-bb[0]; th = bb[3]-bb[1]
    draw.text((cx-tw//2, cy-th//2), text, font=font, fill=fill)

def box(draw, x1,y1,x2,y2, title, subtitle='', fc='#4361EE', wc=15):
    """画一个带顶部彩色条的模块框"""
    rr(draw, x1,y1,x2,y2, r=12, fill='#FFFFFF', outline=fc, w=wc)
    draw.rectangle([x1+wc//2, y1+wc//2, x2-wc//2, y1+44], fill=fc)
    ctext(draw, (x1+x2)//2, y1+22, title, f19, '#FFFFFF')
    if subtitle:
        ctext(draw, (x1+x2)//2, y1+60, subtitle, f14, '#333333')

# ========== 标题 ==========
ctext(draw, W//2, 40, '图1-1  系统总体架构图', f32, C_DARK)

# ========== 左侧 APP端（客户端）==========
AX1,AY1 = 60, 160
AX2,AY2 = 620, 1100
rr(draw, AX1,AY1,AX2,AY2, r=20, fill=C_BG_APP, outline=C_BORDER_APP, w=4)
# 左侧竖标签
draw.rectangle([AX1, AY1, AX1+50, AY2], fill=C_BORDER_APP)
for i,ch in enumerate('APP端\n(客户端)'):
    if ch != '\n':
        ctext(draw, AX1+16, AY1+50+i*34, ch, f19, '#FFFFFF')
    else:
        i += 1

# APP内部模块
app_mods = [
    (130, 200, 590, 290, '首页监测', '实时温湿度/重量/水位'),
    (130, 310, 590, 400, '喂食控制', '手动/自动模式切换'),
    (130, 420, 590, 510, '定时设置', '3组定时任务管理'),
    (130, 530, 590, 620, 'TCP通信', 'Socket客户端'),
]
for x1,y1,x2,y2,t,s in app_mods:
    box(draw, x1,y1,x2,y2, t, s, fc=C_BLUE, wc=2)

# APP底部说明
ctext(draw, (AX1+AX2)//2, AY2-60, '开发语言：Kotlin + Jetpack Compose', f14, '#555')
ctext(draw, (AX1+AX2)//2, AY2-35, 'Android 8.0+', f14, '#555')

# ========== 中间 通信层 ==========
CX = W // 2
# 云朵
for cx,cy,rx,ry in [(CX,540,220,80),(CX-60,530,140,70),(CX+40,530,170,75)]:
    draw.ellipse([cx-rx,cy-ry,cx+rx,cy+ry], fill='#FFFFFF', outline='#99BBEE', width=2)
ctext(draw, CX, 540, 'WiFi 通信层', f22, C_BLUE)
ctext(draw, CX, 575, 'AP模式  |  端口 5000', f_en_s, '#555')

# 协议框
proto_y = 640
rr(draw, CX-320, proto_y, CX+320, proto_y+80, r=10, fill='#FFF8E1', outline='#FFB300', w=2)
ctext(draw, CX, proto_y+22, '通信协议：自定义文本协议', f19, '#333')
ctext(draw, CX, proto_y+52, '数据格式：T25H60W0200L080M1F0P0...  帧结束标志：\\r\\n', f_en_s, '#555')

# 箭头 APP→设备（指令下发）
arrow(draw, AX2+10, 380, CX-220-10, 380, color=C_BLUE, w=3)
arrow(draw, CX+220+10, 380, 1080-10, 380, color=C_BLUE, w=3)
ctext(draw, (AX2+CX-220)//2, 355, 'TCP 指令下发', f16, C_BLUE)
ctext(draw, (CX+220+1080)//2, 355, 'TCP 指令下发', f16, C_BLUE)

# 箭头 设备→APP（数据上传）
arrow(draw, CX+220+10, 490, 1080-10, 490, color=C_GREEN, w=3)
arrow(draw, AX2+10, 490, CX-220-10, 490, color=C_GREEN, w=3)
ctext(draw, (CX+220+1080)//2, 510, '传感器数据上传', f16, C_GREEN)
ctext(draw, (AX2+CX-220)//2, 510, '传感器数据上传', f16, C_GREEN)

# 心跳
ctext(draw, CX, 605, '心跳：APP每5s发 PING ←→ 设备回复 PONG', f_en_s, '#888')

# ========== 右侧 设备端（服务端）==========
DX1,DY1 = 1080, 160
DX2,DY2 = 1640, 1100
rr(draw, DX1,DY1,DX2,DY2, r=20, fill=C_BG_DEV, outline=C_BORDER_DEV, w=4)
# 右侧竖标签
draw.rectangle([DX2-50, DY1, DX2, DY2], fill=C_BORDER_DEV)
for i,ch in enumerate('设备端\n(服务端)'):
    if ch != '\n':
        ctext(draw, DX2-34, DY1+50+i*34, ch, f19, '#FFFFFF')
    else:
        i += 1

# 设备内部模块
dev_mods = [
    (DX1+20, 200, DX2-70, 290, 'STM32 主控', 'F103C8T6 @72MHz'),
    (DX1+20, 310, DX2-70, 400, 'ESP8266 AP', 'TCP Server:5000\nSSID: ZNCWJZZ'),
    (DX1+20, 420, DX2-70, 530, '传感器单元', 'DHT11/HX711/水位'),
    (DX1+20, 550, DX2-70, 640, '执行机构', '步进电机/水泵'),
    (DX1+20, 670, DX2-70, 750, 'TFT 显示', '1.8" 128×160'),
    (DX1+20, 780, DX2-70, 870, 'Flash 存储', '参数掉电保存'),
]
for x1,y1,x2,y2,t,s in dev_mods:
    box(draw, x1,y1,x2,y2, t, s, fc=C_GREEN, wc=2)

# 传感器小图标
sen_y = 890
draw.rectangle([DX1+20, sen_y, DX2-70, sen_y+65], fill='#FFFFFF', outline='#88AA88', width=1)
for i,s in enumerate(['DHT11','HX711','水位','红外']):
    sx = DX1+35 + i*(DX2-DX1-90)//4
    ctext(draw, sx, sen_y+32, s, f14, '#333')

# 底部说明
ctext(draw, (DX1+DX2)//2, DY2-50, '开发环境：Keil5  |  C语言  |  Flash:0x0800F000', f14, '#555')

# ========== 底部说明文字 ==========
note_y = 1150
ctext(draw, W//2, note_y, 
    '架构说明：APP（客户端）主动连接ESP8266 AP热点（ZNCWJZZ），ESP8266作为TCP Server（端口5000）监听连接，APP通过Socket发送控制指令，设备上传传感器数据，双方以"\\r\\n"作为帧结束标志。',
    f16, '#444')

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图1-1_系统总体架构图_V2.png'
img.save(out, 'PNG')
print('生成成功:', out)
print('尺寸:', img.size)
