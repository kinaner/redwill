/* rpp.h - RedWill Script (RPP) compiler and RXE virtual machine.
 *
 *   .rpp  = source text   (compiled on the fly by the OS, or ahead of time with tools/rppc)
 *   .rxe  = executable    (RXE1 bytecode container, runs on the VM below)
 *
 * Freestanding: needs only u8/u16/u32 typedefs. The same file is used by the kernel and by the host
 * compiler tools/rppc.c. Everything that touches the GUI/files is reached through the host callback
 * (builtin ids >= 100); ids 1..99 are pure builtins implemented here. */
#ifndef RPP_H
#define RPP_H

#define RPP_MAXCODE  6000
#define RPP_MAXCONST 4096
#define RPP_MAXG     1024
#define RPP_HEAP     24576
#define RPP_STACK    512
#define RPP_FRAMES   64
#define RPP_MAXFN    64
#define RPP_NAME     20
#define RPP_STEPS    600000
#define S_TAG        0x70000000
#define S_MASK       0xFF000000u

enum { OP_HALT, OP_PUSH, OP_LOADG, OP_STOREG, OP_LOADL, OP_STOREL, OP_LOADA, OP_STOREA, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
       OP_NEG, OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE, OP_NOT, OP_JMP, OP_JZ, OP_JNZ, OP_DUP, OP_POP, OP_CALL, OP_RET, OP_SYS, OP__N };

typedef struct { const char *n; short id, argc; } RpBi;
static const RpBi RPP_BI[] = {
    /* pure builtins (run inside the VM) */
    { "len", 1, 1 }, { "mid", 2, 3 }, { "chr", 3, 1 }, { "asc", 4, 1 }, { "str", 5, 1 }, { "val", 6, 1 }, { "find", 7, 2 },
    { "upper", 8, 1 }, { "lower", 9, 1 }, { "abs", 10, 1 }, { "min", 11, 2 }, { "max", 12, 2 }, { "rnd", 13, 1 }, { "rgb", 14, 3 },
    { "left", 15, 2 }, { "right", 16, 2 }, { "field", 17, 3 }, { "fields", 18, 2 }, { "hex", 19, 1 }, { "trim", 20, 1 },
    /* host builtins (kernel/rpp/rpp_api.h) */
    { "print", 100, 1 }, { "exit", 101, 0 }, { "ticks", 102, 0 }, { "sleep", 103, 1 }, { "beep", 104, 2 }, { "time_str", 105, 0 },
    { "username", 106, 0 }, { "screen_w", 107, 0 }, { "screen_h", 108, 0 }, { "mouse_x", 109, 0 }, { "mouse_y", 110, 0 },
    { "window", 120, 3 }, { "win_title", 121, 2 }, { "win_close", 122, 1 }, { "win_redraw", 123, 1 }, { "win_move", 124, 3 },
    { "label", 130, 4 }, { "button", 131, 7 }, { "textbox", 132, 5 }, { "checkbox", 133, 5 }, { "listbox", 134, 6 }, { "progress", 135, 5 },
    { "set_text", 140, 2 }, { "get_text", 141, 1 }, { "set_checked", 142, 2 }, { "get_checked", 143, 1 }, { "set_value", 144, 2 },
    { "get_value", 145, 1 }, { "list_add", 146, 2 }, { "list_clear", 147, 1 }, { "list_sel", 148, 1 }, { "list_get", 149, 2 },
    { "list_count", 150, 1 }, { "ctl_enable", 151, 2 }, { "ctl_show", 152, 2 }, { "ctl_focus", 153, 1 }, { "list_select", 154, 2 },
    { "on_paint", 160, 2 }, { "on_click", 161, 2 }, { "on_key", 162, 2 }, { "on_timer", 163, 3 }, { "on_close", 164, 2 },
    { "fill_rect", 170, 5 }, { "draw_rect", 171, 5 }, { "draw_text", 172, 4 }, { "draw_line", 173, 5 }, { "fill_circle", 174, 4 },
    { "draw_icon", 175, 4 },
    { "msgbox", 180, 1 }, { "confirm", 181, 1 }, { "open_dialog", 182, 0 }, { "save_dialog", 183, 1 }, { "input_box", 184, 2 },
    { "file_read", 190, 1 }, { "file_write", 191, 2 }, { "file_exists", 192, 1 }, { "file_delete", 193, 1 }, { "dir_list", 194, 1 },
    { "mkdir", 195, 1 }, { "run", 196, 1 }, { "file_append", 197, 2 }, { "is_dir", 198, 1 },
    { 0, 0, 0 } };

typedef struct Vm Vm;
typedef int (*RpHost)(Vm *vm, int id, int *a);
struct Vm {
    int used, busy;
    int code[RPP_MAXCODE]; int ncode;
    u8 kst[RPP_MAXCONST]; int nk;
    int nfn; struct { int addr, np, nl, defined; char name[RPP_NAME]; } fn[RPP_MAXFN];
    int ng; int g[RPP_MAXG];
    u8 heap[2][RPP_HEAP]; int hcur, hn;
    int st[RPP_STACK]; int sp, fp;
    struct { int pc, fp; } cs[RPP_FRAMES]; int nc;
    int err; char errmsg[96];
    u32 rng; int steps;
    int id; void *user;
};

/* ---------------------------------------------------------------- helpers */
static void rp_zero(void *p, int n) { u8 *b = (u8 *)p; for (int i = 0; i < n; i++) b[i] = 0; }
static int rp_len(const char *s) { int n = 0; while (s[n]) n++; return n; }
static int rp_eq(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *a == *b; }
static void rp_cpy(char *d, const char *s, int max) { int i = 0; for (; s[i] && i < max - 1; i++) d[i] = s[i]; d[i] = 0; }
static void rp_fail(Vm *vm, const char *m) { if (!vm->err) { vm->err = 1; rp_cpy(vm->errmsg, m, 96); } }
static char *rp_itoa(char *d, int v) {
    char t[12]; int n = 0; unsigned u = v < 0 ? (unsigned)(-(v + 1)) + 1u : (unsigned)v;
    do { t[n++] = (char)('0' + u % 10); u /= 10; } while (u);
    if (v < 0) *d++ = '-';
    while (n) *d++ = t[--n];
    *d = 0; return d;
}

