/* ---------- desktop drawing ---------- */
static const char *DESKN[6] = { "My Documents", "Browser", "Notepad", "Terminal", "Paint", "Snake" };
static void draw_icons(void) {
    for (int k = 0; k < 6; k++) {
        int x = 20, y = 20 + k * 88, tw = slen(DESKN[k]) * 8, lx = x + 40 - tw / 2;
        icon(DESKI[k], x + 24, y, 32);
        if (desk_sel == k) rect(lx - 2, y + 36, tw + 4, 18, T.pill); else str(lx + 1, y + 38, DESKN[k], 0x101030);
        str(lx, y + 37, DESKN[k], 0xFFFFFF);
    }
}
static void ordfiles(void) { en = 0; for (int i = 0; i < 20; i++) if (fs[i].used) eord[en++] = i; }
static void draw_window(int w) {
    Win *p = &W[w]; int act = (w == top_win()); int x = p->x, y = p->y, ww = p->w, hh = p->h;
    rect(x, y, ww, hh, act ? T.pilld : 0x7A7EA8); vgrad(x + 1, y + 1, ww - 2, 27, act ? T.ta : 0x8E91C0, act ? T.tb : 0xB4B7DA);
    rect(x + 3, y + 28, ww - 6, hh - 31, T.cl);
    const char *title = WN[w]; char tb[40];
    if (w == 0) { char *e = sapp(tb, nname[0] ? nname : "Untitled"); sapp(e, " - Notepad"); title = tb; }
    icon(w, x + 6, y + 6, 16); bstr(x + 28, y + 6, title, 0xFFFFFF);
    int bx = x + ww - 24;
    rect(bx, y + 5, 19, 18, 0xFFFFFF); rect(bx + 1, y + 6, 17, 16, 0xD9534F); bstr(bx + 5, y + 6, "x", 0xFFFFFF); bx -= 22;
    if (CANMAX[w]) { rect(bx, y + 5, 19, 18, 0xFFFFFF); rect(bx + 1, y + 6, 17, 16, T.pill); frame(bx + 5, y + 9, 9, 9, 0xFFFFFF); rect(bx + 5, y + 9, 9, 2, 0xFFFFFF); bx -= 22; }
    rect(bx, y + 5, 19, 18, 0xFFFFFF); rect(bx + 1, y + 6, 17, 16, T.pill); rect(bx + 5, y + 16, 9, 3, 0xFFFFFF);
    int cx = x + 3, cy = y + 28, cw = ww - 6, ch = hh - 31; char b[96];
    if (w == 0) {
        rect(cx, cy, cw, 26, T.bar); button(cx + 6, cy + 3, 44, 20, "New", 0); button(cx + 56, cy + 3, 44, 20, "Save", 0); str(cx + 112, cy + 5, "Ctrl+S saves to My Documents", 0x303050);
        int tx = cx + 3, ty = cy + 30, tw = cw - 6, th_ = ch - 33; rect(tx, ty, tw, th_, 0x7F9DB9); rect(tx + 1, ty + 1, tw - 2, th_ - 2, 0xFFFFFF);
        int cols = (tw - 8) / 8, col = 0, row = 0, maxrow = (th_ - 8) / 16;
        for (int i = 0; i < nlen; i++) { char c = nbuf[i]; if (c == '\n') { col = 0; row++; continue; } if (col >= cols) { col = 0; row++; } if (row >= maxrow) break; chr(tx + 4 + col * 8, ty + 4 + row * 16, c, 0); col++; }
        if (col >= cols) { col = 0; row++; }
        if (act && ((ms_acc / 500) & 1) && row < maxrow) rect(tx + 4 + col * 8, ty + 6 + row * 16, 2, 13, 0);
    } else if (w == 1) {
        rect(cx + 6, cy + 6, 210, 32, 0x7F9DB9); rect(cx + 7, cy + 7, 208, 30, 0xFFFFFF); if (cerr) sapp(b, "Error"); else iapp(b, ccur);
        str(cx + 6 + 210 - 8 - slen(b) * 8, cy + 14, b, 0);
        for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) { int bxx = cx + 6 + c * 54, byy = cy + 46 + r * 46; rect(bxx, byy, 48, 40, (c == 3 || CB[r][c] == '=') ? 0xD0D2E6 : 0xF4F4FA); bevel(bxx, byy, 48, 40, 1); chr(bxx + 20, byy + 12, CB[r][c], CB[r][c] == 'C' ? 0xC02020 : T.pill); }
    } else if (w == 2) {
        ordfiles();
        str(cx + 8, cy + 3, "File   Edit   View   Favorites   Tools   Help", 0x101030);
        rect(cx, cy + 22, cw, 30, T.bar); button(cx + 6, cy + 25, 52, 24, "New", 0); button(cx + 64, cy + 25, 52, 24, "Open", 0); button(cx + 122, cy + 25, 66, 24, "Delete", 0); button(cx + 196, cy + 25, 76, 24, eview ? "Icons" : "Details", 0);
        str(cx + 8, cy + 58, "Address", 0x303050); rect(cx + 70, cy + 55, cw - 80, 22, 0xFFFFFF); frame(cx + 70, cy + 55, cw - 80, 22, 0x808090); str(cx + 76, cy + 58, "C:\\RedWill\\My Documents", 0);
        int bt = cy + 82, bh = ch - 82 - 22;
        vgrad(cx, bt, 176, bh, T.menu, mix(T.menu, 0xFFFFFF, 40, 100));
        rect(cx + 8, bt + 8, 160, 20, T.pill); bstr(cx + 14, bt + 10, "File Tasks", 0xFFFFFF);
        str(cx + 14, bt + 36, "Make a new file", 0x1030D0); rect(cx + 14, bt + 51, 15 * 8, 1, 0x1030D0);
        str(cx + 14, bt + 56, "Open selected", 0x1030D0); rect(cx + 14, bt + 71, 13 * 8, 1, 0x1030D0);
        str(cx + 14, bt + 76, "Delete selected", 0x1030D0); rect(cx + 14, bt + 91, 15 * 8, 1, 0x1030D0);
        rect(cx + 8, bt + 104, 160, 20, T.pill); bstr(cx + 14, bt + 106, "Other Places", 0xFFFFFF);
        str(cx + 14, bt + 132, "RedWill Browser", 0x1030D0); rect(cx + 14, bt + 147, 15 * 8, 1, 0x1030D0);
        str(cx + 14, bt + 152, "Terminal", 0x1030D0); rect(cx + 14, bt + 167, 8 * 8, 1, 0x1030D0);
        rect(cx + 8, bt + 180, 160, 20, T.pill); bstr(cx + 14, bt + 182, "Details", 0xFFFFFF);
        if (esel >= 0 && esel < en) { File *f = &fs[eord[esel]]; str(cx + 14, bt + 208, f->name, 0x101030); char *e = iapp(b, f->len); sapp(e, " bytes"); str(cx + 14, bt + 226, b, 0x404060); str(cx + 14, bt + 244, "Text Document", 0x404060); }
        int lx = cx + 176, lw = cw - 176; rect(lx, bt, lw, bh, 0xFFFFFF);
        if (!eview) { for (int i = 0; i < en; i++) { int gx = lx + 10 + (i % ((lw - 10) / 96 ? (lw - 10) / 96 : 1)) * 96, gy = bt + 10 + (i / ((lw - 10) / 96 ? (lw - 10) / 96 : 1)) * 84; if (i == esel) rect(gx - 2, gy + 36, 92, 36, T.pill); icon(9, gx + 24, gy, 32); str(gx, gy + 40, fs[eord[i]].name, i == esel ? 0xFFFFFF : 0x101030); } }
        else { rect(lx, bt, lw, 20, 0xE4E5F2); bstr(lx + 30, bt + 2, "Name", 0x101030); bstr(lx + lw - 100, bt + 2, "Size", 0x101030);
            for (int i = 0; i < en; i++) { int yy = bt + 22 + i * 20; if (yy + 20 > bt + bh) break; if (i == esel) rect(lx, yy, lw, 20, T.pill); icon(9, lx + 6, yy + 2, 16); str(lx + 30, yy + 2, fs[eord[i]].name, i == esel ? 0xFFFFFF : 0x101030); char *e = iapp(b, fs[eord[i]].len); sapp(e, " B"); str(lx + lw - 100, yy + 2, b, i == esel ? 0xFFFFFF : 0x101030); } }
        rect(cx, cy + ch - 22, cw, 22, T.bar); char *e = iapp(b, en); sapp(e, " object(s)"); str(cx + 8, cy + ch - 19, b, 0x101030);
    } else if (w == 3) {
        rect(cx, cy, cw, ch, 0x101018); int rows = (ch - 8) / 16 - 1; if (rows < 1) rows = 1; int s0 = tn > rows ? tn - rows : 0, yy = cy + 4;
        for (int i = s0; i < tn; i++) { str(cx + 6, yy, tl[i], tfg); yy += 16; }
        char *e = sapp(b, "redwill> "); sapp(e, tin.b); str(cx + 6, yy, b, 0xFFFFFF); if (act && ((ms_acc / 500) & 1)) rect(cx + 6 + (8 + tin.n) * 8, yy + 2, 8, 12, 0xFFFFFF);
    } else if (w == 4) {
        for (int i = 0; i < 10; i++) { rect(cx + 8 + i * 28, cy + 4, 24, 24, PAL[i]); frame(cx + 8 + i * 28, cy + 4, 24, 24, i == pcol ? 0xFF0000 : 0x606060); }
        for (int i = 0; i < 3; i++) { int s = i == 0 ? 1 : i == 1 ? 3 : 6; rect(cx + 296 + i * 26, cy + 4, 22, 24, 0xE4E5F2); bevel(cx + 296 + i * 26, cy + 4, 22, 24, pbr != s); disc(cx + 307 + i * 26, cy + 16, s == 1 ? 1 : s == 3 ? 2 : 4, 0); }
        button(cx + 8, cy + 30, 50, 16, "Clear", 0);
        for (int j = 0; j < PH; j++) { u32 *d = cv + (cy + 48 + j) * SW + cx + 8; if (cy + 48 + j >= SH) break; for (int i = 0; i < PW; i++) if (cx + 8 + i < SW) d[i] = PCV[j * PW + i]; }
        frame(cx + 7, cy + 47, PW + 2, PH + 2, 0x606060);
    } else if (w == 5) {
        xlogo(cx + 20, cy + 20, 72, T.pill, 0x8E94EC); big(cx + 110, cy + 24, "REDWILL", T.pilld, 3);
        str(cx + 110, cy + 76, "Version 1.0", 0x101030); str(cx + 20, cy + 112, "A hobby operating system built from scratch:", 0x101030); str(cx + 20, cy + 130, "custom boot code, i386 kernel, own GUI.", 0x101030);
        char *e = sapp(b, "Resolution: "); e = iapp(e, SW); e = sapp(e, " x "); e = iapp(e, SH); str(cx + 20, cy + 156, b, 0x303050);
        e = sapp(b, "Uptime: "); e = iapp(e, (int)(ms_acc / 1000)); sapp(e, " s"); str(cx + 20, cy + 174, b, 0x303050);
        e = sapp(b, "User: "); e = sapp(e, uname_); e = sapp(e, " on "); sapp(e, cname); str(cx + 20, cy + 192, b, 0x303050);
    } else if (w == 6) {
        char *e = sapp(b, "Score: "); iapp(e, sscore); bstr(cx + 8, cy + 3, b, 0x101030); str(cx + 150, cy + 3, "Arrows / WASD", 0x505070);
        rect(cx + 7, cy + 27, 322, 242, 0x606080); rect(cx + 8, cy + 28, 320, 240, 0x203820);
        for (int i = 0; i < snl; i++) rect(cx + 8 + snx[i] * 16 + 1, cy + 28 + sny[i] * 16 + 1, 14, 14, i ? 0x40D040 : 0x90FF60);
        disc(cx + 8 + sax * 16 + 8, cy + 28 + say * 16 + 8, 6, 0xE03030);
        if (!sstart && salive) cstr(cx + 168, cy + 140, "Press an arrow key to start", 0xFFFFFF);
        if (!salive) { cstr(cx + 168, cy + 126, "GAME OVER", 0xFF8080); cstr(cx + 168, cy + 146, "Press Enter to restart", 0xFFFFFF); }
    } else if (w == 7) {
        bstr(cx + 12, cy + 10, "Theme", T.pilld);
        for (int i = 0; i < 3; i++) { int xx = cx + 12 + i * 112; rect(xx - 2, cy + 32, 106, 66, i == th ? T.pill : 0xB0B0C0); vgrad(xx, cy + 34, 102, 36, TH[i].wa, TH[i].wb); rect(xx, cy + 74, 102, 8, TH[i].bar); cstr(xx + 51, cy + 82, TH[i].name, 0x101030); }
        rect(cx + 12, cy + 118, 18, 18, 0xFFFFFF); bevel(cx + 12, cy + 118, 18, 18, 0); if (clk24) rect(cx + 16, cy + 122, 10, 10, T.pill); str(cx + 40, cy + 119, "24-hour clock", 0x101030);
        rect(cx + 12, cy + 148, 18, 18, 0xFFFFFF); bevel(cx + 12, cy + 148, 18, 18, 0); if (snd) rect(cx + 16, cy + 152, 10, 10, T.pill); str(cx + 40, cy + 149, "System sounds (PC speaker)", 0x101030);
        button(cx + 12, cy + 184, 110, 26, "Test sound", 0); str(cx + 12, cy + 222, "Settings live in memory only.", 0x606080);
    } else {
        rect(cx, cy, cw, 30, T.bar); button(cx + 4, cy + 4, 28, 22, "<", 0); button(cx + 36, cy + 4, 28, 22, ">", 0); button(cx + 68, cy + 4, 40, 22, "Home", 0);
        rect(cx + 114, cy + 4, cw - 186, 22, 0xFFFFFF); frame(cx + 114, cy + 4, cw - 186, 22, bfocus ? T.pill : 0x808090); str(cx + 119, cy + 7, faddr.b, 0); if (bfocus && ((ms_acc / 500) & 1)) rect(cx + 119 + faddr.n * 8, cy + 7, 2, 14, 0);
        button(cx + cw - 66, cy + 4, 28, 22, "Go", 1); button(cx + cw - 34, cy + 4, 14, 10, "", 0);
        rect(cx, cy + 30, cw, ch - 30, 0xFFFFFF); int plain = 0; const char *pg = page_for(faddr.b, &plain);
        int tot = html(pg, plain, cx + 10, cy + 36, cw - 36, bscroll, 0);
        int mxs = tot - (ch - 40); if (mxs < 0) mxs = 0; if (bscroll > mxs) bscroll = mxs; if (bscroll < 0) bscroll = 0;
        /* clip: draw page then repaint chrome over overflow */
        html(pg, plain, cx + 10, cy + 36, cw - 36, bscroll, 1);
        rect(cx, cy, cw, 30, T.bar); button(cx + 4, cy + 4, 28, 22, "<", 0); button(cx + 36, cy + 4, 28, 22, ">", 0); button(cx + 68, cy + 4, 40, 22, "Home", 0);
        rect(cx + 114, cy + 4, cw - 186, 22, 0xFFFFFF); frame(cx + 114, cy + 4, cw - 186, 22, bfocus ? T.pill : 0x808090); str(cx + 119, cy + 7, faddr.b, 0); if (bfocus && ((ms_acc / 500) & 1)) rect(cx + 119 + faddr.n * 8, cy + 7, 2, 14, 0);
        button(cx + cw - 66, cy + 4, 28, 22, "Go", 1);
        rect(cx + cw - 24, cy + 30, 24, ch - 30, T.bar); button(cx + cw - 22, cy + 32, 20, 20, "^", 0); button(cx + cw - 22, cy + ch - 24, 20, 20, "v", 0);
        rect(cx, cy + ch, cw, 3, T.cl);
    }
}
#define MW 360
#define MH 450
static int smy(void) { return SH - 30 - MH; }
static int menu_hit(void) {
    int y0 = smy();
    for (int i = 0; i < 9; i++) if (inr(8, y0 + 52 + i * 34, 184, 32)) return i;
    if (inr(MW - 124, y0 + MH - 44, 116, 32)) return 20;
    if (inr(MW - 244, y0 + MH - 44, 112, 32)) return 21;
    return -1;
}
static const char *SMN[9] = { "My Documents", "RedWill Browser", "Notepad", "Terminal", "Paint", "Calculator", "Snake", "Settings", "About RedWill" };
static void draw_taskbar(void) {
    int tb = SH - 30;
    rect(0, tb, SW, 30, T.bar);
    pill(0, tb, 106, 30, T.pill); rect(0, tb + 28, 100, 2, T.pilld); bstr(10, tb + 7, "REDWILL", 0x000000);
    for (int i = 0; i < 4; i++) rect(118 + i * 35, tb + 2, 2, 26, T.bard);
    static const u8 QL[3] = { 2, 8, 3 };
    for (int i = 0; i < 3; i++) icon(QL[i], 126 + i * 35, tb + 5, 20);
    int bx = 232, n = 0; for (int k = 0; k < NW; k++) n += W[k].open;
    int bw = n ? (SW - 232 - 130) / n : 150; if (bw > 150) bw = 150;
    for (int k = 0; k < NW; k++) {
        if (!W[k].open) continue; int act = (k == top_win());
        rect(bx, tb + 3, bw - 4, 24, act ? T.bardd : T.bard); if (act) bevel(bx, tb + 3, bw - 4, 24, 0);
        icon(k, bx + 3, tb + 7, 16); char t[20]; int j = 0; const char *nm = WN[k]; while (nm[j] && j < (bw - 30) / 8 && j < 18) { t[j] = nm[j]; j++; } t[j] = 0; str(bx + 24, tb + 7, t, T.txt);
        bx += bw;
    }
    rect(SW - 124, tb + 2, 2, 26, T.bardd); rect(SW - 122, tb, 122, 30, T.bard);
    rect(SW - 116, tb + 11, 6, 8, snd ? T.pilld : 0x909090); rect(SW - 110, tb + 8, 4, 14, snd ? T.pilld : 0x909090);
    char b[16]; int h = rh; const char *ap = ""; if (!clk24) { ap = rh >= 12 ? " PM" : " AM"; h = rh % 12; if (!h) h = 12; }
    char *e = clk24 ? two(b, h) : iapp(b, h); *e++ = ':'; e = two(e, rm); sapp(e, ap); str(SW - 98, tb + 7, b, T.txt);
}
static void draw_menu(void) {
    int y0 = smy(); rect(0, y0, MW, MH, T.pilld); rect(2, y0 + 2, MW - 4, MH - 4, T.menu);
    vgrad(2, y0 + 2, MW - 4, 40, T.pill, T.tb); xlogo(10, y0 + 8, 28, 0xFFFFFF, 0xD0D4FF); bstr(48, y0 + 14, uname_, 0xFFFFFF);
    int hv = menu_hit();
    for (int i = 0; i < 9; i++) { int yy = y0 + 52 + i * 34, on = (hv == i); if (on) rect(8, yy, 184, 32, T.pill); icon(SMI[i], 14, yy + 2, 28); bstr(50, yy + 8, SMN[SMI[i] == 2 ? 0 : SMI[i] == 8 ? 1 : SMI[i] == 0 ? 2 : SMI[i] == 3 ? 3 : SMI[i] == 4 ? 4 : SMI[i] == 1 ? 5 : SMI[i] == 6 ? 6 : SMI[i] == 7 ? 7 : 8], on ? 0xFFFFFF : T.txt); }
    frame(200, y0 + 52, 152, 190, 0x585858); rect(201, y0 + 53, 150, 188, mix(T.menu, 0xFFFFFF, 50, 100)); bstr(208, y0 + 58, "Recent files", T.txt);
    ordfiles(); for (int i = 0; i < en && i < 7; i++) { icon(9, 206, y0 + 82 + i * 22, 16); char t[14]; int j = 0; while (fs[eord[i]].name[j] && j < 12) { t[j] = fs[eord[i]].name[j]; j++; } t[j] = 0; str(226, y0 + 82 + i * 22, t, T.txt); }
    str(204, y0 + 252, cname, T.txt); str(204, y0 + 270, "RedWill 1.0", T.txt);
    rect(MW - 244, y0 + MH - 44, 112, 32, hv == 21 ? 0x7A7A92 : 0x6A6A82); cstr(MW - 188, y0 + MH - 36, "Restart", 0xFFFFFF);
    rect(MW - 124, y0 + MH - 44, 116, 32, hv == 20 ? 0x707070 : 0x585858); cstr(MW - 66, y0 + MH - 36, "Shut Down", 0xFFFFFF);
}
static void draw_cal(void) {
    int x = SW - 240, y = SH - 30 - 214; rect(x, y, 240, 214, T.pilld); rect(x + 2, y + 2, 236, 210, 0xF4F5FC); vgrad(x + 2, y + 2, 236, 26, T.pill, T.tb);
    char b[32]; char *e = sapp(b, MON[rmo - 1]); *e++ = ' '; iapp(e, ry); cstr(x + 120, y + 7, b, 0xFFFFFF);
    static const char *DN[7] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    for (int i = 0; i < 7; i++) str(x + 12 + i * 31, y + 34, DN[i], T.pilld);
    int f = dow(ry, rmo, 1), n = dim(ry, rmo);
    for (int d = 1; d <= n; d++) { int c = (f + d - 1) % 7, r = (f + d - 1) / 7, xx = x + 8 + c * 31, yy = y + 54 + r * 24; if (d == rd) rect(xx - 2, yy - 2, 28, 22, T.pill); char t[4]; iapp(t, d); str(xx + (d < 10 ? 8 : 4), yy, t, d == rd ? 0xFFFFFF : 0x101030); }
}
static const char *CTXN[4] = { "New text file", "Open Terminal", "Next theme", "Refresh" };
static void draw_ctx(void) { rect(ctxx, ctxy, 170, 4 * 24 + 4, T.pilld); rect(ctxx + 1, ctxy + 1, 168, 4 * 24 + 2, 0xF4F5FC); for (int i = 0; i < 4; i++) { int on = inr(ctxx + 2, ctxy + 2 + i * 24, 166, 24); if (on) rect(ctxx + 2, ctxy + 2 + i * 24, 166, 24, T.pill); str(ctxx + 10, ctxy + 6 + i * 24, CTXN[i], on ? 0xFFFFFF : 0x101030); } }
static const char *CUR[19] = { "X           ", "XX          ", "X.X         ", "X..X        ", "X...X       ", "X....X      ", "X.....X     ", "X......X    ", "X.......X   ", "X........X  ", "X.....XXXXX ", "X..X..X     ", "X.X X..X    ", "XX  X..X    ", "X    X..X   ", "     X..X   ", "      X..X  ", "      X..X  ", "       XX   " };
static void draw_cursor(void) { for (int j = 0; j < 19; j++) for (int i = 0; i < 12; i++) { char c = CUR[j][i]; if (c == 'X') px(mx + i, my + j, 0); else if (c == '.') px(mx + i, my + j, 0xFFFFFF); } }
static void render(void) {
    if (ph == 1) setup_draw(); else if (ph == 2) login_draw();
    else if (ph == 3) {
        cv = BACK; memcpy(BACK, WALL, (unsigned)SW * SH * 4); draw_icons();
        for (int i = 0; i < NW; i++) { int w = Z[i]; if (W[w].open && !W[w].min) draw_window(w); }
        draw_taskbar(); if (menu) draw_menu(); if (cal) draw_cal(); if (ctxo) draw_ctx();
    }
    cv = BACK; draw_cursor();
}

