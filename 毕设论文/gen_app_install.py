from PIL import Image, ImageDraw, ImageFont
import os

W, H = 1000, 700
BG = (250, 252, 255)
BLUE = (25, 85, 180)
GREEN = (30, 150, 80)
GRAY = (100, 110, 120)
TITLE_BG = (35, 100, 200)
TITLE_FG = (255, 255, 255)
STEP_COLORS = [(35, 100, 200), (40, 140, 70), (200, 140, 30), (180, 60, 60)]

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
d.text((W//2, 35), '图3-1  APP安装步骤示意图', font=tf, fill=TITLE_FG, anchor='mm')

# ── 4个步骤卡片 ──
steps = [
    ('获取安装包', '将app-release.apk通过\n数据线/蓝牙/网络传输\n复制到手机存储中'),
    ('开启安装权限', '进入"设置→安全→更多安全"\n开启"允许安装未知来源应用"\n不同品牌手机路径略有差异'),
    ('执行安装', '打开文件管理器\n找到app-release.apk\n点击按提示完成安装'),
    ('安装完成', '安装成功后桌面出现\n"智能宠物救助站"图标\n点击即可启动软件'),
]

card_w = 200
card_h = 280
gap = 30
start_x = (W - (4 * card_w + 3 * gap)) // 2
start_y = 120

for i, (title, desc) in enumerate(steps):
    cx = start_x + i * (card_w + gap)
    cy = start_y

    # 卡片背景
    d.rounded_rectangle([cx, cy, cx+card_w, cy+card_h], radius=14, fill=(255,255,255), outline=STEP_COLORS[i], width=2)

    # 序号圆
    r = 26
    d.ellipse([cx+card_w//2-r, cy+20-r, cx+card_w//2+r, cy+20+r], fill=STEP_COLORS[i])
    nf = get_font(24, bold=True)
    d.text((cx+card_w//2, cy+20), str(i+1), font=nf, fill=(255,255,255), anchor='mm')

    # 步骤标题
    tf2 = get_font(18, bold=True)
    d.text((cx+card_w//2, cy+70), title, font=tf2, fill=STEP_COLORS[i], anchor='mm')

    # 分隔线
    d.line([cx+30, cy+100, cx+card_w-30, cy+100], fill=STEP_COLORS[i], width=1)

    # 描述文字
    tf3 = get_font(15)
    desc_lines = desc.split('\n')
    for j, line in enumerate(desc_lines):
        d.text((cx+card_w//2, cy+120+j*32), line, font=tf3, fill=GRAY, anchor='mm')

    # 箭头（非最后一个）
    if i < 3:
        arrow_x = cx + card_w + 2
        arrow_y = cy + card_h // 2
        # 水平线
        d.line([arrow_x, arrow_y, arrow_x + gap - 4, arrow_y], fill=(170,180,200), width=3)
        # 箭头头
        d.polygon([(arrow_x+gap-4, arrow_y), (arrow_x+gap-12, arrow_y-7), (arrow_x+gap-12, arrow_y+7)], fill=(170,180,200))

# ── 底部说明框 ──
bottom_y = start_y + card_h + 60
d.rounded_rectangle([60, bottom_y, W-60, bottom_y+120], radius=10, fill=(245,250,255), outline=(180,200,230), width=1)

tf4 = get_font(15)
tips = [
    '提示：安装前请确保手机已开启"允许安装未知来源应用"权限。',
    '若安装时提示"风险"，此为Android系统对非应用商店APK的正常安全提醒，选择"继续安装"即可。',
    '为保证正常使用，建议Android 8.0及以上版本系统。',
]
for j, tip in enumerate(tips):
    # 小圆点
    d.ellipse([72, bottom_y+16+j*36-4, 80, bottom_y+16+j*36+4], fill=BLUE)
    d.text((90, bottom_y+16+j*36), tip, font=tf4, fill=GRAY)

# 保存
out = r'c:\Users\86157\Desktop\ronghe\毕设论文\图3-1_APP安装步骤示意图.png'
img.save(out, 'PNG')
print('生成成功：', out)
print('尺寸：{}x{}'.format(W, H))