/* ---------------------------------------------------------------- strings */
static int rp_isstr(int v) { return ((u32)v & S_MASK) == (u32)S_TAG; }
static const char *rp_str(Vm *vm, int v) {
    if (!rp_isstr(v)) return 0;
    int off = v & 0xFFFFFF;
    if (off < 0x10000) return off < vm->nk ? (const char *)vm->kst + off : "";
    off -= 0x10000; return off < vm->hn ? (const char *)vm->heap[vm->hcur] + off : "";
}
static const char *rp_tostr(Vm *vm, int v, char *buf) { const char *s = rp_str(vm, v); if (s) return s; rp_itoa(buf, v); return buf; }
static void rp_gc(Vm *vm) {
    int o = 1 - vm->hcur, n = 0; u8 *dst = vm->heap[o];
    for (int pass = 0; pass < 2; pass++) {
        int cnt = pass ? vm->sp : vm->ng; int *arr = pass ? vm->st : vm->g;
        for (int i = 0; i < cnt; i++) {
            int v = arr[i]; if (!rp_isstr(v) || (v & 0xFFFFFF) < 0x10000) continue;
            int off = (v & 0xFFFFFF) - 0x10000; if (off >= vm->hn) continue;
            const char *s = (const char *)vm->heap[vm->hcur] + off; int l = rp_len(s) + 1;
            if (n + l > RPP_HEAP) { rp_fail(vm, "out of string memory"); return; }
            for (int k = 0; k < l; k++) dst[n + k] = (u8)s[k];
            arr[i] = S_TAG | (0x10000 + n); n += l;
        }
    }
    vm->hcur = o; vm->hn = n;
}
static int rp_newstr(Vm *vm, const char *s, int n) {
    if (vm->hn + n + 1 > RPP_HEAP) { rp_gc(vm); if (vm->err) return 0; if (vm->hn + n + 1 > RPP_HEAP) { rp_fail(vm, "out of string memory"); return 0; } }
    u8 *d = vm->heap[vm->hcur] + vm->hn; for (int i = 0; i < n; i++) d[i] = (u8)s[i]; d[n] = 0;
    int h = S_TAG | (0x10000 + vm->hn); vm->hn += n + 1; return h;
}
static int rp_cstr(Vm *vm, const char *s) { return rp_newstr(vm, s, rp_len(s)); }
/* GC-safe concat: a and b must still be on the VM stack while this runs */
static int rp_concat(Vm *vm, int a, int b) {
    char ba[12], bb[12]; const char *sa = rp_tostr(vm, a, ba), *sb = rp_tostr(vm, b, bb);
    int la = rp_len(sa), lb = rp_len(sb);
    if (vm->hn + la + lb + 1 > RPP_HEAP) { rp_gc(vm); if (vm->err) return 0; sa = rp_tostr(vm, vm->st[vm->sp - 2], ba); sb = rp_tostr(vm, vm->st[vm->sp - 1], bb); }
    if (vm->hn + la + lb + 1 > RPP_HEAP) { rp_fail(vm, "out of string memory"); return 0; }
    u8 *d = vm->heap[vm->hcur] + vm->hn; for (int i = 0; i < la; i++) d[i] = (u8)sa[i]; for (int i = 0; i < lb; i++) d[la + i] = (u8)sb[i]; d[la + lb] = 0;
    int h = S_TAG | (0x10000 + vm->hn); vm->hn += la + lb + 1; return h;
}
static int rp_cmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return (u8)*a - (u8)*b; }
static char rp_up(char c) { return (c >= 'a' && c <= 'z') ? (char)(c - 32) : c; }
static char rp_lo(char c) { return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c; }

