#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "types.h"

// Dashboard modes
#define DASHBOARD_MODE_SUMMARY 0
#define DASHBOARD_MODE_DETAILED 1
#define DASHBOARD_MODE_GRAPHS 2

// Dashboard state
typedef struct {
    uint8_t mode;
    uint8_t enabled;
    uint32_t refresh_count;
    uint64_t last_refresh_time;
} dashboard_t;

void dashboard_init(void);
void dashboard_enable(void);
void dashboard_disable(void);
void dashboard_refresh(void);
void dashboard_set_mode(uint8_t mode);
void dashboard_draw_summary(void);
void dashboard_draw_detailed(void);
void dashboard_draw_header(void);
void dashboard_draw_footer(void);
void dashboard_print_status(void);

#endif
