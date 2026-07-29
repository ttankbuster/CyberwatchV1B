//data.h
#ifndef DATA_H
#define DATA_H
#include <time.h>
#include <stdbool.h>

#define MAX_EVENTS 32

typedef struct Display Display;


typedef struct {
    float charge;
    void *icon;
} BatteryData;

typedef enum {
    EVENT_NONE,
    EVENT_DISPLAY_ALTERED,
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

typedef enum {
    CYW_HOME,
    CYW_APP_RUNNING,
} CyberwatchState;


#define TAB_ICON_COUNT 6
#define SHUTDOWN_HOLD_TIME_TRIGGER 1.5f // seconds
#define SHUTDOWN_SHOW_PROGRESS 0.3f // time it starts to show shutdown bar
typedef struct CyberwatchData {
    BatteryData battery;
    struct tm time;
    char timeChars[6];
    char dateChars[13];
    EventQueue eventQueue;
    CyberwatchState state;
    int tabIndex;
    int tabCount;
    void* tabIcons[TAB_ICON_COUNT];
    void* tabData;
    bool timerActive;
    char timerChars[9]; // "00:00:00\0"
    struct tm timerLastUpdated;
    int timerH;
    int timerHspinbox;
    char timerHspinboxChars[6];
    int timerM;
    int timerMspinbox;
    char timerMspinboxChars[6];
    int timerS;
    int timerSspinbox;
    char timerSspinboxChars[6];
    bool stopwatchActive;
    char stopwatchChars[9]; // "00:00:00\0"
    struct tm stopwatchLastUpdated;
    int stopwatchH;
    int stopwatchM;
    int stopwatchS;
    bool shutdownHold;
    float shutdownHoldTime;
    float shutdownProgress;
} CyberwatchData;

#define MAX_FOLDERS 32
#define MAX_FILE_NAME 64
#define MAX_FILE_PATH 256

typedef struct {
    char names[MAX_FOLDERS][MAX_FILE_NAME];
    int count;
} FolderList;

FolderList scan_folder(char *path);

void update_data(CyberwatchData *data, Display *display, bool *running);
bool has_event_type(EventQueue *queue, EventType type);

void platform_store_resolved_path(const char *relativePath, char *outBuffer, size_t bufferSize);
char* platform_resolve_path(char *relativePath);

bool load_image(Display *display, const char *path, void *outHandle);

float get_delta(void);
#endif