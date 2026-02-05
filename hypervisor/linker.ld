ENTRY(_start)

SECTIONS {
    . = 0x00100000;
    
    .multiboot_header :
    {
        *(.multiboot_header)
    }
    
    .boot :
    {
        *(.boot)
    }
    
    .text : ALIGN(4K)
    {
        *(.text)
    }
    
    .rodata : ALIGN(4K)
    {
        *(.rodata)
    }
    
    .data : ALIGN(4K)
    {
        *(.data)
    }
    
    .bss : ALIGN(4K)
    {
        *(.bss)
        *(COMMON)
    }
}