/* ---------------------------------------------------------------- pure builtins */
static int rp_core(Vm *vm, int id, int *a) {
    char b0[12], b1[12]; const char *s = 0, *t = 0; int r;
    switch (id) {
    case 1: s = rp_tostr(vm, a[0], b0); return rp_len(s);
    case 2: { s = rp_tostr(vm, a[0], b0); int l = rp_len(s), i = a[1], n = a[2]; if (i < 0) { n += i; i = 0; } if (i > l) i = l; if (n < 0) n = 0; if (i + n > l) n = l - i; return rp_newstr(vm, rp_tostr(vm, a[0], b0) + i, n); }
    case 3: { char c[2] = { (char)a[0], 0 }; return rp_newstr(vm, c, a[0] ? 1 : 0); }
    case 4: s = rp_tostr(vm, a[0], b0); return (u8)s[0];
    case 5: if (rp_isstr(a[0])) return a[0]; rp_itoa(b0, a[0]); return rp_cstr(vm, b0);
    case 6: { s = rp_tostr(vm, a[0], b0); int neg = 0, v = 0; while (*s == ' ') s++; if (*s == '-') { neg = 1; s++; }
              if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) { s += 2; while ((*s >= '0' && *s <= '9') || (rp_lo(*s) >= 'a' && rp_lo(*s) <= 'f')) { v = v * 16 + (*s <= '9' ? *s - '0' : rp_lo(*s) - 'a' + 10); s++; } }
              else while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; }
              return neg ? -v : v; }
    case 7: { s = rp_tostr(vm, a[0], b0); t = rp_tostr(vm, a[1], b1); int ls = rp_len(s), lt = rp_len(t);
              for (int i = 0; i + lt <= ls; i++) { int k = 0; while (k < lt && s[i + k] == t[k]) k++; if (k == lt) return i; } return -1; }
    case 8: case 9: { s = rp_tostr(vm, a[0], b0); int l = rp_len(s); r = rp_newstr(vm, s, l); if (vm->err) return 0; s = rp_str(vm, r); char *d = (char *)vm->heap[vm->hcur] + ((r & 0xFFFFFF) - 0x10000);
              for (int i = 0; i < l; i++) d[i] = id == 8 ? rp_up(d[i]) : rp_lo(d[i]); return r; }
    case 10: return a[0] < 0 ? -a[0] : a[0];
    case 11: return a[0] < a[1] ? a[0] : a[1];
    case 12: return a[0] > a[1] ? a[0] : a[1];
    case 13: { if (a[0] <= 0) return 0; vm->rng ^= vm->rng << 13; vm->rng ^= vm->rng >> 17; vm->rng ^= vm->rng << 5; return (int)((vm->rng >> 4) % (u32)a[0]); }
    case 14: return ((a[0] & 255) << 16) | ((a[1] & 255) << 8) | (a[2] & 255);
    case 15: { s = rp_tostr(vm, a[0], b0); int l = rp_len(s), n = a[1]; if (n < 0) n = 0; if (n > l) n = l; return rp_newstr(vm, rp_tostr(vm, a[0], b0), n); }
    case 16: { s = rp_tostr(vm, a[0], b0); int l = rp_len(s), n = a[1]; if (n < 0) n = 0; if (n > l) n = l; return rp_newstr(vm, rp_tostr(vm, a[0], b0) + l - n, n); }
    case 17: case 18: { s = rp_tostr(vm, a[0], b0); t = rp_tostr(vm, a[1], b1); char sep = t[0]; int idx = id == 17 ? a[2] : -1, f = 0, i = 0, st = 0, l = rp_len(s);
              for (;; i++) { if (i == l || s[i] == sep) { if (f == idx) return rp_newstr(vm, rp_tostr(vm, a[0], b0) + st, i - st); f++; st = i + 1; if (i == l) break; } }
              return id == 18 ? f : rp_newstr(vm, "", 0); }
    case 19: { char h[12]; const char *dg = "0123456789ABCDEF"; u32 v = (u32)a[0]; int n = 0; char t2[10]; do { t2[n++] = dg[v & 15]; v >>= 4; } while (v); int k = 0; while (n) h[k++] = t2[--n]; h[k] = 0; return rp_cstr(vm, h); }
    case 20: { s = rp_tostr(vm, a[0], b0); int l = rp_len(s), i = 0; while (i < l && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) i++; while (l > i && (s[l - 1] == ' ' || s[l - 1] == '\t' || s[l - 1] == '\n' || s[l - 1] == '\r')) l--; return rp_newstr(vm, rp_tostr(vm, a[0], b0) + i, l - i); }
    }
    return 0;
}

