// C3 entry point — plain watchface only, no Cyan/Lua.
// Deliberately excludes anything Cyan-related: not enough RAM headroom on
// this chip yet, and there's no SD card for app storage regardless.
extern "C" {
    #include "data.h"
    #include "clay_ui.h"
    #include "display.h"
}
#include <Arduino.h>

const uint32_t MAXIMUM_ELEMENTS = 50;
CyberwatchData data;
Display display;

void setup() {
    Serial.begin(115200);

    if (!display_init(&display, &data)) {
        Serial.println("Failed to initialise display");
        while (1) delay(1000);
    }

    DisplaySize initialSize = display_get_size(&display);
    clay_ui_init(MAXIMUM_ELEMENTS, display_measure_text, &display, initialSize.width, initialSize.height);
    Serial.println("[esp32] setup()");
}

void loop() {
    Serial.println("[esp32] loop[()");
    bool running = true;
    update_data(&data, &display, &running);

    DisplaySize size = display_get_size(&display);
    Clay_RenderCommandArray commands = clay_cyberwatch(&data, size.width, size.height, false);

    display_clear(&display, (Clay_Color) {0, 0, 0, 255});
    clay_render(&display, &commands, false);
    display_present(&display);
}