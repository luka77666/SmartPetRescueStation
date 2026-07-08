from PIL import Image, ImageDraw, ImageFont
import os

W, H = 1200, 750
BG = (250, 252, 255)
BLUE = (25, 85, 180)
GREEN = (30, 150, 80)
GRAY = (100, 110, 120)
ORANGE = (210, 120, 20)
TITLE_BG = (35, 100, 200)
TITLE_FG = (255, 255, 255)

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
            except:
                pass
    return ImageFont.load_default()

img = Image.new('RGB', (W, H), BG)
d = ImageDraw.Draw(img)

# ── 标题栏 ──
d.rectangle([0, 0, W, 70], fill=TITLE_BG)
tf = get_font(28, bold=True)
d.text((W//2, 35), '图3-2  设备连接配置示意图', font=tf, fill=TITLE_FG, anchor='mm')

# ── 5个步骤（横向排列，上3下2） ──
steps = [
    ('设备通电', '接通DC 5V/2A电源\n设备自动启动\nESP8266建立AP热点'),
    ('连接设备热点', '手机WiFi设置中\n找到" ZNCWJZZ "\n开放网络，无需密码'),
    ('启动APP', '点击桌面图标\n"智能宠物救助站"\n启动本软件'),
    ('输入IP地址', 'APP首页输入框中\n输入192.168.4.1\n点击"连接"按钮'),
    ('连接成功', '跳转至主页界面\n接收传感器数据\n状态显示"已连接"'),
]

# 布局：上3张 (row1) + 下2张 (row2)
top_row = 3
card_w = 260
card_h = 260
gap_x = 40
gap_y = 50
row1_y = 120
row2_y = row1_y + card_h + gap_y

# 第1行3张
row1_w = 3 * card_w + 2 * gap_x
start_x1 = (W - row1_w) // 2

# 第2行2张
row2_w = 2 * card_w + gap_x
start_x2 = (W - row2_w) // 2

positions = []
for i in range(5):
    if i < 3:
        cx = start_x1 + i * (card_w + gap_x)
        cy = row1_y
    else:
        cx = start_x2 + (i - 3) * (card_w + gap_x)
        cy = row2_y
    positions.append((cx, cy))

for i, (title, desc) in enumerate(steps):
    cx, cy = positions[i]

    # 卡片背景
    d.rounded_rectangle([cx, cy, cx+card_w, cy+card_h], radius=14, fill=(255,255,255), outline=BLUE, width=2)

    # 序号圆
    r = 24
    d.ellipse([cx+card_w//2-r, cy+16-r, cx+card_w//2+r, cy+16+r], fill=BLUE)
    nf = get_font(22, bold=True)
    d.text((cx+card_w//2, cy+16), str(i+1), font=nf, fill=(255,255,255), anchor='mm')

    # 标题
    tf2 = get_font(18, bold=True)
    d.text((cx+card_w//2, cy+60), title, font=tf2, fill=BLUE, anchor='mm')

    # 分隔线
    d.line([cx+30, cy+90, cx+card_w-30, cy+90], fill=BLUE, width=1)

    # 描述
    tf3 = get_font(15)
    desc_lines = desc.split('\n')
    for j, line in enumerate(desc_lines):
        d.text((cx+card_w//2, cy+108+j*30), line, font=tf3, fill=GRAY, anchor='mm')

    # 箭头（向右，非行尾）
    if i < 3:
        # 第1行：向右箭头
        if i < 2:
            ax = cx + card_w + 5
            ay = cy + card_h // 2
            d.line([ax, ay, ax+gap_x-10, ay], fill=(170,180,200), width=3)
            d.polygon([(ax+gap_x-10, ay), (ax+gap_x-18, ay-7), (ax+gap_x-18, ay+7)], fill=(170,180,200))

# ── 下行之间的竖直箭头 ──
# 在第3步下方画向下箭头到第4步
arrow_cx = start_x1 + 2 * (card_w + gap_x) + card_w // 2  # 第3步中心x
arrow_x1 = arrow_cx - 50
arrow_y1 = row1_y + card_h
arrow_y2 = row2_y
d.line([arrow_cx, arrow_y1, arrow_cx, arrow_y2-5], fill=(170,180,200), width=3)
d.polygon([(arrow_cx, arrow_y2-5), (arrow_cx-8, arrow_y2-13), (arrow_cx+8, arrow_y2-13)], fill=(170,180,200))
# 向下标签
tl = get_font(13, bold=True)
d.text((arrow_cx+40, (arrow_y1+arrow_y2)//2), '下一步', font=tl, fill=ORANGE, anchor='mm')

# ── 底部提示框 ──
note_y = row2_y + card_h + 40
d.rounded_rectangle([60, note_y, W-60, note_y+60], radius=10, fill=(255,248,240), outline=(220,180,100), width=1)

tf5 = get_font(15, bold=True)
d.text((80, note_y+18), '提示：', font=tf5, fill=ORANGE)
tf6 = get_font(15)
note_text = '如连接失败，请确认手机已连接"ZNCWJZZ"热点且IP地址输入正确。APP支持自动重连功能，网络断开后将自动尝试重新连接。'
d.text((160, note_y+18), note_text, font=tf6, fill=GRAY)

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图3-2_设备连接配置示意图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：{}x{}'.format(W, H))
