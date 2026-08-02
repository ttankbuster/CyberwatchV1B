// data_esp32.cpp
#include <Arduino.h>
#include <SD.h>
#include <RTClib.h>
#include <string.h>
#include <stdlib.h>
#include "esp32_hardware.h"

extern "C" {
    #include "data.h"
}

#define ENCODER_CLK_PIN D0
#define ENCODER_DT_PIN  D2

static RTC_DS3231 rtc;
static bool rtcReady = false;
static bool lastButton1Pressed = false;
static bool lastButton2Pressed = false;
static bool lastDialPressed = false;

static volatile int encoderDelta = 0;
static volatile uint8_t lastEncoded = 0;
static bool encoderInitialized = false;

void IRAM_ATTR encoderISR() {
    uint8_t msb = digitalRead(ENCODER_CLK_PIN);
    uint8_t lsb = digitalRead(ENCODER_DT_PIN);
    uint8_t encoded = (msb << 1) | lsb;
    uint8_t sum = (lastEncoded << 2) | encoded;
    if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderDelta++;
    else if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderDelta--;
    lastEncoded = encoded;
}

static void appendEvent(CyberwatchData *data, EventType type) {
    if (data->eventQueue.len + 1 < MAX_EVENTS) {
        data->eventQueue.events[data->eventQueue.len].type = type;
        data->eventQueue.len += 1;
    }
}

static void pollButtons(CyberwatchData *data) {
    if (!mcpReady) return;

    bool pressed1 = !mcp.digitalRead(MCP_BTN1);
    if (pressed1 != lastButton1Pressed) {
        appendEvent(data, pressed1 ? EVENT_BUTTON1_DOWN : EVENT_BUTTON1_UP);
        lastButton1Pressed = pressed1;
    }

    bool pressed2 = !mcp.digitalRead(MCP_BTN2);
    if (pressed2 != lastButton2Pressed) {
        appendEvent(data, pressed2 ? EVENT_BUTTON2_DOWN : EVENT_BUTTON2_UP);
        lastButton2Pressed = pressed2;
    }

    bool pressedDial = !mcp.digitalRead(MCP_DIAL_SW);
    if (pressedDial != lastDialPressed) {
        appendEvent(data, pressedDial ? EVENT_BUTTON3_DOWN : EVENT_BUTTON3_UP);
        lastDialPressed = pressedDial;
    }
}

static void pollEncoder(CyberwatchData *data) {
    if (!encoderInitialized) {
        pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
        pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), encoderISR, CHANGE);
        attachInterrupt(digitalPinToInterrupt(ENCODER_DT_PIN), encoderISR, CHANGE);
        encoderInitialized = true;
    }

    noInterrupts();
    int delta = encoderDelta;
    encoderDelta = 0;
    interrupts();

    static int accumulated = 0;
    accumulated += delta;
    while (accumulated >= 4) { appendEvent(data, EVENT_SCROLL_UP); accumulated -= 4; }
    while (accumulated <= -4) { appendEvent(data, EVENT_SCROLL_DOWN); accumulated += 4; }
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

    if (!rtcReady) {
        rtcReady = rtc.begin();
        if (rtcReady) Serial.println("[hw] RTC OK");
    }

    pollButtons(data);
    pollEncoder(data);
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

// Strips any leading slashes/path components, returning just the last
// segment — Arduino's File::name() has returned either a bare filename or
// a full path depending on core version, so this is defensive either way.
static const char *baseName(const char *path) {
    const char *lastSlash = strrchr(path, '/');
    return lastSlash ? lastSlash + 1 : path;
}

// Used by Arduino's own SD.* API — no mountpoint prefix needed, these
// paths are relative to the card root.
static void resolveSdApiPath(const char *relativePath, char *outBuffer, size_t bufferSize) {
    if (relativePath[0] == '/') {
        snprintf(outBuffer, bufferSize, "%s", relativePath);
    } else {
        snprintf(outBuffer, bufferSize, "/%s", relativePath);
    }
}

