/* ---------- icons ---------- */
static int IS = 32;
static void ir(int ox, int oy, int ax, int ay, int w, int h, u32 c) { int ww = w * IS / 32, hh = h * IS / 32; if (ww < 1) ww = 1; if (hh < 1) hh = 1; rect(ox + ax * IS / 32, oy + ay * IS / 32, ww, hh, c); }
static void icon(int k, int x, int y, int sz) {
    IS = sz;
    if (k == 0) { ir(x, y, 5, 2, 22, 28, 0xFFFFFF); ir(x, y, 5, 2, 22, 6, 0x3A6FD8); for (int i = 0; i < 4; i++) ir(x, y, 9, 12 + i * 5, 14, 2, 0x6080C0); }
    else if (k == 1) { ir(x, y, 5, 2, 22, 28, 0xB8B8B8); ir(x, y, 8, 5, 16, 6, 0xCFE8C8); for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) ir(x, y, 8 + c * 6, 14 + r * 5, 4, 3, 0x404040); }
    else if (k == 2) { ir(x, y, 2, 8, 28, 20, 0xF0C830); ir(x, y, 2, 5, 12, 5, 0xE0B020); ir(x, y, 2, 12, 28, 16, 0xFAD850); }
    else if (k == 3) { ir(x, y, 2, 4, 28, 24, 0x202028); ir(x, y, 5, 8, 6, 2, 0x70F070); ir(x, y, 9, 12, 2, 2, 0x70F070); ir(x, y, 13, 18, 10, 2, 0xC0C0C0); }
    else if (k == 4) { ir(x, y, 3, 6, 26, 20, 0xD8C090); ir(x, y, 7, 10, 5, 5, 0xE02020); ir(x, y, 14, 9, 5, 5, 0x2050E0); ir(x, y, 20, 14, 5, 5, 0x20A030); ir(x, y, 9, 17, 5, 5, 0xF0D020); }
    else if (k == 5) { ir(x, y, 4, 3, 24, 26, T.pill); ir(x, y, 14, 7, 4, 4, 0xFFFFFF); ir(x, y, 14, 13, 4, 11, 0xFFFFFF); }
    else if (k == 6) { ir(x, y, 3, 3, 26, 26, 0x203820); ir(x, y, 6, 20, 16, 4, 0x40E040); ir(x, y, 18, 10, 4, 14, 0x40E040); ir(x, y, 10, 8, 10, 4, 0x40E040); ir(x, y, 24, 7, 4, 4, 0xE03030); }
    else if (k == 7) { ir(x, y, 6, 6, 20, 20, 0x8890A0); ir(x, y, 10, 10, 12, 12, 0x4A5060); ir(x, y, 13, 13, 6, 6, 0xC8D0E0); ir(x, y, 14, 1, 4, 5, 0x8890A0); ir(x, y, 14, 26, 4, 5, 0x8890A0); ir(x, y, 1, 14, 5, 4, 0x8890A0); ir(x, y, 26, 14, 5, 4, 0x8890A0); }
    else if (k == 8) { ir(x, y, 3, 3, 26, 26, 0x2A70D8); ir(x, y, 8, 6, 8, 5, 0x40C060); ir(x, y, 16, 15, 9, 8, 0x40C060); ir(x, y, 5, 14, 22, 2, 0xA0D0FF); ir(x, y, 15, 3, 2, 26, 0xA0D0FF); }
    else if (k == 9) { ir(x, y, 4, 4, 24, 24, 0xFFFFFF); ir(x, y, 8, 8, 16, 3, 0x6080C0); ir(x, y, 8, 14, 16, 3, 0x6080C0); ir(x, y, 8, 20, 10, 3, 0x6080C0); }
    IS = 32;
}
static const u8 SMI[9] = { 2, 8, 0, 3, 4, 1, 6, 7, 5 }; /* start-menu order */
static const u8 DESKI[6] = { 2, 8, 0, 3, 4, 6 };

