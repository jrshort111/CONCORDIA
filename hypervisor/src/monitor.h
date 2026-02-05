#ifndef MONITOR_H
#define MONITOR_H

#include "types.h"

// Monitoring refresh rates
#define MONITOR_UPDATE_INTERVAL_MS 1000

// Metrics structure
typedef struct {
    uint64_t cpu_cycles;
    uint32_t cpu_load_percent;
    uint64_t memory_used;
    uint64_t memory_free;
    uint32_t context_switches;
} cell_metrics_t;

// System metrics
typedef struct {
    uint64_t uptime_ms;
    uint32_t total_switches;
    uint64_t last_update_time;
    cell_metrics_t linux_metrics;
    cell_metrics_t windows_metrics;
    uint8_t active_cell;
} system_metrics_t;

void monitor_init(void);
void monitor_update_metrics(void);
void monitor_sample_cell_metrics(uint8_t cell_id, cell_metrics_t *metrics);
uint32_t monitor_calculate_cpu_load(cell_metrics_t *metrics);
uint64_t monitor_get_uptime(void);
void monitor_print_summary(void);

#endif
