#!/usr/bin/env python3
"""Generates the RedWill 32x32 icon set (Windows NT 4 style pixel art) into inter32/icons/*.ico.
Usage: python3 tools/gen_icons.py [out_dir]"""
import sys, os, math
from PIL import Image, ImageDraw

K = (0, 0, 0, 255)
def C(h, a=255): return ((h >> 16) & 255, (h >> 8) & 255, h & 255, a)
def sh(c, f):  # lighten (f>0) / darken (f<0)
    r, g, b, a = c
    if f >= 0: return (int(r + (255 - r) * f), int(g + (255 - g) * f), int(b + (255 - b) * f), a)
    return (int(r * (1 + f)), int(g * (1 + f)), int(b * (1 + f)), a)

class Cv:
    def __init__(s): s.im = Image.new('RGBA', (32, 32), (0, 0, 0, 0)); s.d = ImageDraw.Draw(s.im)
    def p(s, x, y, c):
        if 0 <= x < 32 and 0 <= y < 32: s.im.putpixel((x, y), c)
    def r(s, x0, y0, x1, y1, c): s.d.rectangle([x0, y0, x1, y1], fill=c)           # inclusive
    def box(s, x0, y0, x1, y1, fill, line=K): s.d.rectangle([x0, y0, x1, y1], fill=fill, outline=line)
    def vg(s, x0, y0, x1, y1, a, b):                                                  # vertical gradient
        n = max(1, y1 - y0)
        for j in range(y0, y1 + 1):
            t = (j - y0) / n; c = tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(4)); s.r(x0, j, x1, j, c)
    def bevel(s, x0, y0, x1, y1, face, up=True):                                      # NT 3D button edge
        hi, lo = (C(0xFFFFFF), C(0x404040)) if up else (C(0x404040), C(0xFFFFFF))
        s.r(x0, y0, x1, y1, face)
        s.r(x0, y0, x1, y0, hi); s.r(x0, y0, x0, y1, hi)
        s.r(x0, y1, x1, y1, lo); s.r(x1, y0, x1, y1, lo)
    def line(s, pts, c, w=1): s.d.line(pts, fill=c, width=w)
    def poly(s, pts, fill, line=None): s.d.polygon(pts, fill=fill, outline=line)
    def ell(s, x0, y0, x1, y1, fill, line=None): s.d.ellipse([x0, y0, x1, y1], fill=fill, outline=line)
    def glow(s, box, a=70):                                                          # translucent white highlight, clipped to opaque pixels
        ov = Image.new('RGBA', (32, 32), (0, 0, 0, 0)); ImageDraw.Draw(ov).ellipse(box, fill=(255, 255, 255, a))
        for y in range(32):
            for x in range(32):
                if s.im.getpixel((x, y))[3] == 0: ov.putpixel((x, y), (0, 0, 0, 0))
        s.im.alpha_composite(ov)
    def shadow(s):                                                                    # soft drop shadow (down-right)
        sm = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
        for y in range(32):
            for x in range(32):
                if s.im.getpixel((x, y))[3] > 200 and x + 1 < 32 and y + 1 < 32 and s.im.getpixel((x + 1, y + 1))[3] == 0:
                    sm.putpixel((x + 1, y + 1), (0, 0, 0, 90))
        sm.alpha_composite(s.im); s.im = sm; s.d = ImageDraw.Draw(s.im)

GRAY, LIGHT, DARK = C(0xC0C0C0), C(0xE0E0E0), C(0x808080)
YEL, YELD, YELL = C(0xF8D848), C(0xC09810), C(0xFCEA98)

def folder():
    c = Cv()
    c.box(2, 6, 13, 9, YELD)                       # tab
    c.r(3, 7, 12, 8, sh(YEL, -0.1))
    c.box(2, 9, 29, 27, YEL)                       # body back
    c.vg(3, 10, 28, 26, YELL, YEL)
    c.r(3, 10, 28, 10, C(0xFFFFFF))
    c.r(29, 10, 29, 27, YELD); c.r(3, 27, 29, 27, YELD)
    c.r(3, 26, 28, 26, sh(YEL, -0.18))
    c.shadow(); return c.im

