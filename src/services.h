#ifndef SERVICE_H
#define SERVICE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "display.h"

typedef struct EventQueue EventQueue; // forward declaration only — avoids a
                                       // circular #include with data.h (which
                                       // embeds ServiceRegistry BY VALUE and
                                       // therefore must fully include this
                                       // file; services.h can't include
                                       // data.h back without breaking that
                                       // ordering — see chat).

typedef struct Service {
    const char *name;
    bool (*init)(void);
    void (*update)(float dt);
    void (*shutdown)(void);
    bool (*available)(void);
} Service;

#define MAX_SERVICES 16

typedef struct {
    Service *services[MAX_SERVICES];
    int count;
} ServiceRegistry;

void services_init(ServiceRegistry *registry);
bool services_register(ServiceRegistry *registry, Service *service);
void services_update(ServiceRegistry *registry, float dt);
void services_shutdown(ServiceRegistry *registry);
Service *services_find(ServiceRegistry *registry, const char *name);
bool services_is_available(ServiceRegistry *registry, const char *name);
void register_available_services(CyberwatchData *data);

typedef struct {
    Service service;
    struct tm (*now)(void);
} TimeService;

typedef struct {
    Service service;
    bool (*connected)(void);
} WifiService;

typedef struct {
    Service service;
    bool (*connected)(void);
} BluetoothService;

typedef struct {
    Service service;
    float (*charge_percent)(void); // 0.0 - 1.0
    bool (*is_charging)(void);
} BatteryService;

typedef struct {
    Service service;
    void (*shutdown)(void);
    void (*sleep)(void);
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