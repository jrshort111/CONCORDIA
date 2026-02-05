ENTRY(_start)

SECTIONS {
    . = 0x00100000;
    
    .boot BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot_header)
        *(.boot)
    }
    
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }
    
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }
    
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }
    
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(.bss)
        *(COMMON)
    }
}
