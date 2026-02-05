; Multiboot2 header for bootloader
section .multiboot_header
align 8
header_start:
    dd 0xe85250d6                ; magic
    dd 0                         ; architecture (i386)
    dd header_end - header_start ; header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))
    
    align 8
    dw 0
    dw 0
    dd 8
header_end:

section .boot
bits 32

extern main
global _start
_start:
    mov esp, stack_top
    
    ; Disable interrupts
    cli
    
    ; Call main kernel
    call main
    
    ; Halt
    hlt
    jmp $

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:
