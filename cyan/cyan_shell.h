#ifndef CYAN_SHELL_H
#define CYAN_SHELL_H

#define CYAN_SHELL_LINE_MAX 1024
#define CYAN_SHELL_MAX_ARGS 16
#define CYAN_ESCAPE_CHAR '\\'


typedef enum {
    SHELL_PARSE_OK,
    SHELL_PARSE_EMPTY,
    SHELL_PARSE_UNTERMINATED_QUOTE,
    SHELL_PARSE_TRAILING_ESCAPE,
    SHELL_PARSE_TOO_MANY_ARGS
} ShellParseStatus;

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
    int childCount;
} ShellCommand;

typedef struct {
    char lineBuffer[CYAN_SHELL_LINE_MAX];
    int lineLength;
    ShellParse parse;
    
} CyanShell;

bool cyan_shell_line_decompose(ShellParse* parse, char* line);
bool cyan_shell_line_parse(char* line);
bool cyan_shell_read_line();

#endif // CYAN_SHELL_H