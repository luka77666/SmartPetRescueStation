# -*- coding: utf-8 -*-
"""生成图1-1 系统总体架构图"""

from PIL import Image, ImageDraw, ImageFont
import math, os

W, H = 1600, 1000
img = Image.new('RGB', (W, H), color='#F0F4FA')
draw = ImageDraw.Draw(img)

# 尝试加载字体
try:
    font_title = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 34)
    font_head  = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 26)
    font_body  = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 20)
    font_small = ImageFont.truetype('C:/Windows/Fonts/simhei.ttf', 17)
    font_en    = ImageFont.truetype('C:/Windows/Fonts/consola.ttf', 15)
except:
    font_title = ImageFont.load_default()
    font_head  = ImageFont.load_default()
    font_body  = ImageFont.load_default()
    font_small = ImageFont.load_default()
    font_en    = ImageFont.load_default()

# 颜色
C_BLUE   = '#4361EE'
C_GREEN  = '#2D6A4F'
C_DARK   = '#1A1A2E'
C_WHITE  = '#FFFFFF'
C_CLOUD  = '#E8F4FD'
C_LINE   = '#444444'

def draw_rounded_rect(draw, x1, y1, x2, y2, radius=16, fill='#FFFFFF', outline='#AAAAAA', width=2):
    draw.rounded_rectangle([x1, y1, x2, y2], radius=radius,
                          fill=fill, outline=outline, width=width)

def draw_arrow(draw, x1, y1, x2, y2, color='#444444', width=3):
    draw.line([(x1, y1), (x2, y2)], fill=color, width=width)
    angle = math.atan2(y2 - y1, x2 - x1)
    arrow_len = 16
    arrow_angle = math.radians(25)
    ax1 = x2 - arrow_len * math.cos(angle - arrow_angle)
    ay1 = y2 - arrow_len * math.sin(angle - arrow_angle)
    ax2 = x2 - arrow_len * math.cos(angle + arrow_angle)
    ay2 = y2 - arrow_len * math.sin(angle + arrow_angle)
    draw.polygon([(x2, y2), (ax1, ay1), (ax2, ay2)], fill=color)

def center_text(draw, x1, y1, x2, y2, text, font, fill='#000000'):
    bbox = draw.textbbox((0,0), text, font=font)
    tw = bbox[2] - bbox[0]
    th = bbox[3] - bbox[1]
    x = x1 + (x2 - x1 - tw) // 2
    y = y1 + (y2 - y1 - th) // 2
    draw.text((x, y), text, font=font, fill=fill)

