/* rppc - RedWill Script compiler for your PC.
 *   rppc prog.rpp [-o prog.rxe]    compile a script into a RedWill executable
 *   rppc --run prog.rpp            run a script headless on the PC (console programs only; GUI calls need RedWill)
 * Build:  cc -O2 -o rppc tools/rppc.c        (build.sh does this for you) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef unsigned char u8; typedef unsigned short u16; typedef unsigned int u32;
#include "../kernel/rpp/rpp.h"

static int host(Vm *vm, int id, int *a) {
    char b[12];
    switch (id) {
    case 100: { fputs(rp_tostr(vm, a[0], b), stdout); fputc('\n', stdout); return 0; }
    case 101: vm->busy = 2; return 0;
    case 102: return (int)(clock() * 1000 / CLOCKS_PER_SEC);
    case 103: return 0;
    case 105: return rp_cstr(vm, "12:00");
    case 106: return rp_cstr(vm, "user");
    case 190: { FILE *f = fopen(rp_tostr(vm, a[0], b), "rb"); if (!f) return rp_cstr(vm, ""); static char buf[65536]; int n = (int)fread(buf, 1, sizeof buf - 1, f); fclose(f); return rp_newstr(vm, buf, n); }
    case 191: { FILE *f = fopen(rp_tostr(vm, a[0], b), "wb"); if (!f) return 0; char b2[12]; const char *s = rp_tostr(vm, a[1], b2); fwrite(s, 1, strlen(s), f); fclose(f); return 1; }
    case 192: { FILE *f = fopen(rp_tostr(vm, a[0], b), "rb"); if (f) fclose(f); return f != 0; }
    default: rp_fail(vm, "this call needs the RedWill GUI (run the program inside RedWill)"); return 0;
    }
}
int main(int argc, char **argv) {
    const char *in = 0, *out = 0; int run = 0;
    for (int i = 1; i < argc; i++) { if (!strcmp(argv[i], "--run")) run = 1; else if (!strcmp(argv[i], "-o") && i + 1 < argc) out = argv[++i]; else in = argv[i]; }
    if (!in) { fprintf(stderr, "usage: rppc [--run] prog.rpp [-o prog.rxe]\n"); return 2; }
    FILE *f = fopen(in, "rb"); if (!f) { perror(in); return 1; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    char *src = malloc(n + 1); if (fread(src, 1, n, f) != (size_t)n) return 1; src[n] = 0; fclose(f);
    Vm *vm = calloc(1, sizeof(Vm)); Cc *cc = calloc(1, sizeof(Cc));
    if (rpp_compile(vm, src, cc)) { fprintf(stderr, "%s:%s\n", in, strncmp(vm->errmsg, "line ", 5) ? vm->errmsg : vm->errmsg + 5); return 1; }
    if (run) { rpp_host = host; vm->rng = (u32)time(0) | 1; rp_call(vm, 0, 0, 0); if (vm->err) { fprintf(stderr, "runtime error: %s\n", vm->errmsg); return 1; } return 0; }
    char def[512]; if (!out) { strncpy(def, in, 500); char *dot = strrchr(def, '.'); if (dot) *dot = 0; strcat(def, ".rxe"); out = def; }
    u8 *buf = malloc(rpp_rxe_size(vm)); int sz = rpp_save_rxe(vm, buf);
    f = fopen(out, "wb"); if (!f) { perror(out); return 1; } fwrite(buf, 1, sz, f); fclose(f);
    printf("%s -> %s (%d bytes, %d functions)\n", in, out, sz, vm->nfn - 1);
    return 0;
}
