# RedWill hybrid boot sector: lets the same ISO boot from a USB stick / disk via BIOS (INT 13h extensions).
# Reads the El Torito boot image (placed at a fixed LBA inside the ISO) to 0x7C00 and jumps to it. Never writes to disk.
.code16
.global _start
_start:
    cli
    xorw %ax,%ax
    movw %ax,%ds
    movw %ax,%es
    movw %ax,%ss
    movw $0x7C00,%sp
    sti
    cld
    movw $0x7C00,%si
    movw $0x0600,%di
    movw $256,%cx
    rep movsw
    ljmp $0,$go
go:
    movb %dl,drv
    movw cnt,%bx
    movw $0x07C0,seg_
again:
    testw %bx,%bx
    jz done
    movw $16,%cx
    cmpw %cx,%bx
    jae 1f
    movw %bx,%cx
1:  movw %cx,dap_cnt
    movw seg_,%ax
    movw %ax,dap_seg
    movw $dap,%si
    movb drv,%dl
    movb $0x42,%ah
    int $0x13
    jc fail
    movzwl dap_cnt,%eax
    addl %eax,dap_lba
    movw dap_cnt,%cx
    subw %cx,%bx
    shlw $5,%cx
    addw %cx,seg_
    jmp again
done:
    movb drv,%dl
    ljmp $0,$0x7C00
fail:
    movw $msg,%si
2:  lodsb
    testb %al,%al
    jz 3f
    movb $0x0E,%ah
    movw $7,%bx
    int $0x10
    jmp 2b
3:  cli
    hlt
    jmp 3b
msg: .asciz "RedWill: disk read error\r\n"
drv: .byte 0
seg_: .word 0
cnt: .word 0xC0DE
dap: .byte 16,0
dap_cnt: .word 0
dap_off: .word 0
dap_seg: .word 0
dap_lba: .long 0xCAFEF00D
         .long 0