def folder_open():
    c = Cv()
    c.box(2, 5, 12, 8, YELD); c.r(3, 6, 11, 7, sh(YEL, -0.1))
    c.box(2, 8, 22, 26, sh(YEL, -0.25))            # back panel
    c.r(3, 9, 21, 12, sh(YEL, -0.35))
    c.poly([(8, 13), (30, 13), (26, 27), (2, 27)], YEL, K)  # front flap slanted
    c.vg(9, 14, 28, 26, YELL, YEL)
    for x in range(8, 30): c.p(x, 14, C(0xFFFFFF)) if False else None
    c.line([(3, 26), (25, 26)], sh(YEL, -0.2))
    c.line([(9, 14), (29, 14)], C(0xFFFFFF))
    c.shadow(); return c.im

def document():
    c = Cv()
    c.poly([(6, 2), (21, 2), (27, 8), (27, 29), (6, 29)], C(0xFFFFFF), K)
    c.poly([(21, 2), (21, 8), (27, 8)], LIGHT, K)
    c.r(21, 3, 21, 7, K)
    c.r(7, 3, 20, 3, C(0xFFFFFF))
    for i, y in enumerate((12, 15, 18, 21, 24)):
        c.r(9, y, 23 - (i == 4) * 6, y, C(0x404040) if i % 2 == 0 else C(0x606060))
    c.r(26, 9, 26, 28, LIGHT); c.r(7, 28, 26, 28, LIGHT)
    c.shadow(); return c.im

def notepad():
    c = Cv()
    c.box(5, 4, 26, 29, C(0xFFFFFF))
    c.box(5, 4, 26, 9, C(0x000080), K); c.r(6, 5, 25, 8, C(0x1084D0))
    c.r(6, 5, 25, 5, C(0x60B8F0))
    for y in (13, 17, 21, 25): c.r(8, y, 23, y, C(0x6A8CC8))
    c.r(9, 11, 9, 28, C(0xE08080))
    for x in range(7, 26, 3):                       # spiral rings
        c.r(x, 2, x, 6, C(0x303030)); c.p(x, 2, C(0xA0A0A0)); c.p(x, 3, C(0xE0E0E0))
    c.r(25, 10, 25, 28, LIGHT); c.r(6, 28, 25, 28, LIGHT)
    c.shadow(); return c.im

def calculator():
    c = Cv()
    c.box(4, 2, 27, 29, GRAY); c.bevel(5, 3, 26, 28, GRAY)
    c.box(7, 5, 24, 11, C(0x203020), C(0x404040)); c.r(8, 6, 23, 10, C(0xB4D8A8))
    for x, y in ((10, 7), (11, 7), (13, 8), (14, 8), (17, 7), (21, 9), (20, 9)): c.p(x, y, C(0x203020))
    c.r(18, 7, 22, 7, C(0x203020)); c.r(20, 8, 22, 8, C(0x203020))
    cols = [C(0x606060)] * 3 + [C(0xC02020)]
    for r in range(4):
        for k in range(4):
            x = 7 + k * 5; y = 14 + r * 4
            f = C(0xE8E8E8) if k < 3 else C(0xE06060)
            c.bevel(x, y, x + 3, y + 2, f)
    c.shadow(); return c.im

def terminal():
    c = Cv()
    c.box(2, 4, 29, 27, GRAY); c.bevel(3, 5, 28, 26, GRAY)
    c.r(4, 6, 27, 8, C(0x000080)); c.r(4, 6, 27, 6, C(0x2020A8))
    c.r(24, 7, 26, 7, C(0xFFFFFF)); c.r(6, 7, 8, 7, C(0xFFFFFF))
    c.r(4, 9, 27, 25, C(0x000000)); c.r(4, 9, 27, 9, C(0x404040))
    # "C:\>_" prompt
    W = C(0xC0C0C0)
    for p in ((6, 12), (7, 11), (8, 11), (6, 13), (6, 14), (7, 15), (8, 15)): c.p(*p, W)       # C
    c.p(10, 12, W); c.p(10, 14, W)                                                                # :
    for i in range(4): c.p(12 + i, 11 + i, W)                                                     # \
    for p in ((17, 11), (18, 12), (19, 13), (18, 14), (17, 15)): c.p(*p, W)                       # >
    c.r(21, 15, 23, 15, W)
    c.r(6, 19, 15, 19, C(0x808080)); c.r(6, 22, 11, 22, C(0x808080))
    c.shadow(); return c.im

