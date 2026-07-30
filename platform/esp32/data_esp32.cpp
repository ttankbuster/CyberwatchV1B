// data_esp32.cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <RTClib.h>
#include <string.h>
#include <stdlib.h>

extern "C" {
    #include "data.h"
}

static Adafruit_MCP23X17 mcp;
static RTC_DS3231 rtc;
static bool hardwareInitialized = false;
static bool mcpReady = false;
static bool rtcReady = false;
static bool lastButton1Pressed = false; // for edge detection -> DOWN/UP events
static bool lastButton2Pressed = false;

static void scanI2C() {
    Serial.println("[hw] Scanning I2C...");
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("[hw] Found I2C device at 0x%02X\n", address);
        }
    }
}

static void initHardware() {
    Wire.begin(D4, D5);
    scanI2C();
    mcpReady = mcp.begin_I2C();
    if (mcpReady) {
        Serial.println("[hw] MCP23017 OK");
        mcp.pinMode(0, INPUT_PULLUP); // button 1 -> GPA0, active-low
        mcp.pinMode(1, INPUT_PULLUP); // button 2 -> GPA1, active-low
    } else {
        Serial.println("[hw] MCP23017 not found");
    }

    rtcReady = rtc.begin();
    if (rtcReady) {
        Serial.println("[hw] RTC OK");
    } else {
        Serial.println("[hw] RTC not found");
    }

    hardwareInitialized = true;
}

static void appendEvent(CyberwatchData *data, EventType type) {
    if (data->eventQueue.len + 1 < MAX_EVENTS) {
        data->eventQueue.events[data->eventQueue.len].type = type;
        data->eventQueue.len += 1;
    }
}

static void pollButtons(CyberwatchData *data) {
    if (!mcpReady) return;

    bool pressed1 = !mcp.digitalRead(0); // active-low (pulled up, shorted to GND on press)
    if (pressed1 != lastButton1Pressed) {
        appendEvent(data, pressed1 ? EVENT_BUTTON1_DOWN : EVENT_BUTTON1_UP);
        lastButton1Pressed = pressed1;
    }

    bool pressed2 = !mcp.digitalRead(1);
    if (pressed2 != lastButton2Pressed) {
        appendEvent(data, pressed2 ? EVENT_BUTTON2_DOWN : EVENT_BUTTON2_UP);
        lastButton2Pressed = pressed2;
    }
}

static void readRTC(CyberwatchData *data) {
    if (!rtcReady) return;
    DateTime now = rtc.now();
    data->watchface.time.tm_sec  = now.second();
    data->watchface.time.tm_min  = now.minute();
    data->watchface.time.tm_hour = now.hour();
    data->watchface.time.tm_mday = now.day();
    data->watchface.time.tm_mon  = now.month() - 1;
    data->watchface.time.tm_year = now.year() - 1900;
    data->watchface.time.tm_wday = now.dayOfTheWeek();
}

//per-frame delta time in seconds
float get_delta(void) {
    static unsigned long lastMillis = 0;
    unsigned long now = millis();
    float delta = (lastMillis == 0) ? 0.0f : (float) (now - lastMillis) / 1000.0f;
    lastMillis = now;
    return delta;
}

void update_data(CyberwatchData *data, Display *display, bool *running) {
    (void) display;
    *running = true;
    data->eventQueue.len = 0;

    if (!hardwareInitialized) {
        initHardware();
    }

    pollButtons(data);
    readRTC(data);
}

bool has_event_type(EventQueue *queue, EventType type) {
    for (int i = 0; i < queue->len; i++) {
        if (queue->events[i].type == type) {
            return true;
        }
    }
    return false;
}

void platform_store_resolved_path(const char *relativePath, char *outBuffer, size_t bufferSize) {
    snprintf(outBuffer, bufferSize, "/%s", relativePath);
}

char* platform_resolve_path(char *relativePath) {
    size_t bufferSize = 512;
    char *resolvedPath = (char*) malloc(bufferSize);
    if (resolvedPath == NULL) {
        return NULL;
    }
    snprintf(resolvedPath, bufferSize, "/%s", relativePath);
    return resolvedPath;
}

bool load_image(Display *display, const char *path, void *outHandle) {
    (void) display; (void) path; (void) outHandle;
    return false;
}

FolderList scan_folder(char *path) {
    (void) path;
    FolderList result = {0};
    return result;
}