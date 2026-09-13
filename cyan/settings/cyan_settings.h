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
    SettingType type;
    void* (*getter)(void);
    void* (*setter)(void* value);    
} Setting;

typedef struct {
    const char* key;
    void* value;
    SettingType type;
} SettingSpec;

typedef struct {
    const char* key;
    const char* value;
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

int settings_tester(int argc, char** argv);

#endif