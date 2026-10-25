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
static int rpp_run_node(int n); static int rpp_compile_node(int n, const char *outname, int outdir, char *msg);
/* ---- notepad file handling ---- */
static void note_load(int n) {
    const char *d = fs_data(n); int k = 0; while (d[k] && k < 8100) { nbuf[k] = d[k]; k++; } nbuf[k] = 0; nlen = k; nup = 9999;   /* show the top of the file */
    nnode = n; rp_cpy(nname, fsn[n].name, 24); open_win(0);
}
static void note_write(const char *path) {
    char nm[24]; int dir; if (!fs_split(path, nm, &dir)) { music(J_ERR, 3, 0); return; }
    int n = fs_mk(dir, nm, 0); if (n < 0 || !fs_set(n, nbuf, nlen)) { music(J_ERR, 3, 0); dlg_message("Notepad", "Could not save the file (disk full or invalid name).", 0); return; }
    nnode = n; rp_cpy(nname, fsn[n].name, 24); music(J_OK, 2, 0); dirty = 1;
}
static void note_save(void) { dlg_start(DM_SAVE, DP_NOTE_SAVE, "Save As", nname[0] ? nname : "Untitled.txt", nnode >= 0 ? fsn[nnode].parent : fs_docs); }
static void note_open_dlg(void) { dlg_start(DM_OPEN, DP_NOTE_OPEN, "Open", "", nnode >= 0 ? fsn[nnode].parent : fs_docs); }
static void dlg_finish(void) {
    if (D.purpose == DP_NOTE_SAVE) note_write(D.result);
    else if (D.purpose == DP_NOTE_OPEN) { int n = fs_walk(D.result, 0); if (n >= 0 && !fsn[n].dir) note_load(n); }
}
/* ---- terminal ---- */
static int t_find(const char *arg) {                       /* file lookup for run/edit: path, or NAME(.rxe/.rpp) in cwd, red32, red-sdk */
    int n = fs_walk(arg, fs_cwd); if (n >= 0 && !fsn[n].dir) return n;
    static const char *ex[3] = { "", ".rxe", ".rpp" }; int dirs[3] = { fs_cwd, fs_red32, fs_sdk };
    if (!arg[0] || slen(arg) > 18) return -1;
    for (int d = 0; d < 3; d++) for (int e = 0; e < 3; e++) { char nm[28]; char *p = sapp(nm, arg); sapp(p, ex[e]); int c = fs_child(dirs[d], nm); if (c >= 0 && !fsn[c].dir) return c; }
    return -1;
}
static void t_ls(int n) {
    int ent[96], cnt = fs_list(n, ent, 96);
    for (int i = 0; i < cnt; i++) { if (ent[i] < 0) continue; char b[64]; char *p = sapp(b, fsn[ent[i]].name); while (p - b < 26) *p++ = ' '; *p = 0;
        if (fsn[ent[i]].dir) sapp(p, "<DIR>"); else { p = iapp(p, fsn[ent[i]].len); sapp(p, " bytes"); } tprint(b); }
    if (!cnt || (cnt == 1 && ent[0] < 0)) tprint("(empty)");
}
static void trun(void) {
    char cmd[80]; int n = 0; for (int i = 0; i < tin.n; i++) cmd[n++] = tin.b[i]; cmd[n] = 0;
    char ln[96]; char *e = sapp(ln, "redwill> "); sapp(e, cmd); tprint(ln);
    sapp(tlast, cmd); tin.n = 0; tin.b[0] = 0;
    char c0[24]; int k = 0; while (cmd[k] && cmd[k] != ' ' && k < 22) { c0[k] = lo_(cmd[k]); k++; } c0[k] = 0;
    const char *arg = cmd + k; while (*arg == ' ') arg++;
    if (!c0[0]) return;
    if (!scmp(c0, "help")) tprint("Files: ls cd pwd mkdir cat touch rm write edit\nPrograms: run NAME   rppc SRC [OUT]   (.rpp scripts, .rxe programs)\nSystem: ver cls echo time date uptime color beep whoami neofetch\n        browse notepad reboot shutdown");
    else if (!scmp(c0, "ver")) tprint("RedWill OS 1.0 - Red32 API 1.0 (RPP scripts, RXE programs)");
    else if (!scmp(c0, "cls")) tn = 0;
    else if (!scmp(c0, "echo")) tprint(arg);
    else if (!scmp(c0, "pwd")) { char b[100]; fs_path(fs_cwd, b); tprint(b); }
    else if (!scmp(c0, "cd")) { if (!arg[0]) { fs_cwd = fs_docs; } else { int d = fs_walk(arg, fs_cwd); if (d < 0 || !fsn[d].dir) tprint("No such folder."); else fs_cwd = d; } char b[100]; fs_path(fs_cwd, b); tprint(b); }
    else if (!scmp(c0, "ls") || !scmp(c0, "dir")) { int d = arg[0] ? fs_walk(arg, fs_cwd) : fs_cwd; if (d < 0 || !fsn[d].dir) tprint("No such folder."); else t_ls(d); }
    else if (!scmp(c0, "mkdir") || !scmp(c0, "md")) { char nm[24]; int d; if (!arg[0]) tprint("usage: mkdir NAME"); else if (!fs_split(arg, nm, &d) || fs_mk(d, nm, 1) < 0) tprint("Cannot create folder."); else tprint("OK."); }
    else if (!scmp(c0, "cat") || !scmp(c0, "type")) { int i = fs_walk(arg, fs_cwd); if (i < 0 || fsn[i].dir) tprint("File not found."); else tprint(fsn[i].len ? fs_data(i) : "(empty)"); }
    else if (!scmp(c0, "touch")) { char nm[24]; int d; if (!arg[0]) tprint("usage: touch NAME"); else if (!fs_split(arg, nm, &d) || fs_mk(d, nm, 0) < 0) tprint("Cannot create file."); else tprint("OK."); }
    else if (!scmp(c0, "rm") || !scmp(c0, "del")) { int i = fs_walk(arg, fs_cwd); if (i <= 0) tprint("Not found."); else if (!fs_del(i)) tprint("Cannot delete (folder not empty or system folder)."); else tprint("Deleted."); }
    else if (!scmp(c0, "write")) { char nm[24], pth[64]; int j = 0; while (arg[j] && arg[j] != ' ' && j < 60) { pth[j] = arg[j]; j++; } pth[j] = 0; const char *t = arg + j; while (*t == ' ') t++; int d;
        if (!pth[0]) tprint("usage: write NAME text"); else if (!fs_split(pth, nm, &d)) tprint("Invalid path."); else { int f = fs_mk(d, nm, 0); if (f < 0 || !fs_set(f, t, slen(t))) tprint("Cannot write."); else tprint("Written."); } }
    else if (!scmp(c0, "edit")) { if (!arg[0]) { nlen = 0; nbuf[0] = 0; nname[0] = 0; nnode = -1; open_win(0); } else { int i = fs_walk(arg, fs_cwd); if (i < 0 || fsn[i].dir) tprint("File not found."); else note_load(i); } }
    else if (!scmp(c0, "run")) { int i = t_find(arg); if (i < 0) tprint("Program not found. (try: run hello  or  run C:\\RedWill\\red-sdk\\hello.rxe)"); else if (!fs_isext(i, "rxe") && !fs_isext(i, "rpp")) tprint("Not a .rxe program or .rpp script."); else rpp_run_node(i); }
    else if (!scmp(c0, "rppc")) {
        char src[64]; int j = 0; while (arg[j] && arg[j] != ' ' && j < 60) { src[j] = arg[j]; j++; } src[j] = 0; const char *o = arg + j; while (*o == ' ') o++;
        int i = t_find(src); if (i < 0 || !fs_isext(i, "rpp")) { tprint("usage: rppc SCRIPT.rpp [OUT.rxe]"); }
        else { char on[24]; int od = fsn[i].parent; if (o[0]) { int d; if (!fs_split(o, on, &d)) { tprint("Invalid output path."); return; } od = d; } else { rp_cpy(on, fsn[i].name, 24); int l = slen(on); if (l > 4 && on[l - 4] == '.') l -= 4; on[l] = 0; sapp(on + l, ".rxe"); }
               char m[160]; rpp_compile_node(i, on, od, m); tprint(m); } }
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
    else { int i = t_find(c0); if (i >= 0 && (fs_isext(i, "rxe") || fs_isext(i, "rpp"))) rpp_run_node(i); else { tprint("Unknown command. Type help."); music(J_ERR, 3, 0); } }
}
