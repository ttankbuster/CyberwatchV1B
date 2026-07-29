#ifndef SERVICE_H
#define SERVICE_H

#include <stdbool.h>

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
    float (*charge_percent)(void); // 0 to 1.0f
} PowerService;

#endif