// cyan_shell.c
#include "cyan_shell.h"
#include "../app_handling/app_handler.h"
#include "cyan_os.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://brennan.io/2015/01/16/write-a-shell-in-c/
// http://kblomqvist.github.io/2013/03/21/creating-beatiful-command-line-interfaces-for-embedded-systems-part1

bool cyan_shell_line_decompose(ShellParse* parse, char* line) {
    parse->argc = 0;
    parse->status = SHELL_PARSE_EMPTY;
    int cursor = 0;
    bool escaped = false;
    bool in_word = false;
    bool in_quote = false;

    for (int i = 0; i < CYAN_SHELL_LINE_MAX; i++) {
        int char_int = line[i];
        if (char_int == '\0' || char_int == '\n' || char_int == '\r') {
            break;
        }
        if (escaped) {
            if (!in_word) {
                if (parse->argc >= CYAN_SHELL_MAX_ARGS) {
                    parse->status = SHELL_PARSE_TOO_MANY_ARGS;
                    return false;
                }
                parse->argv[parse->argc++] = &line[cursor];
                in_word = true;
            }
            line[cursor++] = (char)char_int;
            escaped = false;
            continue;
        }
        if (char_int == CYAN_ESCAPE_CHAR) {
            if (!in_word) {
                if (parse->argc >= CYAN_SHELL_MAX_ARGS) {
                    parse->status = SHELL_PARSE_TOO_MANY_ARGS;
                    return false;
                }
                parse->argv[parse->argc++] = &line[cursor];
                in_word = true;
            }
            escaped = true;
            continue;
        }

        if (char_int == ' ') {
            if (in_quote) {
                if (!in_word) {
                    if (parse->argc >= CYAN_SHELL_MAX_ARGS) {
                        parse->status = SHELL_PARSE_TOO_MANY_ARGS;
                        return false;
                    }
                    parse->argv[parse->argc++] = &line[cursor];
                    in_word = true;
                }
                line[cursor++] = (char)char_int;
                continue;
            }

            if (in_word) {
                line[cursor++] = '\0';
                in_word = false;
            }
            continue;
        }

        if (char_int == '"') {
            if (!in_word) {
                if (parse->argc >= CYAN_SHELL_MAX_ARGS) {
                    parse->status = SHELL_PARSE_TOO_MANY_ARGS;
                    return false;
                }
                parse->argv[parse->argc++] = &line[cursor];
                in_word = true;
            }
            in_quote = !in_quote;
            continue;
        }

        if (!in_word) {
            if (parse->argc >= CYAN_SHELL_MAX_ARGS) {
                parse->status = SHELL_PARSE_TOO_MANY_ARGS;
                return false;
            }
            parse->argv[parse->argc++] = &line[cursor];
            in_word = true;
        }

        line[cursor++] = (char)char_int;
    }

    if (escaped) {
        parse->status = SHELL_PARSE_TRAILING_ESCAPE;
        return false;
    }
    if (in_word) {
        line[cursor++] = '\0';
    }
    if (in_quote) {
        parse->status = SHELL_PARSE_UNTERMINATED_QUOTE;
        return false;
    }
    if (parse->argc == 0) {
        parse->status = SHELL_PARSE_EMPTY;
        return false;
    }

    parse->status = SHELL_PARSE_OK;
    return true;
}

int cyan_shell_dispatch(const ShellCommand* table, int argc, char** argv) {
    if (argc == 0)
        return SHELL_ERR_USAGE;
    for (const ShellCommand* cmd = table; cmd->name; cmd++) {
        if (strcmp(argv[0], cmd->name) != 0)
            continue;
        if (cmd->children)
            return cyan_shell_dispatch(cmd->children, argc - 1, argv + 1);
        if (!cmd->handler)
            return SHELL_ERR_USAGE;
        return cmd->handler(argc, argv);
    }
    return SHELL_ERR_UNKNOWN;
}

#define HELP_TREE_STEP 4
#define HELP_TEXT_COL 28
#define HELP_MAX_DEPTH 16

static const char HELP_BRANCH_MID[] = "|-- ";
static const char HELP_BRANCH_END[] = "`-- ";
static const char HELP_PIPE[] = "|   ";
static const char HELP_GAP[] = "    ";

static int help_put(char* out, size_t cap, size_t* len, const char* str, size_t n) {
    size_t add = str ? strlen(str) : n;
    if (*len + add >= cap)
        return SHELL_ERR_USAGE;
    if (str)
        memcpy(out + *len, str, add);
    else
        memset(out + *len, ' ', add);
    *len += add;
    out[*len] = '\0';
    return SHELL_OK;
}

