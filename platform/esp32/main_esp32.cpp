//platform/esp32/main_esp32.cpp
extern "C" {
    #include "cyberwatch.h"
}
#include <Arduino.h>
#include "esp32_hardware.h"

static unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.setDebugOutput(true);
    setvbuf(stdout, NULL, _IONBF, 0);

    if (!esp32_hardware_init()) {
        Serial.println("Failed to initialise ESP32 hardware (MCP23017)");
        while (1) delay(1000);
    }
    printf("initialising cyberwatch...\n");
    if (!AppHandler_init()) {
        Serial.println("Failed to initialise Cyberwatch");
        while (1) delay(1000);
    }
    Serial.println("[esp32] setup() complete");
    lastTime = millis();
}

void loop() {
    unsigned long now = millis();
    float dt = (float) (now - lastTime) / 1000.0f;
    lastTime = now;

    bool running = true;
    AppHandler_update(dt, &running);
}