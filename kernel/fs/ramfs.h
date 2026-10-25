/* ---------- RAM file system ---------- */
typedef struct { char name[20]; char d[900]; int len, used; } File;
static File fs[20];
static int fs_find(const char *n) { for (int i = 0; i < 20; i++) if (fs[i].used && !scmp(fs[i].name, n)) return i; return -1; }
static int fs_new(const char *n) {
    for (int i = 0; i < 20; i++) if (!fs[i].used) {
        fs[i].used = 1; fs[i].len = 0; fs[i].d[0] = 0; int k = 0;
        for (; n[k] && k < 19; k++) fs[i].name[k] = up_(n[k]);
        fs[i].name[k] = 0; return i;
    }
    return -1;
}
static void fs_put(const char *n, const char *s) { int i = fs_find(n); if (i < 0) i = fs_new(n); if (i < 0) return; int k = 0; while (s[k] && k < 890) { fs[i].d[k] = s[k]; k++; } fs[i].d[k] = 0; fs[i].len = k; }
static int fs_count(void) { int n = 0; for (int i = 0; i < 20; i++) n += fs[i].used; return n; }
static int fs_uniq(char *out, const char *base) { for (int k = 1; k < 99; k++) { char *e = sapp(out, base); e = iapp(e, k); sapp(e, ".TXT"); if (fs_find(out) < 0) return k; } return 0; }

