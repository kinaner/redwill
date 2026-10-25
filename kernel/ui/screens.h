/* ---------- wallpaper / screens ---------- */
static int hyt[1024];
static void make_wall(void) {
    cv = WALL; vgrad(0, 0, SW, SH, T.wa, T.wb);
    for (int i = 0; i < 6; i++) {
        int cx = SW * (10 + i * 17) / 100, cy = SH * (30 + (i * 37) % 55) / 100, r = 90 + i * 28;
        for (int dy = -r; dy <= r; dy += 1) { int w = 0; while ((w + 1) * (w + 1) <= r * r - dy * dy) w++; int yy = cy + dy; if (yy < 0 || yy >= SH) continue;
            for (int x = cx - w; x <= cx + w; x++) if (x >= 0 && x < SW) WALL[yy * SW + x] = mix(WALL[yy * SW + x], 0xFFFFFF, 18 - i * 2, 100); }
    }
    xlogo(SW - 330, SH - 330, 240, mix(T.pill, 0xFFFFFF, 35, 100), mix(T.pilld, 0xFFFFFF, 30, 100));
    big(SW - 330, SH - 90, "REDWILL", mix(T.pilld, 0xFFFFFF, 40, 100), 4);
    (void)hyt;
}
static void splash_frame(u32 t) {
    cv = BACK; rect(0, 0, SW, SH, 0);
    int x0 = SW / 2 - 200, y0 = SH * 34 / 100;
    xlogo(x0, y0, 96, 0x6B74F0, 0xB8BCFF);
    big(x0 + 116, y0 + 6, "REDWILL", 0xFFFFFF, 5);
    str(x0 + 120, y0 + 100, "Operating System", 0xB8BCFF);
    int bx = SW / 2 - 80, by = y0 + 170;
    rect(bx, by, 160, 18, 0xA0A0C0); rect(bx + 1, by + 1, 158, 16, 0);
    int pos = (int)((t / 70) % 16);
    for (int k = 0; k < 3; k++) { int idx = pos - k; if (idx < 0 || idx > 12) continue; rect(bx + 4 + idx * 12, by + 3, 10, 12, 0x3F48CC); rect(bx + 4 + idx * 12, by + 3, 10, 4, 0x8E94EC); }
    str(16, SH - 36, "RedWill 1.0 - built from scratch", 0x9090B0);
}

