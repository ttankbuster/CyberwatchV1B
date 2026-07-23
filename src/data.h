//data.h
#ifndef DATA_H
#define DATA_H
#include <time.h>
#include <stdbool.h>

#define MAX_EVENTS 32

typedef struct {
    float charge;
    void *icon;
} BatteryData;

typedef enum {
    EVENT_NONE,
    EVENT_BUTTON1_UP,
    EVENT_BUTTON1_DOWN,
    EVENT_BUTTON2_UP,
    EVENT_BUTTON2_DOWN,
    EVENT_BUTTON3_UP,
    EVENT_BUTTON3_DOWN,
    EVENT_SCROLL_UP,
    EVENT_SCROLL_DOWN,

} EventType;

typedef struct {
    EventType type;
} Event;

typedef struct {
    Event events[MAX_EVENTS];
    int len;
} EventQueue;

typedef struct {
    BatteryData battery;
    struct tm time;
    char timeChars[6];
    char dateChars[13];
    EventQueue eventQueue;
} CyberwatchData;

void update_data(CyberwatchData* data);

void platform_resolve_path(const char *relativePath, char *outBuffer, size_t bufferSize);

#endif