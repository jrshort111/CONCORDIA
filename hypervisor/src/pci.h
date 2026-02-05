#ifndef PCI_H
#define PCI_H

#include "types.h"

// PCI configuration space I/O ports
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// PCI device classes for filtering
#define PCI_CLASS_DISPLAY 0x03
#define PCI_CLASS_BRIDGE 0x06

// Helper for reading/writing PCI config space
#define PCI_MAKE_ADDRESS(bus, dev, func, offset) \
    (0x80000000 | ((bus) << 16) | ((dev) << 11) | ((func) << 8) | ((offset) & 0xFC))

#endif
