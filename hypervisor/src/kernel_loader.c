#include "kernel_loader.h"
#include "console.h"
#include "system_manager.h"
#include "types.h"

static kernel_state_t kernel_state = {0};

// Stub kernel data (embedded in hypervisor for now)
// In real implementation, would load from disk/ISO
extern uint8_t linux_stub_start[];
extern uint8_t linux_stub_end[];
extern uint8_t windows_stub_start[];
extern uint8_t windows_stub_end[];

void kernel_loader_init(void) {
    console_write_string("Initializing Kernel Loader...\n");
    
    // Initialize Linux kernel info
    kernel_state.linux.load_address = LINUX_KERNEL_ENTRY;
    kernel_state.linux.entry_point = LINUX_KERNEL_ENTRY;
    kernel_state.linux.size = 0;
    kernel_state.linux.loaded = 0;
    const char *linux_name = "Linux Stub";
    for (int i = 0; i < 32 && linux_name[i]; i++) {
        kernel_state.linux.name[i] = linux_name[i];
    }
    
    // Initialize Windows kernel info
    kernel_state.windows.load_address = WINDOWS_KERNEL_ENTRY;
    kernel_state.windows.entry_point = WINDOWS_KERNEL_ENTRY;
    kernel_state.windows.size = 0;
    kernel_state.windows.loaded = 0;
    const char *windows_name = "Windows Stub";
    for (int i = 0; i < 32 && windows_name[i]; i++) {
        kernel_state.windows.name[i] = windows_name[i];
    }
    
    console_write_string("Kernel Loader initialized\n");
}

void kernel_load_linux_stub(void) {
    console_write_string("Loading Linux stub kernel...\n");
    
    // For testing purposes, we'll use a simple marker
    // In real implementation:
    // 1. Read kernel from ISO
    // 2. Validate ELF/binary format
    // 3. Copy to memory region
    // 4. Relocate if needed
    
    kernel_state.linux.loaded = 1;
    
    console_write_string("  Linux kernel loaded at 0x");
    console_write_hex(LINUX_KERNEL_ENTRY);
    console_write_string("\n");
    console_write_string("  Entry point: 0x");
    console_write_hex(LINUX_KERNEL_ENTRY);
    console_write_string("\n");
}

void kernel_load_windows_stub(void) {
    console_write_string("Loading Windows stub kernel...\n");
    
    // For testing purposes, we'll use a simple marker
    // In real implementation:
    // 1. Read kernel from ISO
    // 2. Validate PE/binary format
    // 3. Copy to memory region
    // 4. Relocate if needed
    
    kernel_state.windows.loaded = 1;
    
    console_write_string("  Windows kernel loaded at 0x");
    console_write_hex(WINDOWS_KERNEL_ENTRY);
    console_write_string("\n");
    console_write_string("  Entry point: 0x");
    console_write_hex(WINDOWS_KERNEL_ENTRY);
    console_write_string("\n");
}

void kernel_boot_linux(void) {
    console_write_string("Booting Linux kernel...\n");
    
    if (!kernel_state.linux.loaded) {
        console_write_string("ERROR: Linux kernel not loaded\n");
        return;
    }
    
    // In real implementation:
    // 1. Setup CPU state (registers, stack)
    // 2. Setup page tables for kernel memory
    // 3. Load IDT/GDT if needed
    // 4. Jump to kernel entry point
    
    console_write_string("Jumping to kernel entry point...\n");
    
    // This would be: jmp LINUX_KERNEL_ENTRY
    // But we can't actually do that without real kernel code
    // In testing, we just confirm the setup would work
}

void kernel_boot_windows(void) {
    console_write_string("Booting Windows kernel...\n");
    
    if (!kernel_state.windows.loaded) {
        console_write_string("ERROR: Windows kernel not loaded\n");
        return;
    }
    
    // In real implementation:
    // 1. Setup CPU state (registers, stack)
    // 2. Setup page tables for kernel memory
    // 3. Load IDT/GDT if needed
    // 4. Jump to kernel entry point
    
    console_write_string("Jumping to kernel entry point...\n");
    
    // This would be: jmp WINDOWS_KERNEL_ENTRY
    // But we can't actually do that without real kernel code
    // In testing, we just confirm the setup would work
}

uint8_t kernel_is_loaded(uint8_t cell_id) {
    if (cell_id == 0) {
        return kernel_state.linux.loaded;
    } else if (cell_id == 1) {
        return kernel_state.windows.loaded;
    }
    return 0;
}

void kernel_loader_print_status(void) {
    console_write_string("Kernel Loader Status:\n");
    
    console_write_string("  Linux Kernel:\n");
    console_write_string("    Name: ");
    console_write_string((const char *)kernel_state.linux.name);
    console_write_string("\n");
    console_write_string("    Address: 0x");
    console_write_hex(kernel_state.linux.load_address);
    console_write_string("\n");
    console_write_string("    Loaded: ");
    console_write_string(kernel_state.linux.loaded ? "Yes\n" : "No\n");
    
    console_write_string("  Windows Kernel:\n");
    console_write_string("    Name: ");
    console_write_string((const char *)kernel_state.windows.name);
    console_write_string("\n");
    console_write_string("    Address: 0x");
    console_write_hex(kernel_state.windows.load_address);
    console_write_string("\n");
    console_write_string("    Loaded: ");
    console_write_string(kernel_state.windows.loaded ? "Yes\n" : "No\n");
}
