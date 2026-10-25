/* ---------- state ---------- */
static int ph = 0; /* 0 boot, 1 setup, 2 login, 3 desktop */
static char cname[32] = "REDWILL-PC", uname_[32] = "RedWill User", upass[32];
typedef struct { char b[96]; int n; } Fld;
static void fld_set(Fld *f, const char *s) { f->n = 0; while (s[f->n] && f->n < 90) { f->b[f->n] = s[f->n]; f->n++; } f->b[f->n] = 0; }
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
/* RedWill "W" logo: four diagonal strokes inside an s x s box (c1 = left half, c2 = right half) */
static void xlogo(int x, int y, int s, u32 c1, u32 c2) {
    int t = s / 5 + 1, w = s - t, q = w / 4, m = w * 35 / 100;
    for (int i = 0; i <= w; i++) {
        int dx0 = i * q / w, dy = i;                       /* stroke 1: down-right  (0,0)->(q,w) */
        rect(x + dx0, y + dy, t, t, c1);
        rect(x + q + i * q / w, y + w - i * (w - m) / w, t, t, c1);          /* stroke 2: up to the middle peak */
        rect(x + 2 * q + i * q / w, y + m + i * (w - m) / w, t, t, c2);      /* stroke 3: down from the peak */
        rect(x + 3 * q + i * q / w, y + w - i, t, t, c2);                    /* stroke 4: up to the top right */
    }
}

