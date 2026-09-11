// cyan_os.c
#include "app_handling/app_handler.h"
#include "clay_ui.h"
#include "console/cyan_console.h"
#include "console/cyan_shell.h"
#include "console/log.h"
#include "data/display.h"
#include "data/services.h"
#include "data/surface.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define DEBUG_MODE false

const uint32_t MAXIMUM_ELEMENTS = 60;
CyanData data;
Display display;
AppHandler app_handler;

bool cyan_launch_app_id(int id) {
    if (app_handler_launch(&app_handler, id, &display)) {
        data.state = CYW_APP_RUNNING;
        app_handler.currentApp = id;
        return true;
    } else {
        printf("Failed to launch app %i - falling back to catalogue\n", id);
        data.state = CYW_HOME;
        return false;
    }
}

static bool screenshot_path_is_literal(const char* path) {
    if (path == NULL || path[0] == '\0') {
        return false;
    }
    if (strchr(path, '/') != NULL || strchr(path, '\\') != NULL) {
        return true;
    }
    if (isalpha((unsigned char)path[0]) && path[1] == ':') {
        return true;
    }
    return false;
}

int cyan_screenshot(char* path_override) {
    char resolvedPath[1024];
    char logPath[1024];
    if (screenshot_path_is_literal(path_override)) {
        snprintf(resolvedPath, sizeof(resolvedPath), "%s", path_override);
        snprintf(logPath, sizeof(logPath), "%s", path_override);
    } else {
        char label[MAX_FILE_NAME + 8];
        if (path_override != NULL && path_override[0] != '\0') {
            snprintf(label, sizeof(label), "%s", path_override);
        } else if (data.state == CYW_APP_RUNNING) {
            snprintf(label, sizeof(label), "app(%s)", cyan_get_running_app()->name);
        } else {
            switch (data.tabs.tabIndex) {
            case 0:
                snprintf(label, sizeof(label), "watchface");
                break;
            case 1:
                snprintf(label, sizeof(label), "apps");
                break;
            case 2:
                snprintf(label, sizeof(label), "timer");
                break;
            case 3:
                snprintf(label, sizeof(label), "stopwatch");
                break;
            default:
                snprintf(label, sizeof(label), "cyan");
                break;
            }
        }

        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d@%H-%M", &data.watchface.time);

        char relativePath[600];
        snprintf(relativePath, sizeof(relativePath), "screenshots/%s%s.png", label, timestamp);
        platform_ensure_directory("screenshots");
        platform_store_resolved_path(relativePath, resolvedPath, sizeof(resolvedPath));
        snprintf(logPath, sizeof(logPath), "%s", relativePath);
    }
    cyan_request_screenshot(resolvedPath);
    cyan_log(VERBOSE_SHELL, "Capturing screenshot -> %s", logPath);
    return 0;
}

void string_to_lowercase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

bool cyan_launch_app_name(char* name) {
    if (name == NULL) {
        return false;
    }

    int match = 0;
    int* matched_ids = malloc(app_handler.appCount * sizeof(int));
    if (matched_ids == NULL) {
        cyan_log(VERBOSE_HIGH, "Catastrophic app launch failure: matched_id malloc failed.");
        return false;
    }
    for (int i = 0; i < app_handler.appCount; i++) {
        AppEntry app_entry = app_handler.apps[i];
        if (app_entry.name == NULL) {
            continue;
        }
        char app_name[MAX_FILE_NAME];
        snprintf(app_name, sizeof(app_name), "%s", app_entry.name);
        string_to_lowercase(app_name);
        string_to_lowercase(name);
        if (strcmp(app_name, name) == 0) {
            matched_ids[match] = i;
            match++;
        }
    }

    if (match == 1) {
        bool launched = cyan_launch_app_id(matched_ids[0]);
        free(matched_ids);
        return launched;
    } else if (match > 1) {
        cyan_log(VERBOSE_HIGH, "app launch failure: multiple IDs match");
    }

    free(matched_ids);
    return false;
}

bool cyan_is_app_running() {
    printf("CURRENT STATE: %i, CYW_APP_RUNNING=%i\n", data.state, CYW_APP_RUNNING);
    return (data.state == CYW_APP_RUNNING);
}

AppEntry* cyan_get_running_app() {
    if (!cyan_is_app_running) {
        printf("Catastrophic error: no app is curently running, but the current app was requested");
    }
    if (app_handler.currentApp == -1) {
        printf("Catastrophic error: current app not set");
    }
    return &app_handler.apps[app_handler.currentApp];
}

bool cyan_exit_app() {
    app_handler_unload(&app_handler);
    data.state = CYW_HOME;
    data.tabs.tabIndex = 1;
    app_handler.currentApp = -1;
    return true;
}

AppHandler* cyan_get_app_handler() { return &app_handler; }
int cyan_get_uptime() { return data.uptime; }

#define SCREENSHOT_PATH_MAX 1024
static char pending_screenshot_path[SCREENSHOT_PATH_MAX];
static bool screenshot_pending = false;

void cyan_request_screenshot(const char* resolvedPath) {
    snprintf(pending_screenshot_path, sizeof(pending_screenshot_path), "%s", resolvedPath);
    screenshot_pending = true;
}

/* Write out any queued screenshot against whatever is currently on the renderer.
 * Called at the end of each cyan_update frame, and once during init so the
 * loading splash is captured before the main loop replaces it. */
static void cyan_capture_pending_screenshot(void) {
    if (!screenshot_pending) {
        return;
    }
    bool ok = display_capture_screenshot(&display, pending_screenshot_path);
    cyan_log(
        VERBOSE_SHELL, ok ? "Screenshot saved: %s" : "Screenshot failed: %s",
        pending_screenshot_path
    );
    screenshot_pending = false;
}

