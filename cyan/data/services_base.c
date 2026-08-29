//services_base.c
#include "services.h"
#include <string.h>

void services_init(ServiceRegistry *registry) {
    memset(registry, 0, sizeof(ServiceRegistry));
}

bool services_register(ServiceRegistry *registry, Service *service) {
    if (service->id < 0 || service->id >= SERVICE_COUNT) return false;
    if (registry->services[service->id] != NULL) return false;
    if (service->init && !service->init()) return false;
    service->dynamicCriticality = service->staticCriticality;
    registry->services[service->id] = service;
    return true;
}

void services_update(ServiceRegistry *registry, float dt) {
    for (int i = 0; i < SERVICE_COUNT; i++) {
        Service *service = registry->services[i];
        if (!service) continue;
        bool isAvailable = service->available ? service->available() : true;
        if (isAvailable && service->update) {
            service->update(dt);
        }
    }
}

void services_shutdown(ServiceRegistry *registry) {
    for (int i = 0; i < SERVICE_COUNT; i++) {
        if (registry->services[i] && registry->services[i]->shutdown) {
            registry->services[i]->shutdown();
        }
    }
    memset(registry, 0, sizeof(ServiceRegistry));
}

Service *services_get(ServiceRegistry *registry, ServiceId id) {
    if (id < 0 || id >= SERVICE_COUNT) return NULL;
    return registry->services[id];
}

bool services_is_available(ServiceRegistry *registry, ServiceId id) {
    Service *s = services_get(registry, id);
    return s && s->available && s->available();
}