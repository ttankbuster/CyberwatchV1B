#include "cyan_settings.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CyanSettings g_settings;

/* Exemplar settings file

// $default token used to reset a setting to its built-in default value. For example, `settings set accent_color $default` will reset the accent color to its default.

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

void platform_store_resolved_path(const char* relativePath, char* outBuffer, size_t bufferSize);
void platform_ensure_directory(const char* relativePath);

#define SETTINGS_DIR ".cyanos"
#define SETTINGS_FILE_RELATIVE SETTINGS_DIR "/settings.txt"
#define SETTINGS_FILE_TMP_RELATIVE SETTINGS_DIR "/settings.txt.tmp"
#define SETTINGS_RESOLVED_PATH_MAX 512


typedef struct {
    const char* key;
    SettingType type;
    void* field;
    size_t size;
    bool reserved;
} SettingRegistryEntry;

static const SettingRegistryEntry SETTINGS_REGISTRY[] = {
    //{key,                  type,         field,              size,                            reserved }
    {"CYAN_SETTINGS_VERSION", SETTING_INT, &g_settings.version, sizeof(g_settings.version), true},
    {"date_format", SETTING_ENUM, &g_settings.dateFormat, sizeof(g_settings.dateFormat), false},
    {"tab_order", SETTING_INT_ARRAY, &g_settings.tabOrder, sizeof(g_settings.tabOrder), false},
    {"font_override", SETTING_STRING, &g_settings.fontOverride, sizeof(g_settings.fontOverride), false},
    {"dev_mode", SETTING_BOOL, &g_settings.devMode, sizeof(g_settings.devMode), false},
    {"accent_color", SETTING_HEX, &g_settings.accentColor, sizeof(g_settings.accentColor), false},
    {"analogue", SETTING_BOOL, &g_settings.analogue, sizeof(g_settings.analogue), false},
    {"analogue_roman", SETTING_BOOL, &g_settings.analogueRoman, sizeof(g_settings.analogueRoman), false},
    {"analogue_all_numerals", SETTING_BOOL, &g_settings.analogueAllNumerals,
     sizeof(g_settings.analogueAllNumerals), false},
};
#define SETTINGS_REGISTRY_COUNT (sizeof(SETTINGS_REGISTRY) / sizeof(SETTINGS_REGISTRY[0]))

static const SettingRegistryEntry* find_registry_entry(const char* key) {
    for (size_t i = 0; i < SETTINGS_REGISTRY_COUNT; i++) {
        if (strcmp(SETTINGS_REGISTRY[i].key, key) == 0) {
            return &SETTINGS_REGISTRY[i];
        }
    }
    return NULL;
}

// Case-insensitive match for the "$default" value token, which resets a single setting to its
// built-in default (e.g. `settings set accent_color $default`, `$DEFAULT`, `$Default`, ...).
static bool is_default_token(const char* value) {
    static const char* TOKEN = "$default";
    if (value == NULL) {
        return false;
    }
    for (size_t i = 0; TOKEN[i] != '\0'; i++) {
        if (value[i] == '\0' || tolower((unsigned char)value[i]) != TOKEN[i]) {
            return false;
        }
    }
    return value[strlen(TOKEN)] == '\0';
}

static void reset_entry_to_default(const SettingRegistryEntry* entry);

bool is_integer(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    if (*str == '-' || *str == '+') {
        str++;
    }
    while (*str != '\0') {
        if (!isdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

bool is_string(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    if (*str == '"') {
        size_t len = strlen(str);
        return len > 1 && str[len - 1] == '"';
    }
    return false;
}

bool is_hex(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    if (*str == '#') {
        str++;
    }
    if (*str == '\0') {
        return false;
    }
    while (*str != '\0') {
        if (!isxdigit((unsigned char)*str)) {
            return false;
        }
        str++;
    }
    return true;
}

bool is_bool(const char* str) {
    return str != NULL && (strcmp(str, "true") == 0 || strcmp(str, "false") == 0);
}

bool is_array(const char* str) {
    if (str == NULL || *str == '\0') {
        return false;
    }
    if (str[0] == '[') {
        size_t len = strlen(str);
        return len > 1 && str[len - 1] == ']';
    }
    return false;
}

static const char* interpret_error_string(int error) {
    switch (error) {
    case CYAN_SETTINGS_INTERPRET_OK:
        return "ok";
    case CYAN_SETTINGS_INTERPRET_EMPTY_VALUE:
        return "empty value";
    case CYAN_SETTINGS_INTERPRET_INVALID_KEY:
        return "invalid key";
    case CYAN_SETTINGS_INTERPRET_CONFLICTING_KEY:
        return "key conflicts with a value literal or reserved status";
    case CYAN_SETTINGS_INTERPRET_RESERVATION_VIOLATION:
        return "key is reserved but wasn't marked with '$'";
    case CYAN_SETTINGS_INTERPRET_UNKNOWN_KEY:
        return "unknown key";
    case CYAN_SETTINGS_INTERPRET_INVALID_VALUE:
        return "value doesn't match the key's type";
    default:
        return "unknown error";
    }
}

static const char* parse_error_string(SettingParseError error) {
    switch (error) {
    case CYAN_SETTINGS_PARSE_OK:
        return "ok";
    case CYAN_SETTINGS_PARSE_ERROR_KEY_QUOTED:
        return "key cannot be quoted";
    case CYAN_SETTINGS_PARSE_ERROR_UNTERMINATED_QUOTE:
        return "unterminated quote";
    case CYAN_SETTINGS_PARSE_ERROR_MISSING_EQUALS:
        return "missing '='";
    case CYAN_SETTINGS_PARSE_ERROR_EMPTY_KEY:
        return "empty key";
    case CYAN_SETTINGS_PARSE_ERROR_KEY_TOO_LONG:
        return "key too long";
    case CYAN_SETTINGS_PARSE_ERROR_VALUE_TOO_LONG:
        return "value too long";
    case CYAN_SETTINGS_PARSE_ERROR_OUT_OF_MEMORY:
        return "out of memory";
    default:
        return "unknown error";
    }
}

static int lookup_setting_key(const char* key, bool reserved, SettingSpec* out_spec) {
    for (size_t i = 0; i < SETTINGS_REGISTRY_COUNT; i++) {
        const SettingRegistryEntry* entry = &SETTINGS_REGISTRY[i];
        if (strcmp(entry->key, key) != 0) {
            continue;
        }
        if (entry->reserved && !reserved) {
            return CYAN_SETTINGS_INTERPRET_RESERVATION_VIOLATION;
        }
        if (!entry->reserved && reserved) {
            return CYAN_SETTINGS_INTERPRET_CONFLICTING_KEY;
        }
        out_spec->key = entry->key;
        out_spec->type = entry->type;
        out_spec->value = entry->field;
        return CYAN_SETTINGS_INTERPRET_OK;
    }
    return CYAN_SETTINGS_INTERPRET_UNKNOWN_KEY;
}

static int interpret_settings_key(const char* str, bool reserved, SettingSpec* out_spec) {
    switch (str[0]) {
    case '"':
    case '[':
    case '#':
        return CYAN_SETTINGS_INTERPRET_INVALID_KEY;
    default:
        break;
    }
    if (isdigit((unsigned char)str[0])) {
        return CYAN_SETTINGS_INTERPRET_INVALID_KEY;
    }
    if (is_bool(str) || is_string(str)) {
        return CYAN_SETTINGS_INTERPRET_CONFLICTING_KEY;
    }
    return lookup_setting_key(str, reserved, out_spec);
}

static bool parse_int_array(const char* str, int* out, size_t capacity) {
    size_t len = strlen(str);
    if (len < 2 || str[0] != '[' || str[len - 1] != ']') {
        return false;
    }
    char body[CYAN_SETTINGS_LINE_MAX];
    size_t bodyLen = len - 2;
    if (bodyLen >= sizeof(body)) {
        return false;
    }
    memcpy(body, str + 1, bodyLen);
    body[bodyLen] = '\0';

    size_t index = 0;
    char* cursor = body;
    while (*cursor != '\0' && index < capacity) {
        char* comma = strchr(cursor, ',');
        if (comma != NULL) {
            *comma = '\0';
        }
        while (*cursor == ' ' || *cursor == '\t') {
            cursor++;
        }
        if (!is_integer(cursor)) {
            return false;
        }
        out[index++] = (int)strtol(cursor, NULL, 10);
        if (comma == NULL) {
            break;
        }
        cursor = comma + 1;
    }
    return true;
}

static bool parse_hex_color(const char* str, uint8_t* out) {
    const char* digits = (str[0] == '#') ? str + 1 : str;
    if (strlen(digits) != 6) {
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if (!isxdigit((unsigned char)digits[i])) {
            return false;
        }
    }
    unsigned long value = strtoul(digits, NULL, 16);
    out[0] = (uint8_t)((value >> 16) & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)(value & 0xFF);
    return true;
}

static bool parse_date_format(const char* str, DateFormat* out) {
    if (strcmp(str, "$DATE_DMY") == 0) {
        *out = DATE_DMY;
    } else if (strcmp(str, "$DATE_MDY") == 0) {
        *out = DATE_MDY;
    } else if (strcmp(str, "$DATE_YMD") == 0) {
        *out = DATE_YMD;
    } else {
        return false;
    }
    return true;
}

static const char* date_format_string(DateFormat format) {
    switch (format) {
    case DATE_DMY:
        return "$DATE_DMY";
    case DATE_MDY:
        return "$DATE_MDY";
    case DATE_YMD:
        return "$DATE_YMD";
    default:
        return "$DATE_DMY";
    }
}

static int apply_setting_value(const SettingSpec* spec, const char* rawValue) {
    if (rawValue == NULL || rawValue[0] == '\0') {
        return CYAN_SETTINGS_INTERPRET_EMPTY_VALUE;
    }
    switch (spec->type) {
    case SETTING_BOOL:
        if (!is_bool(rawValue)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        *(bool*)spec->value = (strcmp(rawValue, "true") == 0);
        return CYAN_SETTINGS_INTERPRET_OK;
    case SETTING_INT:
        if (!is_integer(rawValue)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        *(int*)spec->value = (int)strtol(rawValue, NULL, 10);
        return CYAN_SETTINGS_INTERPRET_OK;
    case SETTING_HEX:
        if (!is_hex(rawValue) || !parse_hex_color(rawValue, (uint8_t*)spec->value)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        return CYAN_SETTINGS_INTERPRET_OK;
    case SETTING_ENUM:
        if (!parse_date_format(rawValue, (DateFormat*)spec->value)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        return CYAN_SETTINGS_INTERPRET_OK;
    case SETTING_STRING: {
        if (!is_string(rawValue)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        size_t len = strlen(rawValue);
        size_t innerLen = len - 2; // strip the surrounding quotes
        if (innerLen >= sizeof(g_settings.fontOverride)) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        snprintf(
            (char*)spec->value, sizeof(g_settings.fontOverride), "%.*s", (int)innerLen, rawValue + 1
        );
        return CYAN_SETTINGS_INTERPRET_OK;
    }
    case SETTING_INT_ARRAY:
        if (!is_array(rawValue) ||
            !parse_int_array(
                rawValue, (int*)spec->value, sizeof(g_settings.tabOrder) / sizeof(int)
            )) {
            return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
        }
        return CYAN_SETTINGS_INTERPRET_OK;
    case SETTING_STRING_ARRAY:
    default:
        return CYAN_SETTINGS_INTERPRET_INVALID_VALUE;
    }
}

static bool format_setting_value(const SettingSpec* spec, char* out, size_t outSize) {
    switch (spec->type) {
    case SETTING_BOOL:
        snprintf(out, outSize, "%s", *(bool*)spec->value ? "true" : "false");
        return true;
    case SETTING_INT:
        snprintf(out, outSize, "%d", *(int*)spec->value);
        return true;
    case SETTING_HEX: {
        uint8_t* color = (uint8_t*)spec->value;
        snprintf(out, outSize, "#%02X%02X%02X", color[0], color[1], color[2]);
        return true;
    }
    case SETTING_ENUM:
        snprintf(out, outSize, "%s", date_format_string(*(DateFormat*)spec->value));
        return true;
    case SETTING_STRING:
        snprintf(out, outSize, "\"%s\"", (char*)spec->value);
        return true;
    case SETTING_INT_ARRAY: {
        int* values = (int*)spec->value;
        size_t count = sizeof(g_settings.tabOrder) / sizeof(int);
        size_t written = 0;
        written += snprintf(out + written, outSize - written, "[");
        for (size_t i = 0; i < count && written < outSize; i++) {
            written +=
                snprintf(out + written, outSize - written, "%s%d", i == 0 ? "" : ",", values[i]);
        }
        if (written < outSize) {
            snprintf(out + written, outSize - written, "]");
        }
        return true;
    }
    case SETTING_STRING_ARRAY:
    default:
        return false;
    }
}

static void interpret_pairs(size_t pair_count, SettingPair* pairs) {
    for (size_t i = 0; i < pair_count; i++) {
        SettingPair pair = pairs[i];
        SettingSpec spec;
        int keyResult = interpret_settings_key(pair.key, pair.reserved, &spec);
        if (keyResult != CYAN_SETTINGS_INTERPRET_OK) {
            cyan_log(
                VERBOSE_MED, "[Settings] skipping '%s': %s", pair.key,
                interpret_error_string(keyResult)
            );
            continue;
        }
        if (is_default_token(pair.value)) {
            const SettingRegistryEntry* entry = find_registry_entry(spec.key);
            if (entry != NULL) {
                reset_entry_to_default(entry);
                cyan_log(
                    VERBOSE_HIGH, "[Settings] %s%s = <default>", pair.reserved ? "$" : "", pair.key
                );
            }
            continue;
        }
        int applyResult = apply_setting_value(&spec, pair.value);
        if (applyResult != CYAN_SETTINGS_INTERPRET_OK) {
            cyan_log(
                VERBOSE_MED, "[Settings] skipping '%s': %s", pair.key,
                interpret_error_string(applyResult)
            );
            continue;
        }
        cyan_log(
            VERBOSE_HIGH, "[Settings] %s%s = %s", pair.reserved ? "$" : "", pair.key, pair.value
        );
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

static SettingPair*
cyan_decompose_settings_line(char* line, size_t* out_count, SettingParseError* out_error) {
    *out_count = 0;
    *out_error = CYAN_SETTINGS_PARSE_OK;
    bool comment_inside_quote = false;
    for (size_t i = 0; line[i] != '\0'; i++) {
        if (line[i] == '"') {
            comment_inside_quote = !comment_inside_quote;
        } else if (!comment_inside_quote && line[i] == '/' && line[i + 1] == '/') {
            line[i] = '\0';
            break;
        }
    }
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
    if (line == NULL || line[0] == '\0') {
        return CYAN_SETTINGS_PARSE_OK;
    }
    if (strlen(line) > 1) {
        if (line[0] == '/' && line[1] == '/') {
            // ignore, the line is a comment
            return CYAN_SETTINGS_PARSE_OK;
        }
    }
    SettingParseError error = CYAN_SETTINGS_PARSE_OK;
    SettingPair* pairs = cyan_decompose_settings_line(line, &pair_count, &error);
    if (pairs == NULL) {
        if (error != CYAN_SETTINGS_PARSE_OK) {
            cyan_log(VERBOSE_MED, "[Settings] failed to parse line: %s", parse_error_string(error));
        }
        return error;
    }
    interpret_pairs(pair_count, pairs);
    free_pairs(pairs, pair_count);
    return CYAN_SETTINGS_PARSE_OK;
}

CyanSettings* cyan_settings_get(void) { return &g_settings; }

static void fill_defaults(CyanSettings* s) {
    memset(s, 0, sizeof(*s));
    s->version = 1;
    s->shell_verbosity = VERBOSE_HIGH;
    s->dateFormat = DATE_DMY;
    s->tabOrder[0] = 0;
    s->tabOrder[1] = 1;
    s->tabOrder[2] = 2;
    s->tabOrder[3] = 3;
    s->fontOverride[0] = '\0';
    s->devMode = false;
    s->accentColor[0] = 0x34;
    s->accentColor[1] = 0xA5;
    s->accentColor[2] = 0xB4;
    s->analogue = false;
    s->analogueRoman = false;
    s->analogueAllNumerals = true;
}

void cyan_settings_set_defaults(void) { fill_defaults(&g_settings); }

// Resets just this one entry's field to its built-in default, leaving every other setting alone.
static void reset_entry_to_default(const SettingRegistryEntry* entry) {
    CyanSettings defaults;
    fill_defaults(&defaults);
    size_t offset = (size_t)((char*)entry->field - (char*)&g_settings);
    memcpy((char*)&g_settings + offset, (char*)&defaults + offset, entry->size);
}

bool cyan_settings_load(void) {
    char resolvedPath[SETTINGS_RESOLVED_PATH_MAX];
    platform_store_resolved_path(SETTINGS_FILE_RELATIVE, resolvedPath, sizeof(resolvedPath));

    FILE* file = fopen(resolvedPath, "r");
    if (file == NULL) {
        cyan_log(VERBOSE_LOW, "[Settings] no settings file at '%s', using defaults", resolvedPath);
        return false;
    }

    char line[CYAN_SETTINGS_LINE_MAX];
    while (fgets(line, sizeof(line), file) != NULL) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }
        cyan_read_line(line);
    }
    fclose(file);
    cyan_log(VERBOSE_LOW, "[Settings] loaded '%s'", resolvedPath);
    return true;
}

bool cyan_settings_save(void) {
    platform_ensure_directory(SETTINGS_DIR);

    char tmpResolvedPath[SETTINGS_RESOLVED_PATH_MAX];
    char finalResolvedPath[SETTINGS_RESOLVED_PATH_MAX];
    platform_store_resolved_path(
        SETTINGS_FILE_TMP_RELATIVE, tmpResolvedPath, sizeof(tmpResolvedPath)
    );
    platform_store_resolved_path(
        SETTINGS_FILE_RELATIVE, finalResolvedPath, sizeof(finalResolvedPath)
    );

    FILE* file = fopen(tmpResolvedPath, "w");
    if (file == NULL) {
        cyan_log(VERBOSE_LOW, "[Settings] failed to open '%s' for writing", tmpResolvedPath);
        return false;
    }

    fprintf(file, "$CYAN_SETTINGS_VERSION=%d;\n", g_settings.version);
    char formatted[CYAN_SETTINGS_LINE_MAX];
    for (size_t i = 0; i < SETTINGS_REGISTRY_COUNT; i++) {
        const SettingRegistryEntry* entry = &SETTINGS_REGISTRY[i];
        if (entry->reserved) {
            continue; // version already written above
        }
        SettingSpec spec = {entry->key, entry->field, entry->type};
        if (!format_setting_value(&spec, formatted, sizeof(formatted))) {
            continue;
        }
        fprintf(file, "%s=%s;\n", entry->key, formatted);
    }

    if (fclose(file) != 0) {
        cyan_log(VERBOSE_LOW, "[Settings] failed to finish writing '%s'", tmpResolvedPath);
        return false;
    }

    remove(finalResolvedPath); // fine if this doesn't exist yet
    if (rename(tmpResolvedPath, finalResolvedPath) != 0) {
        cyan_log(VERBOSE_LOW, "[Settings] failed to replace '%s'", finalResolvedPath);
        return false;
    }

    cyan_log(VERBOSE_LOW, "[Settings] saved '%s'", finalResolvedPath);
    return true;
}

bool cyan_settings_format_value(const char* key, char* out, size_t outSize) {
    const SettingRegistryEntry* entry = find_registry_entry(key);
    if (entry == NULL) {
        snprintf(out, outSize, "<unset>");
        return false;
    }
    SettingSpec spec = {entry->key, entry->field, entry->type};
    return format_setting_value(&spec, out, outSize);
}

bool cyan_settings_set_from_string(const char* key, const char* value) {
    const SettingRegistryEntry* entry = find_registry_entry(key);
    if (entry == NULL) {
        return false;
    }
    if (is_default_token(value)) {
        reset_entry_to_default(entry);
        return true;
    }
    SettingSpec spec = {entry->key, entry->field, entry->type};
    return apply_setting_value(&spec, value) == CYAN_SETTINGS_INTERPRET_OK;
}

void cyan_settings_print_all(void) {
    char formatted[CYAN_SETTINGS_LINE_MAX];
    for (size_t i = 0; i < SETTINGS_REGISTRY_COUNT; i++) {
        const SettingRegistryEntry* entry = &SETTINGS_REGISTRY[i];
        SettingSpec spec = {entry->key, entry->field, entry->type};
        if (!format_setting_value(&spec, formatted, sizeof(formatted))) {
            continue;
        }
        cyan_log(VERBOSE_SHELL, "%-22s = %s", entry->key, formatted);
    }
}

int cyan_settings_resolve_tab_screen(int position) {
    size_t tabCount = sizeof(g_settings.tabOrder) / sizeof(int);
    if (position < 0 || (size_t)position >= tabCount) {
        return 0;
    }
    bool seen[sizeof(g_settings.tabOrder) / sizeof(int)] = {0};
    for (size_t i = 0; i < tabCount; i++) {
        int screen = g_settings.tabOrder[i];
        if (screen < 0 || (size_t)screen >= tabCount || seen[screen]) {
            return position; // not a valid permutation
        }
        seen[screen] = true;
    }
    return g_settings.tabOrder[position];
}

int settings_tester(int argc, char** argv) {
    (void)argc;
    (void)argv;
    char line[] = "dev_mode=true; accent_color=#34A5B4; tab_order=[3,2,1,0]";
    cyan_read_line(line);
    cyan_settings_print_all();
    return 0;
}
