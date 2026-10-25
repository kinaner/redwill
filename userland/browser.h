/* ---------- browser ---------- */
static const char *P_HOME = "<h1>RedWill Browser</h1><p>Welcome to <b>RedWill</b>. This is the built-in web browser. It shows local pages and your files. It cannot reach the internet yet because RedWill has no network driver.</p><hr><ul><li><a href=redwill://about>About this browser</a></li><li><a href=redwill://files>Your files</a></li><li><a href=redwill://help>Help</a></li></ul>";
static const char *P_ABOUT = "<h1>About</h1><p>RedWill Browser 1.0 understands a small part of HTML: headings, paragraphs, <b>bold</b> text, lists, rules and <a href=redwill://home>links</a>.</p><p>Pages ending in .HTM in your files are shown as web pages. Other files are shown as plain text.</p>";
static const char *P_HELP = "<h1>Help</h1><ul><li>Click a link to open it.</li><li>Type an address in the bar and press Enter.</li><li>Use the arrow keys to scroll.</li><li>Try file://README.TXT</li></ul><p><a href=redwill://home>Back to start page</a></p>";
static const char *page_for(const char *u, int *plain) {
    *plain = 0;
    if (!scmp(u, "redwill://home")) return P_HOME;
    if (!scmp(u, "redwill://about")) return P_ABOUT;
    if (!scmp(u, "redwill://help")) return P_HELP;
    if (!scmp(u, "redwill://files")) {
        char *p = sapp(pgbuf, "<h1>My Documents</h1><ul>"); int ent[96], cnt = fs_list(fs_docs, ent, 96);
        for (int i = 0; i < cnt; i++) if (ent[i] >= 0 && !fsn[ent[i]].dir && p - pgbuf < 3500) { p = sapp(p, "<li><a href=file://"); p = sapp(p, fsn[ent[i]].name); p = sapp(p, ">"); p = sapp(p, fsn[ent[i]].name); p = sapp(p, "</a></li>"); }
        sapp(p, "</ul><p><a href=redwill://home>Start page</a></p>"); return pgbuf;
    }
    if (u[0] == 'f' && u[1] == 'i' && u[2] == 'l' && u[3] == 'e' && u[4] == ':' && u[5] == '/' && u[6] == '/') {
        int i = fs_walk(u + 7, fs_docs); if (i >= 0 && fsn[i].dir) i = -1;
        if (i < 0) { sapp(pgbuf, "<h1>File not found</h1><p>No such file. Paths are relative to My Documents.</p><p><a href=redwill://home>Start page</a></p>"); return pgbuf; }
        int k = 0; const char *d = fs_data(i); char *p = pgbuf; while (d[k] && k < 3900) { *p++ = d[k]; k++; } *p = 0;
        if (!fs_isext(i, "htm")) *plain = 1;
        return pgbuf;
    }
    sapp(pgbuf, "<h1>Cannot reach this address</h1><p>RedWill has no network driver yet, so it cannot open internet sites. Local pages and files work: try redwill://home or file://README.TXT.</p><p><a href=redwill://home>Start page</a></p>");
    return pgbuf;
}
static void nav(const char *u, int push) {
    if (push) { if (hp < hn - 1) hn = hp + 1; if (hn >= 8) { for (int i = 1; i < 8; i++) sapp(hist[i - 1], hist[i]); hn = 7; hp = 6; } sapp(hist[hn], u); hist[hn][63] = 0; hn++; hp = hn - 1; }
    fld_set(&faddr, u); bscroll = 0; dirty = 1;
}
static int html(const char *s, int plain, int X, int Y, int Wd, int scroll, int draw) {
    int x = X, y = Y - scroll, sc = 1, bold = 0, inl = 0, ul = 0; char lt[48]; lt[0] = 0; u32 col = 0x101010;
    if (draw) nlk = 0;
    #define NL() do { x = X; y += 16 * sc + 4; } while (0)
    while (*s) {
        if (!plain && *s == '<') {
            char tg[48]; int n = 0; s++;
            while (*s && *s != '>' && n < 46) { tg[n++] = lo_(*s); s++; }
            tg[n] = 0; if (*s == '>') s++;
            if (tg[0] == 'h' && tg[1] == '1') { if (x > X) NL(); y += 6; sc = 2; bold = 1; col = 0x20207A; }
            else if (!scmp(tg, "/h1")) { NL(); sc = 1; bold = 0; col = 0x101010; }
            else if (tg[0] == 'p' && !tg[1]) { if (x > X) NL(); y += 4; }
            else if (!scmp(tg, "/p")) { if (x > X) NL(); }
            else if (!scmp(tg, "br")) NL();
            else if (!scmp(tg, "ul")) { if (x > X) NL(); ul = 1; }
            else if (!scmp(tg, "/ul")) { ul = 0; if (x > X) NL(); }
            else if (tg[0] == 'l' && tg[1] == 'i') { if (x > X) NL(); x = X + 12; if (draw) chr(X + 2, y, '*', 0x3F48CC); }
            else if (!scmp(tg, "hr")) { if (x > X) NL(); if (draw) rect(X, y + 6, Wd, 1, 0x808080); y += 14; }
            else if (!scmp(tg, "b")) { bold = 1; }
            else if (!scmp(tg, "/b")) { bold = 0; }
            else if (tg[0] == 'a' && tg[1] == ' ') { int j = 2; while (tg[j] && !(tg[j] == '=')) j++; if (tg[j] == '=') j++; int q = 0; const char *o = s; (void)o; while (tg[j] && q < 46) lt[q++] = tg[j++]; lt[q] = 0; inl = 1; col = 0x1030D0; }
            else if (!scmp(tg, "/a")) { inl = 0; col = 0x101010; lt[0] = 0; }
            continue;
        }
        if (*s == '\n') { if (plain) NL(); else if (x > X) x += 8; s++; continue; }
        if (*s == ' ') { x += 8 * sc; if (x > X + Wd) NL(); s++; continue; }
        const char *w0 = s; int len = 0;
        while (*s && *s != ' ' && *s != '\n' && !(!plain && *s == '<')) { s++; len++; }
        if (x + len * 8 * sc > X + Wd && x > X + 12) NL();
        if (draw) {
            if (y > -40 && y < SH) {
                for (int i = 0; i < len; i++) { char c1[2] = { w0[i], 0 }; if (sc > 1) big(x + i * 8 * sc, y, c1, col, sc); else { chr(x + i * 8, y, w0[i], col); if (bold) chr(x + i * 8 + 1, y, w0[i], col); } }
                if (inl) rect(x, y + 15, len * 8, 1, col);
            }
            if (inl && nlk < 32 && lt[0]) { LK[nlk].x = x; LK[nlk].y = y; LK[nlk].w = len * 8; LK[nlk].h = 16; sapp(LK[nlk].t, lt); nlk++; }
        }
        x += len * 8 * sc;
    }
    (void)ul;
    return y + 20 + scroll - Y;
}

