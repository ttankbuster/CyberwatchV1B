//platform/esp32/main_esp32.cpp
extern "C" {
    #include "cyan_os.h"
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
    if (!cyan_init()) {
        while (1) delay(1000);
    }
    lastTime = millis();    
    // while(1)delay(1000);
}

void loop() {
    unsigned long now = millis();
    float dt = (float) (now - lastTime) / 1000.0f;
    lastTime = now;

    bool running = true;
    cyan_update(dt, &running);
}