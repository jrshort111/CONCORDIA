#include "monitor.h"
#include "console.h"
#include "system_manager.h"
#include "types.h"

static system_metrics_t system_metrics = {0};
static uint64_t ticks = 0;

void monitor_init(void) {
    console_write_string("Initializing Monitor...\n");
    
    system_metrics.uptime_ms = 0;
    system_metrics.total_switches = 0;
    system_metrics.last_update_time = 0;
    system_metrics.active_cell = system_manager_get_active_cell();
    
    // Initialize metrics
    system_metrics.linux_metrics.cpu_cycles = 0;
    system_metrics.linux_metrics.memory_used = 0;
    system_metrics.linux_metrics.context_switches = 0;
    
    system_metrics.windows_metrics.cpu_cycles = 0;
    system_metrics.windows_metrics.memory_used = 0;
    system_metrics.windows_metrics.context_switches = 0;
    
    console_write_string("Monitor initialized\n");
}

uint64_t monitor_get_uptime(void) {
    return system_metrics.uptime_ms;
}

void monitor_sample_cell_metrics(uint8_t cell_id, cell_metrics_t *metrics) {
    if (cell_id >= 2 || !metrics) return;
    
    // Simulate CPU and memory sampling
    // In a real implementation, would:
    // - Read MSRs for CPU cycle counters
    // - Query page tables for memory usage
    // - Sample performance counters
    
    metrics->cpu_cycles += 1000000;  // Simulate work
    metrics->cpu_load_percent = (ticks % 80) + 10;  // Vary load 10-90%
    metrics->memory_used = (cell_id == 0) ? 
        (8UL * 1024 * 1024 * 1024) :  // Linux: ~8GB used
        (6UL * 1024 * 1024 * 1024);   // Windows: ~6GB used
    metrics->memory_free = 16UL * 1024 * 1024 * 1024 - metrics->memory_used;
    metrics->context_switches += (ticks % 100);
}

uint32_t monitor_calculate_cpu_load(cell_metrics_t *metrics) {
    if (!metrics) return 0;
    return metrics->cpu_load_percent;
}

void monitor_update_metrics(void) {
    ticks++;
    system_metrics.uptime_ms += MONITOR_UPDATE_INTERVAL_MS;
    
    // Sample Linux cell metrics
    monitor_sample_cell_metrics(0, &system_metrics.linux_metrics);
    
    // Sample Windows cell metrics
    monitor_sample_cell_metrics(1, &system_metrics.windows_metrics);
    
    // Update active cell
    system_metrics.active_cell = system_manager_get_active_cell();
    
    // Get switch count from system manager
    system_metrics.total_switches = system_manager_get_active_cell();  // Placeholder
}

void monitor_print_summary(void) {
    console_write_string("=== System Monitor ===\n");
    
    console_write_string("Uptime: ");
    char buf[32];
    uint64_t uptime_sec = system_metrics.uptime_ms / 1000;
    itoa(uptime_sec, buf, 10);
    console_write_string(buf);
    console_write_string("s\n");
    
    console_write_string("\nActive Cell: ");
    console_write_string(system_metrics.active_cell == 0 ? "Linux\n" : "Windows\n");
    
    // Linux metrics
    console_write_string("\nLinux Cell:\n");
    console_write_string("  CPU Load: ");
    itoa(system_metrics.linux_metrics.cpu_load_percent, buf, 10);
    console_write_string(buf);
    console_write_string("%\n");
    
    console_write_string("  Memory Used: ");
    uint64_t mem_gb = system_metrics.linux_metrics.memory_used / (1024 * 1024 * 1024);
    itoa(mem_gb, buf, 10);
    console_write_string(buf);
    console_write_string(" GB / 16 GB\n");
    
    console_write_string("  CPU Cycles: ");
    uint64_t cycles_m = system_metrics.linux_metrics.cpu_cycles / 1000000;
    itoa(cycles_m, buf, 10);
    console_write_string(buf);
    console_write_string("M\n");
    
    // Windows metrics
    console_write_string("\nWindows Cell:\n");
    console_write_string("  CPU Load: ");
    itoa(system_metrics.windows_metrics.cpu_load_percent, buf, 10);
    console_write_string(buf);
    console_write_string("%\n");
    
    console_write_string("  Memory Used: ");
    mem_gb = system_metrics.windows_metrics.memory_used / (1024 * 1024 * 1024);
    itoa(mem_gb, buf, 10);
    console_write_string(buf);
    console_write_string(" GB / 16 GB\n");
    
    console_write_string("  CPU Cycles: ");
    cycles_m = system_metrics.windows_metrics.cpu_cycles / 1000000;
    itoa(cycles_m, buf, 10);
    console_write_string(buf);
    console_write_string("M\n");
    
    console_write_string("\n=====================\n");
}
