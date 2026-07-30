// services_pc.c
#include "../../src/services.h"
#include "../../src/data.h"
#include <time.h>

extern CyberwatchData data;

static bool time_available(void) { return true; }
static struct tm time_now(void) {
    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    return t ? *t : (struct tm){0};
}

TimeService timeService = {
    .service = { .name = "Time", .available = time_available },
    .now = time_now
};


static float pcBatteryCharge = 1.0f;
static bool battery_available(void) { return true; }
static float battery_charge_percent(void) { return pcBatteryCharge; }
static bool battery_is_charging(void) { return false; }

BatteryService batteryService = {
    .service = { .name = "Battery", .available = battery_available },
    .charge_percent = battery_charge_percent,
    .is_charging = battery_is_charging
};

void register_available_services(CyberwatchData *data) {
    services_init(&data->services);
    services_register(&data->services, (Service *) &timeService);
    services_register(&data->services, (Service *) &batteryService);
}