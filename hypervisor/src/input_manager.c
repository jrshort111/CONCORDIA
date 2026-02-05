#include "input_manager.h"
#include "console.h"
#include "system_manager.h"
#include "pci.h"
#include "types.h"

static input_device_t input_device = {0};

// USB helper - read PCI config for USB controller
static uint32_t usb_get_device_info(uint16_t bus, uint16_t dev, uint16_t func, uint16_t offset) {
    uint32_t addr = (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
    uint32_t cfg_addr = addr | 0x80000000;
    
    asm volatile("mov $0xCF8, %%dx; outl %%eax, %%dx" : : "a"(cfg_addr));
    
    uint32_t result;
    asm volatile("mov $0xCFC, %%dx; inl %%dx, %%eax" : "=a"(result));
    
    return result;
}

void input_manager_init(void) {
    console_write_string("Initializing Input Manager...\n");
    
    input_device.device_type = INPUT_DEVICE_USB;
    input_device.state = KEYBOARD_STATE_READY;
    input_device.interrupt_count = 0;
    input_device.device_count = 0;
    input_device.keys.ctrl_pressed = 0;
    input_device.keys.alt_pressed = 0;
    input_device.keys.shift_pressed = 0;
    input_device.keys.last_key = 0;
    input_device.keys.key_count = 0;
    
    console_write_string("  Input device: USB Keyboard\n");
    console_write_string("  Hotkey: Ctrl+Alt+O to switch OS\n");
    console_write_string("Input Manager initialized\n");
}

void input_manager_detect_usb_devices(void) {
    console_write_string("Detecting USB input devices...\n");
    
    uint32_t device_count = 0;
    
    // Scan PCI bus for USB controllers and HID devices
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint16_t dev = 0; dev < 32; dev++) {
            uint32_t vendor_device = usb_get_device_info(bus, dev, 0, 0x00);
            uint16_t vendor = vendor_device & 0xFFFF;
            uint16_t device_id = (vendor_device >> 16) & 0xFFFF;
            
            if (vendor == 0xFFFF) continue;
            
            // Look for USB HID keyboards
            // Common USB keyboard vendors: Logitech (0x046D), Microsoft (0x045E), etc.
            if ((vendor == 0x046D || vendor == 0x045E || vendor == 0x1234) && device_count < MAX_USB_DEVICES) {
                usb_device_t *usb_dev = &input_device.devices[device_count];
                usb_dev->vendor_id = vendor;
                usb_dev->product_id = device_id;
                usb_dev->bus = bus;
                usb_dev->device = dev;
                usb_dev->endpoint = 0x81;  // Standard interrupt endpoint
                
                console_write_string("  USB Keyboard found: ");
                char buf[32];
                itoa(vendor, buf, 16);
                console_write_string(buf);
                console_write_string(":");
                itoa(device_id, buf, 16);
                console_write_string(buf);
                console_write_string("\n");
                
                device_count++;
            }
        }
    }
    
    input_device.device_count = device_count;
    
    if (device_count == 0) {
        console_write_string("  No USB keyboards detected, using simulated input\n");
        input_device.state = KEYBOARD_STATE_READY;
    } else {
        console_write_string("USB input devices detected: ");
        char buf[32];
        itoa(device_count, buf, 10);
        console_write_string(buf);
        console_write_string("\n");
    }
}

uint8_t input_manager_read_key(void) {
    // In a real implementation, would read from USB interrupt endpoint
    // For now, simulate waiting for input
    static uint8_t key = 0;
    
    input_device.interrupt_count++;
    
    return key;
}

void input_manager_process_key(uint8_t scancode) {
    // Handle key press vs key release
    uint8_t is_release = (scancode & KEY_RELEASED) != 0;
    uint8_t key = scancode & ~KEY_RELEASED;
    
    if (is_release) {
        // Key released
        switch (key) {
            case KEY_LCTRL:
                input_device.keys.ctrl_pressed = 0;
                break;
            case KEY_LALT:
                input_device.keys.alt_pressed = 0;
                break;
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                input_device.keys.shift_pressed = 0;
                break;
        }
    } else {
        // Key pressed
        switch (key) {
            case KEY_LCTRL:
                input_device.keys.ctrl_pressed = 1;
                break;
            case KEY_LALT:
                input_device.keys.alt_pressed = 1;
                break;
            case KEY_LSHIFT:
            case KEY_RSHIFT:
                input_device.keys.shift_pressed = 1;
                break;
            default:
                input_device.keys.last_key = key;
                input_device.keys.key_count++;
                break;
        }
    }
}

void input_manager_check_hotkey(void) {
    // Check for Ctrl+Alt+O
    if (input_device.keys.ctrl_pressed && 
        input_device.keys.alt_pressed && 
        input_device.keys.last_key == KEY_O) {
        
        console_write_string("\n[INPUT] Hotkey detected: Ctrl+Alt+O\n");
        
        // Trigger OS switch
        system_manager_switch_cells();
        
        // Clear the key state to avoid repeated switches
        input_device.keys.last_key = 0;
    }
}

void input_manager_route_input(uint8_t cell_id) {
    // In a real implementation, this would:
    // - Determine which cell should receive input
    // - Filter/allow input based on active cell
    // - Handle input redirection via IOMMU
    
    if (cell_id >= 2) return;
}

void input_manager_print_status(void) {
    console_write_string("Input Manager Status:\n");
    console_write_string("  Device type: USB Keyboard\n");
    
    console_write_string("  State: ");
    switch (input_device.state) {
        case KEYBOARD_STATE_READY:
            console_write_string("Ready");
            break;
        case KEYBOARD_STATE_WAITING:
            console_write_string("Waiting");
            break;
        case KEYBOARD_STATE_ERROR:
            console_write_string("Error");
            break;
        default:
            console_write_string("Unknown");
    }
    console_write_string("\n");
    
    console_write_string("  USB devices: ");
    char buf[32];
    itoa(input_device.device_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
    
    console_write_string("  Interrupt count: ");
    itoa(input_device.interrupt_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
    
    console_write_string("  Keys processed: ");
    itoa(input_device.keys.key_count, buf, 10);
    console_write_string(buf);
    console_write_string("\n");
}

void input_manager_handle_interrupt(void) {
    if (input_device.state != KEYBOARD_STATE_READY) {
        return;
    }
    
    // Read key from USB controller
    uint8_t scancode = input_manager_read_key();
    
    // Process the key
    input_manager_process_key(scancode);
    
    // Check for hotkeys
    input_manager_check_hotkey();
    
    // Route input to active cell
    uint8_t active_cell = system_manager_get_active_cell();
    input_manager_route_input(active_cell);
}
