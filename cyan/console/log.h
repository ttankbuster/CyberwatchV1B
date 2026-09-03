#ifndef LOG_H
#define LOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    VERBOSE_SHELL, // the output of shell commands - should always be allowed
    VERBOSE_LOW,   // low verbosity: should be allowed more than med or high
    VERBOSE_MED,
    VERBOSE_HIGH // very verbose message: should be ignored except for debugging
} VerbosityLevel;

typedef void (*LogListener)(VerbosityLevel level, const char* message);
#define MAX_LOG_LISTENERS 4
bool log_add_listener(LogListener listener, VerbosityLevel acceptedLevel);

void cyan_log(VerbosityLevel level, const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif