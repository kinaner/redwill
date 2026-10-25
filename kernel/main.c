/* RedWill OS kernel - i386 protected mode, VBE framebuffer, polled PS/2, PC-speaker sound.
 * Single translation unit: each module below is a header included in dependency order. */
#include "arch/x86.h"
#include "rpp/rpp.h"
#include "gfx/gfx.h"
#include "drivers/timer.h"
#include "drivers/sound.h"
#include "fs/ramfs.h"
#include "ui/state.h"
#include "ui/windows.h"
#include "ui/icons.h"
#include "ui/dialog.h"
#include "../userland/terminal.h"
#include "../userland/calculator.h"
#include "../userland/snake.h"
#include "../userland/browser.h"
#include "rpp/rpp_api.h"
#include "ui/screens.h"
#include "ui/desktop.h"
#include "power.h"
#include "drivers/input.h"
#include "rootfs_data.h"   /* generated at build time from rootfs/ (see tools/mkrootfs.py) */

/* ---------- init / entry ---------- */
static void os_init(void) {
    fs_init();
    for (int i = 0; i < ROOTFS_COUNT; i++) fs_put_path(ROOTFS[i].path, ROOTFS_DATA + ROOTFS[i].off, ROOTFS[i].len);
    ecur = fs_docs; fs_cwd = fs_docs;
    for (int i = 0; i < PW * PH; i++) PCV[i] = 0xFFFFFF;
    fld_set(&fcn, "REDWILL-PC"); fld_set(&fun, "RedWill User"); fld_set(&fpw, "");
    nav("redwill://home", 1); snake_reset(); tprint("RedWill terminal. Type help."); tprint("Programs: run NAME   (examples in C:\\RedWill\\red-sdk)"); fld_set(&tin, "");
    W[2].open = 0;
}
#ifndef HOST
void kernel_main(void) {
    u8 *mi = (u8 *)0x600;
    PITCH = *(u16 *)(mi + 0x10); SW = *(u16 *)(mi + 0x12); SH = *(u16 *)(mi + 0x14); FB = (u32 *)(*(u32 *)(mi + 0x28));
    BACK = (u32 *)0x1000000; WALL = (u32 *)0x1400000; PCV = (u32 *)0x1800000; FONT = (const u8 *)0x90000;
    rpp_set_pools((void *)0x1A00000, (void *)0x1B00000, 0x20000, (void *)0x1C00000);
    pit_init(); now_ms(); mx = SW / 2; my = SH / 2;
    splash_frame(0); flip(); mouse_init(); os_init(); make_wall();
    u32 t0 = now_ms(), last = 0xFFFFFFFF;
    for (;;) { u32 t = now_ms() - t0; if (t > 4500) break; if (t / 70 != last) { last = t / 70; splash_frame(t); flip(); } }
    ph = 1; music(J_SETUP, 17, 1); rtc_read(); dirty = 1;
    u32 lastsec = 0, lastsnake = 0;
    for (;;) {
        u32 t = now_ms();
        if (ph == 3 && W[6].open && !W[6].min && t - lastsnake > (u32)(150 - (sscore > 10 ? 10 : sscore) * 6)) { lastsnake = t; snake_tick(); dirty = 1; }
        if (ph == 3 && !D.active) rpp_tick(t);
        if (t / 1000 != lastsec) { lastsec = t / 1000; if (!(lastsec % 5)) rtc_read(); dirty = 1; }
        os_pump();
    }
}
#endif
