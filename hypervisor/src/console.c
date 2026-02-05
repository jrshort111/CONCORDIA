#include "console.h"
#include "types.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_COLOR 0x0F

#define SERIAL_PORT 0x3F8

static uint16_t *vga_buffer = (uint16_t *)VGA_ADDRESS;
static uint32_t vga_row = 0;
static uint32_t vga_col = 0;

static inline void serial_out(uint16_t port, uint8_t val) {
    asm volatile("out %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t serial_in(uint16_t port) {
    uint8_t ret;
    asm volatile("in %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void) {
    // Initialize UART at 0x3F8 for QEMU
    // Basic initialization: set divisor, line control
    serial_out(SERIAL_PORT + 1, 0x00);  // Disable interrupts
    serial_out(SERIAL_PORT + 3, 0x80);  // Enable DLAB
    serial_out(SERIAL_PORT + 0, 1);     // Divisor low
    serial_out(SERIAL_PORT + 1, 0);     // Divisor high
    serial_out(SERIAL_PORT + 3, 0x03);  // 8N1
    serial_out(SERIAL_PORT + 2, 0xC7);  // FIFO
    serial_out(SERIAL_PORT + 4, 0x0B);  // RTS/DTR
    
    // Send initial marker
    serial_write_string("CONCORDIA START\n");
}

void serial_write_string(const char *str) {
    while (*str) {
        // Wait for transmit buffer
        int timeout = 10000;
        while (timeout-- && ((serial_in(SERIAL_PORT + 5) & 0x20) == 0));
        
        serial_out(SERIAL_PORT, *str);
        str++;
    }
}

void console_init(void) {
    // Minimal initialization - just send marker
    // Skip serial_init and VGA setup to isolate the problem
    const char *boot_msg = "CONCORDIA_BOOT_START\n";
    const char *p = boot_msg;
    while (*p) {
        serial_out(SERIAL_PORT, *p++);
    }
}

void console_write_char(char c) {
    serial_out(SERIAL_PORT, c);
    
    if (c == '\n') {
        vga_row++;
        vga_col = 0;
    } else {
        uint32_t index = vga_row * VGA_WIDTH + vga_col;
        vga_buffer[index] = (VGA_COLOR << 8) | c;
        vga_col++;
    }
    
    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }
    
    if (vga_row >= VGA_HEIGHT) {
        vga_row = VGA_HEIGHT - 1;
    }
}

void console_write_string(const char *str) {
    while (*str) {
        serial_out(SERIAL_PORT, *str++);
    }
}

void itoa(int value, char *str, int base) {
    static const char digits[] = "0123456789abcdef";
    char buffer[33];
    int i = 0;
    int negative = 0;
    
    if (value < 0 && base == 10) {
        negative = 1;
        value = -value;
    }
    
    if (value == 0) {
        *str++ = '0';
        *str = '\0';
        return;
    }
    
    while (value > 0) {
        buffer[i++] = digits[value % base];
        value /= base;
    }
    
    if (negative) {
        *str++ = '-';
    }
    
    while (i > 0) {
        *str++ = buffer[--i];
    }
    *str = '\0';
}

void console_write_hex(uint64_t value) {
    char buf[17];
    static const char digits[] = "0123456789abcdef";
    int i = 15;
    buf[16] = '\0';
    
    if (value == 0) {
        serial_out(SERIAL_PORT, '0');
        return;
    }
    
    while (value > 0 && i >= 0) {
        buf[i] = digits[value & 0xF];
        value >>= 4;
        i--;
    }
    
    console_write_string(&buf[i + 1]);
}
