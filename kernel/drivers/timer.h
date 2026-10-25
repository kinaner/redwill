/* ---------- time ---------- */
static u16 last_pit; static u32 ms_acc, frac_;
static void pit_init(void) { outb(0x43, 0x34); outb(0x40, 0); outb(0x40, 0); }
static u16 pit_read(void) { outb(0x43, 0); u8 lo = inb(0x40); u8 hi = inb(0x40); return (u16)(lo | (hi << 8)); }
static u32 now_ms(void) { u16 c = pit_read(); u16 d = (u16)(last_pit - c); last_pit = c; frac_ += d; ms_acc += frac_ / 1193; frac_ %= 1193; return ms_acc; }
static u8 cmos(u8 r) { outb(0x70, r); return inb(0x71); }
static int rh = 12, rm = 0, rd = 1, rmo = 1, ry = 2026;
static void rtc_read(void) {
#ifndef HOST
    for (int t = 0; t < 2000 && (cmos(0x0A) & 0x80); t++) ;
    u8 s = cmos(0x0B), h = cmos(4), m = cmos(2), d = cmos(7), mo = cmos(8), y = cmos(9);
    int pm = h & 0x80; h &= 0x7F;
    #define BCD(v) ((s & 4) ? (v) : (((v) & 15) + ((v) >> 4) * 10))
    int H = BCD(h);
    if (!(s & 2)) { if (pm && H < 12) H += 12; if (!pm && H == 12) H = 0; }
    rh = H; rm = BCD(m); rd = BCD(d); rmo = BCD(mo); ry = 2000 + BCD(y);
    if (rmo < 1 || rmo > 12) rmo = 1;
    if (rd < 1 || rd > 31) rd = 1;
#endif
}
static int dim(int y, int m) { static const int d[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; return (m == 2 && ((y % 4 == 0 && y % 100) || y % 400 == 0)) ? 29 : d[m - 1]; }
static int dow(int y, int m, int d) { static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 }; if (m < 3) y--; return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7; }
static const char *MON[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

