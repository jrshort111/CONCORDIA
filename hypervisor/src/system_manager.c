#include "system_manager.h"
#include "console.h"
#include "memory.h"
#include "cpu.h"
#include "types.h"

static system_state_t system_state = {0};
static uint32_t switch_counter = 0;

// Get timestamp (simplified - just a counter for now)
static uint64_t get_timestamp(void) {
    static uint64_t ticks = 0;
    return ticks++;
}

void system_manager_init(void) {
    console_write_string("Initializing System Manager...\n");
    
    // Initialize Linux cell
    system_state.cells[0].cell_id = 0;
    system_state.cells[0].state = CELL_STATE_INITIALIZING;
    system_state.cells[0].entry_point = LINUX_MEMORY_START;
    system_state.cells[0].hibernation_addr = LINUX_HIBERNATION_ADDR;
    system_state.cells[0].hibernation_size = LINUX_HIBERNATION_SIZE;
    system_state.cells[0].active_core_count = 6;
    
    // Initialize Windows cell
    system_state.cells[1].cell_id = 1;
    system_state.cells[1].state = CELL_STATE_INITIALIZING;
    system_state.cells[1].entry_point = WINDOWS_MEMORY_START;
    system_state.cells[1].hibernation_addr = WINDOWS_HIBERNATION_ADDR;
    system_state.cells[1].hibernation_size = WINDOWS_HIBERNATION_SIZE;
    system_state.cells[1].active_core_count = 6;
    
    // Start with Linux active
    system_state.active_cell = 0;
    system_state.cells[0].state = CELL_STATE_RUNNING;
    system_state.cells[1].state = CELL_STATE_HIBERNATED;
    
    system_state.switch_count = 0;
    system_state.last_switch_time = get_timestamp();
    
    console_write_string("System Manager initialized\n");
    console_write_string("  Active cell: Linux\n");
    console_write_string("  Linux hibernation: 0x");
    console_write_hex(LINUX_HIBERNATION_ADDR);
    console_write_string(" (16 GB)\n");
    console_write_string("  Windows hibernation: 0x");
    console_write_hex(WINDOWS_HIBERNATION_ADDR);
    console_write_string(" (16 GB)\n");
}

void system_manager_set_active_cell(uint8_t cell_id) {
    if (cell_id >= 2) {
        console_write_string("Invalid cell ID\n");
        return;
    }
    
    if (system_state.active_cell == cell_id) {
        return;  // Already active
    }
    
    system_state.active_cell = cell_id;
}

uint8_t system_manager_get_active_cell(void) {
    return system_state.active_cell;
}

void system_manager_freeze_cores(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    console_write_string("Freezing cores for ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell...\n");
    
    // Mark cores as frozen
    uint32_t start_core = (cell_id == 0) ? 0 : 6;
    uint32_t end_core = start_core + 6;
    
    // In a real implementation, would:
    // - Send IPI (Inter-Processor Interrupt) to cores
    // - Wait for acknowledgment
    // - Put cores into halt state
    
    console_write_string("  Cores ");
    char buf[32];
    itoa(start_core, buf, 10);
    console_write_string(buf);
    console_write_string("-");
    itoa(end_core - 1, buf, 10);
    console_write_string(buf);
    console_write_string(" frozen\n");
}

void system_manager_unfreeze_cores(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    console_write_string("Unfreezing cores for ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell...\n");
    
    // In a real implementation, would:
    // - Send IPI to wake up cores
    // - Restore saved context
    // - Resume execution
    
    uint32_t start_core = (cell_id == 0) ? 0 : 6;
    uint32_t end_core = start_core + 6;
    
    console_write_string("  Cores ");
    char buf[32];
    itoa(start_core, buf, 10);
    console_write_string(buf);
    console_write_string("-");
    itoa(end_core - 1, buf, 10);
    console_write_string(buf);
    console_write_string(" unfrozen\n");
}

