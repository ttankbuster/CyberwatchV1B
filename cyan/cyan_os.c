//cyberwatch.c
#include <stdio.h>
#include <unistd.h>
#include "app_handling/app_handler.h"
#include "clay_ui.h"
#include "data/surface.h"
#include "data/services.h"
#include "data/log.h"
#include "data/display.h"
#include "cyan_shell.h"

#define DEBUG_MODE false

const uint32_t MAXIMUM_ELEMENTS = 60;
CyanData data;
Display display;
AppHandler app_handler;



static bool launch_app(int id) {
    if (app_handler_launch(&app_handler, id, &display)) {
        data.state = CYW_APP_RUNNING;
        return true;
    } else {
        printf("Failed to launch app %i - falling back to catalogue\n", id);
        data.state = CYW_HOME;
        return false;
    }
}

static bool exit_app(AppHandler *app_handler) {
    app_handler_unload(app_handler);
    data.state = CYW_HOME;
    data.tabs.tabIndex = 1;
    return true;
}

static void check_shutdown(CyanData *data, float dt, bool *running) {
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

static void cycle_tab(CyanData *data) {
    data->tabs.tabIndex = (data->tabs.tabIndex + 1) % data->tabs.tabCount;
}

void print_log(VerbosityLevel level, const char *message){
    (void) level;
    printf("%s\n",message);
}

bool cyan_init(void) {
    log_add_listener(display_loading_log_listener, VERBOSE_LOW);
    log_add_listener(print_log, VERBOSE_HIGH);
    cyan_log(VERBOSE_LOW, "[CyanOS] Starting...");

    if (!display_init(&display, &data)) {
        cyan_log(VERBOSE_LOW, "[Display]=FAILED");
        return false;
    }
    cyan_log(VERBOSE_LOW, "[Display]=OK");
    display_loading_screen(&display, 0.0);
    data.tabs.tabCount = 4;
    data.tabs.tabIndex = 0;
    data.state = CYW_HOME;
    DisplaySize initialSize = display_get_size(&display);
    bool clayOk = clay_ui_init(MAXIMUM_ELEMENTS, display_measure_text, &display, initialSize.width, initialSize.height);
    cyan_log(VERBOSE_LOW, "[Clay]=%s", clayOk ? "OK": "FAILED");
    display_loading_screen(&display, 0.2);

    bool appHandlerOk = app_handler_init(&app_handler, &display);
    cyan_log(VERBOSE_LOW, "[AppHandler]=%s", appHandlerOk ? "OK": "FAILED");
    display_loading_screen(&display, 0.2);

    timer_init(&data);
    register_available_services(&data);
    cyan_log(VERBOSE_LOW, "[CyanOS]=OK");
    display_loading_screen(&display, 0.2);
    if (DEBUG_MODE){
        bool run_splashscreen = true;
        while (run_splashscreen){
            update_data(&data, &display, &run_splashscreen);
        }
    }
    display_present(&display);
    return true;
}

void cyan_update(float dt, bool *running) {
    update_data(&data, &display, running);
    check_shutdown(&data, dt, running);

    DisplaySize size = display_get_size(&display);
    Clay_RenderCommandArray commands;

    switch (data.state) {
        case CYW_APP_RUNNING:
            if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) { exit_app(&app_handler); }
            commands = clay_AppHandler_app(&data, &app_handler, size.width, size.height, false, false);
            break;

        case CYW_HOME:
        default:
            if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) { cycle_tab(&data); }
            switch (data.tabs.tabIndex) {
                case 0:
                    commands = clay_watchface(&data, size.width, size.height, true, false);
                    break;
                case 1:
                    commands = clay_AppHandler_catalogue(&data, &app_handler, size.width, size.height, false);
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_UP)) { AppHandler_catalogue_move(&data.appCatalogue, &app_handler, -1); }
                    if (has_event_type(&data.eventQueue, EVENT_SCROLL_DOWN)) { AppHandler_catalogue_move(&data.appCatalogue, &app_handler, 1); }
                    if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) { launch_app(data.appCatalogue.highlightedApp); }
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
                    commands = clay_watchface(&data, size.width, size.height, false, false);
                    break;
            }
            break;
    }

    if (data.state == CYW_APP_RUNNING) {
        Clay_ElementData appContentData = Clay_GetElementData(CLAY_ID("AppContent"));
        if (appContentData.found) {
            surface_set_region(&app_handler.surface,
                (int) appContentData.boundingBox.x,
                (int) appContentData.boundingBox.y,
                (int) appContentData.boundingBox.width,
                (int) appContentData.boundingBox.height);
        }
    }

    if (data.state == CYW_APP_RUNNING) {
        app_handler_dispatch_events(&app_handler, &data.eventQueue);
        app_handler_run_frame(&app_handler, &display, dt);
    }

    display_clear(&display, (Clay_Color) {0, 0, 0, 255});
    clay_render(&display, &commands, false);

    if (data.state == CYW_APP_RUNNING) {
        surface_render(&display, &app_handler.surface);
    } else {
        surface_render(&display, &data.watchface.analogueSurface);
    }

    display_present(&display);
}

void cyan_shutdown(void) {
    app_handler_unload(&app_handler);
    app_handler_shutdown(&app_handler);
    display_shutdown(&display);
}