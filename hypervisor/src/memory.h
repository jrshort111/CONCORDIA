#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

// Memory layout
#define TOTAL_MEMORY          (32UL * 1024 * 1024 * 1024)  // 32 GB total
#define LINUX_MEMORY_SIZE     (16UL * 1024 * 1024 * 1024)  // 16 GB
#define WINDOWS_MEMORY_SIZE   (16UL * 1024 * 1024 * 1024)  // 16 GB
#define HYPERVISOR_MEMORY     (256UL * 1024 * 1024)        // 256 MB for hypervisor

#define LINUX_MEMORY_START    0x0000000000000000
#define LINUX_MEMORY_END      (LINUX_MEMORY_START + LINUX_MEMORY_SIZE)

#define WINDOWS_MEMORY_START  LINUX_MEMORY_END
#define WINDOWS_MEMORY_END    (WINDOWS_MEMORY_START + WINDOWS_MEMORY_SIZE)

#define HYPERVISOR_MEMORY_START 0x100000
#define HYPERVISOR_MEMORY_END   (HYPERVISOR_MEMORY_START + HYPERVISOR_MEMORY)

// Page sizes
#define PAGE_SIZE_4K          4096
#define PAGE_SIZE_2M          (2 * 1024 * 1024)
#define PAGE_SIZE_1G          (1024 * 1024 * 1024)

// Page table structures (x86-64)
#define PAGE_PRESENT          (1UL << 0)
#define PAGE_WRITE            (1UL << 1)
#define PAGE_USER             (1UL << 2)
#define PAGE_PWT              (1UL << 3)
#define PAGE_PCD              (1UL << 4)
#define PAGE_ACCESSED         (1UL << 5)
#define PAGE_DIRTY            (1UL << 6)
#define PAGE_PSE              (1UL << 7)
#define PAGE_GLOBAL           (1UL << 8)
#define PAGE_NX               (1UL << 63)

typedef struct {
    uint64_t entry;
} page_table_entry_t;

typedef struct {
    uint64_t *pml4;
    uint64_t *pdp;
    uint64_t *pd;
    uint64_t *pt;
} page_table_t;

typedef struct {
    uint64_t base;
    uint64_t limit;
    uint64_t allocated;
    char name[32];
} memory_region_t;

void memory_init(void);
void memory_setup_paging(void);
void *memory_alloc(size_t size);
void memory_free(void *ptr);
void memory_setup_cell_boundaries(void);
uint8_t memory_is_linux_address(uint64_t addr);
uint8_t memory_is_windows_address(uint64_t addr);
void memory_print_layout(void);

#endif
