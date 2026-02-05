#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"

void console_init(void);
void console_write_char(char c);
void console_write_string(const char *str);
void serial_write_string(const char *str);
void itoa(int value, char *str, int base);
void console_write_hex(uint64_t value);

#endif
