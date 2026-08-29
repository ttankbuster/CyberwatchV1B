// platform/pc/services_pc.c
#include "../../cyan/data/services.h"
#include "../../cyan/data/data.h"
#include "../../cyan/data/display.h"
#include <time.h>
#include <stdio.h>

extern CyanData data;

static bool time_available(void) { return true; }
static struct tm time_now(void) {
    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    return t ? *t : (struct tm){0};
}

TimeService timeService = {
    .service = { .id = SERVICE_TIME, .name = "Time", .available = time_available,
        .staticCriticality = CRITICALITY_HIGH },
    .now = time_now
};

static bool power_available(void) { return true; }
static bool power_has_battery(void) { return true; }
static float power_battery_percent(void) { return 0.5f; }
static bool power_is_charging(void) { return false; }
static void power_shutdown(void) { /* no real system shutdown on PC implemented*/ }
static void power_sleep(void) { /* no-op on PC */ }

PowerService powerService = {
    .service = { .id = SERVICE_POWER, .name = "Power", .available = power_available,
        .staticCriticality = CRITICALITY_MED },
    .has_battery = power_has_battery,
    .battery_percent = power_battery_percent,
    .is_charging = power_is_charging,
    .shutdown = power_shutdown,
    .sleep = power_sleep
};

void register_available_services(CyanData *data) {
    services_init(&data->services);
    bool timeOk = services_register(&data->services, (Service *) &timeService);
    cyan_log(VERBOSE_LOW, "[Services/Time]=%s", timeOk ? "OK": "FAILED");
    bool powerOk = services_register(&data->services, (Service *) &powerService);
    cyan_log(VERBOSE_LOW, "[Services/Power]=%s", timeOk ? "OK": "FAILED");
}