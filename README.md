# RedWill OS

Bootable i386 hobby OS. Build with `./build.sh [pad_mb]` -> `build/redwill.iso`.

- `boot/`     BIOS boot stub (El Torito), hybrid MBR, linker script
- `kernel/`   kernel: `arch/` (port I/O), `gfx/` (drawing, themes), `drivers/` (timer/RTC, sound, input), `fs/` (RAM filesystem), `ui/` (windows, screens, desktop), `power.h`, `main.c`
- `userland/` apps: terminal, calculator, snake, browser
- `tools/`    `mkiso.py` ISO9660 writer
- `inter32/icons/` icônes 32x32 `.ico` (style Windows NT), générées par `python3 tools/gen_icons.py`
