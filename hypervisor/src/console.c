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
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t serial_in(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void serial_init(void) {
    serial_out(SERIAL_PORT + 1, 0x00);  // Disable all interrupts
    serial_out(SERIAL_PORT + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    serial_out(SERIAL_PORT + 0, 0x03);  // Set divisor to 3 (lo byte)
    serial_out(SERIAL_PORT + 1, 0x00);  // Set divisor to 3 (hi byte)
    serial_out(SERIAL_PORT + 3, 0x03);  // 8 bits, no parity, one stop bit
    serial_out(SERIAL_PORT + 2, 0xC7);  // Enable FIFO
    serial_out(SERIAL_PORT + 4, 0x0B);  // IRQs enabled, RTS/DSR set
}

void serial_write_string(const char *str) {
    while (*str) {
        while ((serial_in(SERIAL_PORT + 5) & 0x20) == 0);  // Wait for transmit buffer
        serial_out(SERIAL_PORT, *str++);
    }
}

void console_init(void) {
    serial_init();
    vga_row = 0;
    vga_col = 0;
    
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (VGA_COLOR << 8) | ' ';
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
        console_write_char(*str++);
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
        console_write_char('0');
        return;
    }
    
    while (value > 0 && i >= 0) {
        buf[i] = digits[value & 0xF];
        value >>= 4;
        i--;
    }
    
    console_write_string(&buf[i + 1]);
}
