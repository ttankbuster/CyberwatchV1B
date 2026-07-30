//cyberwatch.c — platform-neutral. No SDL, no Arduino, no clock()/millis().
#include <stdio.h>
#include "cyberwatch.h"
#include "clay_ui.h"
#include "surface.h"
#include "services.h"

const uint32_t MAXIMUM_ELEMENTS = 60;
CyberwatchData data;
Display display;
Cyan cyan;

static bool launch_app(int id) {
    if (cyan_launch_app(&cyan, id, &display)) {
        data.state = CYW_APP_RUNNING;
        return true;
    } else {
        printf("Failed to launch app %i - falling back to catalogue\n", id);
        data.state = CYW_HOME;
        return false;
    }
}

static bool exit_app(Cyan *cyan) {
    cyan_unload_app(cyan);
    data.state = CYW_HOME;
    data.tabs.tabIndex = 1;
    return true;
}

static void check_shutdown(CyberwatchData *data, float dt, bool *running) {
    ShutdownData *sd = &data->shutdown;

    if (has_event_type(&data->eventQueue, EVENT_BUTTON1_DOWN)) {
        sd->holding = true;
    }
    if (has_event_type(&data->eventQueue, EVENT_BUTTON1_UP)) {
        sd->holding = false;
    }

    if (sd->holding) {
        sd->holdTime += dt;
        if (sd->holdTime > SHUTDOWN_HOLD_TIME_TRIGGER) {
            *running = false;
        }
    } else {
        sd->holdTime -= dt * 4.0f;
        if (sd->holdTime < 0.0f) {
            sd->holdTime = 0.0f;
        }
    }

    float visibleTime = sd->holdTime - SHUTDOWN_SHOW_PROGRESS;
    if (visibleTime <= 0.0f) {
        sd->progress = 0.0f;
    } else {
        sd->progress = visibleTime / (SHUTDOWN_HOLD_TIME_TRIGGER - SHUTDOWN_SHOW_PROGRESS);
        if (sd->progress > 0.98f) {
            sd->progress = 1.0f;
        }
    }
}

static void cycleTab(CyberwatchData *data) {
    data->tabs.tabIndex = (data->tabs.tabIndex + 1) % data->tabs.tabCount;
}

bool cyberwatch_init(void) {
    if (!display_init(&display, &data)) {
        printf("Failed to initialise display\n");
        return false;
    }

    DisplaySize initialSize = display_get_size(&display);
    clay_ui_init(MAXIMUM_ELEMENTS, display_measure_text, &display, initialSize.width, initialSize.height);

    if (!cyan_init(&cyan, &display)) {
        printf("Failed to initialise Cyan\n");
        display_shutdown(&display);
        return false;
    }

    timer_init(&data);
    printf("cyberwatch_init complete\n");
    return true;
}

void cyberwatch_update(float dt, bool *running) {
    update_data(&data, &display, running);
    check_shutdown(&data, dt, running);

    DisplaySize size = display_get_size(&display);
    Clay_RenderCommandArray commands;

    switch (data.state) {
        case CYW_APP_RUNNING:
            if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) { exit_app(&cyan); }
            commands = clay_cyan_app(&data, &cyan, size.width, size.height, false, false);
            break;

        case CYW_HOME:
        default:
            if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) { cycleTab(&data); }
            switch (data.tabs.tabIndex) {
                case 0:
                    commands = clay_watchface(&data, size.width, size.height, false);
                    break;
                case 1:
                    commands = clay_cyan_catalogue(&data, &cyan, size.width, size.height, false);
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_UP)) { cyan_catalogue_move(&cyan, -1); }
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_DOWN)) { cyan_catalogue_move(&cyan, 1); }
                    if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) { launch_app(cyan.highlightedApp); }
                    break;
                case 2:
                    commands = clay_timer(&data, size.width, size.height, false);
                    if (data.timer.active) {
                        data.timer.selectedElement = -1;
                    } else {
                        if (has_event_type(&data.eventQueue, EVENT_BUTTON2_DOWN)) { timer_cycle_element(&data); }
                    }
                    if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) { timer_toggle(&data); }
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_UP)) { timer_spinbox_input(&data, 1); }
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_DOWN)) { timer_spinbox_input(&data, -1); }
                    break;
                case 3:
                    commands = clay_stopwatch(&data, size.width, size.height, false);
                    if (has_event_type(&data.eventQueue, EVENT_BUTTON2_DOWN)) { stopwatch_reset(&data); }
                    if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) { stopwatch_toggle(&data); }
                    break;
                default:
                    commands = clay_watchface(&data, size.width, size.height, false);
                    break;
            }
            break;
    }

    if (data.state == CYW_APP_RUNNING) {
        Clay_ElementData appContentData = Clay_GetElementData(CLAY_ID("AppContent"));
        if (appContentData.found) {
            surface_set_region(&cyan.surface,
                (int) appContentData.boundingBox.x,
                (int) appContentData.boundingBox.y,
                (int) appContentData.boundingBox.width,
                (int) appContentData.boundingBox.height);
        }
    }

    if (data.state == CYW_APP_RUNNING) {
        cyan_dispatch_events(&cyan, &data.eventQueue);
        cyan_run_frame(&cyan, &display, dt);
    }

    display_clear(&display, (Clay_Color) {0, 0, 0, 255});
    clay_render(&display, &commands, false);

    if (data.state == CYW_APP_RUNNING) {
        surface_render(&display, &cyan.surface);
    }

    display_present(&display);
}

void cyberwatch_shutdown(void) {
    cyan_unload_app(&cyan);
    cyan_shutdown(&cyan);
    display_shutdown(&display);
}