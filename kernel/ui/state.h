/* ---------- state ---------- */
static int ph = 0; /* 0 boot, 1 setup, 2 login, 3 desktop */
static char cname[32] = "REDWILL-PC", uname_[32] = "RedWill User", upass[32];
typedef struct { char b[32]; int n; } Fld;
static void fld_set(Fld *f, const char *s) { f->n = 0; while (s[f->n] && f->n < 30) { f->b[f->n] = s[f->n]; f->n++; } f->b[f->n] = 0; }
static void fld_key(Fld *f, int ch, int max) {
    if (ch == 8) { if (f->n) f->b[--f->n] = 0; }
    else if (ch >= 32 && ch < 127 && f->n < max) { f->b[f->n++] = (char)ch; f->b[f->n] = 0; }
}
static int mx = 512, my = 384, mb = 0, mrb = 0, dirty = 1, ctrl = 0, shift = 0, ext = 0;
static int inr(int x, int y, int w, int h) { return mx >= x && mx < x + w && my >= y && my < y + h; }
static void button(int x, int y, int w, int h, const char *l, int on) {
    rect(x, y, w, h, on ? T.pill : 0xE4E5F2); bevel(x, y, w, h, 1);
    str(x + (w - slen(l) * 8) / 2, y + (h - 16) / 2, l, on ? 0xFFFFFF : 0x000000);
}
static void xlogo(int x, int y, int s, u32 c1, u32 c2) { int t = s / 5 + 1; for (int i = 0; i < s - t; i++) { rect(x + i, y + i, t, t, c1); rect(x + s - t - i, y + i, t, t, c2); } }

