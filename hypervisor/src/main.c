#include "types.h"
#include "console.h"
#include "cpu.h"
#include "memory.h"

void main(uint32_t magic, uint32_t addr) {
    console_init();
    console_write_string("=== CONCORDIA Hypervisor ===\n");
    
    // Initialize CPU
    console_write_string("1. Initializing CPU...\n");
    cpu_init();
    
    // Initialize memory
    console_write_string("\n2. Initializing Memory...\n");
    memory_init();
    
    console_write_string("\nHypervisor ready.\n");
    
    while (1) {
        asm volatile("hlt");
    }
}
