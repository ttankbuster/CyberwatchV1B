#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "esp32_hardware.h"
#include "secrets.h"

Adafruit_MCP23X17 mcp;
bool mcpReady = false;
bool sdReady = false;

#define PIN_SD_SCK  D8  // shared with display
#define PIN_SD_MOSI D10 // shared with display
#define PIN_SD_MISO D7
#define PIN_SD_CS   D9

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

static void scanWiFi() {
    Serial.println("Scanning WiFi...");
    int n = WiFi.scanNetworks();
    Serial.printf("Found %d networks:\n", n);
    for (int i = 0; i < n; i++) {
        Serial.printf("  %s (RSSI %d, ch %d, %s)\n",
            WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i),
            WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "secured");
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Connecting");
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.print("IP: "); Serial.println(WiFi.localIP());
        Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
        Serial.print("RSSI: "); Serial.println(WiFi.RSSI());
    } else {
        Serial.printf("\nFailed. Status: %d\n", WiFi.status());
    }

}

bool esp32_hardware_init(void) {
    Wire.begin(D4, D5);
    scanI2C();
    scanWiFi();
    mcpReady = mcp.begin_I2C();
    if (!mcpReady) {
        Serial.println("[hw] MCP23017 not found");
        return false;
    }
    Serial.println("[hw] MCP23017 OK");

    mcp.pinMode(MCP_BTN1, INPUT_PULLUP);
    mcp.pinMode(MCP_BTN2, INPUT_PULLUP);
    mcp.pinMode(MCP_DISP_RST, OUTPUT);
    mcp.pinMode(MCP_DISP_BL, OUTPUT);
    mcp.pinMode(MCP_DIAL_SW, INPUT_PULLUP);

    return true;
}

// Deliberately NOT called from esp32_hardware_init() — must run AFTER the
// display's own Arduino_ESP32SPI has configured the shared bus (called
// from display_init(), right after gfx->begin() succeeds), not before.
// Calling this before display init caused a hang; see chat history.
bool esp32_sd_init(void) {
    // 400kHz — the ESP32 SD library uses this frequency for the entire
    // init handshake, not just post-init transfers. 4MHz default failed
    // on this hardware; see chat history.
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

    sdReady = SD.begin(PIN_SD_CS, SPI, 400000);
    if (sdReady) {
        Serial.printf("[hw] SD card OK, size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    } else {
        Serial.println("[hw] SD card not found");
    }
    return sdReady;
}