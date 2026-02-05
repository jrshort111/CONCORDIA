#include "types.h"
#include "console.h"
#include "cpu.h"
#include "memory.h"
#include "iommu.h"
#include "system_manager.h"
#include "input_manager.h"
#include "monitor.h"
#include "dashboard.h"
#include "kernel_loader.h"

void cmain(uint32_t magic, uint32_t addr) {
    console_init();
    console_write_string("=== CONCORDIA Hypervisor ===\n\n");
    
    // Initialize CPU
    console_write_string("1. Initializing CPU...\n");
    cpu_init();
    
    // Initialize memory
    console_write_string("\n2. Initializing Memory...\n");
    memory_init();
    
    // Initialize IOMMU
    console_write_string("\n3. Initializing IOMMU...\n");
    iommu_init();
    
    // Initialize System Manager
    console_write_string("\n4. Initializing System Manager...\n");
    system_manager_init();
    
    // Initialize Input Manager
    console_write_string("\n5. Initializing Input Manager...\n");
    input_manager_init();
    input_manager_detect_usb_devices();
    
    // Initialize Monitor
    console_write_string("\n6. Initializing Monitor...\n");
    monitor_init();
    
    // Initialize Dashboard
    console_write_string("\n7. Initializing Dashboard...\n");
    dashboard_init();
    
    // Initialize Kernel Loader
    console_write_string("\n8. Initializing Kernel Loader...\n");
    kernel_loader_init();
    kernel_load_linux_stub();
    kernel_load_windows_stub();
    
    // Display initial dashboard
    console_write_string("\n");
    dashboard_refresh();
    
    console_write_string("\nHypervisor ready. Press Ctrl+Alt+O to switch between Linux and Windows.\n");
    
    while (1) {
        asm volatile("hlt");
    }
}
