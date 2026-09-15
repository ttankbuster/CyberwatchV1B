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
    void* value; // points at the live field inside the global CyanSettings instance
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

// Parses one settings-file line (possibly several ';'-separated statements) and applies any
// recognized keys directly onto the global settings instance returned by cyan_settings_get().
SettingParseError cyan_read_line(char* line);

// The single live settings instance. Never NULL.
CyanSettings* cyan_settings_get(void);

// Resets the live settings instance to its built-in defaults, without touching disk.
void cyan_settings_set_defaults(void);

// Loads settings from `.cyanos/settings.txt` (relative to the app's storage root) on top of
// whatever the live instance currently holds. Returns false if the file doesn't exist or couldn't
// be read; missing/invalid individual lines are skipped rather than treated as fatal.
bool cyan_settings_load(void);

// Writes the live settings instance out to `.cyanos/settings.txt`, via a .tmp file that only
// replaces the real file once it's been written in full. Returns false on any I/O failure.
bool cyan_settings_save(void);

// Shell-facing helpers: format/apply a single setting by name. Both return false for an unknown
// key. cyan_settings_format_value writes "<unset>" for a key that isn't currently valid.
bool cyan_settings_format_value(const char* key, char* out, size_t outSize);
bool cyan_settings_set_from_string(const char* key, const char* value);

// Logs every known setting and its current value via cyan_log(VERBOSE_SHELL, ...).
void cyan_settings_print_all(void);

int settings_tester(int argc, char** argv);

CyanSettings g_settings;

#endif
