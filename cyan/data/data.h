#ifndef DATA_H
#define DATA_H
#include <time.h>
#include <stdbool.h>
#include "services.h"
#include "surface.h"

#define MAX_EVENTS 32

typedef struct Display Display;

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

typedef struct EventQueue {
    Event events[MAX_EVENTS];
    int len;
} EventQueue;

typedef enum {
    CYW_HOME,
    CYW_APP_RUNNING,
} CyberwatchState;

#define SHUTDOWN_HOLD_TIME_TRIGGER 1.5f // seconds
#define SHUTDOWN_SHOW_PROGRESS 0.3f     // time it starts to show shutdown bar
#define TIMER_SELECTABLE_ELEMENT_COUNT 3
typedef struct {
    bool active;
    char chars[10]; // "000:00:00\0"
    struct tm lastUpdated;
    int h, m, s;
    int hSpinbox, mSpinbox, sSpinbox;
    char hSpinboxChars[4], mSpinboxChars[3], sSpinboxChars[3];
    int selectedElement;
} TimerData;

typedef struct {
    bool active;
    char chars[10]; // "000:00:00\0"
    struct tm lastUpdated;
    int h, m, s;
} StopwatchData;

typedef struct {
    bool holding;
    float holdTime;
    float progress;
} ShutdownData;

typedef struct {
    struct tm time;
    char timeChars[6];
    char dateChars[13];
    Surface analogueSurface;
} WatchfaceData;

typedef struct {
    int selected_app;
    float catalogueScrollY;
    float contentHeight;
    int highlightedApp;
} AppCatalogue;

typedef struct {
    int tabIndex;
    int tabCount;
    void* tabIcons[2];
    void* tabData;
} TabData;

typedef struct CyanData {
    CyberwatchState state;
    ServiceRegistry services;
    EventQueue eventQueue;
    WatchfaceData watchface;
    TabData tabs;
    TimerData timer;
    AppCatalogue appCatalogue;
    StopwatchData stopwatch;
    ShutdownData shutdown;
    const void *batteryIcon;
    float temperature;
    char temperatureChars[6]; // 23°C [000*C - 999*C]
} CyanData;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t *pixels;
} IconHandle;

#define MAX_FOLDERS 32
#define MAX_FILE_NAME 64
#define MAX_FILE_PATH 256

typedef struct {
    char names[MAX_FOLDERS][MAX_FILE_NAME];
    int count;
} FolderList;

FolderList scan_folder(char *path);


void update_data(CyanData *data, Display *display, bool *running);
bool has_event_type(EventQueue *queue, EventType type);

void platform_store_resolved_path(const char *relativePath, char *outBuffer, size_t bufferSize);
char* platform_resolve_path(char *relativePath);

bool load_image(Display *display, const char *path, void *outHandle);

void timer_init(CyanData *data);
void timer_cycle_element(CyanData *data);
void timer_toggle(CyanData *data);
void timer_spinbox_input(CyanData *data, int difference);

void stopwatch_toggle(CyanData *data);
void stopwatch_reset(CyanData *data);

float get_delta(void);

#endif