#include "cyan_settings.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Exemplar settings file

$CYAN_SETTINGS_VERSION=1;                   // special character for reserved settings - the user
                                            // can write CYAN_VERSION if they want as their own
                                            // variable and it wont conflict because the $ wasnt
                                            // used - though it would be strongly discouraged - it
                                            // just keeps a tidy namespace

date_format = $DATE_DMY|$DATE_MDY|$DATE_YMD // inbuilt DATE enum for day/month/year, monty/day/year
                                            // etc. implementation of arrays, which must be a typed
                                            // array but can hold strings, integers, hex

tab_order=[0,1,2,3];                        // for ordering the tabs - must be checked robustly at
                                            //runtime

font_override="fonts/GFX_Arial";            // for overriding fonts -  stubbed out for now to
                                            // implement later

dev_mode=false;                             // simple boolean implentation

accent_color=#34A5B4;                       // hex implementation using # hastags

// watchface settings                       // c style comments in cyan settings files

analogue_roman=false; analogue_all_numerals=true; analogue=true;  // multiple statements in a single
                                                                  // line are allowed

*/

static int interpret_settings_key(char* str, bool reserved) {
    printf("INTERPRETING KEY: {%s}\n", str);
}

static int interpret_settings_value(char* str) {
    printf("INTERPRETING VALUE: {%s}\n", str);
    if (str == NULL || str[0] == '\0') {
        return CYAN_SETTINGS_INTERPRET_EMPTY_VALUE;
    }
    switch (str[0]) {
    case '"':
        printf("value {%s} is string\n", str);
        break;
    case '[':
        printf("value {%s} is array\n", str);
        break;
    case '#':
        printf("value {%s} is hex\n", str);
        break;
    case '$':
        printf("value {%s} is inbuilt\n", str);
        break;
    default:
        if (isdigit(str[0])) {
            bool is_num = true;
            for (size_t i = 0; i < strlen(str); i++) {
                if (!isdigit(str[i])) {
                    is_num = false;
                }
            }
            if (is_num) {
                printf("value {%s} is number\n", str);
            } else {
                printf("error value {%s} is unknown\n", str);
            }
        } else if (strcmp(str, "false") == 0) {
            printf("value {%s} is false bool\n", str);
        } else if (strcmp(str, "true") == 0) {
            printf("value {%s} is true bool\n", str);
        } else {
            printf("error: value {%s} is unknown\n", str);
        }
        break;
    }
    return CYAN_SETTINGS_INTERPRET_OK;
}

static int interpret_pairs(int pair_count, SettingPair* pairs) {
    for (size_t i = 0; i < pair_count; i++) {
        SettingPair pair = pairs[i];
        interpret_settings_key(pair.key, pair.reserved);
        interpret_settings_value(pair.value);
    }
}

static bool range_is_blank(const char* line, size_t begin, size_t end) {
    for (size_t i = begin; i < end; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }
    return true;
}

static void free_pairs(SettingPair* pairs, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(pairs[i].key);
        free(pairs[i].value);
    }
    free(pairs);
}

static char* copy_text(const char* text, size_t length) {
    char* copy = malloc(length + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, length);
    copy[length] = '\0';
    return copy;
}

