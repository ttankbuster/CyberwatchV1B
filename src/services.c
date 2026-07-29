#include "services.h"

void services_update(ServiceRegistry *registry, float dt) {
    for (int i = 0; i < registry->count; i++) {
        Service *service = registry->services[i];

        if (service->available()) {
            service->update(dt);
        }
    }
}