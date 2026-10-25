/* ---------- icons ---------- */
#include "icons_data.h" /* generated at build time from inter32/icons (see tools/ico2c.py) */
/* Draws icon k (index into ICON_NAMES) at size sz: box-filtered downscale, alpha blended. */
static void icon(int k, int x, int y, int sz) {
    if (k < 0 || k >= ICO_COUNT || sz <= 0) return;
    const u32 *s = ICO_DATA[k];
    for (int dy = 0; dy < sz; dy++) {
        int y0 = dy * 32 / sz, y1 = (dy + 1) * 32 / sz; if (y1 <= y0) y1 = y0 + 1; if (y1 > 32) y1 = 32;
        for (int dx = 0; dx < sz; dx++) {
            int x0 = dx * 32 / sz, x1 = (dx + 1) * 32 / sz; if (x1 <= x0) x1 = x0 + 1; if (x1 > 32) x1 = 32;
            u32 sa = 0, sr = 0, sg = 0, sb = 0, n = 0;
            for (int j = y0; j < y1; j++) for (int i = x0; i < x1; i++) {
                u32 c = s[j * 32 + i], a = c >> 24; n++; sa += a;
                sr += ((c >> 16) & 255) * a; sg += ((c >> 8) & 255) * a; sb += (c & 255) * a;
            }
            if (!sa) continue;
            u32 col = ((sr / sa) << 16) | ((sg / sa) << 8) | (sb / sa);
            int X = x + dx, Y = y + dy;
            if ((unsigned)X >= (unsigned)SW || (unsigned)Y >= (unsigned)SH) continue;
            u32 A = sa / n; cv[Y * SW + X] = A >= 250 ? col : mix(cv[Y * SW + X], col, (int)A, 255);
        }
    }
}
/* extra icons: 10 = program (.rxe), 11 = script (.rpp) */
static int icon_win(int k) { return k < 9 ? k : 10; }
static int file_icon(int n) { if (n < 0) return 9; if (fsn[n].dir) return 2; if (fs_isext(n, "rxe")) return 10; if (fs_isext(n, "rpp")) return 11; if (fs_isext(n, "htm")) return 8; return 9; }
static const u8 SMI[9] = { 2, 8, 0, 3, 4, 1, 6, 7, 5 }; /* start-menu order */
static const u8 DESKI[6] = { 2, 8, 0, 3, 4, 6 };

