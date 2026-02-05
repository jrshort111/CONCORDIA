#include "memory.h"
#include "console.h"
#include "types.h"

// Simple allocator for now
static uint64_t heap_start = HYPERVISOR_MEMORY_START;
static uint64_t heap_current = HYPERVISOR_MEMORY_START;
static uint64_t heap_end = HYPERVISOR_MEMORY_END;

static memory_region_t regions[4] = {0};
static uint32_t region_count = 0;

static inline uint64_t read_cr3(void) {
    uint64_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static inline void write_cr3(uint64_t cr3) {
    asm volatile("mov %0, %%cr3" : : "r"(cr3));
}

static inline uint64_t read_cr4(void) {
    uint64_t cr4;
    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    return cr4;
}

static inline void write_cr4(uint64_t cr4) {
    asm volatile("mov %0, %%cr4" : : "r"(cr4));
}

// Allocate 2MB pages for now (simpler than 4KB)
static uint64_t *alloc_page_table(void) {
    if (heap_current + PAGE_SIZE_4K > heap_end) {
        return 0;
    }
    
    uint64_t *table = (uint64_t *)heap_current;
    heap_current += PAGE_SIZE_4K;
    
    // Clear the table
    for (int i = 0; i < 512; i++) {
        table[i] = 0;
    }
    
    return table;
}

void memory_setup_paging(void) {
    console_write_string("Setting up paging...\n");
    
    // Allocate top-level page table (PML4)
    uint64_t *pml4 = alloc_page_table();
    if (!pml4) {
        console_write_string("ERROR: Failed to allocate PML4\n");
        return;
    }
    
    // Map identity for first 32GB with 2MB pages
    // Each PML4 entry points to a PDPT (512 entries)
    // Each PDPT entry points to a PD (512 entries)
    // Each PD entry can be a 2MB page (PSE bit set)
    
    // For now, identity map using 2MB pages
    uint64_t *pdp = alloc_page_table();
    uint64_t *pd = alloc_page_table();
    
    if (!pdp || !pd) {
        console_write_string("ERROR: Failed to allocate page directory\n");
        return;
    }
    
    // PML4[0] -> PDP
    pml4[0] = (uint64_t)pdp | PAGE_PRESENT | PAGE_WRITE;
    
    // PDP[0] -> PD
    pdp[0] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITE;
    
    // Fill PD with 2MB pages (512 entries = 1GB)
    for (int i = 0; i < 512; i++) {
        uint64_t page_addr = (uint64_t)i * PAGE_SIZE_2M;
        pd[i] = page_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_PSE | PAGE_GLOBAL;
    }
    
    // Set CR3 to point to PML4
    write_cr3((uint64_t)pml4);
    
    // Enable CR4.PGE for global pages
    uint64_t cr4 = read_cr4();
    cr4 |= (1 << 7);  // CR4.PGE
    write_cr4(cr4);
    
    console_write_string("Paging initialized\n");
}

void memory_setup_cell_boundaries(void) {
    console_write_string("Setting up memory cell boundaries...\n");
    
    // Linux cell
    regions[0].base = LINUX_MEMORY_START;
    regions[0].limit = LINUX_MEMORY_SIZE;
    regions[0].allocated = 0;
    char *name1 = (char *)regions[0].name;
    const char *str1 = "Linux Cell";
    for (int i = 0; i < 32 && str1[i]; i++) name1[i] = str1[i];
    
    // Windows cell
    regions[1].base = WINDOWS_MEMORY_START;
    regions[1].limit = WINDOWS_MEMORY_SIZE;
    regions[1].allocated = 0;
    char *name2 = (char *)regions[1].name;
    const char *str2 = "Windows Cell";
    for (int i = 0; i < 32 && str2[i]; i++) name2[i] = str2[i];
    
    // Hypervisor region
    regions[2].base = HYPERVISOR_MEMORY_START;
    regions[2].limit = HYPERVISOR_MEMORY;
    regions[2].allocated = 0;
    char *name3 = (char *)regions[2].name;
    const char *str3 = "Hypervisor";
    for (int i = 0; i < 32 && str3[i]; i++) name3[i] = str3[i];
    
    region_count = 3;
    
    console_write_string("Memory regions configured:\n");
    console_write_string("  Linux:      0x");
    console_write_hex(LINUX_MEMORY_START);
    console_write_string(" - 0x");
    console_write_hex(LINUX_MEMORY_END);
    console_write_string(" (16 GB)\n");
    
    console_write_string("  Windows:    0x");
    console_write_hex(WINDOWS_MEMORY_START);
    console_write_string(" - 0x");
    console_write_hex(WINDOWS_MEMORY_END);
    console_write_string(" (16 GB)\n");
}

uint8_t memory_is_linux_address(uint64_t addr) {
    return (addr >= LINUX_MEMORY_START && addr < LINUX_MEMORY_END) ? 1 : 0;
}

uint8_t memory_is_windows_address(uint64_t addr) {
    return (addr >= WINDOWS_MEMORY_START && addr < WINDOWS_MEMORY_END) ? 1 : 0;
}

void *memory_alloc(size_t size) {
    // Simple bump allocator from hypervisor heap
    if (heap_current + size > heap_end) {
        return 0;
    }
    
    void *ptr = (void *)heap_current;
    heap_current += size;
    
    return ptr;
}

void memory_free(void *ptr) {
    // For now, no-op (would need a real allocator)
    (void)ptr;
}

void memory_print_layout(void) {
    console_write_string("Memory Layout:\n");
    console_write_string("  Total:      32 GB\n");
    console_write_string("  Linux Cell: 16 GB\n");
    console_write_string("  Windows Cell: 16 GB\n");
}

void memory_init(void) {
    console_write_string("Initializing memory subsystem...\n");
    
    // Setup memory regions
    memory_setup_cell_boundaries();
    
    // Setup paging (would be done, but needs working CR3 support)
    // memory_setup_paging();
    console_write_string("Memory initialization complete\n");
}
