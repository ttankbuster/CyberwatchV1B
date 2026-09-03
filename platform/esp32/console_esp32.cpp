// console_esp32.cpp - ESP32 console backend for the interactive Cyan shell.
// Routes the shell over the USB-CDC serial link. Intentionally thin - the line
// editing / prompt logic all lives in cyan/console/cyan_console.c.
#include <Arduino.h>

#include "cyan_console_io.h" // has its own extern "C" guard

extern "C" void console_io_open(void) {
    /* Serial is brought up in setup(); nothing to do here. */
}

extern "C" void console_io_close(void) {}

extern "C" bool console_io_read(int* out_byte) {
    if (Serial.available() <= 0)
        return false;
    int c = Serial.read();
    if (c < 0)
        return false;
    *out_byte = c & 0xFF;
    return true;
}

extern "C" void console_io_write(const char* bytes, size_t len) {
    Serial.write(reinterpret_cast<const uint8_t*>(bytes), len);
}
