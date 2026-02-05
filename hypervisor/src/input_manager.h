#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "types.h"

// USB HID constants
#define USB_CLASS_HID 0x03
#define USB_SUBCLASS_BOOT 0x01
#define USB_PROTOCOL_KEYBOARD 0x01

// USB HID scan codes (same as PS/2 for compatibility)
#define KEY_ESCAPE 0x01
#define KEY_1 0x02
#define KEY_O 0x18
#define KEY_LCTRL 0x1D
#define KEY_LSHIFT 0x2A
#define KEY_RSHIFT 0x36
#define KEY_LALT 0x38

// Key states
#define KEY_RELEASED 0x80
#define KEYBOARD_STATE_READY 0
#define KEYBOARD_STATE_WAITING 1
#define KEYBOARD_STATE_ERROR 2

// Input device types
#define INPUT_DEVICE_USB 1

// USB device info
#define MAX_USB_DEVICES 16

typedef struct {
    uint16_t vendor_id;
    uint16_t product_id;
    uint8_t bus;
    uint8_t device;
    uint8_t endpoint;
} usb_device_t;

// Keyboard state tracking
typedef struct {
    uint8_t ctrl_pressed;
    uint8_t alt_pressed;
    uint8_t shift_pressed;
    uint8_t last_key;
    uint32_t key_count;
} keyboard_state_t;

typedef struct {
    uint8_t device_type;
    uint8_t state;
    uint32_t interrupt_count;
    keyboard_state_t keys;
    usb_device_t devices[MAX_USB_DEVICES];
    uint32_t device_count;
} input_device_t;

void input_manager_init(void);
void input_manager_detect_usb_devices(void);
void input_manager_handle_interrupt(void);
uint8_t input_manager_read_key(void);
void input_manager_process_key(uint8_t scancode);
void input_manager_check_hotkey(void);
void input_manager_route_input(uint8_t cell_id);
void input_manager_print_status(void);

#endif
