//cyan_shell.c
#include "cyan_os.h"
#include "cyan_shell.h"

//https://brennan.io/2015/01/16/write-a-shell-in-c/
//http://kblomqvist.github.io/2013/03/21/creating-beatiful-command-line-interfaces-for-embedded-systems-part1


bool cyan_shell_line_decompose(ShellParse* parse, char* line) {
    parse->argc = 0;
    parse->status = SHELL_PARSE_EMPTY;
    int cursor = 0;
    bool escaped = false;
    bool in_word = false;
    bool in_quote = false;
    const char* start_word = NULL;

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
            line[cursor++] = (char) char_int;
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
                line[cursor++] = (char) char_int;
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

        line[cursor++] = (char) char_int;
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

_Bool cyan_shell_line_parse(char* line) {
    if (!line) {
        return false;
    } else if (line == NULL) {
        return false;
    }

    return true;
}

_Bool cyan_shell_read_line() {
  int bufsize = CYAN_SHELL_LINE_MAX;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  
}