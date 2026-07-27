//main.c
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "data.h"
#include "clay_ui.h"
#include "display.h"
#include "surface.h"
#include "../cyan/cyan.h"

const uint32_t MAXIMUM_ELEMENTS = 50;
CyberwatchData data;
Display display;
Cyan cyan;


bool launch_app(){
    if (cyan_launch_app(&cyan, 1 , &display)) {
        data.state = CYW_APP_RUNNING;
        return true;
    } else {
        printf("Failed to launch app 1 - falling back to catalogue\n");
        data.state = CYW_CATALOGUE;
        return false;
    }
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    printf("main.c\n");

    if (!display_init(&display, &data)) {
        printf("Failed to initialise display\n");
        return 1;
    }

    DisplaySize initialSize = display_get_size(&display);
    clay_ui_init(MAXIMUM_ELEMENTS, display_measure_text, &display, initialSize.width, initialSize.height);

    if (!cyan_init(&cyan)) {
        printf("Failed to initialise Cyan\n");
        display_shutdown(&display);
        return 1;
    }

    printf("setup complete: starting.\n");

    clock_t lastTime = clock();
    bool running = true;
    data.state = CYW_HOME;
    while (running) {
        clock_t now = clock();
        float dt = (float) (now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;

        update_data(&data, &display, &running);
        DisplaySize size = display_get_size(&display);
        Clay_RenderCommandArray commands;

        switch (data.state) {
            case CYW_HOME:
                commands = clay_cyberwatch(&data, size.width, size.height, true);
                break;
            case CYW_APP_RUNNING:
                commands = clay_cyan_app(&data, &cyan, size.width, size.height, false, false);
                break;
            case CYW_CATALOGUE:
                commands = clay_cyan_catalogue(&data, &cyan, size.width, size.height, false);
                break;
            default:
                commands = clay_cyberwatch(&data, size.width, size.height, true);
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

        cyan_dispatch_events(&cyan, &data.eventQueue);
        if (data.state == CYW_APP_RUNNING) {
            cyan_run_frame(&cyan, &display, dt);
        }

        display_clear(&display, (Clay_Color) {0, 0, 0, 255});
        clay_render(&display, &commands, false);

        if (data.state == CYW_APP_RUNNING) {
            surface_render(&display, &cyan.surface);
        }

        display_present(&display);
    }
    cyan_unload_app(&cyan);
    cyan_shutdown(&cyan);
    display_shutdown(&display);
    return 0;
}