// Used for anything that ends up going through standard C fopen() (Lua's
// stock luaL_dofile, in particular) — DOES need the "/sd" VFS mountpoint
// prefix. See chat: this is the default mountpoint SD.begin() uses when
// no explicit mountpoint argument is given.
void platform_store_resolved_path(const char *relativePath, char *outBuffer, size_t bufferSize) {
    if (relativePath[0] == '/') {
        snprintf(outBuffer, bufferSize, "/sd%s", relativePath);
    } else {
        snprintf(outBuffer, bufferSize, "/sd/%s", relativePath);
    }
}

char* platform_resolve_path(char *relativePath) {
    size_t bufferSize = 512;
    char *resolvedPath = (char*) malloc(bufferSize);
    if (resolvedPath == NULL) {
        return NULL;
    }
    platform_store_resolved_path(relativePath, resolvedPath, bufferSize);
    return resolvedPath;
}

FolderList scan_folder(char *path) {
    FolderList result = {0};
    if (!sdReady) {
        Serial.println("[SD] scan_folder: SD not ready");
        return result;
    }

    char resolvedPath[256];
    resolveSdApiPath(path, resolvedPath, sizeof(resolvedPath));

    File dir = SD.open(resolvedPath);
    if (!dir || !dir.isDirectory()) {
        Serial.printf("[SD] scan_folder: could not open '%s'\n", resolvedPath);
        return result;
    }

    File entry;
    while ((entry = dir.openNextFile()) && result.count < MAX_FOLDERS) {
        if (entry.isDirectory()) {
            const char *name = baseName(entry.name());
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                snprintf(result.names[result.count], MAX_FILE_NAME, "%s", name);
                result.count++;
            }
        }
        entry.close();
    }
    dir.close();
    return result;
}

// --- Minimal 24-bit uncompressed BMP -> RGB565 decoder ---
// Same design discussed earlier for LittleFS — identical logic works
// unchanged against SD, since both share Arduino's File/fs::FS interface.
#pragma pack(push, 1)
struct BmpHeader {
    uint16_t signature;
    uint32_t fileSize;
    uint32_t reserved;
    uint32_t dataOffset;
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;
    uint32_t compression;
};
#pragma pack(pop)

static uint16_t *loadBmpAsRgb565(File &file, int *outWidth, int *outHeight) {
    BmpHeader header;
    file.read((uint8_t *) &header, sizeof(header));
    if (header.signature != 0x4D42 || header.bitsPerPixel != 24 || header.compression != 0) {
        Serial.println("[SD] load_image: unsupported BMP format (need 24-bit uncompressed)");
        return NULL;
    }

    int width = header.width;
    int height = abs(header.height);
    bool bottomUp = header.height > 0;
    int rowSize = ((width * 3 + 3) / 4) * 4; // BMP rows pad to 4-byte boundary

    uint16_t *pixels = (uint16_t *) malloc(width * height * sizeof(uint16_t));
    if (!pixels) {
        Serial.println("[SD] load_image: out of memory for pixel buffer");
        return NULL;
    }

    uint8_t *rowBuf = (uint8_t *) malloc(rowSize);
    if (!rowBuf) {
        free(pixels);
        Serial.println("[SD] load_image: out of memory for row buffer");
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        file.seek(header.dataOffset + (bottomUp ? (height - 1 - y) : y) * rowSize);
        file.read(rowBuf, rowSize);
        for (int x = 0; x < width; x++) {
            uint8_t b = rowBuf[x * 3 + 0], g = rowBuf[x * 3 + 1], r = rowBuf[x * 3 + 2];
            pixels[y * width + x] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        }
    }
    free(rowBuf);

    *outWidth = width;
    *outHeight = height;
    return pixels;
}

bool load_image(Display *display, const char *path, void *outHandle) {
    (void) display;
    if (!sdReady || outHandle == NULL) return false;

    char resolvedPath[256];
    resolveSdApiPath(path, resolvedPath, sizeof(resolvedPath));

    File file = SD.open(resolvedPath);
    if (!file) {
        Serial.printf("[SD] load_image: could not open '%s'\n", resolvedPath);
        return false;
    }

    int width, height;
    uint16_t *pixels = loadBmpAsRgb565(file, &width, &height);
    file.close();

    if (!pixels) return false;

    *(uint16_t **) outHandle = pixels;
    return true;
}