def paint():
    c = Cv()
    pal = [(16, 3), (5, 9), (3, 17), (9, 24), (19, 27), (27, 21), (29, 13), (25, 6)]
    c.poly([(5, 8), (12, 3), (22, 3), (29, 9), (30, 18), (25, 27), (16, 29), (7, 27), (2, 20), (2, 13)], C(0xE0C078), K)
    c.poly([(6, 9), (12, 4), (22, 4), (28, 9)], C(0xF4DC98))
    c.ell(18, 19, 24, 25, C(0x00000000), K); c.ell(19, 20, 23, 24, C(0x00000000))  # thumb hole (transparent)
    c.ell(18, 19, 24, 25, (0, 0, 0, 0)); c.d.ellipse([18, 19, 24, 25], outline=K)
    for (x, y, col) in ((7, 10, 0xE02020), (13, 7, 0xF0D020), (20, 7, 0x20B040), (25, 12, 0x2050E0), (6, 17, 0xA030C0)):
        c.ell(x, y, x + 4, y + 4, C(col), K); c.p(x + 1, y + 1, sh(C(col), 0.6))
    # brush
    c.line([(14, 28), (28, 12)], K, 3); c.line([(14, 28), (28, 12)], C(0xA06828), 1)
    c.poly([(11, 31), (13, 26), (16, 29)], C(0xE02020), K)
    c.shadow(); return c.im

def snake():
    c = Cv()
    c.box(2, 2, 29, 29, C(0x103010)); c.bevel(3, 3, 28, 28, C(0x1C4A1C), False)
    for g in range(6, 28, 4): c.r(g, 4, g, 27, C(0x225822)); c.r(4, g, 27, g, C(0x225822))
    body = [(7, 24), (11, 24), (15, 24), (15, 20), (15, 16), (19, 16), (23, 16), (23, 12), (23, 8)]
    for (x, y) in body:
        c.box(x - 2, y - 2, x + 2, y + 2, C(0x30C830), C(0x0A5A0A)); c.r(x - 1, y - 1, x + 1, y - 1, C(0x80F080))
    x, y = 23, 8
    c.box(x - 3, y - 3, x + 3, y + 2, C(0x40E040), C(0x0A5A0A))
    c.p(x - 2, y - 2, C(0xFFFFFF)); c.p(x + 2, y - 2, C(0xFFFFFF)); c.p(x - 2, y - 1, K); c.p(x + 2, y - 1, K)
    c.r(x, y - 5, x, y - 4, C(0xE02020)); c.p(x - 1, y - 6, C(0xE02020)); c.p(x + 1, y - 6, C(0xE02020))
    c.ell(6, 8, 10, 12, C(0xE03030), K); c.p(7, 9, C(0xFFA0A0))                    # apple
    c.shadow(); return c.im

def settings():
    c = Cv()
    cx = cy = 15.5
    pts = []
    for i in range(16):                                                                # 8 teeth gear polygon
        a0 = i * math.pi / 8
        rr = 14 if i % 2 == 0 else 10.5
        for da in (-0.17, 0.17) if i % 2 == 0 else (-0.2, 0.2):
            a = a0 + da; pts.append((cx + rr * math.cos(a), cy + rr * math.sin(a)))
    c.poly(pts, C(0xA0A8B8), K)
    c.ell(7, 7, 24, 24, C(0x8890A4), K)
    c.d.arc([8, 8, 23, 23], 180, 280, fill=C(0xE8ECF8), width=1)
    c.ell(12, 12, 19, 19, C(0x303848), K)
    c.d.arc([12, 12, 19, 19], 180, 270, fill=C(0x7080A0))
    c.shadow(); return c.im

def globe():
    c = Cv()
    c.ell(2, 2, 29, 29, C(0x1850C8), K)
    c.ell(3, 3, 28, 28, C(0x2878E8))
    # continents (clipped to circle)
    land = Image.new('L', (32, 32), 0); ld = ImageDraw.Draw(land)
    ld.polygon([(7, 7), (13, 5), (15, 8), (12, 11), (14, 15), (11, 20), (8, 17), (6, 12)], fill=255)
    ld.polygon([(18, 10), (24, 9), (27, 14), (24, 18), (21, 25), (18, 21), (19, 15)], fill=255)
    ld.polygon([(12, 24), (16, 26), (14, 28), (10, 27)], fill=255)
    mask = Image.new('L', (32, 32), 0); ImageDraw.Draw(mask).ellipse([3, 3, 28, 28], fill=255)
    for y in range(32):
        for x in range(32):
            if land.getpixel((x, y)) and mask.getpixel((x, y)):
                c.p(x, y, C(0x38B048) if (x + y) % 5 else C(0x2C9A3C))
    c.d.arc([3, 3, 28, 28], 0, 360, fill=C(0x0C3C98))
    for (a, b) in ((14, 8), (11, 5)):
        pass
    c.d.arc([9, 3, 22, 28], 0, 360, fill=C(0x90C0FF)); c.d.line([(3, 15), (28, 15)], fill=C(0x90C0FF))
    c.d.arc([3, 8, 28, 22], 0, 180, fill=C(0x90C0FF))
    c.glow((7, 5, 13, 9), 70)                                            # specular
    c.shadow(); return c.im

