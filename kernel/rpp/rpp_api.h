/* ---------- RedWill API: runs .rpp / .rxe programs and gives them windows, controls, drawing, dialogs, files ---------- */
#define NVM 6
#define NSWIN 8
#define NCTL 128
#define NITEMS 24
enum { C_LABEL = 1, C_BUTTON, C_TEXT, C_CHECK, C_LIST, C_PROG };
typedef struct { u8 used, type, en, vis, chk, nitems; short win; int x, y, w, h, val, handler, sel; char text[64]; char items[NITEMS][28]; } Ctl;
typedef struct { u8 used, vm; char title[40]; int cw, ch, onpaint, onclick, onkey, ontimer, onclose, tint, focus; u32 tlast; } ScWin;
static Vm *VMS[NVM]; static u8 vm_exit[NVM];
static Ctl *CT; static ScWin SWIN[NSWIN];
static Cc CCS;
static int pc_x, pc_y, pc_w, pc_h, in_paint, ev_mx, ev_my;
static void rpp_set_pools(void *fsp, void *vms, int stride, void *ct) {
    FSP = (u8 *)fsp; for (int i = 0; i < NVM; i++) { VMS[i] = (Vm *)((u8 *)vms + (long)i * stride); rp_zero(VMS[i], (int)sizeof(Vm)); }
    CT = (Ctl *)ct; rp_zero(CT, (int)(sizeof(Ctl) * NCTL));
}
static int vm_windows(Vm *vm) { int n = 0; for (int i = 0; i < NSWIN; i++) if (SWIN[i].used && SWIN[i].vm == vm->id) n++; return n; }
static void swin_free(int i) {
    for (int k = 0; k < NCTL; k++) if (CT[k].used && CT[k].win == i) CT[k].used = 0;
    SWIN[i].used = 0; W[9 + i].open = 0; dirty = 1;
}
static void vm_kill(Vm *vm) { for (int i = 0; i < NSWIN; i++) if (SWIN[i].used && SWIN[i].vm == vm->id) swin_free(i); vm->used = 0; dirty = 1; }
static int vm_invoke(Vm *vm, int fn, const int *args, int na) {
    int r = rp_call(vm, fn, args, na);
    if (vm->err) { char m[140]; char *e = sapp(m, vm->errmsg); (void)e; dlg_message("Program error", m, 0); music(J_ERR, 3, 0); vm_kill(vm); }
    else if (vm_exit[vm->id] || !vm_windows(vm)) vm_kill(vm);
    dirty = 1; return r;
}
static int rpp_host_cb(Vm *vm, int id, int *a);
static int rpp_launch(const char *name, const char *src, const u8 *bin, int blen) {
    int i = -1; for (int k = 0; k < NVM; k++) if (!VMS[k]->used) { i = k; break; }
    if (i < 0) { dlg_message("RedWill", "Too many programs are running (max 6). Close one first.", 0); return 0; }
    Vm *vm = VMS[i]; int bad = src ? rpp_compile(vm, src, &CCS) : rpp_load_rxe(vm, bin, blen);
    if (bad) { char m[200]; char *e = sapp(m, name); e = sapp(e, ": "); sapp(e, vm->errmsg); dlg_message("Cannot run program", m, 0); music(J_ERR, 3, 0); return 0; }
    vm->used = 1; vm->id = i; vm->busy = 0; vm->rng = (now_ms() * 2654435761u) | 1; vm_exit[i] = 0; rpp_host = rpp_host_cb;
    vm_invoke(vm, 0, 0, 0);
    return 1;
}
static int rpp_run_node(int n) {
    if (n < 0 || fsn[n].dir) return 0;
    if (fs_isext(n, "rxe")) return rpp_launch(fsn[n].name, 0, (const u8 *)fs_data(n), fsn[n].len);
    return rpp_launch(fsn[n].name, fs_data(n), 0, 0);
}
/* compile a script node to a .rxe file next to it (or at outpath); returns message in msg */
static int rpp_compile_node(int n, const char *outname, int outdir, char *msg) {
    static Vm *cvm; cvm = VMS[NVM - 1]; if (cvm->used) { sapp(msg, "all VM slots are busy"); return 0; }
    if (rpp_compile(cvm, fs_data(n), &CCS)) { char *e = sapp(msg, fsn[n].name); e = sapp(e, ": "); sapp(e, cvm->errmsg); return 0; }
    static u8 buf[40000]; int sz = rpp_save_rxe(cvm, buf); int o = fs_mk(outdir, outname, 0);
    if (o < 0 || !fs_set(o, buf, sz)) { sapp(msg, "cannot write output file"); return 0; }
    char *e = sapp(msg, outname); e = sapp(e, " written ("); e = iapp(e, sz); sapp(e, " bytes)"); return 1;
}

/* ---- script windows ---- */
static ScWin *win_get(Vm *vm, int h) { int s = h - 1; return (s >= 0 && s < NSWIN && SWIN[s].used && SWIN[s].vm == vm->id) ? &SWIN[s] : 0; }
static Ctl *ctl_get(Vm *vm, int h) { int k = h - 1; if (k < 0 || k >= NCTL || !CT[k].used) return 0; return win_get(vm, CT[k].win + 1) ? &CT[k] : 0; }
static int ctl_new(Vm *vm, int win, int type, int x, int y, int w, int h, const char *text, int handler) {
    ScWin *sw = win_get(vm, win); if (!sw) return 0;
    for (int k = 0; k < NCTL; k++) if (!CT[k].used) {
        Ctl *c = &CT[k]; rp_zero(c, (int)sizeof(Ctl)); c->used = 1; c->type = (u8)type; c->en = 1; c->vis = 1; c->win = (short)(win - 1);
        c->x = x; c->y = y; c->w = w; c->h = h; c->handler = handler; c->sel = -1; if (text) rp_cpy(c->text, text, 64); return k + 1;
    }
    rp_fail(vm, "too many controls"); return 0;
}
static void sw_clip_rect(int x, int y, int w, int h, u32 c) {
    int x1 = x + w, y1 = y + h; if (x < 0) x = 0; if (y < 0) y = 0; if (x1 > pc_w) x1 = pc_w; if (y1 > pc_h) y1 = pc_h;
    if (x1 > x && y1 > y) rect(pc_x + x, pc_y + y, x1 - x, y1 - y, c);
}
static void sw_text(int x, int y, const char *s, u32 c) { for (; *s; s++, x += 8) if (x >= 0 && y >= 0 && x + 8 <= pc_w && y + 16 <= pc_h) chr(pc_x + x, pc_y + y, *s, c); }
static void swin_draw(int w, int cx, int cy, int cw, int ch) {
    int s = w - 9; ScWin *sw = &SWIN[s]; if (!sw->used) return;
    pc_x = cx; pc_y = cy; pc_w = cw; pc_h = ch;
    rect(cx, cy, cw, ch, 0xF2F3FA);
    if (sw->onpaint) { in_paint = 1; int h = s + 1; vm_invoke(VMS[sw->vm], sw->onpaint, &h, 1); in_paint = 0; if (!sw->used) return; pc_x = cx; pc_y = cy; pc_w = cw; pc_h = ch; }
    int act = (w == top_win());
    for (int k = 0; k < NCTL; k++) {
        Ctl *c = &CT[k]; if (!c->used || c->win != s || !c->vis) continue;
        int x = cx + c->x, y = cy + c->y; if (c->type != C_LABEL && (c->x < 0 || c->y < 0 || c->x + c->w > cw || c->y + c->h > ch)) continue;
        u32 fg = c->en ? 0x101030 : 0x909098;
        if (c->type == C_LABEL) { const char *p = c->text; int yy = c->y; char ln[64]; for (;;) { int n = 0; while (*p && *p != '\n' && n < 62) ln[n++] = *p++; ln[n] = 0; sw_text(c->x, yy, ln, fg); yy += 16; if (*p == '\n') p++; else break; } }
        else if (c->type == C_BUTTON) { button(x, y, c->w, c->h, c->text, 0); if (!c->en) rect(x + 2, y + 2, c->w - 4, c->h - 4, 0xD8D9E4), str(x + (c->w - slen(c->text) * 8) / 2, y + (c->h - 16) / 2, c->text, 0x909098); }
        else if (c->type == C_TEXT) {
            rect(x, y, c->w, c->h, 0xFFFFFF); frame(x, y, c->w, c->h, (sw->focus == k + 1 && act) ? T.pill : 0x808090);
            int vis = (c->w - 8) / 8, n = slen(c->text), off = n > vis ? n - vis : 0; str(x + 4, y + (c->h - 16) / 2, c->text + off, 0);
            if (sw->focus == k + 1 && act && ((ms_acc / 500) & 1)) rect(x + 4 + (n - off) * 8, y + (c->h - 16) / 2, 2, 14, 0);
        } else if (c->type == C_CHECK) {
            rect(x, y, 16, 16, 0xFFFFFF); bevel(x, y, 16, 16, 0); if (c->chk) { rect(x + 4, y + 4, 8, 8, T.pill); }
            str(x + 24, y, c->text, fg);
        } else if (c->type == C_LIST) {
            rect(x, y, c->w, c->h, 0xFFFFFF); frame(x, y, c->w, c->h, 0x7F9DB9); int rows = (c->h - 4) / 16, first = 0; if (c->sel >= rows) first = c->sel - rows + 1;
            for (int r = 0; r < rows && first + r < c->nitems; r++) { int i = first + r; if (i == c->sel) rect(x + 2, y + 2 + r * 16, c->w - 4, 16, T.pill); str(x + 4, y + 2 + r * 16, c->items[i], i == c->sel ? 0xFFFFFF : 0x101030); }
        } else if (c->type == C_PROG) {
            rect(x, y, c->w, c->h, 0xFFFFFF); frame(x, y, c->w, c->h, 0x606080); int v = c->val < 0 ? 0 : c->val > 100 ? 100 : c->val; rect(x + 2, y + 2, (c->w - 4) * v / 100, c->h - 4, T.pill);
        }
    }
}
static void swin_closed(int w) {
    int s = w - 9; if (s < 0 || s >= NSWIN || !SWIN[s].used) return;
    Vm *vm = VMS[SWIN[s].vm]; swin_free(s); if (vm->used && !vm_windows(vm)) vm_kill(vm);
}
static int swin_try_close(int w) {                     /* returns 1 if the script cancelled the close */
    int s = w - 9; if (s < 0 || s >= NSWIN || !SWIN[s].used) return 0;
    if (SWIN[s].onclose) { int h = s + 1; Vm *vm = VMS[SWIN[s].vm]; int r = vm_invoke(vm, SWIN[s].onclose, &h, 1); if (!SWIN[s].used) return 1; if (r) return 1; }
    return 0;
}
static void swin_click(int w, int rx, int ry) {
    int s = w - 9; ScWin *sw = &SWIN[s]; if (!sw->used) return; Vm *vm = VMS[sw->vm];
    for (int k = NCTL - 1; k >= 0; k--) {
        Ctl *c = &CT[k]; if (!c->used || c->win != s || !c->vis || !c->en) continue;
        if (rx < c->x || ry < c->y || rx >= c->x + c->w || ry >= c->y + c->h) continue;
        int h = k + 1;
        if (c->type == C_BUTTON) { if (c->handler) vm_invoke(vm, c->handler, &h, 1); }
        else if (c->type == C_TEXT) sw->focus = h;
        else if (c->type == C_CHECK) { c->chk = !c->chk; if (c->handler) { int a[2] = { h, c->chk }; vm_invoke(vm, c->handler, a, 2); } }
        else if (c->type == C_LIST) {
            int rows = (c->h - 4) / 16, first = c->sel >= rows ? c->sel - rows + 1 : 0, i = first + (ry - c->y - 2) / 16;
            if (i >= 0 && i < c->nitems) { c->sel = i; if (c->handler) { int a[2] = { h, i }; vm_invoke(vm, c->handler, a, 2); } }
        }
        return;
    }
    sw->focus = 0;
    if (sw->onclick) { int a[2] = { rx, ry }; ev_mx = rx; ev_my = ry; vm_invoke(vm, sw->onclick, a, 2); }
}
static void swin_key(int w, int ch) {
    int s = w - 9; ScWin *sw = &SWIN[s]; if (!sw->used) return; Vm *vm = VMS[sw->vm];
    if (sw->focus && CT[sw->focus - 1].used && CT[sw->focus - 1].type == C_TEXT) {
        Ctl *c = &CT[sw->focus - 1]; int n = slen(c->text);
        if (ch == 8) { if (n) c->text[n - 1] = 0; } else if (ch >= 32 && ch < 127 && n < 62) { c->text[n] = (char)ch; c->text[n + 1] = 0; } else if (ch != '\n') return;
        if (ch != '\n') return;
    }
    if (sw->onkey) { int a[1] = { ch }; vm_invoke(vm, sw->onkey, a, 1); }
}
static void rpp_tick(u32 t) {
    for (int i = 0; i < NSWIN; i++) {
        ScWin *sw = &SWIN[i]; if (!sw->used || !sw->ontimer || sw->tint <= 0) continue;
        if (t - sw->tlast >= (u32)sw->tint) { sw->tlast = t; int h = i + 1; vm_invoke(VMS[sw->vm], sw->ontimer, &h, 1); }
    }
}

