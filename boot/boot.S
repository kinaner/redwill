# RedWill boot stub: loaded at 0x7C00 by BIOS (El Torito no-emulation), whole image already in RAM.
.section .boot, "awx"
.code16
.global _start
_start:
    cli
    ljmp $0, $realstart
realstart:
    xorw %ax,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    movw $0x7C00,%sp
    sti
    cld
    # copy the BIOS 8x16 ROM font to 0x90000 (above the stack, clear of the kernel image)
    movw $0x1130,%ax
    movw $0x0600,%bx
    int $0x10
    movw %es,%ax
    movw %ax,%ds
    movw %bp,%si
    movw $0x9000,%ax
    movw %ax,%es
    xorw %di,%di
    movw $2048,%cx
    rep movsw
    xorw %ax,%ax
    movw %ax,%ds
    movw %ax,%es
    # ask BIOS for VBE info (mode list)
    movl $0x32454256,0x0800          # "VBE2"
    movw $0x4F00,%ax
    movw $0x0800,%di
    int $0x10
    cmpw $0x004F,%ax
    jne vbefail
    movw 0x080E,%ax
    movw %ax,list_off
    movw 0x0810,%ax
    movw %ax,list_seg
    movw $1024,want_w
    movw $768,want_h
    call scan
    jnc found
    movw $800,want_w
    movw $600,want_h
    call scan
    jnc found
    movw $640,want_w
    movw $480,want_h
    call scan
    jnc found
vbefail:
    movw $msg_fail,%si
1:  lodsb
    testb %al,%al
    jz 2f
    movb $0x0E,%ah
    movw $7,%bx
    int $0x10
    jmp 1b
2:  cli
    hlt
    jmp 2b

# scan VBE mode list for want_w x want_h x 32bpp with linear framebuffer; sets the mode. CF=0 on success
scan:
    movw list_off,%si
3:  movw list_seg,%ax
    movw %ax,%fs
    movw %fs:(%si),%cx
    addw $2,%si
    cmpw $0xFFFF,%cx
    je 9f
    movw %si,save_si
    movw %cx,cur_mode
    movw $0x4F01,%ax
    movw $0x0600,%di
    int $0x10
    movw save_si,%si
    cmpw $0x004F,%ax
    jne 3b
    movw 0x0600,%ax
    andw $0x0081,%ax
    cmpw $0x0081,%ax
    jne 3b
    movw 0x0612,%ax
    cmpw want_w,%ax
    jne 3b
    movw 0x0614,%ax
    cmpw want_h,%ax
    jne 3b
    cmpb $32,0x0619
    jne 3b
    cmpb $6,0x061B
    jne 3b
    movw cur_mode,%bx
    orw $0x4000,%bx
    movw $0x4F02,%ax
    int $0x10
    movw save_si,%si
    cmpw $0x004F,%ax
    jne 3b
    clc
    ret
9:  stc
    ret

found:
    cli
    inb $0x92,%al
    orb $2,%al
    andb $0xFE,%al
    outb %al,$0x92
    lgdtl gdt_desc
    movl %cr0,%eax
    orl $1,%eax
    movl %eax,%cr0
    ljmpl $0x08,$pm_entry

list_off: .word 0
list_seg: .word 0
want_w:   .word 0
want_h:   .word 0
cur_mode: .word 0
save_si:  .word 0
msg_fail: .asciz "RedWill: no usable VBE 32-bit video mode found.\r\n"

.align 8
gdt:
    .quad 0
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
gdt_end:
gdt_desc:
    .word gdt_end - gdt - 1
    .long gdt

.code32
pm_entry:
    movw $0x10,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%fs
    movw %ax,%gs
    movw %ax,%ss
    movl $0x90000,%esp
    movl $__bss_start,%edi
    movl $__bss_end,%ecx
    subl %edi,%ecx
    xorl %eax,%eax
    cld
    rep stosb
    call kernel_main
4:  cli
    hlt
    jmp 4b
