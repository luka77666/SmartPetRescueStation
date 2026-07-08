from PIL import Image, ImageDraw, ImageFont
import os, textwrap

W, H = 1400, 900
BG = (250, 252, 255)
BLUE = (25, 85, 180)
GREEN = (30, 150, 80)
ORANGE = (210, 120, 20)
GRAY = (100, 110, 120)
LINE_C = (160, 175, 200)
TITLE_BG = (35, 100, 200)
TITLE_FG = (255, 255, 255)
BOX_BG_BLUE = (230, 240, 255)
BOX_BG_GREEN = (230, 250, 235)
BOX_BORDER_BLUE = (60, 130, 220)
BOX_BORDER_GREEN = (50, 170, 90)
CLOUD_BG = (245, 245, 255)
CLOUD_BORDER = (140, 140, 200)

def get_font(size, bold=False):
    candidates = [
        'C:/Windows/Fonts/msyhbd.ttc' if bold else 'C:/Windows/Fonts/msyh.ttc',
        'C:/Windows/Fonts/simhei.ttf',
        'C:/Windows/Fonts/simsun.ttc',
    ]
    for path in candidates:
        if os.path.exists(path):
            try:
                return ImageFont.truetype(path, size=size)
            except Exception:
                pass
    return ImageFont.load_default()

img = Image.new('RGB', (W, H), BG)
d = ImageDraw.Draw(img)

