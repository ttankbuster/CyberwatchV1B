// test/test_cyan_shell_repl/shell_repl.c
#include "cyan_shell.h"
#include <stdio.h>
#include <string.h>

static int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("commands: help, version, echo, app\n");
    return 0;
}

static int cmd_version(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("Cyan OS v0.1\n");
    return 0;
}

static int cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++)
        printf("%s%s", argv[i], i + 1 < argc ? " " : "\n");
    if (argc == 1)
        printf("\n");
    return 0;
}

static int cmd_app_list(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("hello_cyan\npong\nreader\nRNG\n");
    return 0;
}

static int cmd_app_launch(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: app launch \"<name>\"\n");
        return 1;
    }
    printf("launching '%s'\n", argv[1]);
    return 0;
}

static const ShellCommand APP_CMDS[] = {
    {"list", "list installed apps", cmd_app_list, NULL, NULL},
    {"launch", "launch an app", cmd_app_launch, NULL, "<name / id>"},
    {NULL}
};

static const ShellCommand ROOT_CMDS[] = {
    {"help", "show commands", cmd_help, NULL, NULL},
    {"version", "show version", cmd_version, NULL, NULL},
    {"echo", "echo arguments", cmd_echo, NULL, NULL},
    {"app", "app management", NULL, APP_CMDS, NULL},
    {NULL}
};

static void show_parse(const ShellParse* parse) {
    printf("  argc=%d", parse->argc);
    for (int i = 0; i < parse->argc; i++)
        printf(" [%s]", parse->argv[i]);
    printf("\n");
}

int main(void) {
    char line[CYAN_SHELL_LINE_MAX];
    bool verbose = true;

    printf("Cyan shell harness. ':q' to quit, ':v' to toggle parse output.\n");

    while (1) {
        printf("cyan> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin))
            break;

        if (strncmp(line, ":q", 2) == 0)
            break;
        if (strncmp(line, ":v", 2) == 0) {
            verbose = !verbose;
            printf("parse output %s\n", verbose ? "on" : "off");
            continue;
        }

        ShellParse parse;
        cyan_shell_line_decompose(&parse, line);

        if (verbose)
            show_parse(&parse);

        switch (parse.status) {
        case SHELL_PARSE_EMPTY:
            continue;
        case SHELL_PARSE_UNTERMINATED_QUOTE:
            printf("error: unterminated quote\n");
            continue;
        case SHELL_PARSE_TRAILING_ESCAPE:
            printf("error: trailing backslash\n");
            continue;
        case SHELL_PARSE_TOO_MANY_ARGS:
            printf("error: too many arguments\n");
            continue;
        case SHELL_PARSE_OK:
            break;
        }

        int result = cyan_shell_dispatch(ROOT_CMDS, parse.argc, parse.argv);
        if (result == SHELL_ERR_UNKNOWN)
            printf("unknown command: %s\n", parse.argv[0]);
        else if (result == SHELL_ERR_USAGE)
            printf("usage error\n");
    }

    return 0;
}