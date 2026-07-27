// data_esp32.cpp
// Renamed from .c — needs millis() from Arduino.h, a C++ header, same
// reasoning as display_st7789.c -> .cpp earlier in this project.
#include <Arduino.h>

extern "C" {
    #include "data.h"
}

// Per-frame delta time in seconds, matching data_pc.c's role for the PC
// build. First call after boot returns 0 (no prior frame to diff against).
float get_delta(void) {
    static unsigned long lastMillis = 0;
    unsigned long now = millis();
    float delta = (lastMillis == 0) ? 0.0f : (float) (now - lastMillis) / 1000.0f;
    lastMillis = now;
    return delta;
}

// C3 stub for now — deliberately minimal. No RTC and no button hardware
// are wired into this codebase yet (matches your own "watchface only, no
// full peripheral set on the C3 yet" plan), so this only does the one
// thing that's actually load-bearing right now: reset the event queue so
// clay_cyberwatch doesn't process stale events from a previous frame.
//
// TODO once RTC (DS3231/RV-3028) is wired: populate data->time here.
// TODO once MCP23017 buttons are wired: push Event entries into
//      data->eventQueue here, same pattern as data_pc.c's SDL key handling.
// TODO once battery monitoring is wired: populate data->battery.charge.
void update_data(CyberwatchData *data, Display *display, bool *running) {
    (void) display;
    *running = true;
    data->eventQueue.len = 0;
}