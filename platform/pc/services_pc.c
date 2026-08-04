// platform/pc/services_pc.c
#include "../../cyan/data/services.h"
#include "../../cyan/data/data.h"
#include "../../cyan/data/services.h"
#include <time.h>

extern CyberwatchData data;

static bool time_available(void) { return true; }
static struct tm time_now(void) {
    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    return t ? *t : (struct tm){0};
}

TimeService timeService = {
    .service = { .id = SERVICE_TIME, .name = "Time", .available = time_available },
    .now = time_now
};

static float pcBatteryCharge = 1.0f;
static bool power_available(void) { return true; }
static bool power_has_battery(void) { return true; }
static float power_battery_percent(void) { return pcBatteryCharge; }
static bool power_is_charging(void) { return false; }
static void power_shutdown(void) { /* no real system shutdown on PC implemented*/ }
static void power_sleep(void) { /* no-op on PC */ }

PowerService powerService = {
    .service = { .id = SERVICE_POWER, .name = "Power", .available = power_available },
    .has_battery = power_has_battery,
    .battery_percent = power_battery_percent,
    .is_charging = power_is_charging,
    .shutdown = power_shutdown,
    .sleep = power_sleep
};

void register_available_services(CyberwatchData *data) {
    services_init(&data->services);
    services_register(&data->services, (Service *) &timeService);
    services_register(&data->services, (Service *) &powerService);
    // services_register(&data->services, (Service *) &powerService)
}