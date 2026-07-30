//services.c
#include "services.h"
#include <string.h>

void services_init(ServiceRegistry *registry) {
    memset(registry, 0, sizeof(ServiceRegistry));
}

bool services_register(ServiceRegistry *registry, Service *service) {
    if (registry->count >= MAX_SERVICES) return false;
    if (service->init && !service->init()) return false;
    registry->services[registry->count++] = service;
    return true;
}

void services_update(ServiceRegistry *registry, float dt) {
    for (int i = 0; i < registry->count; i++) {
        Service *service = registry->services[i];
        bool isAvailable = service->available ? service->available() : true;
        if (isAvailable && service->update) {
            service->update(dt);
        }
    }
}

void services_shutdown(ServiceRegistry *registry) {
    for (int i = 0; i < registry->count; i++) {
        if (registry->services[i]->shutdown) {
            registry->services[i]->shutdown();
        }
    }
    registry->count = 0;
}

Service *services_find(ServiceRegistry *registry, const char *name) {
    for (int i = 0; i < registry->count; i++) {
        if (strcmp(registry->services[i]->name, name) == 0) {
            return registry->services[i];
        }
    }
    return NULL;
}

bool services_is_available(ServiceRegistry *registry, const char *name) {
    Service *s = services_find(registry, name);
    return s && s->available && s->available();
}