static int help_walk(
    const ShellCommand* commands, const char* prefix, int depth, char* out, size_t cap, size_t* len
) {
    if (depth >= HELP_MAX_DEPTH)
        return SHELL_ERR_UNKNOWN;

    const ShellCommand* last = commands;
    for (const ShellCommand* c = commands; c->name; c++)
        last = c;

    for (const ShellCommand* cmd = commands; cmd->name; cmd++) {
        bool is_last = (cmd == last);
        int status = help_put(out, cap, len, prefix, 0);
        if (status == SHELL_OK)
            status = help_put(out, cap, len, is_last ? HELP_BRANCH_END : HELP_BRANCH_MID, 0);
        if (status == SHELL_OK)
            status = help_put(out, cap, len, cmd->name, 0);
        if (status == SHELL_OK && cmd->args) {
            if ((status = help_put(out, cap, len, " ", 0)) == SHELL_OK)
                status = help_put(out, cap, len, cmd->args, 0);
        }
        if (status != SHELL_OK)
            return status;

        size_t cells = (size_t)(HELP_TREE_STEP * (depth + 1)) + strlen(cmd->name);
        if (cmd->args)
            cells += 1 + strlen(cmd->args);
        size_t pad = cells < HELP_TEXT_COL ? HELP_TEXT_COL - cells : 1;

        status = help_put(out, cap, len, NULL, pad);
        if (status == SHELL_OK)
            status = help_put(out, cap, len, cmd->help ? cmd->help : "", 0);
        if (status == SHELL_OK)
            status = help_put(out, cap, len, "\n", 0);
        if (status != SHELL_OK)
            return status;

        if (cmd->children) {
            char child_prefix[128];
            int written = snprintf(
                child_prefix, sizeof(child_prefix), "%s%s", prefix, is_last ? HELP_GAP : HELP_PIPE
            );
            if (written < 0 || (size_t)written >= sizeof(child_prefix))
                return SHELL_ERR_USAGE;
            status = help_walk(cmd->children, child_prefix, depth + 1, out, cap, len);
            if (status != SHELL_OK)
                return status;
        }
    }
    return SHELL_OK;
}

int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char output[1024];
    output[0] = '\0';
    size_t length = 0;

    int status = help_walk(CYAN_SHELL_ROOT_CMDS, "", 0, output, sizeof(output), &length);
    if (status != SHELL_OK)
        return status;

    // cyan_log() caps each message at 256 bytes, do one line at a time
    cyan_log(VERBOSE_SHELL, "Commands:");
    for (char* line = output; *line;) {
        char* nl = strchr(line, '\n');
        if (nl)
            *nl = '\0';
        cyan_log(VERBOSE_SHELL, "%s", line);
        if (!nl)
            break;
        line = nl + 1;
    }
    return SHELL_OK;
}

int cmd_version(int argc, char** argv) {
    (void)argc;
    (void)argv;
    cyan_log(VERBOSE_SHELL, "%s", CYAN_VERSION);
    return 0;
}

static int cmd_app_perm_add(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("COMMAND: cmd_app_perm_add\n");
    return 0;
}

static int cmd_app_perm_rm(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("COMMAND: cmd_app_perm_rm\n");
    return 0;
}

static int cmd_app_perm_list(int argc, char** argv) {
    (void)argc;
    (void)argv;
    printf("COMMAND: cmd_app_perm_list\n");
    return 0;
}

static int cmd_app_list(int argc, char** argv) {
    (void)argc;
    (void)argv;
    AppHandler* app_handler = cyan_get_app_handler();
    app_handler_show_apps(app_handler, true);
    return 0;
}

bool is_number(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    char* endptr;
    strtod(str, &endptr);
    return (endptr != str && *endptr == '\0');
}

static int cmd_app_launch(int argc, char** argv) {
    if (argc < 2) {
        return SHELL_ERR_USAGE;
    }
    char* app = argv[1];
    if (is_number(app)) {
        int app_id = (int)strtol(app, NULL, 10);
        cyan_launch_app_id(app_id);
    } else {
        cyan_launch_app_name(app);
    }
    return 0;
}

static int cmd_app_exit(int argc, char** argv) {
    if (!cyan_is_app_running()) {
        cyan_log(VERBOSE_SHELL, "No app currently running.");
        return 0;
    } else if (cyan_exit_app()) {
        cyan_log(VERBOSE_LOW, "App exited successfully.");
        return 0;
    } else {
        return 1;
    }
}

static const ShellCommand APP_PERMISSION_CMDS[] = {
    {"add", "Add a permission", cmd_app_perm_add, NULL, "<permission>"},
    {"remove", "Remove a permission", cmd_app_perm_rm, NULL, "<permission>"},
    {"list", "List granted permissions", cmd_app_perm_list, NULL, NULL},
    {NULL}
};

static const ShellCommand APP_CMDS[] = {
    {"list", "List installed apps", cmd_app_list, NULL, NULL},
    {"launch", "Launch an app", cmd_app_launch, NULL, "<name>"},
    {"exit", "Exit the currently running app", cmd_app_exit, NULL, NULL},
    {"permission", "Manage permissions", NULL, APP_PERMISSION_CMDS, NULL},
    {NULL}
};

const ShellCommand CYAN_SHELL_ROOT_CMDS[] = {
    {"help", "Show commands", cmd_help, NULL, NULL},
    {"version", "Show version", cmd_version, NULL, NULL},
    {"app", "App management", NULL, APP_CMDS, NULL},
    {NULL}
};
