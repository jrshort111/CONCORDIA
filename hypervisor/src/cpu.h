#ifndef CPU_H
#define CPU_H

#include "types.h"

#define MAX_CPUS 32
#define LINUX_CORES_START 0
#define LINUX_CORES_END 5
#define WINDOWS_CORES_START 6
#define WINDOWS_CORES_END 11

typedef struct {
    uint32_t apic_id;
    uint32_t core_id;
    uint32_t package_id;
    uint8_t assigned_to_linux;
    uint8_t online;
} cpu_info_t;

typedef struct {
    uint64_t base;
    uint16_t limit;
} gdt_descriptor_t;

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} idt_entry_t;

typedef struct {
    uint32_t flags;
    uint32_t link;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint32_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_offset;
} tss_t;

void cpu_init(void);
void cpu_detect_cores(void);
uint32_t cpu_get_apic_id(void);
uint8_t cpu_is_linux_core(uint32_t apic_id);
void cpu_setup_gdt(void);
void cpu_setup_idt(void);
void cpu_enable_features(void);

#endif
