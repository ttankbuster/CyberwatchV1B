//platform/esp32/main_esp32.cpp
extern "C" {
    #include "cyberwatch.h"
}
#include <Arduino.h>

static unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);

    if (!cyberwatch_init()) {
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
    cyberwatch_update(dt, &running);
}