def about():
    c = Cv()
    c.ell(2, 2, 29, 29, C(0x000080), K); c.ell(3, 3, 28, 28, C(0x1060D0))
    c.glow((5, 4, 26, 15), 60)
    c.d.ellipse([3, 3, 28, 28], outline=C(0x80C0FF))
    c.r(14, 7, 17, 10, C(0xFFFFFF)); c.r(13, 13, 17, 14, C(0xFFFFFF)); c.r(14, 14, 17, 22, C(0xFFFFFF))
    c.r(12, 23, 19, 24, C(0xFFFFFF)); c.r(12, 13, 13, 14, C(0xFFFFFF))
    c.ell(2, 2, 29, 29, (0, 0, 0, 0)) if False else None
    c.d.ellipse([2, 2, 29, 29], outline=K)
    c.shadow(); return c.im

def computer():
    c = Cv()
    # monitor
    c.box(3, 2, 28, 19, C(0xD8D4C8)); c.bevel(4, 3, 27, 18, C(0xD8D4C8))
    c.box(6, 5, 25, 15, K, C(0x606060)); c.vg(7, 6, 24, 14, C(0x40A0F0), C(0x1060C0))
    c.r(8, 7, 14, 7, C(0xB0E0FF))
    c.r(11, 19, 20, 20, C(0xB8B4A8)); c.r(8, 21, 23, 22, C(0xD8D4C8)); c.r(8, 22, 23, 22, C(0x808080)); c.r(8, 21, 23, 21, C(0xFFFFFF))
    c.r(26, 17, 26, 17, C(0x40E040))
    # base unit
    c.box(2, 24, 29, 29, C(0xD8D4C8)); c.r(3, 25, 28, 25, C(0xFFFFFF))
    c.r(5, 27, 14, 27, K); c.r(18, 27, 24, 27, C(0x808080)); c.p(26, 27, C(0x40E040))
    c.shadow(); return c.im

def drive():
    c = Cv()
    c.poly([(3, 12), (8, 6), (27, 6), (30, 12)], C(0xE8E8E8), K)
    c.box(2, 12, 29, 24, C(0xB4B4B4)); c.bevel(3, 13, 28, 23, C(0xC8C8C8))
    c.r(6, 18, 22, 18, C(0x606060)); c.r(6, 19, 22, 19, C(0xF0F0F0))
    c.r(24, 17, 26, 19, C(0x30D030)); c.d.rectangle([24, 17, 26, 19], outline=C(0x106010))
    c.r(3, 25, 28, 26, C(0x606060)); c.r(2, 24, 29, 24, K)
    c.shadow(); return c.im