# ── 标题栏 ──
d.rectangle([0, 0, W, 60], fill=TITLE_BG)
tf = get_font(26, bold=True)
d.text((W//2, 30), '图1-1  系统总体架构图', font=tf, fill=TITLE_FG, anchor='mm')

# ── 左侧 APP端 ──
app_x, app_y = 30, 100
app_w, app_h = 280, 740
d.rounded_rectangle([app_x, app_y, app_x+app_w, app_y+app_h], radius=12, fill=BOX_BG_BLUE, outline=BOX_BORDER_BLUE, width=2)
tf2 = get_font(20, bold=True)
d.text((app_x+app_w//2, app_y+24), 'Android APP 客户端', font=tf2, fill=BLUE, anchor='mm')

app_items = [
    ('首页监测', '实时显示温度、湿度\n重量、水位数据'),
    ('喂食控制', '手动/自动模式切换\n步进电机正反转控制'),
    ('定时设置', '3组定时任务管理\n喂食量、启用状态设置'),
    ('阈值设置', '水位阈值 SWxxx\n称重阈值 SFxxxx'),
    ('通信管理', 'TCP Socket 连接\n数据帧收发与解析'),
    ('连接管理', 'IP地址配置\n连接状态显示'),
]
iy = app_y + 56
for title, desc in app_items:
    ibox = [app_x+12, iy, app_x+app_w-12, iy+100]
    d.rounded_rectangle(ibox, radius=6, fill=(255,255,255), outline=(180,200,230), width=1)
    tf3 = get_font(16, bold=True)
    d.text((ibox[0]+10, iy+12), title, font=tf3, fill=BLUE)
    tf4 = get_font(13)
    for j, line in enumerate(desc.split('\n')):
        d.text((ibox[0]+10, iy+36+j*22), line, font=tf4, fill=GRAY)
    iy += 112

# ── 右侧 设备端 ──
dev_x, dev_y = 1090, 100
dev_w, dev_h = 280, 740
d.rounded_rectangle([dev_x, dev_y, dev_x+dev_w, dev_y+dev_h], radius=12, fill=BOX_BG_GREEN, outline=BOX_BORDER_GREEN, width=2)
tf5 = get_font(20, bold=True)
d.text((dev_x+dev_w//2, dev_y+24), '设备端 服务端', font=tf5, fill=GREEN, anchor='mm')

dev_items = [
    ('STM32主控', 'STM32F103C8T6\nCortex-M3 72MHz'),
    ('ESP8266通信', 'AP模式 TCP Server\nSSID: ZNCWJZZ 端口5000'),
    ('传感器单元', 'DHT11温湿度\nHX711称重 水位ADC'),
    ('执行机构', '28BYJ-48步进电机\n5V微型水泵'),
    ('TFT显示', 'ST7735 128×160\nSPI接口实时显示'),
    ('Flash存储', '参数掉电保存\n水位阈值、称重阈值'),
]
dy = dev_y + 56
for title, desc in dev_items:
    dbox = [dev_x+12, dy, dev_x+dev_w-12, dy+100]
    d.rounded_rectangle(dbox, radius=6, fill=(255,255,255), outline=(170,220,180), width=1)
    tf6 = get_font(16, bold=True)
    d.text((dbox[0]+10, dy+12), title, font=tf6, fill=GREEN)
    tf7 = get_font(13)
    for j, line in enumerate(desc.split('\n')):
        d.text((dbox[0]+10, dy+36+j*22), line, font=tf7, fill=GRAY)
    dy += 112

# ── 中间通信云朵 ──
cx, cy = 480, 380
cw, ch = 440, 200
d.rounded_rectangle([cx, cy, cx+cw, cy+ch], radius=30, fill=CLOUD_BG, outline=CLOUD_BORDER, width=2)
tf8 = get_font(18, bold=True)
d.text((cx+cw//2, cy+28), 'WiFi 通信层（C/S架构）', font=tf8, fill=(90,70,180), anchor='mm')
tf9 = get_font(15)
d.text((cx+cw//2, cy+60), 'TCP Socket  ·  AP模式  ·  端口5000', font=tf9, fill=GRAY, anchor='mm')

# 通信协议细节
proto_items = [
    ('数据上传（设备→APP）', 'T25H60W0200L080M1F0P0WT080...'),
    ('指令下发（APP→设备）', 'KA/KB/KC/KD/KE/SWxxx/SFxxxx'),
    ('帧格式', '换行符\\r\\n 作为帧结束标志'),
]
py = cy + 88
for label, val in proto_items:
    tf10 = get_font(13, bold=True)
    d.text((cx+20, py), label+':', font=tf10, fill=ORANGE)
    tw = d.textlength(label+':', font=tf10)
    tf11 = get_font(13)
    d.text((cx+20+tw+4, py), val, font=tf11, fill=GRAY)
    py += 26

# ── 箭头：APP → 通信层 ──
ax1, ay1 = app_x+app_w, app_y+app_h//2
ax2, ay2 = cx, cy+ch//2
# 水平箭头
d.line([ax1, ay1, cx, ay1], fill=LINE_C, width=3)
d.line([cx, ay1, cx, ay2], fill=LINE_C, width=3)
# 箭头头部
for off in [(-10,5), (-10,-5)]:
    d.polygon([(cx, ay2), (cx+off[0], ay2+off[1])], fill=LINE_C)
# 标签
tf12 = get_font(14, bold=True)
d.text(((ax1+cx)//2, ay1-18), 'TCP指令下发', font=tf12, fill=BLUE, anchor='mm')
tf12b = get_font(12)
d.text(((ax1+cx)//2, ay1+18), '(KA/KB/KC/KD/KE/SW/SF/ST/SD/SE)', font=tf12b, fill=GRAY, anchor='mm')

# ── 箭头：通信层 → 设备 ──
bx1, by1 = cx+cw, cy+ch//2
bx2, by2 = dev_x, cy+ch//2
d.line([bx1, by1, dev_x, by1], fill=LINE_C, width=3)
# 箭头头部
for off in [(10,5), (10,-5)]:
    d.polygon([(dev_x, by2), (dev_x+off[0], by2+off[1])], fill=LINE_C)
# 标签
tf13 = get_font(14, bold=True)
d.text(((bx1+dev_x)//2, by1-18), '传感器数据上传', font=tf13, fill=GREEN, anchor='mm')
tf13b = get_font(12)
d.text(((bx1+dev_x)//2, by1+18), '(T/H/W/L/M/F/P/WT/FT/T1~T3)', font=tf13b, fill=GRAY, anchor='mm')

# ── 底部说明 ──
note_y = H - 80
d.line([30, note_y, W-30, note_y], fill=(200,200,210), width=1)
tf14 = get_font(15, bold=True)
d.text((W//2, note_y+20), '架构说明', font=tf14, fill=GRAY, anchor='mm')
tf15 = get_font(13)
note_lines = [
    'C/S架构：Android APP作为客户端，设备端（ESP8266 AP模式）作为服务端，通过TCP协议通信。',
    '数据帧格式：设备端每5秒上报一次传感器数据，以换行符\\r\\n结束；APP下发控制命令，设备端实时响应。',
]
for j, line in enumerate(note_lines):
    d.text((W//2, note_y+44+j*26), line, font=tf15, fill=GRAY, anchor='mm')

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图1-1_系统总体架构图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：{}x{}'.format(W, H))
