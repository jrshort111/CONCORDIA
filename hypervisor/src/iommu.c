#include "iommu.h"
#include "console.h"
#include "pci.h"
#include "types.h"

static iommu_t iommu_state = {0};

// Read from PCI configuration space (bus, device, function, offset)
static uint32_t pci_read_config(uint16_t bus, uint16_t dev, uint16_t func, uint16_t offset) {
    uint32_t addr = (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
    uint32_t result;
    
    // Write address to CONFIG_ADDRESS (0xCF8)
    asm volatile("mov $0xCF8, %%dx; outl %%eax, %%dx"
                 : : "a"(addr | 0x80000000));
    
    // Read from CONFIG_DATA (0xCFC)
    asm volatile("mov $0xCFC, %%dx; inl %%dx, %%eax"
                 : "=a"(result));
    
    return result;
}

// Write to PCI configuration space
static void pci_write_config(uint16_t bus, uint16_t dev, uint16_t func, uint16_t offset, uint32_t value) {
    uint32_t addr = (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
    
    // Write address to CONFIG_ADDRESS
    asm volatile("mov $0xCF8, %%dx; outl %%eax, %%dx"
                 : : "a"(addr | 0x80000000));
    
    // Write to CONFIG_DATA
    asm volatile("mov $0xCFC, %%dx; outl %%eax, %%dx"
                 : : "a"(value));
}

void iommu_detect(void) {
    console_write_string("Detecting IOMMU...\n");
    
    // Scan PCI bus 0 for IOMMU capability
    // Look for AMD-Vi (vendor 0x1022, device 0x1447-0x1457)
    for (uint16_t dev = 0; dev < 32; dev++) {
        uint32_t vendor_device = pci_read_config(0, dev, 0, 0x00);
        uint16_t vendor = vendor_device & 0xFFFF;
        uint16_t device = (vendor_device >> 16) & 0xFFFF;
        
        // AMD-Vi IOMMU detection
        if (vendor == 0x1022 && (device >= 0x1447 && device <= 0x1457)) {
            console_write_string("  Found AMD-Vi IOMMU at PCI ");
            char buf[32];
            itoa(dev, buf, 10);
            console_write_string(buf);
            console_write_string("\n");
            
            iommu_state.type = IOMMU_TYPE_AMDVI;
            iommu_state.enabled = 1;
            
            // Get base address from capability structure
            uint32_t cap_ptr = pci_read_config(0, dev, 0, 0x34) & 0xFF;
            while (cap_ptr != 0) {
                uint32_t cap = pci_read_config(0, dev, 0, cap_ptr);
                uint8_t cap_id = cap & 0xFF;
                
                if (cap_id == 0x0F) {
                    // IOMMU MMIO base is at offset cap_ptr + 4
                    uint32_t base_low = pci_read_config(0, dev, 0, cap_ptr + 4);
                    uint32_t base_high = pci_read_config(0, dev, 0, cap_ptr + 8);
                    iommu_state.base_addr = ((uint64_t)base_high << 32) | (base_low & 0xFFFFF000);
                    
                    console_write_string("  IOMMU MMIO base: 0x");
                    console_write_hex(iommu_state.base_addr);
                    console_write_string("\n");
                    break;
                }
                cap_ptr = (cap >> 8) & 0xFF;
            }
            return;
        }
    }
    
    console_write_string("  No IOMMU detected\n");
    iommu_state.enabled = 0;
}

uint8_t iommu_is_available(void) {
    return iommu_state.enabled;
}

void iommu_setup_device_groups(void) {
    console_write_string("Scanning IOMMU groups...\n");
    
    uint32_t group_count = 0;
    
    // Scan PCI buses for devices and their IOMMU groups
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            uint32_t vendor_device = pci_read_config(bus, dev, 0, 0x00);
            uint16_t vendor = vendor_device & 0xFFFF;
            
            if (vendor == 0xFFFF) continue;  // No device
            
            uint16_t device = (vendor_device >> 16) & 0xFFFF;
            
            // GPU device detection
            // NVIDIA: 0x10DE, AMD: 0x1002
            if (vendor == 0x1002 || vendor == 0x10DE) {
                if (group_count < MAX_IOMMU_GROUPS) {
                    iommu_group_t *group = &iommu_state.groups[group_count];
                    group->group_id = group_count;
                    group->device_count = 1;
                    group->devices[0].bus = bus;
                    group->devices[0].device = dev;
                    group->devices[0].function = 0;
                    
                    // Assign AMD GPU to Linux, NVIDIA to Windows
                    group->devices[0].assigned_to_linux = (vendor == 0x1002) ? 1 : 0;
                    group->assigned_to_linux = group->devices[0].assigned_to_linux;
                    
                    console_write_string("  Group ");
                    char buf[32];
                    itoa(group_count, buf, 10);
                    console_write_string(buf);
                    console_write_string(": ");
                    
                    if (vendor == 0x1002) {
                        console_write_string("AMD GPU (");
                        itoa(0x1002, buf, 16);
                        console_write_string(buf);
                    } else {
                        console_write_string("NVIDIA GPU (");
                        itoa(0x10DE, buf, 16);
                        console_write_string(buf);
                    }
                    console_write_string(") -> ");
                    console_write_string(group->assigned_to_linux ? "Linux\n" : "Windows\n");
                    
                    group_count++;
                }
            }
        }
    }
    
    iommu_state.group_count = group_count;
    console_write_string("Device groups configured: ");
    char buf[32];
    itoa(group_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
}

uint8_t iommu_is_linux_device(uint16_t bus, uint16_t device, uint16_t function) {
    for (uint32_t i = 0; i < iommu_state.group_count; i++) {
        iommu_group_t *group = &iommu_state.groups[i];
        for (uint8_t j = 0; j < group->device_count; j++) {
            pcie_device_t *dev = &group->devices[j];
            if (dev->bus == bus && dev->device == device && dev->function == function) {
                return dev->assigned_to_linux;
            }
        }
    }
    return 0;
}

void iommu_assign_device_to_linux(uint16_t bus, uint16_t device, uint16_t function) {
    for (uint32_t i = 0; i < iommu_state.group_count; i++) {
        iommu_group_t *group = &iommu_state.groups[i];
        for (uint8_t j = 0; j < group->device_count; j++) {
            pcie_device_t *dev = &group->devices[j];
            if (dev->bus == bus && dev->device == device && dev->function == function) {
                dev->assigned_to_linux = 1;
                group->assigned_to_linux = 1;
            }
        }
    }
}

void iommu_assign_device_to_windows(uint16_t bus, uint16_t device, uint16_t function) {
    for (uint32_t i = 0; i < iommu_state.group_count; i++) {
        iommu_group_t *group = &iommu_state.groups[i];
        for (uint8_t j = 0; j < group->device_count; j++) {
            pcie_device_t *dev = &group->devices[j];
            if (dev->bus == bus && dev->device == device && dev->function == function) {
                dev->assigned_to_linux = 0;
                group->assigned_to_linux = 0;
            }
        }
    }
}

void iommu_enable_amdvi(void) {
    if (!iommu_state.enabled || iommu_state.type != IOMMU_TYPE_AMDVI) {
        console_write_string("AMD-Vi not available\n");
        return;
    }
    
    console_write_string("Enabling AMD-Vi...\n");
    
    // Read control register
    volatile uint32_t *ctrl_reg = (volatile uint32_t *)(iommu_state.base_addr + AMDVI_MMIO_CONTROL_OFFSET);
    uint32_t ctrl = *ctrl_reg;
    
    // Enable IOMMU
    ctrl |= IOMMU_CONTROL_IOMMU_EN;
    ctrl |= IOMMU_CONTROL_COHERENT;
    
    *ctrl_reg = ctrl;
    
    // Verify enable
    uint32_t verify = *ctrl_reg;
    if (verify & IOMMU_CONTROL_IOMMU_EN) {
        console_write_string("AMD-Vi enabled successfully\n");
    } else {
        console_write_string("Failed to enable AMD-Vi\n");
    }
}

void iommu_print_status(void) {
    console_write_string("IOMMU Status:\n");
    if (iommu_state.enabled) {
        console_write_string("  Status: Enabled\n");
        console_write_string("  Type: ");
        if (iommu_state.type == IOMMU_TYPE_AMDVI) {
            console_write_string("AMD-Vi\n");
        } else if (iommu_state.type == IOMMU_TYPE_INTEL_VTD) {
            console_write_string("Intel VT-d\n");
        }
        console_write_string("  Groups: ");
        char buf[32];
        itoa(iommu_state.group_count, buf, 10);
        console_write_string(buf);
        console_write_string("\n");
    } else {
        console_write_string("  Status: Not detected\n");
    }
}

void iommu_init(void) {
    console_write_string("Initializing IOMMU subsystem...\n");
    
    // Detect IOMMU
    iommu_detect();
    
    if (!iommu_state.enabled) {
        console_write_string("WARNING: IOMMU not available, device assignment will be unsafe\n");
        return;
    }
    
    // Setup device groups
    iommu_setup_device_groups();
    
    // Enable IOMMU
    if (iommu_state.type == IOMMU_TYPE_AMDVI) {
        iommu_enable_amdvi();
    }
    
    iommu_print_status();
    console_write_string("IOMMU initialization complete\n");
}