def recycle():
    c = Cv()
    c.poly([(6, 8), (25, 8), (23, 29), (8, 29)], C(0xC8CCD0), K)
    for x in range(9, 24, 3): c.line([(x, 11), (x - (x - 15) // 7, 27)], C(0x808890))
    c.r(7, 9, 24, 9, C(0xFFFFFF))
    c.box(4, 5, 27, 8, C(0xB0B4B8)); c.r(5, 6, 26, 6, C(0xF0F0F0))
    c.box(12, 2, 19, 5, C(0xA0A4A8)); c.r(14, 3, 17, 3, C(0x606468))
    G = C(0x18A028)
    c.d.arc([10, 12, 21, 23], 40, 330, fill=G, width=2)
    c.poly([(19, 11), (24, 15), (18, 16)], G)
    c.shadow(); return c.im

def logo():
    c = Cv()
    c.ell(1, 1, 30, 30, C(0x600000), K); c.vg(3, 3, 28, 28, C(0xE83030), C(0x900808))
    c.ell(1, 1, 30, 30, (0, 0, 0, 0)) if False else None
    m = Image.new('L', (32, 32), 0); ImageDraw.Draw(m).ellipse([2, 2, 29, 29], fill=255)
    base = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    for y in range(32):
        t = y / 31; col = (int(240 - 110 * t), int(60 - 50 * t), int(60 - 50 * t), 255)
        for x in range(32):
            if m.getpixel((x, y)): base.putpixel((x, y), col)
    c.im = base; c.d = ImageDraw.Draw(c.im)
    c.d.ellipse([1, 1, 30, 30], outline=K)
    c.d.arc([3, 3, 28, 28], 200, 330, fill=(255, 190, 190, 255))
    W = C(0xFFFFFF)                                                                      # "W"
    for x0, x1 in ((7, 10), (13, 16), (19, 22)): pass
    c.line([(7, 9), (10, 22)], W, 3); c.line([(10, 22), (16, 13)], W, 3)
    c.line([(16, 13), (22, 22)], W, 3); c.line([(22, 22), (25, 9)], W, 3)
    c.shadow(); return c.im

def power():
    c = Cv()
    c.ell(2, 2, 29, 29, C(0xA00000), K); c.ell(3, 3, 28, 28, C(0xE02828))
    c.glow((5, 4, 26, 14), 70)
    c.d.arc([8, 9, 23, 24], 300, 240, fill=C(0xFFFFFF), width=3)
    c.r(14, 6, 16, 16, C(0xFFFFFF))
    c.shadow(); return c.im

def user():
    c = Cv()
    c.box(3, 3, 28, 28, C(0xC0C0C0)); c.bevel(4, 4, 27, 27, C(0x7090C8), False)
    c.ell(11, 7, 20, 16, C(0xF0C8A0), K)
    c.poly([(6, 28), (7, 21), (12, 18), (19, 18), (25, 21), (26, 28)], C(0x2050A0), K)
    c.r(14, 16, 17, 18, C(0xF0C8A0))
    c.shadow(); return c.im

def rxe():
    c = Cv()
    c.box(2, 4, 29, 27, GRAY); c.bevel(3, 5, 28, 26, GRAY)
    c.r(4, 6, 27, 9, C(0x600000)); c.vg(4, 6, 27, 9, C(0xE04040), C(0x801010)); c.r(23, 7, 26, 8, C(0xFFFFFF))
    c.r(4, 10, 27, 25, C(0xF2F3FA)); c.r(4, 10, 27, 10, C(0xB0B0C0))
    c.box(7, 13, 14, 17, C(0xE8E8E8), C(0x404040)); c.r(8, 14, 13, 14, C(0xFFFFFF))
    c.line([(16, 15), (25, 15)], C(0x808090)); c.line([(16, 18), (23, 18)], C(0x808090))
    c.d.polygon([(9, 20), (14, 23), (9, 26)], fill=C(0xD02020), outline=K)
    c.shadow(); return c.im

def rpp():
    c = Cv()
    c.poly([(6, 2), (21, 2), (27, 8), (27, 29), (6, 29)], C(0xFFFFFF), K)
    c.poly([(21, 2), (21, 8), (27, 8)], LIGHT, K)
    # "{ }" code braces in red + text lines
    R = C(0xC02020)
    for p_ in ((11, 12), (10, 13), (10, 14), (9, 15), (10, 16), (10, 17), (11, 18)): c.p(*p_, R)
    for p_ in ((21, 12), (22, 13), (22, 14), (23, 15), (22, 16), (22, 17), (21, 18)): c.p(*p_, R)
    c.r(14, 14, 18, 14, C(0x4060C0)); c.r(14, 17, 17, 17, C(0x4060C0))
    c.r(9, 22, 24, 22, C(0x808090)); c.r(9, 25, 20, 25, C(0x808090))
    c.r(26, 9, 26, 28, LIGHT); c.r(7, 28, 26, 28, LIGHT)
    c.shadow(); return c.im

ICONS = [("folder", folder), ("folder_open", folder_open), ("document", document), ("notepad", notepad),
         ("calculator", calculator), ("terminal", terminal), ("paint", paint), ("snake", snake),
         ("settings", settings), ("browser", globe), ("about", about), ("computer", computer),
         ("drive", drive), ("recycle_bin", recycle), ("redwill", logo), ("shutdown", power), ("user", user), ("rxe", rxe), ("rpp", rpp)]

def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'inter32', 'icons')
    os.makedirs(out, exist_ok=True)
    for name, fn in ICONS:
        im = fn()
        im.save(os.path.join(out, name + '.ico'), format='ICO', sizes=[(32, 32)])
        print('wrote', name + '.ico')
if __name__ == '__main__': main()
