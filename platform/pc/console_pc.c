// console_pc.c
#include "cyan_console_io.h"
#include <stdio.h>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HANDLE s_in = INVALID_HANDLE_VALUE;
static HANDLE s_out = INVALID_HANDLE_VALUE;
static bool s_allocated_console;

void console_io_open(void) {
    if (s_in != INVALID_HANDLE_VALUE)
        return;

#ifdef CYAN_CONSOLE_REUSE_TERMINAL
    if (GetConsoleWindow() == NULL && AllocConsole())
        s_allocated_console = true;
#else
    FreeConsole();
    if (AllocConsole())
        s_allocated_console = true;
#endif

    s_in = CreateFileA(
        "CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL
    );
    s_out = CreateFileA(
        "CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
        OPEN_EXISTING, 0, NULL
    );

    if (s_in != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(s_in, &mode)) {
            mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
            mode |= ENABLE_PROCESSED_INPUT;
            SetConsoleMode(s_in, mode);
        }
        FlushConsoleInputBuffer(s_in);
    }
    if (s_out != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(s_out, &mode))
            SetConsoleMode(s_out, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    }

    if (s_allocated_console) {
        SetConsoleTitleA("cyan console");
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        freopen_s(&f, "CONOUT$", "w", stderr);
        freopen_s(&f, "CONIN$", "r", stdin);
    }
}

void console_io_close(void) {
    if (s_in != INVALID_HANDLE_VALUE) {
        CloseHandle(s_in);
        s_in = INVALID_HANDLE_VALUE;
    }
    if (s_out != INVALID_HANDLE_VALUE) {
        CloseHandle(s_out);
        s_out = INVALID_HANDLE_VALUE;
    }
    if (s_allocated_console) {
        FreeConsole();
        s_allocated_console = false;
    }
}

bool console_io_read(int* out_byte) {
    if (s_in == INVALID_HANDLE_VALUE)
        return false;

    for (;;) {
        DWORD pending = 0;
        if (!GetNumberOfConsoleInputEvents(s_in, &pending) || pending == 0)
            return false;

        INPUT_RECORD rec;
        DWORD got = 0;
        if (!ReadConsoleInputA(s_in, &rec, 1, &got) || got == 0)
            return false;

        if (rec.EventType != KEY_EVENT || !rec.Event.KeyEvent.bKeyDown)
            continue;

        char ch = rec.Event.KeyEvent.uChar.AsciiChar;
        if (ch == 0)
            continue;

        *out_byte = (unsigned char)ch;
        return true;
    }
}

void console_io_write(const char* bytes, size_t len) {
    if (s_out == INVALID_HANDLE_VALUE) {
        fwrite(bytes, 1, len, stdout);
        fflush(stdout);
        return;
    }
    DWORD written = 0;
    WriteConsoleA(s_out, bytes, (DWORD)len, &written, NULL);
}

#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

static struct termios s_saved_termios;
static int s_saved_flags;
static bool s_open;

void console_io_open(void) {
    if (s_open)
        return;
    tcgetattr(STDIN_FILENO, &s_saved_termios);
    struct termios raw = s_saved_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    s_saved_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, s_saved_flags | O_NONBLOCK);
    s_open = true;
}

void console_io_close(void) {
    if (!s_open)
        return;
    tcsetattr(STDIN_FILENO, TCSANOW, &s_saved_termios);
    fcntl(STDIN_FILENO, F_SETFL, s_saved_flags);
    s_open = false;
}

bool console_io_read(int* out_byte) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) != 1)
        return false;
    *out_byte = c;
    return true;
}

void console_io_write(const char* bytes, size_t len) {
    ssize_t w = write(STDOUT_FILENO, bytes, len);
    (void)w;
}
#endif
