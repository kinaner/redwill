/* ---------- RAM file system: hierarchical, C:\RedWill\{red32,red-sdk,My Documents} ---------- */
#define FS_MAX 192
#define FS_POOL (1024 * 1024)
typedef struct { char name[24]; short parent; u8 used, dir; int off, len, cap; } FNode;
static FNode fsn[FS_MAX];
static u8 *FSP;                 /* data pool, allocated in kernel_main (fixed high memory) */
static int fs_top;
static int fs_cwd, fs_docs, fs_red32, fs_sdk;     /* shell current dir + well-known folders */
static int fs_ieq(const char *a, const char *b) { while (*a && up_(*a) == up_(*b)) { a++; b++; } return *a == *b; }
static int fs_child(int dir, const char *n) { for (int i = 0; i < FS_MAX; i++) if (fsn[i].used && fsn[i].parent == dir && i != dir && fs_ieq(fsn[i].name, n)) return i; return -1; }
static int fs_mk(int dir, const char *n, int isdir) {
    int e = fs_child(dir, n); if (e >= 0) return fsn[e].dir == isdir ? e : -1;
    if (!n[0]) return -1;
    for (int i = 1; i < FS_MAX; i++) if (!fsn[i].used) {
        fsn[i].used = 1; fsn[i].dir = (u8)isdir; fsn[i].parent = (short)dir; fsn[i].off = fsn[i].len = fsn[i].cap = 0;
        int k = 0; for (; n[k] && k < 23; k++) fsn[i].name[k] = n[k]; fsn[i].name[k] = 0; return i;
    }
    return -1;
}
static const char *fs_data(int n) { return (n >= 0 && fsn[n].used && !fsn[n].dir && fsn[n].cap) ? (const char *)FSP + fsn[n].off : ""; }
static int fs_set(int n, const void *d, int len) {            /* replace file contents; NUL-terminated for text use */
    if (n < 0 || fsn[n].dir || len < 0) return 0;
    if (len + 1 > fsn[n].cap) {
        int cap = (len + 1 + 255) & ~255; if (fs_top + cap > FS_POOL) return 0;
        fsn[n].off = fs_top; fsn[n].cap = cap; fs_top += cap;
    }
    u8 *p = FSP + fsn[n].off; const u8 *s = (const u8 *)d; for (int i = 0; i < len; i++) p[i] = s[i]; p[len] = 0; fsn[n].len = len; return 1;
}
static int fs_nchild(int dir) { int c = 0; for (int i = 1; i < FS_MAX; i++) if (fsn[i].used && fsn[i].parent == dir) c++; return c; }
static int fs_del(int n) { if (n <= 0 || n == fs_docs || n == fs_red32 || n == fs_sdk || (fsn[n].dir && fs_nchild(n))) return 0; fsn[n].used = 0; return 1; }
static int fs_walk(const char *p, int start) {                /* resolve "C:\A\B", "\A", "A\B", "..", "." */
    int cur = start; if (p[0] == '\\' || p[0] == '/') cur = 0;
    while (*p) {
        while (*p == '\\' || *p == '/') p++;
        char c[24]; int k = 0; while (*p && *p != '\\' && *p != '/') { if (k < 23) c[k++] = *p; p++; } c[k] = 0;
        if (!k) continue;
        if (!scmp(c, ".")) continue;
        if (!scmp(c, "..")) { if (cur) cur = fsn[cur].parent; continue; }
        if (fs_ieq(c, "C:")) { cur = 0; continue; }
        if (!fsn[cur].dir) return -1;
        cur = fs_child(cur, c); if (cur < 0) return -1;
    }
    return cur;
}
static char *fs_path(int n, char *out) {                      /* "C:\RedWill\red32\X" */
    int chain[16], d = 0; while (n > 0 && d < 16) { chain[d++] = n; n = fsn[n].parent; }
    char *e = sapp(out, "C:"); if (!d) { *e++ = '\\'; *e = 0; return e; }
    while (d) { *e++ = '\\'; e = sapp(e, fsn[chain[--d]].name); } return e;
}
static const char *fs_ext(const char *n) { int l = slen(n); for (int i = l - 1; i >= 0 && i > l - 6; i--) if (n[i] == '.') return n + i + 1; return ""; }
static int fs_isext(int n, const char *x) { return n >= 0 && !fsn[n].dir && fs_ieq(fs_ext(fsn[n].name), x); }
static int fs_put_path(const char *path, const void *d, int len) {   /* create dirs as needed, write file */
    int cur = 0; const char *p = path;
    for (;;) {
        while (*p == '\\' || *p == '/') p++; if (!*p) return 0;
        char c[24]; int k = 0; while (*p && *p != '\\' && *p != '/') { if (k < 23) c[k++] = *p; p++; } c[k] = 0;
        if (fs_ieq(c, "C:") && cur == 0) continue;
        int last = !*p; int n = fs_mk(cur, c, !last); if (n < 0) return 0;
        if (last) return fs_set(n, d, len);
        cur = n;
    }
}
/* split "dir\\name" into a directory node and a file name; relative paths start at the shell's current dir */
static int fs_split(const char *path, char *name, int *dir) {
    int l = slen(path), k = l - 1; while (k >= 0 && path[k] != '\\' && path[k] != '/') k--;
    char pre[100]; int pl = k < 0 ? 0 : k; if (pl > 98) pl = 98; for (int i = 0; i < pl; i++) pre[i] = path[i]; pre[pl] = 0;
    rp_cpy(name, path + k + 1, 24);
    if (k < 0) *dir = fs_cwd; else if (k == 0) *dir = 0; else *dir = fs_walk(pre, fs_cwd);
    return *dir >= 0 && fsn[*dir].dir && name[0];
}
static int fs_uniq(int dir, char *out, const char *base, const char *ext) {
    for (int k = 1; k < 999; k++) { char *e = sapp(out, base); e = iapp(e, k); *e++ = '.'; sapp(e, ext); if (fs_child(dir, out) < 0) return k; }
    return 0;
}
static void fs_init(void) {
    for (int i = 0; i < FS_MAX; i++) fsn[i].used = 0; fs_top = 0;
    fsn[0].used = 1; fsn[0].dir = 1; fsn[0].parent = 0; sapp(fsn[0].name, "C:");
    int rw = fs_mk(0, "RedWill", 1);
    fs_red32 = fs_mk(rw, "red32", 1); fs_sdk = fs_mk(rw, "red-sdk", 1); fs_docs = fs_mk(rw, "My Documents", 1);
    fs_cwd = fs_docs;
}
