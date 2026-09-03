// esp32_hardware.cpp
#include "esp32_hardware.h"
#include "../../cyan/console/log.h"
#include "secrets.h"
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>

Adafruit_MCP23X17 mcp;
bool mcpReady = false;
bool sdReady = false;

#define PIN_SD_SCK D8   // shared with display
#define PIN_SD_MOSI D10 // shared with display
#define PIN_SD_MISO D7
#define PIN_SD_CS D9

static void scanI2C() {
    cyan_log(VERBOSE_LOW, "[Hardware/I2C] Scanning...");
    int found = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            cyan_log(VERBOSE_HIGH, ">    Found device: 0x%02X", address);
            found++;
        }
    }
    cyan_log(VERBOSE_LOW, "[Hardware/I2C] OK: %d device(s) found.", found);
}

static void connectWiFi() {
    cyan_log(VERBOSE_LOW, "[Services/WiFi] Scanning...");
    int n = WiFi.scanNetworks();
    cyan_log(VERBOSE_LOW, "[Services/WiFi] OK: %d network(s) found.", n);
    for (int i = 0; i < n; i++) {
        cyan_log(
            VERBOSE_HIGH, ">    %s (RSSI %d dBm, ch %d, %s)", WiFi.SSID(i).c_str(), WiFi.RSSI(i),
            WiFi.channel(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "secured"
        );
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    cyan_log(VERBOSE_LOW, "[Services/WiFi] Connecting...");
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        cyan_log(VERBOSE_LOW, "[Services/WiFi] OK: connected.");
        cyan_log(VERBOSE_LOW, ">    IP: %s", WiFi.localIP().toString().c_str());
        cyan_log(VERBOSE_LOW, ">    Gateway: %s", WiFi.gatewayIP().toString().c_str());
        cyan_log(VERBOSE_LOW, ">    RSSI: %d dBm", WiFi.RSSI());
    } else {
        cyan_log(VERBOSE_LOW, "[Services/WiFi] FAILED: status %d.", WiFi.status());
    }
}

bool esp32_hardware_init(void) {
    Wire.begin(D4, D5);
    scanI2C();
    connectWiFi();
    mcpReady = mcp.begin_I2C();
    if (!mcpReady) {
        cyan_log(VERBOSE_LOW, "[Hardware/MCP23017] FAILED: device not found.");
        return false;
    }
    cyan_log(VERBOSE_LOW, "[Hardware/MCP23017] OK.");

    mcp.pinMode(MCP_BTN1, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN2, INPUT_PULLUP);
    mcp.pinMode(MCP_DISP_RST, OUTPUT);
    mcp.pinMode(MCP_DISP_BL, OUTPUT);
    mcp.pinMode(MCP_DIAL_SW, INPUT_PULLUP);

    return true;
}

// must run AFTER the display's own Arduino_ESP32SPI has configured the shared bus, called from
// display_init() Calling this before display init caused hang
bool esp32_sd_init(void) {
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    sdReady = SD.begin(PIN_SD_CS, SPI, 400000);
    if (sdReady) {
        cyan_log(VERBOSE_LOW, "[Hardware/SDCard] OK: %llu MB.", SD.cardSize() / (1024 * 1024));
    } else {
        cyan_log(VERBOSE_LOW, "[Hardware/SDCard] FAILED: card not found.");
    }
    return sdReady;
}