static SettingParseError
parse_statement(const char* line, size_t begin, size_t end, SettingPair* pair) {
    char key[CYAN_SETTINGS_LINE_MAX];
    char value[CYAN_SETTINGS_LINE_MAX];
    size_t key_length = 0;
    size_t value_length = 0;
    bool reserved = false;
    size_t i = begin;
    while (i < end && (line[i] == ' ' || line[i] == '\t')) {
        i++;
    }
    while (i < end && line[i] != '=') {
        char c = line[i];
        i++;
        if (c == '"') {
            return CYAN_SETTINGS_PARSE_ERROR_KEY_QUOTED;
        }
        if (c == '$') {
            reserved = true;
            continue;
        }
        if (key_length + 1 >= CYAN_SETTINGS_LINE_MAX) {
            return CYAN_SETTINGS_PARSE_ERROR_KEY_TOO_LONG;
        }
        key[key_length++] = c;
    }
    if (i >= end) {
        return CYAN_SETTINGS_PARSE_ERROR_MISSING_EQUALS;
    }
    i++;
    while (key_length > 0 && (key[key_length - 1] == ' ' || key[key_length - 1] == '\t')) {
        key_length--;
    }
    if (key_length == 0) {
        return CYAN_SETTINGS_PARSE_ERROR_EMPTY_KEY;
    }
    bool inside_quote = false;
    while (i < end) {
        char c = line[i];
        i++;
        if (c == '$') {
            reserved = true;
            continue;
        }
        if (c == '"') {
            inside_quote = !inside_quote;
        } else if (c == ' ') {
            continue;
        } else if (c == '=' && !inside_quote) {
            continue;
        }
        if (value_length + 1 >= CYAN_SETTINGS_LINE_MAX) {
            return CYAN_SETTINGS_PARSE_ERROR_VALUE_TOO_LONG;
        }
        value[value_length++] = c;
    }
    if (inside_quote) {
        return CYAN_SETTINGS_PARSE_ERROR_UNTERMINATED_QUOTE;
    }
    while (value_length > 0 &&
           (value[value_length - 1] == ' ' || value[value_length - 1] == '\t')) {
        value_length--;
    }
    char* key_copy = copy_text(key, key_length);
    char* value_copy = copy_text(value, value_length);
    if (key_copy == NULL || value_copy == NULL) {
        free(key_copy);
        free(value_copy);
        return CYAN_SETTINGS_PARSE_ERROR_OUT_OF_MEMORY;
    }
    pair->key = key_copy;
    pair->value = value_copy;
    pair->reserved = reserved;
    return CYAN_SETTINGS_PARSE_OK;
}

SettingPair*
cyan_decompose_settings_line(char* line, size_t* out_count, SettingParseError* out_error) {
    *out_count = 0;
    *out_error = CYAN_SETTINGS_PARSE_OK;
    size_t max_pairs = 1;
    bool counting_inside_quote = false;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == '"') {
            counting_inside_quote = !counting_inside_quote;
        } else if (line[i] == ';' && !counting_inside_quote) {
            max_pairs++;
        }
    }
    SettingPair* pairs = malloc(max_pairs * sizeof(SettingPair));
    if (pairs == NULL) {
        *out_error = CYAN_SETTINGS_PARSE_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    size_t pair_count = 0;
    size_t statement_begin = 0;
    bool inside_quote = false;
    bool finished = false;
    for (size_t i = 0; !finished; i++) {
        char c = line[i];
        if (c == '"') {
            inside_quote = !inside_quote;
        }
        bool at_end = (c == '\0');
        bool at_separator = (c == ';' && !inside_quote);
        if (at_end || at_separator) {
            if (!range_is_blank(line, statement_begin, i)) {
                SettingParseError err =
                    parse_statement(line, statement_begin, i, &pairs[pair_count]);
                if (err != CYAN_SETTINGS_PARSE_OK) {
                    *out_error = err;
                    free_pairs(pairs, pair_count);
                    return NULL;
                }
                pair_count++;
            }
            statement_begin = i + 1;
        }
        if (at_end) {
            finished = true;
        }
    }
    if (pair_count == 0) {
        free(pairs);
        return NULL;
    }
    *out_count = pair_count;
    return pairs;
}

SettingParseError cyan_read_line(char* line) {
    size_t pair_count = 0;
    SettingParseError error = CYAN_SETTINGS_PARSE_OK;
    SettingPair* pairs = cyan_decompose_settings_line(line, &pair_count, &error);
    for (size_t i = 0; i < pair_count; i++) {
        printf(
            "  [%zu] key='%s' value='%s' reserved=%d\n", i, pairs[i].key, pairs[i].value,
            pairs[i].reserved
        );
    }
    if (pairs == NULL) {
        return error;
    }
    interpret_pairs(pair_count, pairs);
    free_pairs(pairs, pair_count);
    return CYAN_SETTINGS_PARSE_OK;
}

int settings_tester(int argc, char** argv) {
    (void)argc;
    (void)argv;
    char line[] = "font_override=\"fonts/GFX_Arial\"";
    size_t pair_count = 0;
    cyan_read_line(line);
}