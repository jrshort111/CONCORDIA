#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "types.h"

// Cell states
#define CELL_STATE_RUNNING 0
#define CELL_STATE_HIBERNATED 1
#define CELL_STATE_INITIALIZING 2
#define CELL_STATE_ERROR 3

// Hibernation location - use reserved area after hypervisor memory
#define HIBERNATION_BASE 0x10000000  // 256MB offset
#define LINUX_HIBERNATION_SIZE (16UL * 1024 * 1024 * 1024)   // 16GB
#define WINDOWS_HIBERNATION_SIZE (16UL * 1024 * 1024 * 1024) // 16GB

#define LINUX_HIBERNATION_ADDR HIBERNATION_BASE
#define WINDOWS_HIBERNATION_ADDR (HIBERNATION_BASE + LINUX_HIBERNATION_SIZE)

// CPU context (for saving/restoring state)
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cr0, cr2, cr3, cr4;
    uint64_t dr0, dr1, dr2, dr3;
} cpu_context_t;

// Cell state
typedef struct {
    uint32_t cell_id;  // 0 = Linux, 1 = Windows
    uint8_t state;
    uint64_t entry_point;
    cpu_context_t context;
    uint64_t hibernation_addr;
    uint64_t hibernation_size;
    uint32_t active_core_count;
    uint32_t hibernation_blocks_used;
} cell_t;

// System state
typedef struct {
    cell_t cells[2];  // Linux and Windows
    uint8_t active_cell;  // 0 = Linux, 1 = Windows
    uint64_t switch_count;
    uint64_t last_switch_time;
} system_state_t;

void system_manager_init(void);
void system_manager_set_active_cell(uint8_t cell_id);
uint8_t system_manager_get_active_cell(void);
void system_manager_switch_cells(void);
void system_manager_hibernate_cell(uint8_t cell_id);
void system_manager_resume_cell(uint8_t cell_id);
void system_manager_freeze_cores(uint8_t cell_id);
void system_manager_unfreeze_cores(uint8_t cell_id);
void system_manager_save_cell_state(uint8_t cell_id);
void system_manager_restore_cell_state(uint8_t cell_id);
void system_manager_print_status(void);

#endif
