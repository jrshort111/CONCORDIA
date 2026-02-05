; Linux stub kernel entry point
; This minimal kernel boots and prints confirmation, then loops

[BITS 64]
[ORG 0x0]

global linux_entry

linux_entry:
    ; Clear registers
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    
    ; Setup minimal stack (at high end of Linux memory)
    mov rsp, 0x0FFFF000
    
    ; Write message to serial console
    mov rsi, linux_banner
    call write_serial_string
    
    ; Loop forever (waiting for hibernation)
    jmp linux_loop

linux_loop:
    ; In real implementation, this would:
    ; - Handle system calls
    ; - Respond to hibernation signals
    ; - Return to hibernation state when signaled
    
    hlt
    jmp linux_loop

write_serial_string:
    ; rsi = string pointer
    mov rdx, 0x3F8  ; Serial port
.loop:
    lodsb           ; Load byte from [rsi] into al
    test al, al
    jz .done
    
    ; Wait for serial ready
    mov rcx, 0
.wait:
    mov dx, 0x3FD   ; Serial status port
    in al, dx
    test al, 0x20   ; Check transmit buffer empty
    jnz .send
    dec ecx
    jnz .wait
    
.send:
    mov dx, 0x3F8
    lodsb
    out dx, al
    jmp .loop
    
.done:
    ret

linux_banner:
    db "=== LINUX STUB KERNEL ===", 13, 10
    db "Linux cell booted successfully!", 13, 10
    db "Ready to handle switching via Ctrl+Alt+O", 13, 10
    db 0
