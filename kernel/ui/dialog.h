/* ---------- modal dialogs: Save As / Open (with a mini explorer), message box, confirm, input box ---------- */
enum { DM_SAVE, DM_OPEN, DM_MSG, DM_CONFIRM, DM_INPUT };
enum { DP_NOTE_SAVE, DP_NOTE_OPEN, DP_SCRIPT };
typedef struct {
    int active, mode, purpose, blocking, ok;
    int cur, sel, scroll, nent, ent[96];
    char title[40], msg[240], result[120]; Fld name; u32 lastclick; int lastsel;
} Dlg;
static Dlg D;
static void os_pump(void);                 /* one iteration of the main loop (input, music, redraw) */
static void dlg_finish(void);              /* called when a non-blocking dialog is accepted */
#define DW 560
#define DH 410
static int dlg_x(void) { return (SW - (D.mode <= DM_OPEN ? DW : 440)) / 2; }
static int dlg_y(void) { return (SH - (D.mode <= DM_OPEN ? DH : 190)) / 2 - 20; }

/* directory listing shared by the explorer and the file dialogs: entry -1 is "..", dirs first */
static int fs_list(int dir, int *out, int max) {
    int n = 0; if (dir != 0 && n < max) out[n++] = -1;
    for (int pass = 0; pass < 2; pass++) for (int i = 1; i < FS_MAX && n < max; i++) if (fsn[i].used && fsn[i].parent == dir && fsn[i].dir == (pass == 0)) out[n++] = i;
    return n;
}
static void dlg_reload(void) { D.nent = fs_list(D.cur, D.ent, 96); D.sel = -1; D.scroll = 0; }
static void dlg_start(int mode, int purpose, const char *title, const char *name, int dir) {
    D.active = 1; D.mode = mode; D.purpose = purpose; D.ok = 0; D.cur = (dir >= 0 && fsn[dir].used && fsn[dir].dir) ? dir : fs_docs; D.lastsel = -1;
    rp_cpy(D.title, title, 40); D.msg[0] = 0; D.result[0] = 0; fld_set(&D.name, name ? name : ""); dlg_reload(); dirty = 1;
}
static void dlg_message(const char *title, const char *text, int confirm) {   /* non-blocking unless the caller pumps */
    D.active = 1; D.mode = confirm ? DM_CONFIRM : DM_MSG; D.purpose = DP_SCRIPT; D.ok = 0; D.blocking = 0; rp_cpy(D.title, title, 40); rp_cpy(D.msg, text, 240); dirty = 1;
}
static int dlg_modal(void) { D.blocking = 1; while (D.active) os_pump(); D.blocking = 0; return D.ok; }
static void dlg_close(int ok) {
    D.ok = ok; D.active = 0; dirty = 1;
    if (ok && (D.mode == DM_SAVE || D.mode == DM_OPEN)) { char *e = fs_path(D.cur, D.result); if (D.cur) { *e++ = '\\'; } sapp(e, D.name.b); }
    if (!D.blocking && ok && D.mode <= DM_OPEN) dlg_finish();
}
static int dlg_accept(void) {                                              /* Save/Open pressed or Enter */
    if (D.mode == DM_MSG) { dlg_close(1); return 1; }
    if (D.mode == DM_CONFIRM || D.mode == DM_INPUT) { dlg_close(1); return 1; }
    char *n = D.name.b; if (!n[0]) { music(J_ERR, 3, 0); return 0; }
    int ex = fs_child(D.cur, n);
    if (ex >= 0 && fsn[ex].dir) { D.cur = ex; fld_set(&D.name, ""); dlg_reload(); return 0; }   /* typed a folder name: enter it */
    if (D.mode == DM_OPEN && ex < 0) { music(J_ERR, 3, 0); return 0; }
    if (D.mode == DM_SAVE && D.purpose == DP_NOTE_SAVE) { int dot = 0; for (int i = 0; n[i]; i++) if (n[i] == '.') dot = 1; if (!dot && D.name.n < 26) { fld_key(&D.name, '.', 30); fld_key(&D.name, 'T', 30); fld_key(&D.name, 'X', 30); fld_key(&D.name, 'T', 30); } }
    dlg_close(1); return 1;
}
static void dlg_up(void) { if (D.cur) { D.cur = fsn[D.cur].parent; dlg_reload(); } }
static void dlg_newfolder(void) { char nm[24]; for (int k = 1; k < 99; k++) { char *e = sapp(nm, "New Folder"); if (k > 1) { *e++ = ' '; iapp(e, k); } if (fs_child(D.cur, nm) < 0) break; } int i = fs_mk(D.cur, nm, 1); if (i < 0) { music(J_ERR, 3, 0); return; } dlg_reload(); for (int k = 0; k < D.nent; k++) if (D.ent[k] == i) D.sel = k; }
#define DLR 11
static void dlg_click(void) {
    int x = dlg_x(), y = dlg_y(); dirty = 1;
    if (D.mode >= DM_MSG) {
        int bw = 340; (void)bw; int by = y + 190 - 44;
        if (D.mode == DM_MSG) { if (inr(x + 440 / 2 - 40, by, 80, 28)) dlg_accept(); }
        else { if (inr(x + 440 / 2 - 90, by, 80, 28)) dlg_accept(); else if (inr(x + 440 / 2 + 10, by, 80, 28)) dlg_close(0); }
        return;
    }
    if (inr(x + DW - 24, y + 4, 20, 20)) { dlg_close(0); return; }
    if (inr(x + DW - 150, y + 36, 60, 24)) { dlg_up(); return; }
    if (inr(x + DW - 84, y + 36, 72, 24)) { dlg_newfolder(); return; }
    int lx = x + 12, ly = y + 70, lw = DW - 24 - 22;
    if (inr(x + DW - 34, ly, 22, 22)) { if (D.scroll > 0) D.scroll--; return; }
    if (inr(x + DW - 34, ly + DLR * 20 - 22, 22, 22)) { if (D.scroll + DLR < D.nent) D.scroll++; return; }
    if (inr(lx, ly, lw, DLR * 20)) {
        int i = D.scroll + (my - ly) / 20;
        if (i >= 0 && i < D.nent) {
            int e = D.ent[i], dbl = (i == D.lastsel && ms_acc - D.lastclick < 500); D.lastsel = i; D.lastclick = ms_acc; D.sel = i;
            if (e < 0) { if (dbl) dlg_up(); return; }
            if (fsn[e].dir) { if (dbl) { D.cur = e; fld_set(&D.name, ""); dlg_reload(); } return; }
            fld_set(&D.name, fsn[e].name); if (dbl) dlg_accept();
        }
        return;
    }
    if (inr(x + DW - 110, y + DH - 76, 98, 28)) { dlg_accept(); return; }
    if (inr(x + DW - 110, y + DH - 42, 98, 28)) { dlg_close(0); return; }
}
static void dlg_key(int ch) {
    dirty = 1;
    if (ch == 27) { dlg_close(0); return; }
    if (ch == '\n') { dlg_accept(); return; }
    if (D.mode == DM_SAVE || D.mode == DM_OPEN) {
        if (ch == 18 && D.sel < D.nent - 1) { D.sel++; if (D.sel >= D.scroll + DLR) D.scroll++; int e = D.ent[D.sel]; if (e >= 0 && !fsn[e].dir) fld_set(&D.name, fsn[e].name); return; }
        if (ch == 17 && D.sel > 0) { D.sel--; if (D.sel < D.scroll) D.scroll--; int e = D.ent[D.sel]; if (e >= 0 && !fsn[e].dir) fld_set(&D.name, fsn[e].name); return; }
        fld_key(&D.name, ch, 23); return;
    }
    if (D.mode == DM_INPUT) fld_key(&D.name, ch, 30);
}
static void dlg_draw_wrapped(int x, int y, int w, const char *s, u32 c) {          /* word wrap */
    int cols = w / 8, col = 0, row = 0;
    while (*s && row < 6) {
        if (*s == '\n') { col = 0; row++; s++; continue; }
        if (*s == ' ' && col == 0 && row) { s++; continue; }
        int wl = 0; while (s[wl] && s[wl] != ' ' && s[wl] != '\n') wl++;
        if (col && col + wl > cols) { col = 0; row++; if (row >= 6) break; }
        for (int i = 0; i < wl; i++) { if (col >= cols) { col = 0; row++; if (row >= 6) return; } chr(x + col * 8, y + row * 16, s[i], c); col++; }
        s += wl; if (*s == ' ') { col++; s++; }
    }
}
static void dlg_draw(void) {
    if (!D.active) return;
    cv = BACK; int x = dlg_x(), y = dlg_y();
    if (D.mode >= DM_MSG) {
        rect(x + 5, y + 5, 440, 190, 0x404058); rect(x, y, 440, 190, T.pilld); vgrad(x + 1, y + 1, 438, 27, T.ta, T.tb); bstr(x + 10, y + 6, D.title, 0xFFFFFF);
        rect(x + 3, y + 28, 434, 159, T.cl); dlg_draw_wrapped(x + 20, y + 44, 400, D.msg, 0x101030);
        if (D.mode == DM_INPUT) { rect(x + 20, y + 110, 400, 24, 0xFFFFFF); frame(x + 20, y + 110, 400, 24, T.pill); str(x + 26, y + 114, D.name.b, 0); if ((ms_acc / 500) & 1) rect(x + 26 + D.name.n * 8, y + 114, 2, 14, 0); }
        int by = y + 190 - 44;
        if (D.mode == DM_MSG) button(x + 220 - 40, by, 80, 28, "OK", 1);
        else { button(x + 220 - 90, by, 80, 28, D.mode == DM_INPUT ? "OK" : "Yes", 1); button(x + 220 + 10, by, 80, 28, D.mode == DM_INPUT ? "Cancel" : "No", 0); }
        return;
    }
    rect(x + 6, y + 6, DW, DH, 0x404058); rect(x, y, DW, DH, T.pilld); vgrad(x + 1, y + 1, DW - 2, 27, T.ta, T.tb);
    icon(2, x + 6, y + 6, 16); bstr(x + 28, y + 6, D.title, 0xFFFFFF);
    rect(x + DW - 24, y + 4, 19, 18, 0xFFFFFF); rect(x + DW - 23, y + 5, 17, 16, 0xD9534F); bstr(x + DW - 19, y + 5, "x", 0xFFFFFF);
    rect(x + 3, y + 28, DW - 6, DH - 31, T.cl);
    str(x + 12, y + 41, "Look in:", 0x101030);
    rect(x + 84, y + 36, DW - 84 - 160, 24, 0xFFFFFF); frame(x + 84, y + 36, DW - 84 - 160, 24, 0x808090);
    char pb[100]; fs_path(D.cur, pb); int pl = slen(pb); const char *ps = pb; if (pl > (DW - 84 - 170) / 8) ps = pb + pl - (DW - 84 - 170) / 8; str(x + 90, y + 40, ps, 0);
    button(x + DW - 150, y + 36, 60, 24, "Up", 0); button(x + DW - 84, y + 36, 72, 24, "New dir", 0);
    int lx = x + 12, ly = y + 70, lw = DW - 24 - 22;
    rect(lx - 1, ly - 1, lw + 24, DLR * 20 + 2, 0x7F9DB9); rect(lx, ly, lw, DLR * 20, 0xFFFFFF);
    for (int r = 0; r < DLR; r++) {
        int i = D.scroll + r; if (i >= D.nent) break; int e = D.ent[i], yy = ly + r * 20, on = (i == D.sel);
        if (on) rect(lx, yy, lw, 20, T.pill);
        u32 fg = on ? 0xFFFFFF : 0x101030;
        if (e < 0) { icon(2, lx + 4, yy + 2, 16); str(lx + 28, yy + 2, "..", fg); continue; }
        icon(fsn[e].dir ? 2 : file_icon(e), lx + 4, yy + 2, 16); str(lx + 28, yy + 2, fsn[e].name, fg);
        char b[24]; if (fsn[e].dir) sapp(b, "<DIR>"); else { char *p = iapp(b, fsn[e].len); sapp(p, " B"); } str(lx + lw - 100, yy + 2, b, fg);
    }
    rect(x + DW - 34, ly, 22, DLR * 20, T.bar); button(x + DW - 34, ly, 22, 22, "^", 0); button(x + DW - 34, ly + DLR * 20 - 22, 22, 22, "v", 0);
    str(x + 12, y + DH - 68, "File name:", 0x101030);
    rect(x + 100, y + DH - 74, DW - 100 - 124, 24, 0xFFFFFF); frame(x + 100, y + DH - 74, DW - 100 - 124, 24, T.pill);
    str(x + 106, y + DH - 70, D.name.b, 0); if ((ms_acc / 500) & 1) rect(x + 106 + D.name.n * 8, y + DH - 70, 2, 14, 0);
    str(x + 12, y + DH - 36, D.mode == DM_SAVE ? "Choose a folder, type a name, then Save." : "Select a file, then Open.", 0x505070);
    button(x + DW - 110, y + DH - 76, 98, 28, D.mode == DM_SAVE ? "Save" : "Open", 1); button(x + DW - 110, y + DH - 42, 98, 28, "Cancel", 0);
}