void system_manager_save_cell_state(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    cell_t *cell = &system_state.cells[cell_id];
    
    console_write_string("Saving state for ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell to hibernation...\n");
    
    // Save memory (would copy from cell's memory to hibernation area)
    // In reality: memcpy(cell->hibernation_addr, cell->entry_point, cell->hibernation_size)
    // But we can't do actual copies in this context, so we track that we did it
    
    cell->hibernation_blocks_used = cell->hibernation_size / (2 * 1024 * 1024);  // 2MB blocks
    
    console_write_string("  Saved ");
    char buf[32];
    itoa(cell->hibernation_blocks_used, buf, 10);
    console_write_string(buf);
    console_write_string(" x 2MB blocks\n");
}

void system_manager_restore_cell_state(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    cell_t *cell = &system_state.cells[cell_id];
    
    console_write_string("Restoring state for ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell from hibernation...\n");
    
    // Restore memory (would copy from hibernation back to cell's memory)
    // In reality: memcpy(cell->entry_point, cell->hibernation_addr, cell->hibernation_size)
    
    console_write_string("  Restored ");
    char buf[32];
    itoa(cell->hibernation_blocks_used, buf, 10);
    console_write_string(buf);
    console_write_string(" x 2MB blocks\n");
}

void system_manager_hibernate_cell(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    cell_t *cell = &system_state.cells[cell_id];
    
    console_write_string("Hibernating ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell...\n");
    
    // Freeze cores
    system_manager_freeze_cores(cell_id);
    
    // Save state
    system_manager_save_cell_state(cell_id);
    
    // Update state
    cell->state = CELL_STATE_HIBERNATED;
    
    console_write_string("Cell hibernated\n");
}

void system_manager_resume_cell(uint8_t cell_id) {
    if (cell_id >= 2) return;
    
    cell_t *cell = &system_state.cells[cell_id];
    
    console_write_string("Resuming ");
    console_write_string(cell_id == 0 ? "Linux" : "Windows");
    console_write_string(" cell...\n");
    
    // Restore state
    system_manager_restore_cell_state(cell_id);
    
    // Unfreeze cores
    system_manager_unfreeze_cores(cell_id);
    
    // Update state
    cell->state = CELL_STATE_RUNNING;
    
    console_write_string("Cell resumed\n");
}

void system_manager_switch_cells(void) {
    uint8_t current = system_state.active_cell;
    uint8_t next = (current == 0) ? 1 : 0;
    
    console_write_string("\n===== SWITCHING CELLS =====\n");
    console_write_string("From: ");
    console_write_string(current == 0 ? "Linux" : "Windows");
    console_write_string(" -> To: ");
    console_write_string(next == 0 ? "Linux" : "Windows");
    console_write_string("\n");
    
    // Hibernate current cell
    system_manager_hibernate_cell(current);
    
    console_write_string("\n");
    
    // Resume next cell
    system_manager_resume_cell(next);
    
    // Update active cell
    system_state.active_cell = next;
    system_state.switch_count++;
    system_state.last_switch_time = get_timestamp();
    
    console_write_string("===== SWITCH COMPLETE =====\n\n");
}

void system_manager_print_status(void) {
    console_write_string("System Status:\n");
    console_write_string("  Active cell: ");
    console_write_string(system_state.active_cell == 0 ? "Linux\n" : "Windows\n");
    
    for (int i = 0; i < 2; i++) {
        cell_t *cell = &system_state.cells[i];
        console_write_string("  ");
        console_write_string(i == 0 ? "Linux" : "Windows");
        console_write_string(": ");
        
        switch (cell->state) {
            case CELL_STATE_RUNNING:
                console_write_string("Running");
                break;
            case CELL_STATE_HIBERNATED:
                console_write_string("Hibernated");
                break;
            case CELL_STATE_INITIALIZING:
                console_write_string("Initializing");
                break;
            case CELL_STATE_ERROR:
                console_write_string("Error");
                break;
            default:
                console_write_string("Unknown");
        }
        console_write_string(" (");
        char buf[32];
        itoa(cell->active_core_count, buf, 10);
        console_write_string(buf);
        console_write_string(" cores)\n");
    }
    
    console_write_string("  Total switches: ");
    char buf[32];
    itoa(system_state.switch_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
}
