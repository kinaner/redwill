/* RedWill OS kernel - i386 protected mode, VBE framebuffer, polled PS/2, PC-speaker sound.
 * Single translation unit: each module below is a header included in dependency order. */
#include "arch/x86.h"
#include "gfx/gfx.h"
#include "drivers/timer.h"
#include "drivers/sound.h"
#include "fs/ramfs.h"
#include "ui/state.h"
#include "ui/windows.h"
#include "../userland/terminal.h"
#include "../userland/calculator.h"
#include "../userland/snake.h"
#include "../userland/browser.h"
#include "ui/icons.h"
#include "ui/screens.h"
#include "ui/desktop.h"
#include "power.h"
#include "drivers/input.h"

/* ---------- init / entry ---------- */
static void os_init(void) {
    fs_put("README.TXT", "Welcome to RedWill!\nThis is a hobby OS written from scratch.\nOpen the Terminal and type help.");
    fs_put("HELLO.TXT", "Hello, world!");
    fs_put("PAGE.HTM", "<h1>My page</h1><p>This file is a <b>web page</b>. Open it in the browser with file://PAGE.HTM</p><p><a href=redwill://home>Start page</a></p>");
    fs_put("TODO.TXT", "- networking\n- disk driver\n- more apps");
    for (int i = 0; i < PW * PH; i++) PCV[i] = 0xFFFFFF;
    fld_set(&fcn, "REDWILL-PC"); fld_set(&fun, "RedWill User"); fld_set(&fpw, "");
    nav("redwill://home", 1); snake_reset(); tprint("RedWill terminal. Type help."); fld_set(&tin, "");
    W[2].open = 0;
}
#ifndef HOST
void kernel_main(void) {
    u8 *mi = (u8 *)0x600;
    PITCH = *(u16 *)(mi + 0x10); SW = *(u16 *)(mi + 0x12); SH = *(u16 *)(mi + 0x14); FB = (u32 *)(*(u32 *)(mi + 0x28));
    BACK = (u32 *)0x1000000; WALL = (u32 *)0x1400000; PCV = (u32 *)0x1800000; FONT = (const u8 *)0x60000;
    pit_init(); now_ms(); mx = SW / 2; my = SH / 2;
    splash_frame(0); flip(); mouse_init(); os_init(); make_wall();
    u32 t0 = now_ms(), last = 0xFFFFFFFF;
    for (;;) { u32 t = now_ms() - t0; if (t > 4500) break; if (t / 70 != last) { last = t / 70; splash_frame(t); flip(); } }
    ph = 1; music(J_SETUP, 17, 1); rtc_read(); dirty = 1;
    u32 lastsec = 0, lastblink = 0, lastsnake = 0;
    for (;;) {
        u32 t = now_ms(); poll_input(); music_tick();
        if (ph == 3 && W[6].open && !W[6].min && t - lastsnake > (u32)(150 - (sscore > 10 ? 10 : sscore) * 6)) { lastsnake = t; snake_tick(); dirty = 1; }
        if (t / 1000 != lastsec) { lastsec = t / 1000; if (!(lastsec % 5)) rtc_read(); dirty = 1; }
        if (t / 500 != lastblink) { lastblink = t / 500; dirty = 1; }
        if (dirty) { dirty = 0; render(); flip(); }
    }
}
#endif
