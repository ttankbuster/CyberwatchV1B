#ifndef CYAN_CONSOLE_IO_H
#define CYAN_CONSOLE_IO_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Thin per-platform console backend for the interactive Cyan shell.

void console_io_open(void);
void console_io_close(void);
bool console_io_read(int* out_byte);
void console_io_write(const char* bytes, size_t len);

#ifdef __cplusplus
}
#endif

#endif // CYAN_CONSOLE_IO_H
