//main.c
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "data.h"
#include "clay_ui.h"
#include "display.h"
#include "../cyan/cyan.h"

const uint32_t MAXIMUM_ELEMENTS = 50;
CyberwatchData data;
Display display;
Cyan cyan;

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

    if (!cyan_init(&cyan, "cyan/apps/hello_cyan/hello_cyan.lua")) {
        printf("Failed to initialise Cyan\n");
        display_shutdown(&display);
        return 1;
    }

    printf("setup complete: starting.\n");
    bool running = true;
    while (running) {
        update_data(&data);
        cyan_dispatch_events(&cyan, &data.eventQueue);

        display_poll_events(&display, &running);

        DisplaySize size = display_get_size(&display);
        Clay_RenderCommandArray commands = clay_cyberwatch(&data, size.width, size.height, true);

        display_clear(&display, (Clay_Color) {0, 0, 0, 255});
        clay_render(&display, &commands, false);
        display_present(&display);
    }

    printf("SDL - shutdown\n");
    cyan_shutdown(&cyan);
    display_shutdown(&display);
    return 0;
}