/* ---------- terminal ---------- */
static void tprint(const char *s) {
    for (;;) {
        char line[80]; int n = 0;
        while (*s && *s != '\n' && n < 72) line[n++] = *s++;
        line[n] = 0;
        if (tn >= 30) { for (int i = 1; i < 30; i++) { for (int k = 0; k < 80; k++) tl[i - 1][k] = tl[i][k]; } tn = 29; }
        sapp(tl[tn++], line);
        if (*s == '\n') s++;
        if (!*s) break;
    }
}
static void shutdown_pc(void); static void restart_pc(void);
static void note_open(int i) { int k = 0; while (fs[i].d[k] && k < 890) { nbuf[k] = fs[i].d[k]; k++; } nbuf[k] = 0; nlen = k; sapp(nname, fs[i].name); open_win(0); }
static void note_save(void) {
    if (!nname[0]) { char b[24]; fs_uniq(b, "NOTE"); sapp(nname, b); }
    int i = fs_find(nname); if (i < 0) i = fs_new(nname); if (i < 0) return;
    for (int k = 0; k < nlen; k++) fs[i].d[k] = nbuf[k];
    fs[i].d[nlen] = 0; fs[i].len = nlen; music(J_OK, 2, 0);
}
static void trun(void) {
    char cmd[80]; int n = 0; for (int i = 0; i < tin.n; i++) cmd[n++] = tin.b[i]; cmd[n] = 0;
    char ln[96]; char *e = sapp(ln, "redwill> "); sapp(e, cmd); tprint(ln);
    sapp(tlast, cmd); tin.n = 0; tin.b[0] = 0;
    char c0[24]; int k = 0; while (cmd[k] && cmd[k] != ' ' && k < 22) { c0[k] = lo_(cmd[k]); k++; } c0[k] = 0;
    const char *arg = cmd + k; while (*arg == ' ') arg++;
    if (!c0[0]) return;
    if (!scmp(c0, "help")) tprint("help ver cls echo ls cat touch rm write time date uptime\ncolor beep whoami neofetch browse notepad reboot shutdown");
    else if (!scmp(c0, "ver")) tprint("RedWill OS 1.0 (original hobby kernel, i386)");
    else if (!scmp(c0, "cls")) tn = 0;
    else if (!scmp(c0, "echo")) tprint(arg);
    else if (!scmp(c0, "ls") || !scmp(c0, "dir")) { for (int i = 0; i < 20; i++) if (fs[i].used) { char b[48]; char *p = sapp(b, fs[i].name); p = sapp(p, "  "); p = iapp(p, fs[i].len); sapp(p, " bytes"); tprint(b); } }
    else if (!scmp(c0, "cat")) { char u[24]; int j = 0; while (arg[j] && j < 19) { u[j] = up_(arg[j]); j++; } u[j] = 0; int i = fs_find(u); if (i < 0) tprint("File not found."); else tprint(fs[i].len ? fs[i].d : "(empty)"); }
    else if (!scmp(c0, "touch")) { char u[24]; int j = 0; while (arg[j] && j < 19) { u[j] = up_(arg[j]); j++; } u[j] = 0; if (!u[0]) tprint("usage: touch NAME"); else if (fs_find(u) >= 0 || fs_new(u) >= 0) tprint("OK."); else tprint("Disk full."); }
    else if (!scmp(c0, "rm")) { char u[24]; int j = 0; while (arg[j] && j < 19) { u[j] = up_(arg[j]); j++; } u[j] = 0; int i = fs_find(u); if (i < 0) tprint("File not found."); else { fs[i].used = 0; tprint("Deleted."); } }
    else if (!scmp(c0, "write")) { char u[24]; int j = 0; while (arg[j] && arg[j] != ' ' && j < 19) { u[j] = up_(arg[j]); j++; } u[j] = 0; const char *t = arg + j; while (*t == ' ') t++; if (!u[0]) tprint("usage: write NAME text"); else { fs_put(u, t); tprint("Written."); } }
    else if (!scmp(c0, "time")) { char b[16]; char *p = two(b, rh); *p++ = ':'; two(p, rm); tprint(b); }
    else if (!scmp(c0, "date")) { char b[32]; char *p = iapp(b, rd); *p++ = ' '; p = sapp(p, MON[rmo - 1]); *p++ = ' '; iapp(p, ry); tprint(b); }
    else if (!scmp(c0, "uptime")) { char b[32]; char *p = sapp(b, "up "); p = iapp(p, (int)(ms_acc / 1000)); sapp(p, " seconds"); tprint(b); }
    else if (!scmp(c0, "color")) { u32 cs[4] = { 0xD8DAFF, 0x60FF60, 0xFFC040, 0xFFFFFF }; tfg = cs[(arg[0] - '0') & 3]; }
    else if (!scmp(c0, "beep")) music(J_OK, 2, 0);
    else if (!scmp(c0, "whoami")) tprint(uname_);
    else if (!scmp(c0, "neofetch")) { char b[200]; char *p = sapp(b, "\\  /   "); p = sapp(p, uname_); p = sapp(p, "@"); p = sapp(p, cname); p = sapp(p, "\n \\/    OS: RedWill 1.0\n /\\    Theme: "); p = sapp(p, T.name); p = sapp(p, "\n/  \\   Res: "); p = iapp(p, SW); p = sapp(p, "x"); iapp(p, SH); tprint(b); }
    else if (!scmp(c0, "notepad")) open_win(0);
    else if (!scmp(c0, "browse")) open_win(8);
    else if (!scmp(c0, "reboot")) restart_pc();
    else if (!scmp(c0, "shutdown")) shutdown_pc();
    else { tprint("Unknown command. Type help."); music(J_ERR, 3, 0); }
}

