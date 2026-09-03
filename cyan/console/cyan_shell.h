#ifndef CYAN_SHELL_H
#define CYAN_SHELL_H
#define CYAN_SHELL_LINE_MAX 1024
#define CYAN_SHELL_MAX_ARGS 16
#define CYAN_ESCAPE_CHAR '\\'
#include <stdbool.h>

typedef enum {
    SHELL_PARSE_OK,
    SHELL_PARSE_EMPTY,
    SHELL_PARSE_UNTERMINATED_QUOTE,
    SHELL_PARSE_TRAILING_ESCAPE,
    SHELL_PARSE_TOO_MANY_ARGS
} ShellParseStatus;

typedef enum { SHELL_VERBOSE_LOW, SHELL_VERBOSE_HIGH } ShellVerbosity;

typedef enum { SHELL_OK = 0, SHELL_ERR_UNKNOWN = -1, SHELL_ERR_USAGE = -2 } ShellDispatchStatus;

typedef struct {
    int argc;
    char* argv[CYAN_SHELL_MAX_ARGS];
    ShellParseStatus status;
} ShellParse;

typedef struct ShellCommand {
    const char* name;
    const char* help;
    int (*handler)(int argc, char** argv);
    const struct ShellCommand* children;
    const char* args;
} ShellCommand;

bool cyan_shell_line_decompose(ShellParse* parse, char* line);

int cyan_shell_dispatch(const ShellCommand* table, int argc, char** argv);

extern const ShellCommand CYAN_SHELL_ROOT_CMDS[];

#endif // CYAN_SHELL_H
