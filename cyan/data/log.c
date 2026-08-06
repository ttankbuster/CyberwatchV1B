#include "log.h"
#include <stdio.h>
#include <stdarg.h>

typedef struct {
    LogListener listener;
    VerbosityLevel acceptedLevel;
} LogListenerEntry;

static LogListenerEntry listeners[MAX_LOG_LISTENERS];
static int listenerCount = 0;

bool log_add_listener(LogListener listener, VerbosityLevel acceptedLevel) {
    if (listenerCount >= MAX_LOG_LISTENERS) return false;
    listeners[listenerCount].listener = listener;
    listeners[listenerCount].acceptedLevel = acceptedLevel;
    listenerCount++;
    return true;
}

void cyan_log(VerbosityLevel level, const char *fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    for (int i = 0; i < listenerCount; i++) {
        if (level <= listeners[i].acceptedLevel) {
            listeners[i].listener(level, buffer);
        }
    }
}