/* ---------------------------------------------------------------- virtual machine */
static RpHost rpp_host;
#define VMERR(m) do { rp_fail(vm, m); goto done; } while (0)
/* run function fn with args; returns its result (0 on error: check vm->err) */
static int rp_call(Vm *vm, int fn, const int *args, int na) {
    if (vm->busy || vm->err) return 0;
    if (fn < 0 || fn >= vm->nfn || !vm->fn[fn].defined) return 0;
    vm->busy = 1; vm->sp = 0; vm->fp = 0; vm->nc = 0; vm->steps = 0;
    for (int i = 0; i < na && i < 8; i++) vm->st[vm->sp++] = args[i];
    int pc = -1, ret = 0;
    vm->cs[vm->nc].pc = -1; vm->cs[vm->nc].fp = 0; vm->nc++;
    { int np = vm->fn[fn].np; while (na < np) { vm->st[vm->sp++] = 0; na++; } if (na > np) vm->sp -= na - np; vm->fp = vm->sp - np;
      for (int i = np; i < vm->fn[fn].nl; i++) vm->st[vm->sp++] = 0; pc = vm->fn[fn].addr; }
    for (;;) {
        if (pc < 0 || pc >= vm->ncode) VMERR("bad program counter");
        if (++vm->steps > RPP_STEPS) VMERR("script ran too long (infinite loop?)");
        if (vm->sp > RPP_STACK - 16) VMERR("stack overflow");
        int op = vm->code[pc++];
        switch (op) {
        case OP_HALT: ret = 0; goto done;
        case OP_PUSH: vm->st[vm->sp++] = vm->code[pc++]; break;
        case OP_LOADG: { int i = vm->code[pc++]; if (i < 0 || i >= vm->ng) VMERR("bad global"); vm->st[vm->sp++] = vm->g[i]; break; }
        case OP_STOREG: { int i = vm->code[pc++]; if (i < 0 || i >= vm->ng) VMERR("bad global"); vm->g[i] = vm->st[--vm->sp]; break; }
        case OP_LOADL: { int i = vm->code[pc++]; if (vm->fp + i >= RPP_STACK || vm->fp + i < 0) VMERR("bad local"); vm->st[vm->sp++] = vm->st[vm->fp + i]; break; }
        case OP_STOREL: { int i = vm->code[pc++]; if (vm->fp + i >= RPP_STACK || vm->fp + i < 0) VMERR("bad local"); vm->st[vm->fp + i] = vm->st[--vm->sp]; break; }
        case OP_LOADA: { int b = vm->code[pc++], n = vm->code[pc++], i = vm->st[--vm->sp]; if (i < 0 || i >= n || b + n > vm->ng) VMERR("array index out of range"); vm->st[vm->sp++] = vm->g[b + i]; break; }
        case OP_STOREA: { int b = vm->code[pc++], n = vm->code[pc++], v = vm->st[--vm->sp], i = vm->st[--vm->sp]; if (i < 0 || i >= n || b + n > vm->ng) VMERR("array index out of range"); vm->g[b + i] = v; break; }
        case OP_ADD: { int y = vm->st[vm->sp - 1], x = vm->st[vm->sp - 2];
            if (rp_isstr(x) || rp_isstr(y)) { int r = rp_concat(vm, x, y); if (vm->err) goto done; vm->sp -= 2; vm->st[vm->sp++] = r; } else { vm->sp--; vm->st[vm->sp - 1] = x + y; } break; }
        case OP_SUB: vm->sp--; vm->st[vm->sp - 1] -= vm->st[vm->sp]; break;
        case OP_MUL: vm->sp--; vm->st[vm->sp - 1] *= vm->st[vm->sp]; break;
        case OP_DIV: { int y = vm->st[--vm->sp]; if (!y) VMERR("division by zero"); vm->st[vm->sp - 1] /= y; break; }
        case OP_MOD: { int y = vm->st[--vm->sp]; if (!y) VMERR("division by zero"); vm->st[vm->sp - 1] %= y; break; }
        case OP_NEG: vm->st[vm->sp - 1] = -vm->st[vm->sp - 1]; break;
        case OP_EQ: case OP_NE: case OP_LT: case OP_GT: case OP_LE: case OP_GE: {
            int y = vm->st[--vm->sp], x = vm->st[vm->sp - 1], c;
            if (rp_isstr(x) && rp_isstr(y)) c = rp_cmp(rp_str(vm, x), rp_str(vm, y)); else c = x < y ? -1 : x > y ? 1 : 0;
            int r = op == OP_EQ ? c == 0 : op == OP_NE ? c != 0 : op == OP_LT ? c < 0 : op == OP_GT ? c > 0 : op == OP_LE ? c <= 0 : c >= 0;
            vm->st[vm->sp - 1] = r; break; }
        case OP_NOT: vm->st[vm->sp - 1] = !vm->st[vm->sp - 1]; break;
        case OP_JMP: pc = vm->code[pc]; break;
        case OP_JZ: { int t = vm->code[pc++]; if (!vm->st[--vm->sp]) pc = t; break; }
        case OP_JNZ: { int t = vm->code[pc++]; if (vm->st[--vm->sp]) pc = t; break; }
        case OP_DUP: vm->st[vm->sp] = vm->st[vm->sp - 1]; vm->sp++; break;
        case OP_POP: vm->sp--; break;
        case OP_CALL: {
            int f = vm->code[pc++], n = vm->code[pc++];
            if (f < 0 || f >= vm->nfn || !vm->fn[f].defined) VMERR("call to undefined function");
            if (vm->nc >= RPP_FRAMES) VMERR("call stack overflow");
            int np = vm->fn[f].np; while (n < np) { vm->st[vm->sp++] = 0; n++; } if (n > np) vm->sp -= n - np;
            vm->cs[vm->nc].pc = pc; vm->cs[vm->nc].fp = vm->fp; vm->nc++;
            vm->fp = vm->sp - np; for (int i = np; i < vm->fn[f].nl; i++) vm->st[vm->sp++] = 0;
            pc = vm->fn[f].addr; break; }
        case OP_RET: {
            int r = vm->st[--vm->sp]; vm->sp = vm->fp; vm->nc--; pc = vm->cs[vm->nc].pc; vm->fp = vm->cs[vm->nc].fp;
            if (pc < 0) { ret = r; goto done; }
            vm->st[vm->sp++] = r; break; }
        case OP_SYS: {
            int id = vm->code[pc++], n = vm->code[pc++]; if (n < 0 || n > vm->sp) VMERR("bad syscall");
            int *a = vm->st + vm->sp - n, r = 0;
            if (id < 100) r = rp_core(vm, id, a); else if (rpp_host) r = rpp_host(vm, id, a); else VMERR("no host for this call");
            if (vm->err) goto done; vm->sp -= n; vm->st[vm->sp++] = r;
            if (vm->err == 0 && vm->busy == 2) { vm->busy = 1; ret = 0; goto done; }   /* exit() */
            break; }
        default: VMERR("bad opcode");
        }
    }
done:
    vm->busy = 0; vm->sp = 0; vm->nc = 0; return ret;
}

