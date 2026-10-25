/* ---------- sound: original jingles on the PC speaker, sequenced without blocking ---------- */
typedef struct { u16 f, ms; } Nt;
static const Nt *mseq; static int mlen, mpos, mloop; static u32 mnext;
static void spk(u32 f) {
    if (!snd || !f) { outb(0x61, (u8)(inb(0x61) & ~3)); return; }
    u32 d = 1193180 / f; outb(0x43, 0xB6); outb(0x42, (u8)(d & 255)); outb(0x42, (u8)(d >> 8)); outb(0x61, (u8)(inb(0x61) | 3));
}
static void music(const Nt *s, int n, int loop) { mseq = s; mlen = n; mpos = 0; mloop = loop; mnext = 0; }
static void music_stop(void) { mseq = 0; spk(0); }
static void music_tick(void) {
    if (!mseq || ms_acc < mnext) return;
    if (mpos >= mlen) { if (mloop) mpos = 0; else { music_stop(); return; } }
    spk(mseq[mpos].f); mnext = ms_acc + mseq[mpos].ms; mpos++;
}
static const Nt J_START[] = { { 523, 110 }, { 659, 110 }, { 784, 110 }, { 1047, 240 }, { 0, 40 }, { 988, 90 }, { 1047, 300 } };
static const Nt J_SETUP[] = { { 440, 200 }, { 523, 200 }, { 659, 200 }, { 523, 200 }, { 392, 200 }, { 494, 200 }, { 587, 200 }, { 494, 200 },
                              { 349, 200 }, { 440, 200 }, { 523, 200 }, { 440, 200 }, { 392, 200 }, { 494, 200 }, { 587, 200 }, { 784, 300 }, { 0, 200 } };
static const Nt J_OFF[] = { { 784, 150 }, { 659, 150 }, { 523, 150 }, { 392, 350 } };
static const Nt J_ERR[] = { { 220, 160 }, { 0, 40 }, { 196, 220 } };
static const Nt J_OK[] = { { 880, 70 }, { 1175, 110 } };
static void beep(u32 f, u32 ms) { if (!snd) return; spk(f); u32 t = now_ms(); while (now_ms() - t < ms) ; spk(0); }