# ========== 标题 ==========
title = '图1-1  系统总体架构图'
bbox = draw.textbbox((0,0), title, font=font_title)
tw = bbox[2] - bbox[0]
draw.text(((W - tw) // 2, 30), title, font=font_title, fill=C_DARK)

# ========== 左侧：APP端 ==========
AX1, AY1 = 60, 140
AX2, AY2 = 580, 920
draw_rounded_rect(draw, AX1, AY1, AX2, AY2, radius=22,
                  fill='#EEF2FF', outline=C_BLUE, width=3)
# 左侧蓝色标签
draw.rectangle([AX1, AY1, AX1+48, AY2], fill=C_BLUE, outline=C_BLUE)
for i, ch in enumerate('APP端'):
    draw.text((AX1+8, AY1+40+i*40), ch, font=font_head, fill=C_WHITE)

# APP内部模块
app_modules = [
    ('首页监测', 200, C_BLUE),
    ('喂食控制', 290, C_BLUE),
    ('定时设置', 380, C_BLUE),
    ('通信管理', 470, C_BLUE),
    ('连接管理', 560, C_BLUE),
]
for name, ay, color in app_modules:
    draw_rounded_rect(draw, AX1+65, ay, AX2-20, ay+68, radius=12,
                      fill=color, outline=color, width=2)
    center_text(draw, AX1+65, ay, AX2-20, ay+68, name, font_body, C_WHITE)

# ========== 中间：WiFi通信云 ==========
cloud_cx = W // 2
cloud_cy = 480

# 画云朵
draw.ellipse([cloud_cx-200, cloud_cy-70, cloud_cx+200, cloud_cy+70],
              fill=C_CLOUD, outline='#88BBCC', width=2)
draw.ellipse([cloud_cx-160, cloud_cy-95, cloud_cx+100, cloud_cy+45],
              fill=C_CLOUD, outline='#88BBCC', width=2)
draw.ellipse([cloud_cx-80, cloud_cy-80, cloud_cx+180, cloud_cy+55],
              fill=C_CLOUD, outline='#88BBCC', width=2)

center_text(draw, cloud_cx-120, cloud_cy-20, cloud_cx+120, cloud_cy+20,
            'WiFi 通信', font_head, C_BLUE)

# 协议标注
proto_text = 'TCP Socket  |  AP模式  |  端口 5000'
bbox = draw.textbbox((0,0), proto_text, font=font_en)
tw = bbox[2]-bbox[0]
draw.text(((W-tw)//2, cloud_cy+90), proto_text, font=font_en, fill='#555555')

# 箭头：APP → 设备（指令下发）
draw_arrow(draw, AX2+15, 320, cloud_cx-200-15, 320, color=C_BLUE, width=3)
draw_arrow(draw, cloud_cx+200+15, 320, 1020-15, 320, color=C_BLUE, width=3)
bbox = draw.textbbox((0,0), 'TCP 指令下发', font=font_small)
tw = bbox[2]-bbox[0]
draw.text(((W-tw)//2, 295), 'TCP 指令下发', font=font_small, fill=C_BLUE)

# 箭头：设备 → APP（数据上传）
draw_arrow(draw, cloud_cx+200+15, 560, 1020-15, 560, color='#E63946', width=3)
draw_arrow(draw, AX2+15, 560, cloud_cx-200-15, 560, color='#E63946', width=3)
bbox = draw.textbbox((0,0), '传感器数据上传', font=font_small)
tw = bbox[2]-bbox[0]
draw.text(((W-tw)//2, 575), '传感器数据上传', font=font_small, fill='#E63946')

# ========== 右侧：设备端 ==========
DX1, DY1 = 1020, 140
DX2, DY2 = 1540, 920
draw_rounded_rect(draw, DX1, DY1, DX2, DY2, radius=22,
                  fill='#E8F5E9', outline=C_GREEN, width=3)
# 右侧绿色标签
draw.rectangle([DX2-48, DY1, DX2, DY2], fill=C_GREEN, outline=C_GREEN)
for i, ch in enumerate('设备端'):
    draw.text((DX2-36, DY1+40+i*40), ch, font=font_head, fill=C_WHITE)

# 设备内部模块
dev_modules = [
    ('STM32 主控', 200, C_GREEN, '核心控制'),
    ('ESP8266 AP', 290, '#1B4332', 'WiFi热点'),
    ('传感器单元', 390, '#2D6A4F', 'DHT11/HX711/水位'),
    ('执行机构', 480, '#1B4332', '步进电机/水泵'),
    ('TFT 显示', 570, '#2D6A4F', '1.8寸 LCD'),
]
for name, dy, color, desc in dev_modules:
    draw_rounded_rect(draw, DX1+20, dy, DX2-65, dy+68, radius=12,
                      fill=color, outline=color, width=2)
    center_text(draw, DX1+20, dy, DX2-65, dy+40, name, font_body, C_WHITE)
    bbox = draw.textbbox((0,0), desc, font=font_small)
    tw = bbox[2]-bbox[0]
    tx = DX1+20 + (DX2-65-DX1-20 - tw)//2
    draw.text((tx, dy+44), desc, font=font_small, fill='#CCDDCC')

# 传感器小图标区域
sensor_y = 680
draw_rounded_rect(draw, DX1+20, sensor_y, DX2-65, sensor_y+55, radius=10,
                  fill='#FFFFFF', outline='#88AA88', width=1)
sensors = ['DHT11', 'HX711', '水位', '红外']
sx = DX1+30
for s in sensors:
    bbox = draw.textbbox((0,0), s, font=font_small)
    tw = bbox[2]-bbox[0]
    draw.text((sx, sensor_y+18), s, font=font_small, fill=C_DARK)
    sx += (DX2-65-DX1-40) // 4

# ========== 底部说明 ==========
note_y = 950
note = 'Android APP（客户端）←TCP→ ESP8266（服务端，AP模式，IP：192.168.4.1，端口：5000）'
bbox = draw.textbbox((0,0), note, font=font_small)
tw = bbox[2]-bbox[0]
draw.text(((W-tw)//2, note_y), note, font=font_small, fill='#333333')

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图1-1_系统总体架构图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：', img.size)
