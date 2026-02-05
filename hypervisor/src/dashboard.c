#include "dashboard.h"
#include "console.h"
#include "monitor.h"
#include "system_manager.h"
#include "types.h"

static dashboard_t dashboard = {0};

void dashboard_init(void) {
    console_write_string("Initializing Dashboard...\n");
    
    dashboard.mode = DASHBOARD_MODE_SUMMARY;
    dashboard.enabled = 1;
    dashboard.refresh_count = 0;
    dashboard.last_refresh_time = 0;
    
    console_write_string("Dashboard initialized (mode: summary)\n");
}

void dashboard_enable(void) {
    dashboard.enabled = 1;
    console_write_string("Dashboard enabled\n");
}

void dashboard_disable(void) {
    dashboard.enabled = 0;
    console_write_string("Dashboard disabled\n");
}

void dashboard_set_mode(uint8_t mode) {
    if (mode >= 3) return;
    dashboard.mode = mode;
}

void dashboard_draw_header(void) {
    console_write_string("\n");
    console_write_string("╔════════════════════════════════════════════════════════════╗\n");
    console_write_string("║         CONCORDIA DUAL-OS HYPERVISOR DASHBOARD             ║\n");
    console_write_string("╚════════════════════════════════════════════════════════════╝\n");
    console_write_string("\n");
}

void dashboard_draw_summary(void) {
    dashboard_draw_header();
    
    uint8_t active = system_manager_get_active_cell();
    
    console_write_string("┌─ System Status ─────────────────────────────────────────────┐\n");
    console_write_string("│ Active OS:     ");
    console_write_string(active == 0 ? "Linux (cores 0-5)     " : "Windows (cores 6-11)  ");
    console_write_string("                   │\n");
    
    console_write_string("│ Hotkey:        Ctrl+Alt+O to switch OS                      │\n");
    console_write_string("└─────────────────────────────────────────────────────────────┘\n\n");
    
    console_write_string("┌─ Linux Cell (AMD GPU - RX 7600) ──────────────────────────┐\n");
    console_write_string("│ Status:        ");
    console_write_string(active == 0 ? "RUNNING" : "HIBERNATED");
    console_write_string("                              │\n");
    console_write_string("│ CPU Cores:     0-5 (6 cores available)                     │\n");
    console_write_string("│ Memory:        16 GB allocated, usage ~50%                 │\n");
    console_write_string("│ GPU:           AMD Radeon RX 7600 (IOMMU Group 28-29)    │\n");
    console_write_string("└─────────────────────────────────────────────────────────────┘\n\n");
    
    console_write_string("┌─ Windows Cell (NVIDIA GPU - RTX 3050) ────────────────────┐\n");
    console_write_string("│ Status:        ");
    console_write_string(active == 1 ? "RUNNING" : "HIBERNATED");
    console_write_string("                              │\n");
    console_write_string("│ CPU Cores:     6-11 (6 cores available)                    │\n");
    console_write_string("│ Memory:        16 GB allocated, usage ~40%                 │\n");
    console_write_string("│ GPU:           NVIDIA GeForce RTX 3050 (IOMMU Group 30)  │\n");
    console_write_string("└─────────────────────────────────────────────────────────────┘\n\n");
    
    console_write_string("┌─ Input Manager ─────────────────────────────────────────────┐\n");
    console_write_string("│ Device:        USB Keyboard & Mouse                        │\n");
    console_write_string("│ Routing:       ");
    console_write_string(active == 0 ? "→ Linux Cell" : "→ Windows Cell");
    console_write_string("                            │\n");
    console_write_string("└─────────────────────────────────────────────────────────────┘\n");
}

void dashboard_draw_detailed(void) {
    dashboard_draw_header();
    
    console_write_string("Detailed Metrics:\n\n");
    
    monitor_print_summary();
}

void dashboard_draw_footer(void) {
    console_write_string("\n[Press Ctrl+Alt+O to switch | ESC to exit dashboard]\n");
}

void dashboard_refresh(void) {
    if (!dashboard.enabled) return;
    
    dashboard.refresh_count++;
    
    // Update monitor metrics
    monitor_update_metrics();
    
    // Clear screen (in a real implementation)
    // console_clear();
    
    // Draw based on current mode
    switch (dashboard.mode) {
        case DASHBOARD_MODE_SUMMARY:
            dashboard_draw_summary();
            break;
        case DASHBOARD_MODE_DETAILED:
            dashboard_draw_detailed();
            break;
        case DASHBOARD_MODE_GRAPHS:
            console_write_string("Graph mode not yet implemented\n");
            break;
        default:
            dashboard_draw_summary();
    }
    
    dashboard_draw_footer();
}

void dashboard_print_status(void) {
    console_write_string("Dashboard Status:\n");
    console_write_string("  Enabled: ");
    console_write_string(dashboard.enabled ? "Yes\n" : "No\n");
    
    console_write_string("  Mode: ");
    switch (dashboard.mode) {
        case DASHBOARD_MODE_SUMMARY:
            console_write_string("Summary");
            break;
        case DASHBOARD_MODE_DETAILED:
            console_write_string("Detailed");
            break;
        case DASHBOARD_MODE_GRAPHS:
            console_write_string("Graphs");
            break;
        default:
            console_write_string("Unknown");
    }
    console_write_string("\n");
    
    console_write_string("  Refreshes: ");
    char buf[32];
    itoa(dashboard.refresh_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
}