/* ---- paths ---- */
static int rpp_write_file(const char *path, const char *text, int append) {
    char nm[24]; int dir; if (!fs_split(path, nm, &dir)) return 0;
    int n = fs_mk(dir, nm, 0); if (n < 0) return 0;
    if (!append) return fs_set(n, text, slen(text));
    int ol = fsn[n].len, tl = slen(text); static char tmp[16384]; if (ol + tl > 16380) return 0;
    for (int i = 0; i < ol; i++) tmp[i] = fs_data(n)[i]; for (int i = 0; i <= tl; i++) tmp[ol + i] = text[i]; return fs_set(n, tmp, ol + tl);
}

/* ---- host builtins ---- */
static int rpp_host_cb(Vm *vm, int id, int *a) {
    char b0[12], b1[12], b2[12]; ScWin *sw; Ctl *c;
    switch (id) {
    case 100: tprint(rp_tostr(vm, a[0], b0)); return 0;
    case 101: vm_exit[vm->id] = 1; vm->busy = 2; return 0;
    case 102: return (int)now_ms();
    case 103: { int ms = a[0] > 5000 ? 5000 : a[0]; u32 t0 = now_ms(); while (now_ms() - t0 < (u32)ms) os_pump(); return 0; }
    case 104: beep((u32)a[0], (u32)(a[1] > 2000 ? 2000 : a[1])); return 0;
    case 105: { char b[8]; char *p = two(b, rh); *p++ = ':'; two(p, rm); return rp_cstr(vm, b); }
    case 106: return rp_cstr(vm, uname_);
    case 107: return SW; case 108: return SH;
    case 109: return ev_mx; case 110: return ev_my;
    case 120: {                                                   /* window(title, w, h) */
        int s = -1; for (int i = 0; i < NSWIN; i++) if (!SWIN[i].used) { s = i; break; }
        if (s < 0) { rp_fail(vm, "too many windows"); return 0; }
        int cw = a[1] < 80 ? 80 : a[1] > SW - 40 ? SW - 40 : a[1], ch = a[2] < 40 ? 40 : a[2] > SH - 100 ? SH - 100 : a[2];
        ScWin *w = &SWIN[s]; rp_zero(w, (int)sizeof(ScWin)); w->used = 1; w->vm = (u8)vm->id; w->cw = cw; w->ch = ch; w->tlast = now_ms();
        rp_cpy(w->title, rp_tostr(vm, a[0], b0), 40); WN[9 + s] = w->title;
        Win *p = &W[9 + s]; p->w = cw + 6; p->h = ch + 31; p->x = 60 + s * 30; p->y = 40 + s * 28; if (p->x + p->w > SW) p->x = SW - p->w; if (p->y + p->h > SH - 30) p->y = SH - 30 - p->h; if (p->x < 0) p->x = 0; if (p->y < 0) p->y = 0;
        p->max = 0; open_win(9 + s); return s + 1; }
    case 121: if ((sw = win_get(vm, a[0]))) rp_cpy(sw->title, rp_tostr(vm, a[1], b0), 40); return 0;
    case 122: if (win_get(vm, a[0])) { swin_free(a[0] - 1); } return 0;
    case 123: dirty = 1; return 0;
    case 124: if (win_get(vm, a[0])) { W[9 + a[0] - 1].x = a[1]; W[9 + a[0] - 1].y = a[2]; } return 0;
    case 130: return ctl_new(vm, a[0], C_LABEL, a[1], a[2], 8 * slen(rp_tostr(vm, a[3], b0)), 16, rp_tostr(vm, a[3], b0), 0);
    case 131: return ctl_new(vm, a[0], C_BUTTON, a[1], a[2], a[3], a[4], rp_tostr(vm, a[5], b0), a[6]);
    case 132: return ctl_new(vm, a[0], C_TEXT, a[1], a[2], a[3], 24, rp_tostr(vm, a[4], b0), 0);
    case 133: return ctl_new(vm, a[0], C_CHECK, a[1], a[2], 24 + 8 * slen(rp_tostr(vm, a[3], b0)), 16, rp_tostr(vm, a[3], b0), a[4]);
    case 134: return ctl_new(vm, a[0], C_LIST, a[1], a[2], a[3], a[4], 0, a[5]);
    case 135: { int k = ctl_new(vm, a[0], C_PROG, a[1], a[2], a[3], 18, 0, 0); if (k) CT[k - 1].val = a[4]; return k; }
    case 140: if ((c = ctl_get(vm, a[0])) && c->type != C_LIST) { rp_cpy(c->text, rp_tostr(vm, a[1], b0), 64); if (c->type == C_LABEL) c->w = 8 * slen(c->text); } return 0;
    case 141: if ((c = ctl_get(vm, a[0]))) return rp_cstr(vm, c->text); return rp_cstr(vm, "");
    case 142: if ((c = ctl_get(vm, a[0]))) c->chk = (u8)(a[1] != 0); return 0;
    case 143: if ((c = ctl_get(vm, a[0]))) return c->chk; return 0;
    case 144: if ((c = ctl_get(vm, a[0]))) c->val = a[1]; return 0;
    case 145: if ((c = ctl_get(vm, a[0]))) return c->val; return 0;
    case 146: if ((c = ctl_get(vm, a[0])) && c->type == C_LIST && c->nitems < NITEMS) { rp_cpy(c->items[c->nitems++], rp_tostr(vm, a[1], b0), 28); } return 0;
    case 147: if ((c = ctl_get(vm, a[0]))) { c->nitems = 0; c->sel = -1; } return 0;
    case 148: if ((c = ctl_get(vm, a[0]))) return c->sel; return -1;
    case 149: if ((c = ctl_get(vm, a[0])) && a[1] >= 0 && a[1] < c->nitems) return rp_cstr(vm, c->items[a[1]]); return rp_cstr(vm, "");
    case 150: if ((c = ctl_get(vm, a[0]))) return c->nitems; return 0;
    case 151: if ((c = ctl_get(vm, a[0]))) c->en = (u8)(a[1] != 0); return 0;
    case 152: if ((c = ctl_get(vm, a[0]))) c->vis = (u8)(a[1] != 0); return 0;
    case 153: if ((c = ctl_get(vm, a[0])) && (sw = win_get(vm, c->win + 1))) sw->focus = a[0]; return 0;
    case 154: if ((c = ctl_get(vm, a[0])) && a[1] < c->nitems) c->sel = a[1]; return 0;
    case 160: case 161: case 162: case 163: case 164: {
        int h = a[0], hd = id == 163 ? a[2] : a[1];
        if (!(sw = win_get(vm, h))) return 0;
        if (id == 160) sw->onpaint = hd; else if (id == 161) sw->onclick = hd; else if (id == 162) sw->onkey = hd; else if (id == 164) sw->onclose = hd; else { sw->ontimer = hd; sw->tint = a[1] < 10 ? 10 : a[1]; sw->tlast = now_ms(); }
        return 0; }
    case 170: if (in_paint) sw_clip_rect(a[0], a[1], a[2], a[3], (u32)a[4] & 0xFFFFFF); return 0;
    case 171: if (in_paint) { u32 col = (u32)a[4] & 0xFFFFFF; sw_clip_rect(a[0], a[1], a[2], 1, col); sw_clip_rect(a[0], a[1] + a[3] - 1, a[2], 1, col); sw_clip_rect(a[0], a[1], 1, a[3], col); sw_clip_rect(a[0] + a[2] - 1, a[1], 1, a[3], col); } return 0;
    case 172: if (in_paint) sw_text(a[0], a[1], rp_tostr(vm, a[2], b0), (u32)a[3] & 0xFFFFFF); return 0;
    case 173: if (in_paint) {
        int x0 = a[0], y0 = a[1], x1 = a[2], y1 = a[3], dx = x1 > x0 ? x1 - x0 : x0 - x1, dy = y1 > y0 ? y1 - y0 : y0 - y1, n = dx > dy ? dx : dy; if (n > 4000) n = 4000; if (!n) n = 1;
        for (int i = 0; i <= n; i++) sw_clip_rect(x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, 1, 1, (u32)a[4] & 0xFFFFFF); } return 0;
    case 174: if (in_paint) { int r = a[2] > 400 ? 400 : a[2]; for (int dy = -r; dy <= r; dy++) { int w2 = 0; while ((w2 + 1) * (w2 + 1) <= r * r - dy * dy) w2++; sw_clip_rect(a[0] - w2, a[1] + dy, 2 * w2 + 1, 1, (u32)a[3] & 0xFFFFFF); } } return 0;
    case 175: if (in_paint && a[2] >= 0 && a[2] < ICO_COUNT && a[0] >= 0 && a[1] >= 0 && a[3] > 0 && a[0] + a[3] <= pc_w && a[1] + a[3] <= pc_h) icon(a[2], pc_x + a[0], pc_y + a[1], a[3]); return 0;
    case 180: dlg_message("RedWill", rp_tostr(vm, a[0], b0), 0); dlg_modal(); return 1;
    case 181: dlg_message("RedWill", rp_tostr(vm, a[0], b0), 1); return dlg_modal();
    case 182: dlg_start(DM_OPEN, DP_SCRIPT, "Open", "", fs_cwd); if (!dlg_modal()) return rp_cstr(vm, ""); return rp_cstr(vm, D.result);
    case 183: dlg_start(DM_SAVE, DP_SCRIPT, "Save As", rp_tostr(vm, a[0], b0), fs_cwd); if (!dlg_modal()) return rp_cstr(vm, ""); return rp_cstr(vm, D.result);
    case 184: dlg_message(rp_tostr(vm, a[0], b0), "", 0); D.mode = DM_INPUT; fld_set(&D.name, rp_tostr(vm, a[1], b1)); rp_cpy(D.msg, rp_tostr(vm, a[0], b0), 240); if (!dlg_modal()) return rp_cstr(vm, ""); return rp_cstr(vm, D.name.b);
    case 190: { int n = fs_walk(rp_tostr(vm, a[0], b0), fs_cwd); if (n < 0 || fsn[n].dir) return rp_cstr(vm, ""); return rp_newstr(vm, fs_data(n), fsn[n].len); }
    case 191: return rpp_write_file(rp_tostr(vm, a[0], b0), rp_tostr(vm, a[1], b1), 0);
    case 197: return rpp_write_file(rp_tostr(vm, a[0], b0), rp_tostr(vm, a[1], b1), 1);
    case 192: return fs_walk(rp_tostr(vm, a[0], b0), fs_cwd) >= 0;
    case 198: { int n = fs_walk(rp_tostr(vm, a[0], b0), fs_cwd); return n >= 0 && fsn[n].dir; }
    case 193: { int n = fs_walk(rp_tostr(vm, a[0], b0), fs_cwd); return n > 0 && fs_del(n); }
    case 194: {
        static char dl[3000]; int n = fs_walk(rp_tostr(vm, a[0], b0), fs_cwd); dl[0] = 0; if (n < 0 || !fsn[n].dir) return rp_cstr(vm, ""); char *e = dl; int ent[96], cnt = fs_list(n, ent, 96);
        for (int i = 0; i < cnt; i++) { if (ent[i] < 0) continue; if (e - dl > 2900) break; e = sapp(e, fsn[ent[i]].name); if (fsn[ent[i]].dir) e = sapp(e, "\\"); *e++ = '\n'; *e = 0; }
        if (e > dl && e[-1] == '\n') e[-1] = 0; return rp_cstr(vm, dl); }
    case 195: { char nm[24]; int dir; if (!fs_split(rp_tostr(vm, a[0], b0), nm, &dir)) return 0; return fs_mk(dir, nm, 1) >= 0; }
    case 196: { int n = fs_walk(rp_tostr(vm, a[0], b0), fs_cwd); if (n < 0) return 0; return rpp_run_node(n); }
    }
    (void)b2; rp_fail(vm, "unknown API call"); return 0;
}
