#!/bin/sh
# usage: ./build.sh [pad_mb]   -> build/redwill.iso  (+ build/rppc, the script compiler for your PC)
set -e
cd "$(dirname "$0")"
mkdir -p build
CF="-m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -mgeneral-regs-only -O2 -fno-builtin -fno-asynchronous-unwind-tables -fcf-protection=none -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -Wall"
# 1. host tools: rppc compiles .rpp -> .rxe on your PC
cc -O2 -w -o build/rppc tools/rppc.c
# 2. generated data: icons (inter32/icons/*.ico), root file system (rootfs/ + compiled examples)
python3 tools/ico2c.py inter32/icons build/icons_data.h
python3 tools/mkrootfs.py rootfs build/rppc build/rootfs_data.h
# 3. kernel + boot code
gcc $CF -c boot/boot.S -o build/boot.o
gcc $CF -Ibuild -c kernel/main.c -o build/kernel.o
ld -m elf_i386 -T boot/link.ld -nostdlib --build-id=none -o build/kernel.elf build/boot.o build/kernel.o
objcopy -O binary build/kernel.elf build/boot.bin
gcc -m32 -c boot/mbr.S -o build/mbr.o
ld -m elf_i386 -Ttext 0x600 -nostdlib --build-id=none -o build/mbr.elf build/mbr.o 2>/dev/null
objcopy -O binary build/mbr.elf build/mbr.bin
if [ -n "$1" ]; then python3 tools/mkiso.py build/boot.bin build/redwill.iso build/mbr.bin --pad-mb "$1"; else python3 tools/mkiso.py build/boot.bin build/redwill.iso build/mbr.bin; fi
