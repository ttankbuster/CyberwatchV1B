#ifndef CYAN_SETTINGS_H
#define CYAN_SETTINGS_H
#define CYAN_SETTINGS_LINE_MAX 1024
#include "../console/log.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    SETTING_BOOL,
    SETTING_INT,
    SETTING_HEX,
    SETTING_ENUM,
    SETTING_STRING,
    SETTING_INT_ARRAY,
    SETTING_STRING_ARRAY,
} SettingType;

typedef struct {
    const char* key;
    void* value;
    SettingType type;
} SettingSpec;

typedef struct {
    char* key;
    char* value;
    bool reserved;
} SettingPair;

typedef enum {
    CYAN_SETTINGS_PARSE_OK,
    CYAN_SETTINGS_PARSE_ERROR_KEY_QUOTED,
    CYAN_SETTINGS_PARSE_ERROR_UNTERMINATED_QUOTE,
    CYAN_SETTINGS_PARSE_ERROR_MISSING_EQUALS = -1,
    CYAN_SETTINGS_PARSE_ERROR_EMPTY_KEY = -2,
    CYAN_SETTINGS_PARSE_ERROR_KEY_TOO_LONG = -3,
    CYAN_SETTINGS_PARSE_ERROR_VALUE_TOO_LONG = -4,
    CYAN_SETTINGS_PARSE_ERROR_OUT_OF_MEMORY = -5
} SettingParseError;

typedef enum {
    CYAN_SETTINGS_INTERPRET_OK,
    CYAN_SETTINGS_INTERPRET_EMPTY_VALUE,
    CYAN_SETTINGS_INTERPRET_INVALID_KEY,
    CYAN_SETTINGS_INTERPRET_CONFLICTING_KEY,
    CYAN_SETTINGS_INTERPRET_RESERVATION_VIOLATION,
    CYAN_SETTINGS_INTERPRET_UNKNOWN_KEY,
    CYAN_SETTINGS_INTERPRET_INVALID_VALUE,
} SettingInterpretError;



typedef enum { DATE_DMY, DATE_MDY, DATE_YMD } DateFormat;

typedef struct {
    // cyan
    int version;
    VerbosityLevel shell_verbosity;
    DateFormat dateFormat;
    int tabOrder[4];
    char fontOverride[64];
    bool devMode;
    uint8_t accentColor[3];
    // watchface
    bool analogue;
    bool analogueRoman;
    bool analogueAllNumerals;
} CyanSettings;

SettingParseError cyan_read_line(char* line);
CyanSettings* cyan_settings_get(void);
void cyan_settings_set_defaults(void);
bool cyan_settings_load(void);
bool cyan_settings_save(void);
bool cyan_settings_format_value(const char* key, char* out, size_t outSize);
bool cyan_settings_set_from_string(const char* key, const char* value);
void cyan_settings_print_all(void);
<<<<<<< HEAD

// Resolves which screen id should render at a given tab position, per the tab_order setting.
// Falls back to identity (position == screen id) if tab_order isn't a valid permutation of
// [0, tab_order length).
int cyan_settings_resolve_tab_screen(int position);

=======
>>>>>>> 46ae89490506ea908522b11788f50b8e0f273993
int settings_tester(int argc, char** argv);
extern CyanSettings g_settings;

#endif
