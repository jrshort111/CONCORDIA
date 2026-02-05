#ifndef IOMMU_H
#define IOMMU_H

#include "types.h"

// IOMMU types
#define IOMMU_TYPE_AMDVI 1
#define IOMMU_TYPE_INTEL_VTD 2

// AMD-Vi MMIO registers
#define AMDVI_MMIO_CAP_HDR_OFFSET 0x00
#define AMDVI_MMIO_RANGE_OFFSET 0x04
#define AMDVI_MMIO_CONTROL_OFFSET 0x18
#define AMDVI_MMIO_COMMAND_OFFSET 0x20
#define AMDVI_MMIO_EVENT_OFFSET 0x30
#define AMDVI_MMIO_PPR_OFFSET 0x38
#define AMDVI_MMIO_STATUS_OFFSET 0x2C

// IOMMU Control register bits
#define IOMMU_CONTROL_IOMMU_EN (1UL << 0)
#define IOMMU_CONTROL_HT_TUN_EN (1UL << 1)
#define IOMMU_CONTROL_EVT_LOG_EN (1UL << 2)
#define IOMMU_CONTROL_EVT_INT_EN (1UL << 3)
#define IOMMU_CONTROL_COMP_WAIT_INT_EN (1UL << 4)
#define IOMMU_CONTROL_PPR_INT_EN (1UL << 5)
#define IOMMU_CONTROL_PPR_EN (1UL << 6)
#define IOMMU_CONTROL_COHERENT (1UL << 17)

// PCIe device assignment
#define MAX_IOMMU_GROUPS 64
#define MAX_DEVICES_PER_GROUP 16

typedef struct {
    uint16_t bus;
    uint16_t device;
    uint16_t function;
    uint8_t assigned_to_linux;
} pcie_device_t;

typedef struct {
    uint32_t group_id;
    uint8_t assigned_to_linux;
    uint8_t device_count;
    pcie_device_t devices[MAX_DEVICES_PER_GROUP];
} iommu_group_t;

typedef struct {
    uint64_t base_addr;
    uint32_t cap_id;
    uint8_t type;
    uint8_t enabled;
    iommu_group_t groups[MAX_IOMMU_GROUPS];
    uint32_t group_count;
} iommu_t;

void iommu_init(void);
void iommu_detect(void);
uint8_t iommu_is_available(void);
void iommu_enable_amdvi(void);
void iommu_setup_device_groups(void);
uint8_t iommu_is_linux_device(uint16_t bus, uint16_t device, uint16_t function);
void iommu_assign_device_to_linux(uint16_t bus, uint16_t device, uint16_t function);
void iommu_assign_device_to_windows(uint16_t bus, uint16_t device, uint16_t function);
void iommu_print_status(void);

#endif