/* ---------------------------------------------------------------- compiler */
enum { T_EOF, T_NUM, T_STR, T_ID, K_VAR = 10, K_FUNC, K_IF, K_ELSE, K_WHILE, K_FOR, K_RET, K_BRK, K_CONT, T_EQ = 20, T_NE, T_LE, T_GE, T_AND, T_OR, T_PE, T_ME };
typedef struct {
    Vm *vm; const char *s; int p, line, t, tv; char ts[256]; int err; char *emsg;
    int infn, nl; char ln[48][RPP_NAME];
    int ngn; char gn[256][RPP_NAME]; int gb[256], gz[256];
    int nlp; struct { int brk[24], nb, cont[24], nc, top; } lp[8];
} Cc;
static void cc_err(Cc *c, const char *m) {
    if (c->err) return; c->err = 1; char *d = c->emsg; d[0] = 0; char *e = d; const char *l = "line "; while (*l) *e++ = *l++; e = rp_itoa(e, c->line);
    *e++ = ':'; *e++ = ' '; int n = 0; while (m[n] && n < 70) { *e++ = m[n]; n++; } *e = 0;
}
static int cc_isid(char ch, int first) { return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (!first && ch >= '0' && ch <= '9'); }
static void cc_next(Cc *c) {
    const char *s = c->s; int p = c->p;
    for (;;) {
        while (s[p] == ' ' || s[p] == '\t' || s[p] == '\r' || s[p] == '\n') { if (s[p] == '\n') c->line++; p++; }
        if (s[p] == '/' && s[p + 1] == '/') { while (s[p] && s[p] != '\n') p++; continue; }
        if (s[p] == '/' && s[p + 1] == '*') { p += 2; while (s[p] && !(s[p] == '*' && s[p + 1] == '/')) { if (s[p] == '\n') c->line++; p++; } if (s[p]) p += 2; continue; }
        break;
    }
    c->ts[0] = 0; c->tv = 0;
    if (!s[p]) { c->t = T_EOF; c->p = p; return; }
    char ch = s[p];
    if (ch >= '0' && ch <= '9') {
        int v = 0;
        if (ch == '0' && (s[p + 1] == 'x' || s[p + 1] == 'X')) { p += 2; while ((s[p] >= '0' && s[p] <= '9') || (rp_lo(s[p]) >= 'a' && rp_lo(s[p]) <= 'f')) { v = v * 16 + (s[p] <= '9' ? s[p] - '0' : rp_lo(s[p]) - 'a' + 10); p++; } }
        else while (s[p] >= '0' && s[p] <= '9') { v = v * 10 + (s[p] - '0'); p++; }
        c->t = T_NUM; c->tv = v; c->p = p; return;
    }
    if (cc_isid(ch, 1)) {
        int n = 0; while (cc_isid(s[p], 0)) { if (n < RPP_NAME - 1) c->ts[n++] = s[p]; p++; } c->ts[n] = 0; c->p = p; c->t = T_ID;
        static const char *KW[] = { "var", "func", "if", "else", "while", "for", "return", "break", "continue" };
        for (int i = 0; i < 9; i++) if (rp_eq(c->ts, KW[i])) c->t = K_VAR + i;
        return;
    }
    if (ch == '"') {
        p++; int n = 0;
        while (s[p] && s[p] != '"') {
            char d = s[p++];
            if (d == '\\') { char e = s[p++]; d = e == 'n' ? '\n' : e == 't' ? '\t' : e == 'r' ? '\r' : e == '0' ? ' ' : e; }
            else if (d == '\n') c->line++;
            if (n < 254) c->ts[n++] = d;
        }
        if (!s[p]) cc_err(c, "unterminated string"); else p++;
        c->ts[n] = 0; c->t = T_STR; c->tv = n; c->p = p; return;
    }
    if (ch == '\'') { int v = (u8)s[p + 1]; if (s[p + 1] == '\\') { char e = s[p + 2]; v = e == 'n' ? 10 : e == 't' ? 9 : e; p++; } if (s[p + 2] == '\'') p += 3; else { cc_err(c, "bad character literal"); p++; } c->t = T_NUM; c->tv = v; c->p = p; return; }
    char n2 = s[p + 1];
    if (ch == '=' && n2 == '=') { c->t = T_EQ; c->p = p + 2; return; }
    if (ch == '!' && n2 == '=') { c->t = T_NE; c->p = p + 2; return; }
    if (ch == '<' && n2 == '=') { c->t = T_LE; c->p = p + 2; return; }
    if (ch == '>' && n2 == '=') { c->t = T_GE; c->p = p + 2; return; }
    if (ch == '&' && n2 == '&') { c->t = T_AND; c->p = p + 2; return; }
    if (ch == '|' && n2 == '|') { c->t = T_OR; c->p = p + 2; return; }
    if (ch == '+' && n2 == '=') { c->t = T_PE; c->p = p + 2; return; }
    if (ch == '-' && n2 == '=') { c->t = T_ME; c->p = p + 2; return; }
    c->t = (u8)ch; c->p = p + 1;
}
static void cc_emit(Cc *c, int w) { if (c->vm->ncode >= RPP_MAXCODE - 4) { cc_err(c, "program too large"); return; } c->vm->code[c->vm->ncode++] = w; }
static void cc_op1(Cc *c, int op, int a) { cc_emit(c, op); cc_emit(c, a); }
static int cc_jmp(Cc *c, int op) { cc_emit(c, op); int at = c->vm->ncode; cc_emit(c, 0); return at; }
static void cc_patch(Cc *c, int at) { if (!c->err) c->vm->code[at] = c->vm->ncode; }
static void cc_expect(Cc *c, int t, const char *m) { if (c->t != t) cc_err(c, m); else cc_next(c); }
static int cc_const(Cc *c, const char *s, int n) {
    Vm *vm = c->vm; if (vm->nk + n + 1 > RPP_MAXCONST) { cc_err(c, "too many string constants"); return 0; }
    int off = vm->nk; for (int i = 0; i < n; i++) vm->kst[vm->nk++] = (u8)s[i]; vm->kst[vm->nk++] = 0; return S_TAG | off;
}
static int cc_fn(Cc *c, const char *name, int create) {
    Vm *vm = c->vm; for (int i = 1; i < vm->nfn; i++) if (rp_eq(vm->fn[i].name, name)) return i;
    if (!create) return -1; if (vm->nfn >= RPP_MAXFN) { cc_err(c, "too many functions"); return -1; }
    int i = vm->nfn++; rp_cpy(vm->fn[i].name, name, RPP_NAME); vm->fn[i].defined = 0; return i;
}
static int cc_local(Cc *c, const char *n) { for (int i = 0; i < c->nl; i++) if (rp_eq(c->ln[i], n)) return i; return -1; }
static int cc_global(Cc *c, const char *n) { for (int i = 0; i < c->ngn; i++) if (rp_eq(c->gn[i], n)) return i; return -1; }
static int cc_builtin(const char *n) { for (int i = 0; RPP_BI[i].n; i++) if (rp_eq(RPP_BI[i].n, n)) return i; return -1; }
static void p_expr(Cc *c);
static void p_primary(Cc *c) {
    if (c->err) return;
    if (c->t == T_NUM) { cc_op1(c, OP_PUSH, c->tv); cc_next(c); return; }
    if (c->t == T_STR) { int h = cc_const(c, c->ts, c->tv); cc_op1(c, OP_PUSH, h); cc_next(c); return; }
    if (c->t == '(') { cc_next(c); p_expr(c); cc_expect(c, ')', "expected ')'"); return; }
    if (c->t == '-') { cc_next(c); p_primary(c); cc_emit(c, OP_NEG); return; }
    if (c->t == '!') { cc_next(c); p_primary(c); cc_emit(c, OP_NOT); return; }
    if (c->t != T_ID) { cc_err(c, "expected an expression"); return; }
    char name[RPP_NAME]; rp_cpy(name, c->ts, RPP_NAME); cc_next(c);
    if (c->t == '(') {                                           /* call */
        cc_next(c); int n = 0;
        if (c->t != ')') for (;;) { p_expr(c); n++; if (c->err) return; if (c->t == ',') cc_next(c); else break; }
        cc_expect(c, ')', "expected ')' after arguments");
        int b = cc_builtin(name);
        if (b >= 0) { if (n != RPP_BI[b].argc) { cc_err(c, "wrong number of arguments"); return; } cc_emit(c, OP_SYS); cc_emit(c, RPP_BI[b].id); cc_emit(c, n); }
        else { int f = cc_fn(c, name, 1); if (f < 0) return; cc_emit(c, OP_CALL); cc_emit(c, f); cc_emit(c, n); }
        return;
    }
    int l = cc_local(c, name);
    if (l >= 0) { cc_op1(c, OP_LOADL, l); return; }
    int g = cc_global(c, name);
    if (g >= 0) {
        if (c->gz[g]) { if (c->t != '[') { cc_err(c, "array needs an index"); return; } cc_next(c); p_expr(c); cc_expect(c, ']', "expected ']'"); cc_emit(c, OP_LOADA); cc_emit(c, c->gb[g]); cc_emit(c, c->gz[g]); }
        else cc_op1(c, OP_LOADG, c->gb[g]);
        return;
    }
    { int f = cc_fn(c, name, 1); if (f < 0) return; cc_op1(c, OP_PUSH, f); }   /* bare function name = handler reference */
}
static void p_mul(Cc *c) { p_primary(c); while (!c->err && (c->t == '*' || c->t == '/' || c->t == '%')) { int t = c->t; cc_next(c); p_primary(c); cc_emit(c, t == '*' ? OP_MUL : t == '/' ? OP_DIV : OP_MOD); } }
static void p_add(Cc *c) { p_mul(c); while (!c->err && (c->t == '+' || c->t == '-')) { int t = c->t; cc_next(c); p_mul(c); cc_emit(c, t == '+' ? OP_ADD : OP_SUB); } }
static void p_rel(Cc *c) { p_add(c); while (!c->err && (c->t == '<' || c->t == '>' || c->t == T_LE || c->t == T_GE)) { int t = c->t; cc_next(c); p_add(c); cc_emit(c, t == '<' ? OP_LT : t == '>' ? OP_GT : t == T_LE ? OP_LE : OP_GE); } }
static void p_eq(Cc *c) { p_rel(c); while (!c->err && (c->t == T_EQ || c->t == T_NE)) { int t = c->t; cc_next(c); p_rel(c); cc_emit(c, t == T_EQ ? OP_EQ : OP_NE); } }
static void p_and(Cc *c) { p_eq(c); while (!c->err && c->t == T_AND) { cc_next(c); cc_emit(c, OP_DUP); int j = cc_jmp(c, OP_JZ); cc_emit(c, OP_POP); p_eq(c); cc_patch(c, j); } }
static void p_expr(Cc *c) { p_and(c); while (!c->err && c->t == T_OR) { cc_next(c); cc_emit(c, OP_DUP); int j = cc_jmp(c, OP_JNZ); cc_emit(c, OP_POP); p_and(c); cc_patch(c, j); } }

