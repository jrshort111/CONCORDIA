#ifndef KERNEL_LOADER_H
#define KERNEL_LOADER_H

#include "types.h"

// Kernel entry points
#define LINUX_KERNEL_ENTRY 0x0
#define WINDOWS_KERNEL_ENTRY 0x400000000

// Kernel info
typedef struct {
    uint64_t entry_point;
    uint64_t load_address;
    uint64_t size;
    uint8_t loaded;
    char name[32];
} kernel_info_t;

typedef struct {
    kernel_info_t linux;
    kernel_info_t windows;
} kernel_state_t;

void kernel_loader_init(void);
void kernel_load_linux_stub(void);
void kernel_load_windows_stub(void);
void kernel_boot_linux(void);
void kernel_boot_windows(void);
uint8_t kernel_is_loaded(uint8_t cell_id);
void kernel_loader_print_status(void);

#endif