static void check_shutdown(CyanData* data, float dt, bool* running) {
    ShutdownData* sd = &data->shutdown;

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

static void cycle_tab(CyanData* data) {
    data->tabs.tabIndex = (data->tabs.tabIndex + 1) % data->tabs.tabCount;
}

bool cyan_init(void) {
    log_add_listener(display_loading_log_listener, VERBOSE_LOW);
    cyan_console_init(); /* interactive shell; also the VERBOSE_HIGH console log sink */
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
    data.uptime = 0;
    data.watchface.numeralsShowAll = true; // change to settings after implemented
    data.watchface.numeralsRoman = true;
    DisplaySize initialSize = display_get_size(&display);
    bool clayOk = clay_ui_init(
        MAXIMUM_ELEMENTS, display_measure_text, &display, initialSize.width, initialSize.height
    );
    cyan_log(VERBOSE_LOW, "[Clay]=%s", clayOk ? "OK" : "FAILED");
    display_loading_screen(&display, 0.2);

    bool appHandlerOk = app_handler_init(&app_handler, &display);
    cyan_log(VERBOSE_LOW, "[AppHandler]=%s", appHandlerOk ? "OK" : "FAILED");
    display_loading_screen(&display, 0.4);

    timer_init(&data);
    register_available_services(&data);
    cyan_log(VERBOSE_LOW, "[CyanOS]=OK");
    display_loading_screen(&display, 0.8);
    // cyan_screenshot("cyan-loading");
    // cyan_capture_pending_screenshot(); /* grab the splash before the main loop redraws */
    if (DEBUG_MODE) {
        bool run_splashscreen = true;
        while (run_splashscreen) {
            update_data(&data, &display, &run_splashscreen);
        }
    }
    display_present(&display);
    return true;
}

void cyan_update(float dt, bool* running) {
    update_data(&data, &display, running);

    check_shutdown(&data, dt, running);
    cyan_console_poll();

    DisplaySize size = display_get_size(&display);
    Clay_RenderCommandArray clay_commands;

    switch (data.state) {
    case CYW_APP_RUNNING:
        if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) {
            cyan_exit_app();
        }
        clay_commands =
            clay_app_handler_app(&data, &app_handler, size.width, size.height, false, false);
        break;

    case CYW_HOME:
    default:
        if (has_event_type(&data.eventQueue, EVENT_BUTTON1_DOWN)) {
            cycle_tab(&data);
        }
        switch (data.tabs.tabIndex) {
        case 0:
            clay_commands = clay_watchface(&data, size.width, size.height, false, false);
            break;
        case 1:
            clay_commands =
                clay_app_handler_catalogue(&data, &app_handler, size.width, size.height, false);
            if (has_event_type(&data.eventQueue, EVENT_SCROLL_UP)) {
                app_handler_catalogue_move(&data.appCatalogue, &app_handler, -1);
            }
            if (has_event_type(&data.eventQueue, EVENT_SCROLL_DOWN)) {
                app_handler_catalogue_move(&data.appCatalogue, &app_handler, 1);
            }
            if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) {
                cyan_launch_app_id(data.appCatalogue.highlightedApp);
            }
            break;
        case 2:
            clay_commands = clay_timer(&data, size.width, size.height, false);
            if (data.timer.active) {
                data.timer.selectedElement = -1;
            } else {
                if (has_event_type(&data.eventQueue, EVENT_BUTTON2_DOWN)) {
                    timer_cycle_element(&data);
                }
            }
            if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) {
                timer_toggle(&data);
            }
            if (has_event_type(&data.eventQueue, EVENT_SCROLL_UP)) {
                timer_spinbox_input(&data, 1);
            }
            if (has_event_type(&data.eventQueue, EVENT_SCROLL_DOWN)) {
                timer_spinbox_input(&data, -1);
            }
            break;
        case 3:
            clay_commands = clay_stopwatch(&data, size.width, size.height, false);
            if (has_event_type(&data.eventQueue, EVENT_BUTTON2_DOWN)) {
                stopwatch_reset(&data);
            }
            if (has_event_type(&data.eventQueue, EVENT_BUTTON3_DOWN)) {
                stopwatch_toggle(&data);
            }
            break;
        default:
            clay_commands = clay_watchface(&data, size.width, size.height, false, false);
            break;
        }
        break;
    }

    if (data.state == CYW_APP_RUNNING) {
        Clay_ElementData appContentData = Clay_GetElementData(CLAY_ID("AppContent"));
        if (appContentData.found) {
            surface_set_region(
                &app_handler.surface, (int)appContentData.boundingBox.x,
                (int)appContentData.boundingBox.y, (int)appContentData.boundingBox.width,
                (int)appContentData.boundingBox.height
            );
        }
    }

    if (data.state == CYW_APP_RUNNING) {
        app_handler_dispatch_events(&app_handler, &data.eventQueue);
        app_handler_run_frame(&app_handler, &display, dt);
    }

    display_clear(&display, (Clay_Color){0, 0, 0, 255});
    clay_render(&display, &clay_commands, false);

    if (data.state == CYW_APP_RUNNING) {
        surface_render(&display, &app_handler.surface);
    } else {
        surface_render(&display, &data.watchface.analogueSurface);
    }

    cyan_capture_pending_screenshot();

    display_present(&display);
}

void cyan_shutdown(void) {
    cyan_console_shutdown();
    app_handler_unload(&app_handler);
    app_handler_shutdown(&app_handler);
    display_shutdown(&display);
}