static void p_stmt(Cc *c);
static void p_block(Cc *c) { cc_expect(c, '{', "expected '{'"); while (!c->err && c->t != '}' && c->t != T_EOF) p_stmt(c); cc_expect(c, '}', "expected '}'"); }
static void p_store(Cc *c, const char *name) {              /* value already on the stack */
    int l = cc_local(c, name); if (l >= 0) { cc_op1(c, OP_STOREL, l); return; }
    int g = cc_global(c, name); if (g >= 0 && !c->gz[g]) { cc_op1(c, OP_STOREG, c->gb[g]); return; }
    cc_err(c, "cannot assign to this name");
}
static void p_load_name(Cc *c, const char *name) { int l = cc_local(c, name); if (l >= 0) { cc_op1(c, OP_LOADL, l); return; } int g = cc_global(c, name); if (g >= 0 && !c->gz[g]) { cc_op1(c, OP_LOADG, c->gb[g]); return; } cc_err(c, "unknown variable"); }
static void p_var(Cc *c) {
    cc_next(c);
    for (;;) {
        if (c->t != T_ID) { cc_err(c, "expected a variable name"); return; }
        char name[RPP_NAME]; rp_cpy(name, c->ts, RPP_NAME); cc_next(c);
        if (cc_builtin(name) >= 0 || cc_fn(c, name, 0) >= 0) { cc_err(c, "name already used by a function"); return; }
        if (c->infn) {
            if (cc_local(c, name) >= 0) { cc_err(c, "variable declared twice"); return; }
            if (c->nl >= 48) { cc_err(c, "too many local variables"); return; }
            if (c->t == '[') { cc_err(c, "arrays must be global"); return; }
            int idx = c->nl; rp_cpy(c->ln[c->nl++], name, RPP_NAME);
            if (c->t == '=') { cc_next(c); p_expr(c); } else cc_op1(c, OP_PUSH, 0);
            cc_op1(c, OP_STOREL, idx);
        } else {
            if (cc_global(c, name) >= 0) { cc_err(c, "variable declared twice"); return; }
            if (c->ngn >= 256) { cc_err(c, "too many globals"); return; }
            int gi = c->ngn++; rp_cpy(c->gn[gi], name, RPP_NAME); c->gz[gi] = 0; c->gb[gi] = c->vm->ng;
            if (c->t == '[') {
                cc_next(c); if (c->t != T_NUM || c->tv < 1) { cc_err(c, "array size must be a number"); return; }
                int n = c->tv; cc_next(c); cc_expect(c, ']', "expected ']'");
                if (c->vm->ng + n > RPP_MAXG) { cc_err(c, "not enough global memory"); return; }
                c->gz[gi] = n; c->vm->ng += n;
            } else {
                if (c->vm->ng + 1 > RPP_MAXG) { cc_err(c, "not enough global memory"); return; }
                c->vm->ng++;
                if (c->t == '=') { cc_next(c); p_expr(c); } else cc_op1(c, OP_PUSH, 0);
                cc_op1(c, OP_STOREG, c->gb[gi]);
            }
        }
        if (c->err) return;
        if (c->t == ',') { cc_next(c); continue; }
        break;
    }
    cc_expect(c, ';', "expected ';'");
}
static void p_loop_end(Cc *c, int cont_target) {
    int i = c->nlp - 1;
    for (int k = 0; k < c->lp[i].nb; k++) cc_patch(c, c->lp[i].brk[k]);
    for (int k = 0; k < c->lp[i].nc; k++) if (!c->err) c->vm->code[c->lp[i].cont[k]] = cont_target;
    c->nlp--;
}
static void p_stmt(Cc *c) {
    if (c->err) return;
    Vm *vm = c->vm;
    switch (c->t) {
    case ';': cc_next(c); return;
    case '{': p_block(c); return;
    case K_VAR: p_var(c); return;
    case K_FUNC: {
        if (c->infn) { cc_err(c, "functions cannot be nested"); return; }
        cc_next(c); if (c->t != T_ID) { cc_err(c, "expected a function name"); return; }
        if (cc_builtin(c->ts) >= 0 || cc_global(c, c->ts) >= 0) { cc_err(c, "name already in use"); return; }
        int f = cc_fn(c, c->ts, 1); if (f < 0) return; if (vm->fn[f].defined) { cc_err(c, "function defined twice"); return; }
        cc_next(c); cc_expect(c, '(', "expected '('");
        c->nl = 0; int np = 0;
        if (c->t != ')') for (;;) { if (c->t != T_ID) { cc_err(c, "expected a parameter name"); return; } if (c->nl >= 8) { cc_err(c, "too many parameters (max 8)"); return; } rp_cpy(c->ln[c->nl++], c->ts, RPP_NAME); np++; cc_next(c); if (c->t == ',') cc_next(c); else break; }
        cc_expect(c, ')', "expected ')'");
        int skip = cc_jmp(c, OP_JMP); vm->fn[f].addr = vm->ncode; vm->fn[f].np = np; vm->fn[f].defined = 1; c->infn = 1;
        p_block(c); cc_op1(c, OP_PUSH, 0); cc_emit(c, OP_RET); vm->fn[f].nl = c->nl; c->infn = 0; c->nl = 0; cc_patch(c, skip); return; }
    case K_IF: {
        cc_next(c); cc_expect(c, '(', "expected '(' after if"); p_expr(c); cc_expect(c, ')', "expected ')'");
        int jf = cc_jmp(c, OP_JZ); p_stmt(c);
        if (c->t == K_ELSE) { cc_next(c); int je = cc_jmp(c, OP_JMP); cc_patch(c, jf); p_stmt(c); cc_patch(c, je); } else cc_patch(c, jf);
        return; }
    case K_WHILE: {
        if (c->nlp >= 8) { cc_err(c, "loops nested too deeply"); return; }
        int top = vm->ncode; cc_next(c); cc_expect(c, '(', "expected '(' after while"); p_expr(c); cc_expect(c, ')', "expected ')'");
        int jf = cc_jmp(c, OP_JZ); int li = c->nlp++; c->lp[li].nb = c->lp[li].nc = 0; c->lp[li].top = top;
        p_stmt(c); cc_op1(c, OP_JMP, top); cc_patch(c, jf); p_loop_end(c, top); return; }
    case K_FOR: {
        if (c->nlp >= 8) { cc_err(c, "loops nested too deeply"); return; }
        cc_next(c); cc_expect(c, '(', "expected '(' after for");
        if (c->t != ';') p_stmt(c); else cc_next(c);                       /* init (a statement incl. its ';') */
        int top = vm->ncode; if (c->t != ';') p_expr(c); else cc_op1(c, OP_PUSH, 1); cc_expect(c, ';', "expected ';'");
        int jf = cc_jmp(c, OP_JZ), jb = cc_jmp(c, OP_JMP), step = vm->ncode;
        if (c->t != ')') { /* step: assignment without ';' */
            if (c->t != T_ID) { cc_err(c, "expected an assignment in for-step"); return; }
            char name[RPP_NAME]; rp_cpy(name, c->ts, RPP_NAME); cc_next(c);
            if (c->t == '=') { cc_next(c); p_expr(c); p_store(c, name); }
            else if (c->t == T_PE || c->t == T_ME) { int t = c->t; cc_next(c); p_load_name(c, name); p_expr(c); cc_emit(c, t == T_PE ? OP_ADD : OP_SUB); p_store(c, name); }
            else cc_err(c, "expected '=' in for-step");
        }
        cc_expect(c, ')', "expected ')'"); cc_op1(c, OP_JMP, top); cc_patch(c, jb);
        int li = c->nlp++; c->lp[li].nb = c->lp[li].nc = 0; c->lp[li].top = step;
        p_stmt(c); cc_op1(c, OP_JMP, step); cc_patch(c, jf); p_loop_end(c, step); return; }
    case K_RET: {
        if (!c->infn) { cc_err(c, "return outside a function"); return; }
        cc_next(c); if (c->t != ';') p_expr(c); else cc_op1(c, OP_PUSH, 0); cc_emit(c, OP_RET); cc_expect(c, ';', "expected ';'"); return; }
    case K_BRK: case K_CONT: {
        int isb = c->t == K_BRK; cc_next(c); if (!c->nlp) { cc_err(c, "break/continue outside a loop"); return; }
        int li = c->nlp - 1; int at = cc_jmp(c, OP_JMP);
        if (isb) { if (c->lp[li].nb >= 24) { cc_err(c, "too many breaks"); return; } c->lp[li].brk[c->lp[li].nb++] = at; }
        else { if (c->lp[li].nc >= 24) { cc_err(c, "too many continues"); return; } c->lp[li].cont[c->lp[li].nc++] = at; }
        cc_expect(c, ';', "expected ';'"); return; }
    case T_ID: {
        char name[RPP_NAME]; rp_cpy(name, c->ts, RPP_NAME);
        int sp = c->p, sl = c->line, st = c->t, stv = c->tv, sn = vm->ncode; char sts[256]; rp_cpy(sts, c->ts, 256);
        cc_next(c);
        if (c->t == '=') { cc_next(c); p_expr(c); p_store(c, name); cc_expect(c, ';', "expected ';'"); return; }
        if (c->t == T_PE || c->t == T_ME) { int t = c->t; cc_next(c); p_load_name(c, name); p_expr(c); cc_emit(c, t == T_PE ? OP_ADD : OP_SUB); p_store(c, name); cc_expect(c, ';', "expected ';'"); return; }
        if (c->t == '[') {
            int g = cc_global(c, name);
            if (g >= 0 && c->gz[g] && cc_local(c, name) < 0) {
                cc_next(c); p_expr(c); cc_expect(c, ']', "expected ']'");
                if (c->t == '=') { cc_next(c); p_expr(c); cc_emit(c, OP_STOREA); cc_emit(c, c->gb[g]); cc_emit(c, c->gz[g]); cc_expect(c, ';', "expected ';'"); return; }
            }
        }
        /* not an assignment: rewind and parse as an expression statement */
        c->p = sp; c->line = sl; c->t = st; c->tv = stv; rp_cpy(c->ts, sts, 256); vm->ncode = sn;
        p_expr(c); cc_emit(c, OP_POP); cc_expect(c, ';', "expected ';'"); return; }
    default:
        p_expr(c); cc_emit(c, OP_POP); cc_expect(c, ';', "expected ';'"); return;
    }
}
/* compiles source into vm. returns 0 on success, otherwise a message is left in vm->errmsg */
static int rpp_compile(Vm *vm, const char *src, Cc *c) {
    rp_zero(vm, (int)sizeof(Vm)); rp_zero(c, (int)sizeof(Cc));
    c->vm = vm; c->s = src; c->line = 1; c->emsg = vm->errmsg; vm->nfn = 1; vm->fn[0].defined = 1; vm->fn[0].addr = 0; vm->rng = 2463534242u;
    cc_next(c);
    while (!c->err && c->t != T_EOF) p_stmt(c);
    cc_op1(c, OP_PUSH, 0); cc_emit(c, OP_RET);
    if (!c->err) for (int i = 1; i < vm->nfn; i++) if (!vm->fn[i].defined) { rp_cpy(c->emsg, "unknown name: ", 96); int l = rp_len(c->emsg); rp_cpy(c->emsg + l, vm->fn[i].name, 96 - l); c->err = 1; break; }
    if (c->err) { vm->err = 1; return 1; }
    return 0;
}

