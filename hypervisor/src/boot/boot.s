; Multiboot2 header for GRUB (better 64-bit support)
section .multiboot_header
align 8

header_start:
    dd 0xe85250d6                ; magic
    dd 0                         ; architecture (i386)
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
    
    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
header_end:

section .boot
bits 32

extern cmain
global _start
_start:
    cli
    mov esp, 0x10d000
    call cmain
    hlt
    jmp $
