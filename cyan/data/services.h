#ifndef SERVICE_H
#define SERVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "display.h"

typedef struct EventQueue EventQueue;

typedef enum {
    SERVICE_TIME,
    SERVICE_DISPLAY,
    SERVICE_INPUT,
    SERVICE_INPUT_DISPLAY,
    SERVICE_STORAGE,
    SERVICE_POWER,
    SERVICE_NETWORK,
    SERVICE_BLUETOOTH,
    SERVICE_NOTIFICATIONS,
    SERVICE_LOCATION,
    SERVICE_AUDIO,
    SERVICE_VIBRATION,
    SERVICE_COUNT // must stay last
} ServiceId;

typedef struct Service {
    ServiceId id;
    const char *name;
    bool (*init)(void);
    void (*update)(float dt);
    void (*shutdown)(void);
    bool (*available)(void);
} Service;

typedef struct {
    Service *services[SERVICE_COUNT];
} ServiceRegistry;

void services_init(ServiceRegistry *registry);
bool services_register(ServiceRegistry *registry, Service *service); // reads service->id to place it
void services_update(ServiceRegistry *registry, float dt);
void services_shutdown(ServiceRegistry *registry);
Service *services_get(ServiceRegistry *registry, ServiceId id);
bool services_is_available(ServiceRegistry *registry, ServiceId id);
void register_available_services(CyberwatchData *data);

typedef struct {
    Service service;
    struct tm (*now)(void);
} TimeService;

typedef struct {
    Service service;
    bool (*connected)(void);
    bool (*has_wifi)(void);
    int (*get_wifi_strength)(void);
    bool (*has_ethernet)(void);
} NetworkService;

typedef struct {
    Service service;
    bool (*connected)(void);
} BluetoothService;

typedef struct {
    Service service;
    int (*app_count)(void);
    const char *(*get_app_name)(int index);
    void *(*get_app_icon)(int index);
    bool (*launch)(int index);
    bool (*is_running)(void);
    void (*run_frame)(float dt);
    void (*dispatch_events)(EventQueue *queue);
    void (*exit)(void);
} AppsService;

typedef struct {
    Service service;
    void (*shutdown)(void);
    void (*sleep)(void);
    bool (*has_battery)(void);
    float (*battery_percent)(void); // 0.0 - 1.0
    bool (*is_charging)(void);
} PowerService;

typedef struct {
    Service service;
    DisplaySize (*get_size)(void);
} DisplayService;

typedef struct {
    Service service;
    void (*poll_events)(EventQueue *queue); // buttons / dial
} InputService;

typedef struct {
    Service service;
    bool (*get_pointer)(int *x, int *y, bool *pressed); // touchscreen / mouse
} InputDisplayService;

typedef struct {
    Service service;
    bool (*read_file)(const char *path, void *buffer, size_t bufferSize, size_t *outSize);
    bool (*write_file)(const char *path, const void *data, size_t size);
} StorageService;

typedef struct {
    Service service;
    int (*pending_count)(void);
} NotificationService;

#endif