/* ---------------------------------------------------------------- RXE container: "RXE1" ncode nk ng nfn {addr np nl}* code* consts */
static void rp_w32(u8 *d, int *o, u32 v) { d[(*o)++] = (u8)v; d[(*o)++] = (u8)(v >> 8); d[(*o)++] = (u8)(v >> 16); d[(*o)++] = (u8)(v >> 24); }
static u32 rp_r32(const u8 *d, int *o) { u32 v = d[*o] | (d[*o + 1] << 8) | (d[*o + 2] << 16) | ((u32)d[*o + 3] << 24); *o += 4; return v; }
static int rpp_rxe_size(Vm *vm) { return 4 + 16 + vm->nfn * 12 + vm->ncode * 4 + vm->nk; }
static int rpp_save_rxe(Vm *vm, u8 *d) {
    int o = 0; d[o++] = 'R'; d[o++] = 'X'; d[o++] = 'E'; d[o++] = '1';
    rp_w32(d, &o, (u32)vm->ncode); rp_w32(d, &o, (u32)vm->nk); rp_w32(d, &o, (u32)vm->ng); rp_w32(d, &o, (u32)vm->nfn);
    for (int i = 0; i < vm->nfn; i++) { rp_w32(d, &o, (u32)vm->fn[i].addr); rp_w32(d, &o, (u32)vm->fn[i].np); rp_w32(d, &o, (u32)vm->fn[i].nl); }
    for (int i = 0; i < vm->ncode; i++) rp_w32(d, &o, (u32)vm->code[i]);
    for (int i = 0; i < vm->nk; i++) d[o++] = vm->kst[i];
    return o;
}
/* returns 0 on success */
static int rpp_load_rxe(Vm *vm, const u8 *d, int len) {
    rp_zero(vm, (int)sizeof(Vm)); vm->rng = 2463534242u;
    if (len < 20 || d[0] != 'R' || d[1] != 'X' || d[2] != 'E' || d[3] != '1') { rp_fail(vm, "not a valid .rxe file"); return 1; }
    int o = 4; int ncode = (int)rp_r32(d, &o), nk = (int)rp_r32(d, &o), ng = (int)rp_r32(d, &o), nfn = (int)rp_r32(d, &o);
    if (ncode < 0 || ncode > RPP_MAXCODE || nk < 0 || nk > RPP_MAXCONST || ng < 0 || ng > RPP_MAXG || nfn < 1 || nfn > RPP_MAXFN || 20 + nfn * 12 + ncode * 4 + nk > len) { rp_fail(vm, "corrupt .rxe file"); return 1; }
    vm->ncode = ncode; vm->nk = nk; vm->ng = ng; vm->nfn = nfn;
    for (int i = 0; i < nfn; i++) { vm->fn[i].addr = (int)rp_r32(d, &o); vm->fn[i].np = (int)rp_r32(d, &o); vm->fn[i].nl = (int)rp_r32(d, &o); vm->fn[i].defined = 1; if (vm->fn[i].np < 0 || vm->fn[i].np > 8 || vm->fn[i].nl < vm->fn[i].np || vm->fn[i].nl > 64) { rp_fail(vm, "corrupt .rxe file"); return 1; } }
    for (int i = 0; i < ncode; i++) vm->code[i] = (int)rp_r32(d, &o);
    for (int i = 0; i < nk; i++) vm->kst[i] = d[o++];
    return 0;
}
#endif
