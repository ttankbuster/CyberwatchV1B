//esp32_hardware.h
#ifndef ESP32_HARDWARE_H
#define ESP32_HARDWARE_H

#include <Adafruit_MCP23X17.h>
#include <WiFi.h>
#include "../../cyan/data/display.h"

extern Adafruit_MCP23X17 mcp;
extern bool mcpReady;
extern bool sdReady;
bool esp32_hardware_init(void);
bool esp32_sd_init(void);
#define MCP_BTN1     0
#define MCP_BTN2     1
#define MCP_DISP_RST 2
#define MCP_DISP_BL  3
#define MCP_DIAL_SW  4

#endif