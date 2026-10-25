static int SW = 1024, SH = 768, PITCH = 4096;
static u32 *FB, *BACK, *WALL, *cv, *PCV;
static const u8 *FONT;
#define PW 380
#define PH 250

/* ---------- drawing ---------- */
static u32 mix(u32 a, u32 b, int t, int n) {
    if (n <= 0 || t <= 0) return a;
    if (t > n) t = n;
    int ar = (a >> 16) & 255, ag = (a >> 8) & 255, ab = a & 255, br = (b >> 16) & 255, bg = (b >> 8) & 255, bb = b & 255;
    return (u32)(((ar + (br - ar) * t / n) << 16) | ((ag + (bg - ag) * t / n) << 8) | (ab + (bb - ab) * t / n));
}
static void px(int x, int y, u32 c) { if ((unsigned)x < (unsigned)SW && (unsigned)y < (unsigned)SH) cv[y * SW + x] = c; }
static void rect(int x, int y, int w, int h, u32 c) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > SW) w = SW - x;
    if (y + h > SH) h = SH - y;
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++) { u32 *p = cv + (y + j) * SW + x; for (int i = 0; i < w; i++) p[i] = c; }
}
static void vgrad(int x, int y, int w, int h, u32 a, u32 b) { for (int j = 0; j < h; j++) rect(x, y + j, w, 1, mix(a, b, j, h > 1 ? h - 1 : 1)); }
static void frame(int x, int y, int w, int h, u32 c) { rect(x, y, w, 1, c); rect(x, y + h - 1, w, 1, c); rect(x, y, 1, h, c); rect(x + w - 1, y, 1, h, c); }
static void bevel(int x, int y, int w, int h, int up) {
    u32 hi = 0xFFFFFF, lo = 0x808080;
    if (!up) { u32 t = hi; hi = lo; lo = t; }
    rect(x, y, w, 1, hi); rect(x, y, 1, h, hi); rect(x, y + h - 1, w, 1, lo); rect(x + w - 1, y, 1, h, lo);
}
static void disc(int cx, int cy, int r, u32 c) {
    for (int dy = -r; dy <= r; dy++) { int w = 0; while ((w + 1) * (w + 1) <= r * r - dy * dy) w++; rect(cx - w, cy + dy, 2 * w + 1, 1, c); }
}
static void pill(int x, int y, int w, int h, u32 c) {
    int r = h / 2; rect(x, y, w - r, h, c);
    for (int dy = -r; dy < r; dy++) { int d = 0; while ((d + 1) * (d + 1) <= r * r - dy * dy) d++; rect(x + w - r, y + r + dy, d, 1, c); }
}
static void chr(int x, int y, int ch, u32 c) {
    const u8 *g = FONT + (ch & 255) * 16;
    for (int j = 0; j < 16; j++) { u8 b = g[j]; if (!b) continue; for (int i = 0; i < 8; i++) if (b & (0x80 >> i)) px(x + i, y + j, c); }
}
static int slen(const char *s) { int n = 0; while (s[n]) n++; return n; }
static void str(int x, int y, const char *s, u32 c) { for (; *s; s++, x += 8) chr(x, y, *s, c); }
static void bstr(int x, int y, const char *s, u32 c) { str(x, y, s, c); str(x + 1, y, s, c); }
static void cstr(int cx, int y, const char *s, u32 c) { str(cx - slen(s) * 4, y, s, c); }
static void big(int x, int y, const char *s, u32 c, int k) {
    for (; *s; s++, x += 8 * k) { const u8 *g = FONT + ((u8)*s) * 16; for (int j = 0; j < 16; j++) for (int i = 0; i < 8; i++) if (g[j] & (0x80 >> i)) rect(x + i * k, y + j * k, k, k, c); }
}
static char *sapp(char *d, const char *s) { while (*s) *d++ = *s++; *d = 0; return d; }
static char *iapp(char *d, int v) {
    char t[12]; int n = 0; unsigned u = v < 0 ? (unsigned)(-v) : (unsigned)v;
    if (v < 0) *d++ = '-';
    if (!u) t[n++] = '0';
    while (u) { t[n++] = (char)('0' + u % 10); u /= 10; }
    while (n) *d++ = t[--n];
    *d = 0; return d;
}
static char *two(char *d, int v) { *d++ = (char)('0' + v / 10 % 10); *d++ = (char)('0' + v % 10); *d = 0; return d; }
static int scmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (u8)*a - (u8)*b; }
static char up_(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }
static char lo_(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }
static void flip(void) {
    for (int y = 0; y < SH; y++) {
        u32 *d = (u32 *)((u8 *)FB + y * PITCH), *s = BACK + y * SW;
#ifndef HOST
        int n = SW; __asm__ volatile("rep movsl" : "+D"(d), "+S"(s), "+c"(n)::"memory");
#else
        memcpy(d, s, SW * 4);
#endif
    }
}

/* ---------- themes ---------- */
typedef struct { u32 bar, bard, bardd, pill, pilld, menu, ta, tb, wa, wb, cl, txt; const char *name; } Th;
static const Th TH[3] = {
    { 0xA7AADA, 0x9B9ECB, 0x888CC7, 0x3F48CC, 0x2A3088, 0xB2B4D9, 0x3F48CC, 0x8E94EC, 0x7C80D0, 0xD6D8F6, 0xEEEFFA, 0x1E2260, "Lavender" },
    { 0xA9C8E8, 0x93B6DA, 0x7FA4CC, 0x1E64C8, 0x14408C, 0xB8D2EC, 0x1E64C8, 0x6AAEF0, 0x2F79D0, 0xBFE0FA, 0xEEF5FC, 0x0E2A5A, "Ocean" },
    { 0x4A4D58, 0x3E414B, 0x32343D, 0x7A5CE0, 0x4B34A0, 0x555966, 0x5A4CC0, 0x8E80F0, 0x20222C, 0x4A4E66, 0xD8DAE4, 0xE8E8F8, "Graphite" } };
static int th = 0, clk24 = 0, snd = 1;
#define T TH[th]

