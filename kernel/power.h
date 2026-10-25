/* ---------- power ---------- */
static void restart_pc(void) { outb(0x64, 0xFE); for (;;) ; }
static void shutdown_pc(void) {
    cv = BACK; vgrad(0, 0, SW, SH, 0x5A7EDC, 0x2F55C0); rect(0, 0, SW, 90, T.pilld); rect(0, SH - 90, SW, 90, T.pilld);
    const char *a = "RedWill is shutting down..."; big((SW - slen(a) * 16) / 2, SH / 2 - 16, a, 0xFFFFFF, 2); flip();
    music(J_OFF, 4, 0); u32 t0 = now_ms(); while (now_ms() - t0 < 1800) music_tick(); music_stop();
    outw(0x604, 0x2000); outw(0xB004, 0x2000); outw(0x4004, 0x3400); outw(0x1004, 0x3C00); outw(0x1004, 0x2000);
    t0 = now_ms(); while (now_ms() - t0 < 1000) ;
    restart_pc();
}

