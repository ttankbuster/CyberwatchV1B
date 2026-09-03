// cyan_console.c
#include "cyan_console.h"
#include "cyan_console_io.h"
#include "cyan_shell.h"
#include "log.h"
#include <string.h>

#define CYAN_CONSOLE_PROMPT "cyan> "
#define CYAN_CONSOLE_PROMPT_LEN (sizeof(CYAN_CONSOLE_PROMPT) - 1)

typedef struct {
    char line[CYAN_SHELL_LINE_MAX];
    int len;
    bool promptOnScreen;
    bool dispatching;
    bool sawCR;
    bool started;
} CyanConsole;

static CyanConsole g_console;

static void console_puts(const char* s) { console_io_write(s, strlen(s)); }

static void erase_input_line(void) {
    if (!g_console.promptOnScreen)
        return;

    char blank[CYAN_CONSOLE_PROMPT_LEN + CYAN_SHELL_LINE_MAX + 2];
    int visible = (int)CYAN_CONSOLE_PROMPT_LEN + g_console.len;
    int n = 0;
    blank[n++] = '\r';
    for (int i = 0; i < visible && n < (int)sizeof(blank) - 1; i++)
        blank[n++] = ' ';
    blank[n++] = '\r';
    console_io_write(blank, (size_t)n);
    g_console.promptOnScreen = false;
}

static void draw_input_line(void) {
    console_puts(CYAN_CONSOLE_PROMPT);
    if (g_console.len > 0)
        console_io_write(g_console.line, (size_t)g_console.len);
    g_console.promptOnScreen = true;
}

static void console_log_listener(VerbosityLevel level, const char* message) {
    (void)level;
    bool restore = g_console.promptOnScreen && !g_console.dispatching;
    if (restore)
        erase_input_line();
    console_puts(message);
    console_puts("\r\n");
    if (restore)
        draw_input_line();
}

static void run_line(void) {
    ShellParse parse;
    g_console.line[g_console.len] = '\0';
    cyan_shell_line_decompose(&parse, g_console.line);

    switch (parse.status) {
    case SHELL_PARSE_EMPTY:
        break;
    case SHELL_PARSE_UNTERMINATED_QUOTE:
        console_puts("error: unterminated quote\r\n");
        break;
    case SHELL_PARSE_TRAILING_ESCAPE:
        console_puts("error: trailing backslash\r\n");
        break;
    case SHELL_PARSE_TOO_MANY_ARGS:
        console_puts("error: too many arguments\r\n");
        break;
    case SHELL_PARSE_OK: {
        g_console.dispatching = true;
        int rc = cyan_shell_dispatch(CYAN_SHELL_ROOT_CMDS, parse.argc, parse.argv);
        g_console.dispatching = false;
        if (rc == SHELL_ERR_UNKNOWN) {
            console_puts("unknown command: ");
            console_puts(parse.argv[0]);
            console_puts("\r\n");
        } else if (rc == SHELL_ERR_USAGE) {
            console_puts("incomplete command: ");
            console_puts(parse.argv[0]);
            console_puts("\r\n");
        }
        break;
    }
    }
}

static void handle_char(int c) {
    if (c == '\n' && g_console.sawCR) {
        g_console.sawCR = false;
        return;
    }
    g_console.sawCR = (c == '\r');

    if (c == '\r' || c == '\n') {
        console_puts("\r\n");
        g_console.promptOnScreen = false;
        run_line();
        g_console.len = 0;
        g_console.line[0] = '\0';
        draw_input_line();
        return;
    }

    if (c == '\b' || c == 0x7F) {
        if (g_console.len > 0) {
            g_console.len--;
            g_console.line[g_console.len] = '\0';
            console_puts("\b \b");
        }
        return;
    }

    if (c >= 0x20 && c < 0x7F && g_console.len < CYAN_SHELL_LINE_MAX - 1) {
        char ch = (char)c;
        g_console.line[g_console.len++] = ch;
        g_console.line[g_console.len] = '\0';
        console_io_write(&ch, 1);
    }
}

void cyan_console_init(void) {
    if (g_console.started)
        return;
    memset(&g_console, 0, sizeof(g_console));
    g_console.started = true;
    console_io_open();
    log_add_listener(console_log_listener, VERBOSE_HIGH);
    draw_input_line();
}

void cyan_console_poll(void) {
    if (!g_console.started)
        return;
    int c;
    while (console_io_read(&c))
        handle_char(c);
}

void cyan_console_shutdown(void) {
    if (!g_console.started)
        return;
    console_io_close();
    g_console.started = false;
}
