/* ---------- input handling ---------- */
static void select_node(int n) { ordfiles(); for (int k = 0; k < en; k++) if (eord[k] == n) esel = k; }
static void do_new_file(void) { char n[24]; fs_uniq(ecur, n, "NEW", "TXT"); int i = fs_mk(ecur, n, 0); if (i < 0) { music(J_ERR, 3, 0); return; } select_node(i); }
static void do_new_folder(void) { char nm[24]; for (int k = 1; k < 99; k++) { char *e = sapp(nm, "New Folder"); if (k > 1) { *e++ = ' '; iapp(e, k); } if (fs_child(ecur, nm) < 0) break; } int i = fs_mk(ecur, nm, 1); if (i < 0) { music(J_ERR, 3, 0); return; } select_node(i); }
static void explorer_goto(int d) { ecur = d; esel = -1; dirty = 1; }
static void explorer_open_entry(int i) {
    ordfiles(); if (i < 0 || i >= en) return; int e = eord[i];
    if (e < 0) { explorer_goto(fsn[ecur].parent); return; }
    if (fsn[e].dir) { explorer_goto(e); return; }
    if (fs_isext(e, "rxe") || fs_isext(e, "rpp")) rpp_run_node(e); else note_load(e);
}
static void explorer_open_sel(void) { explorer_open_entry(esel); }
static void explorer_edit_sel(void) { ordfiles(); if (esel >= 0 && esel < en && eord[esel] >= 0 && !fsn[eord[esel]].dir) note_load(eord[esel]); }
static void explorer_del_sel(void) { ordfiles(); if (esel >= 0 && esel < en && eord[esel] >= 0) { if (!fs_del(eord[esel])) { music(J_ERR, 3, 0); dlg_message("Delete", "Cannot delete: the folder is not empty, or it is a system folder.", 0); } esel = -1; } }
static void explorer_tool(int i) {
    if (i == 0) do_new_file(); else if (i == 1) do_new_folder(); else if (i == 2) explorer_open_sel(); else if (i == 3) explorer_edit_sel();
    else if (i == 4) explorer_del_sel(); else if (i == 5) { if (ecur) explorer_goto(fsn[ecur].parent); } else eview = !eview;
}
static void app_click(int w, int rx, int ry, int dbl) {
    Win *p = &W[w]; int cw = p->w - 6, ch = p->h - 31;
    #define IN(x, y, ww, hh) (rx >= (x) && rx < (x) + (ww) && ry >= (y) && ry < (y) + (hh))
    if (w == 0) { if (IN(6, 3, 44, 20)) { nlen = 0; nname[0] = 0; nbuf[0] = 0; nnode = -1; nup = 0; } else if (IN(56, 3, 44, 20)) note_open_dlg(); else if (IN(106, 3, 44, 20)) note_save(); else if (IN(156, 3, 44, 20)) rpp_launch(nname[0] ? nname : "Untitled", nbuf, 0, 0); }
    else if (w == 1) { for (int r = 0; r < 4; r++) for (int c = 0; c < 4; c++) if (IN(6 + c * 54, 46 + r * 46, 48, 40)) calc_key(CB[r][c]); }
    else if (w == 2) {
        ordfiles(); int bt = 82, lx = 176, lw = cw - 176;
        int tool = -1; for (int i = 0; i < 7; i++) if (IN(EXBX[i], 25, EXBW[i], 24)) tool = i;
        if (tool >= 0) { explorer_tool(tool); return; }
        for (int i = 0; i < 5; i++) if (IN(14, bt + EXT_Y(i), slen(EXT_[i]) * 8, 16)) { explorer_tool(i); return; }
        for (int i = 0; i < 4; i++) if (IN(14, bt + EXP_Y(i), slen(EXP_[i]) * 8, 16)) { if (i == 0) explorer_goto(fs_docs); else if (i == 1) explorer_goto(fs_sdk); else if (i == 2) explorer_goto(fs_red32); else open_win(3); return; }
        if (IN(lx, bt, lw, ch - bt - 22)) {
            int hit = -1;
            if (!eview) { int cols = (lw - 10) / 96; if (cols < 1) cols = 1; for (int i = 0; i < en; i++) { int gx = lx + 10 + (i % cols) * 96, gy = bt + 10 + (i / cols) * 84; if (IN(gx - 2, gy, 92, 72)) hit = i; } }
            else { int i = (ry - bt - 22) / 20; if (ry >= bt + 22 && i >= 0 && i < en) hit = i; }
            if (hit >= 0) { if (hit == esel && dbl) explorer_open_entry(hit); else esel = hit; } else esel = -1;
        }
    } else if (w >= 9) { swin_click(w, rx, ry);
    } else if (w == 4) {
        for (int i = 0; i < 10; i++) if (IN(8 + i * 28, 4, 24, 24)) pcol = i;
        for (int i = 0; i < 3; i++) if (IN(296 + i * 26, 4, 22, 24)) pbr = i == 0 ? 1 : i == 1 ? 3 : 6;
        if (IN(8, 30, 50, 16)) for (int i = 0; i < PW * PH; i++) PCV[i] = 0xFFFFFF;
        if (IN(8, 48, PW, PH)) { pdown = 1; plx = rx - 8; ply = ry - 48; int r = pbr / 2; for (int dy = -r; dy <= r; dy++) for (int dx = -r; dx <= r; dx++) { int xx = plx + dx, yy = ply + dy; if (xx >= 0 && yy >= 0 && xx < PW && yy < PH) PCV[yy * PW + xx] = PAL[pcol]; } }
    } else if (w == 7) {
        for (int i = 0; i < 3; i++) if (IN(12 + i * 112, 34, 102, 64)) { th = i; make_wall(); }
        if (IN(12, 118, 200, 20)) clk24 = !clk24; if (IN(12, 148, 300, 20)) { snd = !snd; if (!snd) music_stop(); }
        if (IN(12, 184, 110, 26)) music(J_START, 7, 0);
    } else if (w == 8) {
        if (IN(4, 4, 28, 22)) { if (hp > 0) { hp--; nav(hist[hp], 0); } }
        else if (IN(36, 4, 28, 22)) { if (hp < hn - 1) { hp++; nav(hist[hp], 0); } }
        else if (IN(68, 4, 40, 22)) nav("redwill://home", 1);
        else if (IN(114, 4, cw - 186, 22)) bfocus = 1;
        else if (IN(cw - 66, 4, 28, 22)) { bfocus = 0; nav(faddr.b, 1); }
        else if (IN(cw - 22, 32, 20, 20)) bscroll -= 48; else if (IN(cw - 22, ch - 24, 20, 20)) bscroll += 48;
        else { bfocus = 0; for (int i = 0; i < nlk; i++) if (IN(LK[i].x - (p->x + 3), LK[i].y - (p->y + 28), LK[i].w, LK[i].h)) { char t[48]; sapp(t, LK[i].t); nav(t, 1); break; } }
    }
}
static void on_down(void) {
    int tb = SH - 30;
    if (cal) { if (inr(SW - 240, tb - 214, 240, 214)) return; if (!inr(SW - 122, tb, 122, 30)) cal = 0; }
    if (ctxo) {
        int hit = -1; for (int i = 0; i < 4; i++) if (inr(ctxx + 2, ctxy + 2 + i * 24, 166, 24)) hit = i;
        ctxo = 0; dirty = 1;
        if (hit == 0) { ecur = fs_docs; do_new_file(); open_win(2); } else if (hit == 1) open_win(3); else if (hit == 2) { th = (th + 1) % 3; make_wall(); }
        return;
    }
    if (menu) {
        int h = menu_hit();
        if (h >= 0) { menu = 0; dirty = 1; if (h < 9) { static const u8 mp[9] = { 2, 8, 0, 3, 4, 1, 6, 7, 5 }; open_win(mp[h]); } else if (h == 21) restart_pc(); else shutdown_pc(); return; }
        if (inr(0, smy(), MW, MH)) return;
        if (!inr(0, tb, 106, 30)) menu = 0;
    }
    dirty = 1;
    if (my >= tb) {
        if (mx < 106) { menu = !menu; return; }
        for (int i = 0; i < 3; i++) if (inr(120 + i * 35, tb, 33, 30)) { static const u8 ql[3] = { 2, 8, 3 }; open_win(ql[i]); return; }
        if (mx >= SW - 122) { if (mx < SW - 90) { snd = !snd; if (!snd) music_stop(); else music(J_OK, 2, 0); } else { rtc_read(); cal = !cal; } return; }
        int n = 0; for (int k = 0; k < NW; k++) n += W[k].open; int bw = n ? (SW - 232 - 130) / n : 150; if (bw > 150) bw = 150; int bx = 232;
        for (int k = 0; k < NW; k++) { if (!W[k].open) continue; if (inr(bx, tb + 3, bw - 4, 24)) { if (W[k].min) open_win(k); else if (k == top_win()) W[k].min = 1; else raise_win(k); return; } bx += bw; }
        return;
    }
    for (int i = NW - 1; i >= 0; i--) {
        int w = Z[i]; Win *p = &W[w]; if (!p->open || p->min || !inr(p->x, p->y, p->w, p->h)) continue;
        raise_win(w); int bx = p->x + p->w - 24;
        if (inr(bx, p->y + 5, 19, 18)) { if (w >= 9) { if (!swin_try_close(w)) swin_closed(w); return; } p->open = 0; if (w == 4) pdown = 0; return; }
        bx -= 22;
        if (CANMAX[w]) { if (inr(bx, p->y + 5, 19, 18)) { if (!p->max) { p->rx = p->x; p->ry = p->y; p->rw = p->w; p->rh = p->h; p->x = 0; p->y = 0; p->w = SW; p->h = SH - 30; p->max = 1; } else { p->x = p->rx; p->y = p->ry; p->w = p->rw; p->h = p->rh; p->max = 0; } return; } bx -= 22; }
        if (inr(bx, p->y + 5, 19, 18)) { p->min = 1; return; }
        if (my < p->y + 28) {
            u32 t = ms_acc;
            if (CANMAX[w] && lastwin == w && t - lastc2 < 450) { if (!p->max) { p->rx = p->x; p->ry = p->y; p->rw = p->w; p->rh = p->h; p->x = 0; p->y = 0; p->w = SW; p->h = SH - 30; p->max = 1; } else { p->x = p->rx; p->y = p->ry; p->w = p->rw; p->h = p->rh; p->max = 0; } lastwin = -1; return; }
            lastwin = w; lastc2 = t; if (!p->max) { drag = w; ddx = mx - p->x; ddy = my - p->y; } return;
        }
        int dbl = (ms_acc - lastclick < 500); lastclick = ms_acc;
        app_click(w, mx - (p->x + 3), my - (p->y + 28), dbl); return;
    }
    int hit = -1; for (int k = 0; k < 6; k++) if (inr(20, 20 + k * 88, 80, 70)) hit = k;
    if (hit >= 0) { if (desk_sel == hit && ms_acc - lastclick < 600) open_win(DESKI[hit]); desk_sel = hit; lastclick = ms_acc; } else desk_sel = -1;
}
static void on_rdown(void) {
    if (ph != 3 || D.active) return;
    menu = 0; cal = 0; dirty = 1;
    if (my >= SH - 30) return;
    for (int i = NW - 1; i >= 0; i--) { Win *p = &W[Z[i]]; if (p->open && !p->min && inr(p->x, p->y, p->w, p->h)) return; }
    ctxo = 1; ctxx = mx; ctxy = my; if (ctxx + 170 > SW) ctxx = SW - 170; if (ctxy + 100 > SH) ctxy = SH - 100;
}
static int mpkt[3], mi_;
static void paint_move(void) {
    if (!pdown) return; Win *p = &W[4]; int rx = mx - (p->x + 3 + 8), ry = my - (p->y + 28 + 48);
    int x0 = plx, y0 = ply, x1 = rx, y1 = ry, n = (x1 > x0 ? x1 - x0 : x0 - x1); int m2 = (y1 > y0 ? y1 - y0 : y0 - y1); if (m2 > n) n = m2; if (n < 1) n = 1;
    for (int s = 0; s <= n; s++) { int xx = x0 + (x1 - x0) * s / n, yy = y0 + (y1 - y0) * s / n, r = pbr / 2; for (int dy = -r; dy <= r; dy++) for (int dx = -r; dx <= r; dx++) { int a = xx + dx, b = yy + dy; if (a >= 0 && b >= 0 && a < PW && b < PH) PCV[b * PW + a] = PAL[pcol]; } }
    plx = rx; ply = ry;
}
static void mouse_pkt(void) {
    int f = mpkt[0]; if (f & 0xC0) return;
    int dx = mpkt[1] - ((f << 4) & 0x100), dy = mpkt[2] - ((f << 3) & 0x100);
    mx += dx; my -= dy;
    if (mx < 0) mx = 0;
    if (my < 0) my = 0;
    if (mx > SW - 1) mx = SW - 1;
    if (my > SH - 1) my = SH - 1;
    if (dx || dy) { dirty = 1; if (drag >= 0 && (f & 1)) { W[drag].x = mx - ddx; W[drag].y = my - ddy; if (W[drag].y < 0) W[drag].y = 0; if (W[drag].y > SH - 60) W[drag].y = SH - 60; } paint_move(); }
    int nb = f & 1, nr = (f >> 1) & 1;
    if (nb && !mb) { mb = 1; if (ph == 3) { if (D.active) dlg_click(); else on_down(); } else if (ph == 1) { setup_click(); dirty = 1; } else if (ph == 2) { dirty = 1; if (inr(SW - 130, SH - 56, 100, 28)) shutdown_pc(); else if (!upass[0]) { ph = 3; music(J_START, 7, 0); rtc_read(); } } }
    else if (!nb && mb) { mb = 0; drag = -1; pdown = 0; dirty = 1; }
    if (nr && !mrb) { mrb = 1; on_rdown(); } else if (!nr && mrb) mrb = 0;
}
static const char KN[] = "\0\033" "1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 ";
static const char KS[] = "\0\033" "!@#$%^&*()_+\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 ";
static void login_try(void) {
    if (!scmp(fpw.b, upass)) { ph = 3; music(J_START, 7, 0); rtc_read(); dirty = 1; } else { sfoc = 7; fld_set(&fpw, ""); music(J_ERR, 3, 0); dirty = 1; }
}
static void key_ch(int ch) {
    dirty = 1;
    if (ph == 1) {
        if (sp == 2) { Fld *fl[3] = { &fcn, &fun, &fpw }; if (ch == '\t') sfoc = (sfoc + 1) % 3; else if (ch == '\n') { sfoc = (sfoc + 1) % 3; } else fld_key(fl[sfoc], ch, 24); }
        else if (ch == '\n') { if (sp == 5) to_login(); else if (sp < 4) setup_next(); }
        return;
    }
    if (ph == 2) { if (ch == '\n') { if (!upass[0]) { ph = 3; music(J_START, 7, 0); rtc_read(); } else login_try(); } else if (upass[0]) { sfoc = 0; fld_key(&fpw, ch, 24); } return; }
    if (ph != 3) return;
    if (D.active) { dlg_key(ch); return; }
    if (ch == 27) { menu = 0; cal = 0; ctxo = 0; return; }
    int w = top_win();
    if (w == 0) {
        if (ctrl && (ch == 's' || ch == 'S')) { note_save(); return; }
        if (ctrl && (ch == 'o' || ch == 'O')) { note_open_dlg(); return; }
        if (ch == 17) { nup += 1; return; } if (ch == 18) { if (nup) nup--; return; } if (ch == 22) { nup += 10; return; } if (ch == 23) { nup = nup > 10 ? nup - 10 : 0; return; }
        nup = 0;
        if (ch == 8) { if (nlen) nbuf[--nlen] = 0; } else if ((ch == '\n' || (ch >= 32 && ch < 127)) && nlen < 8100) { nbuf[nlen++] = (char)ch; nbuf[nlen] = 0; } else if (ch == '\t' && nlen < 8090) for (int i = 0; i < 4; i++) nbuf[nlen++] = ' ';
        nbuf[nlen] = 0;
    } else if (w == 1) {
        if (ch == 8) { if (!cfresh) ccur /= 10; } else if (ch == '\n') calc_key('='); else if (ch == 'c' || ch == 'C') calc_key('C');
        else if ((ch >= '0' && ch <= '9') || ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=') calc_key((char)ch);
    } else if (w == 2) { if (ch == '\n') explorer_open_sel(); else if (ch == 21) explorer_del_sel(); else if (ch == 8 && ecur) explorer_goto(fsn[ecur].parent); else if (ch == 18 && esel < en - 1) esel++; else if (ch == 17 && esel > 0) esel--; }
    else if (w >= 9) swin_key(w, ch);
    else if (w == 3) { if (ch == '\n') trun(); else if (ch == 17) fld_set(&tin, tlast); else fld_key(&tin, ch, 70); }
    else if (w == 6) {
        if (ch == '\n' && !salive) { snake_reset(); return; }
        int ndx = sdx, ndy = sdy;
        if (ch == 17 || ch == 'w') { ndx = 0; ndy = -1; } else if (ch == 18 || ch == 's') { ndx = 0; ndy = 1; } else if (ch == 19 || ch == 'a') { ndx = -1; ndy = 0; } else if (ch == 20 || ch == 'd') { ndx = 1; ndy = 0; } else return;
        if (!(ndx == -sdx && ndy == -sdy) || snl < 2) { sdx = ndx; sdy = ndy; } sstart = 1;
    } else if (w == 8) {
        if (bfocus) { if (ch == '\n') { bfocus = 0; nav(faddr.b, 1); } else fld_key(&faddr, ch, 60); }
        else if (ch == 18) bscroll += 32; else if (ch == 17) bscroll -= 32; else if (ch == 23) bscroll += 200; else if (ch == 22) bscroll -= 200;
    }
}
static void key_scan(u8 d) {
    if (d == 0xE0) { ext = 1; return; }
    int e = ext; ext = 0;
    if (!e && d == 0x1D) { ctrl = 1; return; } if (!e && d == 0x9D) { ctrl = 0; return; }
    if (!e && (d == 0x2A || d == 0x36)) { shift = 1; return; } if (!e && (d == 0xAA || d == 0xB6)) { shift = 0; return; }
    if (d & 0x80) return;
    if (e) { int c = d == 0x48 ? 17 : d == 0x50 ? 18 : d == 0x4B ? 19 : d == 0x4D ? 20 : d == 0x53 ? 21 : d == 0x49 ? 22 : d == 0x51 ? 23 : 0; if (c) key_ch(c); return; }
    if (d < sizeof(KN) - 1) { char c = shift ? KS[d] : KN[d]; if (c) key_ch(c); }
}
static void poll_input(void) {
    for (int n = 0; n < 64; n++) {
        u8 st = inb(0x64); if (!(st & 1)) break; u8 d = inb(0x60);
        if (st & 0x20) { if (mi_ == 0 && !(d & 8)) continue; mpkt[mi_++] = d; if (mi_ == 3) { mi_ = 0; mouse_pkt(); } } else key_scan(d);
    }
}
static void kbc_w(void) { for (int i = 0; i < 100000; i++) if (!(inb(0x64) & 2)) return; }
static void kbc_r(void) { for (int i = 0; i < 100000; i++) if (inb(0x64) & 1) return; }
static void mouse_w(u8 v) { kbc_w(); outb(0x64, 0xD4); kbc_w(); outb(0x60, v); }
static void mouse_init(void) {
    for (int i = 0; i < 64 && (inb(0x64) & 1); i++) inb(0x60);
    kbc_w(); outb(0x64, 0xA8); kbc_w(); outb(0x64, 0x20); kbc_r(); u8 cfg = inb(0x60); cfg |= 2; cfg &= (u8)~0x20;
    kbc_w(); outb(0x64, 0x60); kbc_w(); outb(0x60, cfg); mouse_w(0xF6); kbc_r(); inb(0x60); mouse_w(0xF4); kbc_r(); inb(0x60);
}


/* one iteration of the main loop: used by the main loop itself and by modal dialogs / sleep() inside programs */
static void os_pump(void) {
    static u32 lastblink2;
    u32 t = now_ms(); poll_input(); music_tick();
    if (t / 500 != lastblink2) { lastblink2 = t / 500; dirty = 1; }
    if (dirty) { dirty = 0; render(); flip(); }
}