/* ---------- setup wizard (original design) ---------- */
static int sp = 0, sfoc = 0, sacc = 0; static Fld fcn, fun, fpw; static u32 sprog0;
static const char *SSTEP[6] = { "Welcome", "License", "Your computer", "Appearance", "Installing", "Finish" };
static const char *SPROG[5] = { "Preparing virtual disk (memory)", "Creating your user account", "Applying appearance", "Configuring sound", "Starting services" };
static int setup_x(void) { return (SW - 780) / 2; }
static int setup_y(void) { return (SH - 540) / 2; }
static void setup_draw(void) {
    cv = BACK; vgrad(0, 0, SW, SH, T.wa, T.wb);
    int X = setup_x(), Y = setup_y();
    rect(X - 2, Y - 2, 784, 544, T.pilld); rect(X, Y, 780, 540, 0xF2F3FB);
    vgrad(X, Y, 780, 46, T.pill, T.tb); xlogo(X + 12, Y + 8, 30, 0xFFFFFF, 0xC8CCFF); bstr(X + 52, Y + 14, "RedWill Setup", 0xFFFFFF);
    rect(X, Y + 46, 220, 440, T.menu);
    for (int i = 0; i < 6; i++) { int yy = Y + 66 + i * 36; if (i == sp) rect(X + 6, yy - 6, 208, 28, T.pill); disc(X + 24, yy + 8, 6, i < sp ? 0x30A050 : (i == sp ? 0xFFFFFF : 0x8088A8)); bstr(X + 40, yy, SSTEP[i], i == sp ? 0xFFFFFF : 0x202040); }
    int cx = X + 244, cy = Y + 66;
    char b[64];
    if (sp == 0) {
        big(cx, cy, "Welcome", T.pilld, 3);
        str(cx, cy + 70, "Thank you for trying RedWill, a hobby operating system", 0x101010); str(cx, cy + 90, "written from scratch.", 0x101010);
        str(cx, cy + 130, "This wizard will ask a few questions and set up your", 0x101010); str(cx, cy + 150, "account. Your settings are kept in memory only: RedWill", 0x101010); str(cx, cy + 170, "does not write to any disk.", 0x101010);
        str(cx, cy + 220, "Click Next to continue, or Express for default settings.", 0x404060);
    } else if (sp == 1) {
        big(cx, cy, "License", T.pilld, 3);
        str(cx, cy + 70, "RedWill is provided as is, without warranty of any kind.", 0x101010); str(cx, cy + 90, "You may use, copy and change it freely. It contains no", 0x101010); str(cx, cy + 110, "third-party proprietary software.", 0x101010);
        rect(cx, cy + 170, 18, 18, 0xFFFFFF); bevel(cx, cy + 170, 18, 18, 0); if (sacc) { rect(cx + 4, cy + 174, 10, 10, T.pill); }
        str(cx + 28, cy + 171, "I accept these terms", 0x101010);
    } else if (sp == 2) {
        big(cx, cy, "Your computer", T.pilld, 3);
        const char *lb[3] = { "Computer name", "Your name", "Password (optional)" }; Fld *fl[3] = { &fcn, &fun, &fpw };
        for (int i = 0; i < 3; i++) { str(cx, cy + 70 + i * 62, lb[i], 0x202040); rect(cx, cy + 92 + i * 62, 300, 26, 0xFFFFFF); frame(cx, cy + 92 + i * 62, 300, 26, sfoc == i ? T.pill : 0x808090);
            char sh[34]; int n = 0; for (; n < fl[i]->n; n++) sh[n] = (i == 2) ? '*' : fl[i]->b[n]; sh[n] = 0; str(cx + 6, cy + 97 + i * 62, sh, 0x000000); if (sfoc == i && (ms_acc / 500) % 2) rect(cx + 6 + n * 8, cy + 97 + i * 62, 2, 16, 0); }
        str(cx, cy + 262, "Click a box or press Tab to switch fields.", 0x404060);
    } else if (sp == 3) {
        big(cx, cy, "Appearance", T.pilld, 3);
        for (int i = 0; i < 3; i++) { int xx = cx + i * 158; rect(xx - 2, cy + 68, 148, 116, i == th ? T.pill : 0xB0B0C0); vgrad(xx, cy + 70, 144, 70, TH[i].wa, TH[i].wb); rect(xx, cy + 150, 144, 12, TH[i].bar); rect(xx, cy + 150, 40, 12, TH[i].pill); cstr(xx + 72, cy + 164, TH[i].name, 0x101010); }
        rect(cx, cy + 206, 18, 18, 0xFFFFFF); bevel(cx, cy + 206, 18, 18, 0); if (clk24) rect(cx + 4, cy + 210, 10, 10, T.pill); str(cx + 28, cy + 207, "24-hour clock", 0x101010);
        rect(cx, cy + 236, 18, 18, 0xFFFFFF); bevel(cx, cy + 236, 18, 18, 0); if (snd) rect(cx + 4, cy + 240, 10, 10, T.pill); str(cx + 28, cy + 237, "System sounds (PC speaker)", 0x101010);
    } else if (sp == 4) {
        big(cx, cy, "Installing", T.pilld, 3);
        u32 el = ms_acc - sprog0; int pc = (int)(el / 60); if (pc > 100) pc = 100;
        for (int i = 0; i < 5; i++) { int on = pc >= (i + 1) * 20; str(cx, cy + 70 + i * 26, on ? "[done]" : (pc >= i * 20 ? "[....]" : "      "), on ? 0x208040 : 0x606080); str(cx + 64, cy + 70 + i * 26, SPROG[i], 0x101010); }
        rect(cx, cy + 230, 440, 22, 0xFFFFFF); frame(cx, cy + 230, 440, 22, 0x808090);
        for (int i = 0; i < pc * 36 / 100; i++) { rect(cx + 3 + i * 12, cy + 233, 10, 16, T.pill); rect(cx + 3 + i * 12, cy + 233, 10, 5, T.tb); }
        if (pc >= 100) { sp = 5; dirty = 1; music(J_START, 7, 0); }
    } else {
        big(cx, cy, "All done", T.pilld, 3);
        char *p = sapp(b, "Welcome, "); p = sapp(p, fun.n ? fun.b : "RedWill User"); sapp(p, "!"); bstr(cx, cy + 74, b, 0x101010);
        str(cx, cy + 110, "Setup is complete. Click Finish to sign in.", 0x101010);
    }
    rect(X, Y + 486, 780, 54, 0xE0E2F2);
    if (sp > 0 && sp < 4) button(X + 520, Y + 500, 80, 28, "< Back", 0);
    if (sp < 4) button(X + 610, Y + 500, 80, 28, sp == 3 ? "Install" : "Next >", 1);
    if (sp == 0) button(X + 20, Y + 500, 100, 28, "Express", 0);
    if (sp == 5) button(X + 610, Y + 500, 80, 28, "Finish", 1);
    str(X + 20, Y + 506, sp == 0 ? "" : "", 0);
}
static void setup_apply(void) {
    sapp(cname, fcn.n ? fcn.b : "REDWILL-PC"); sapp(uname_, fun.n ? fun.b : "RedWill User"); sapp(upass, fpw.b);
    cname[31] = 0;
}
static void setup_next(void) {
    if (sp == 1 && !sacc) { music(J_ERR, 3, 0); return; }
    if (sp == 3) { setup_apply(); sp = 4; sprog0 = ms_acc; dirty = 1; make_wall(); return; }
    sp++; dirty = 1;
}
static void to_login(void) { setup_apply(); music_stop(); ph = 2; sfoc = 0; fld_set(&fpw, ""); dirty = 1; make_wall(); }
static void setup_click(void) {
    int X = setup_x(), Y = setup_y(), cx = X + 244, cy = Y + 66;
    if (sp == 0 && inr(X + 20, Y + 500, 100, 28)) { fld_set(&fcn, "REDWILL-PC"); fld_set(&fun, "RedWill User"); fld_set(&fpw, ""); to_login(); return; }
    if (sp > 0 && sp < 4 && inr(X + 520, Y + 500, 80, 28)) { sp--; dirty = 1; return; }
    if (inr(X + 610, Y + 500, 80, 28)) { if (sp == 5) to_login(); else if (sp < 4) setup_next(); return; }
    if (sp == 1 && inr(cx, cy + 170, 200, 20)) { sacc = !sacc; dirty = 1; }
    if (sp == 2) for (int i = 0; i < 3; i++) if (inr(cx, cy + 92 + i * 62, 300, 26)) { sfoc = i; dirty = 1; }
    if (sp == 3) {
        for (int i = 0; i < 3; i++) if (inr(cx + i * 158, cy + 68, 148, 116)) { th = i; make_wall(); dirty = 1; }
        if (inr(cx, cy + 206, 200, 20)) { clk24 = !clk24; dirty = 1; }
        if (inr(cx, cy + 236, 300, 20)) { snd = !snd; if (snd) music(J_OK, 2, 0); else music_stop(); dirty = 1; }
    }
}
static void login_draw(void) {
    cv = BACK; vgrad(0, 0, SW, SH, T.wa, T.wb); rect(0, 0, SW, 80, T.pilld); rect(0, SH - 80, SW, 80, T.pilld);
    rect(0, 80, SW, 2, 0xE8E8FF); rect(0, SH - 82, SW, 2, 0xE8E8FF);
    xlogo(SW / 2 - 330, SH / 2 - 80, 110, 0xFFFFFF, 0xC8CCFF); big(SW / 2 - 200, SH / 2 - 70, "REDWILL", 0xFFFFFF, 5);
    int ux = SW / 2 + 20, uy = SH / 2 - 60; rect(ux - 4, uy - 4, 328, 128, T.pilld); rect(ux, uy, 320, 120, 0xEEEFFA);
    rect(ux + 12, uy + 12, 72, 72, T.pill); disc(ux + 48, uy + 36, 14, 0xFFFFFF); rect(ux + 24, uy + 56, 48, 24, 0xFFFFFF);
    bstr(ux + 100, uy + 14, uname_, 0x101040); str(ux + 100, uy + 36, cname, 0x606080);
    if (upass[0]) { str(ux + 100, uy + 60, "Password:", 0x202040); rect(ux + 100, uy + 80, 200, 24, 0xFFFFFF); frame(ux + 100, uy + 80, 200, 24, T.pill);
        char sh[34]; int n = 0; for (; n < fpw.n; n++) sh[n] = '*'; sh[n] = 0; str(ux + 106, uy + 84, sh, 0); if ((ms_acc / 500) % 2) rect(ux + 106 + n * 8, uy + 84, 2, 16, 0); }
    else str(ux + 100, uy + 70, "Click here to sign in", 0x202040);
    if (sfoc == 7) bstr(ux, uy + 134, "Wrong password. Try again.", 0xFFE0E0);
    str(24, SH - 56, "Press Enter to sign in", 0xE0E0FF);
    button(SW - 130, SH - 56, 100, 28, "Shut down", 0);
}

