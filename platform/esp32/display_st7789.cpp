extern "C" {
    #include "display.h"
    #include "../../assets/icons/battery_icon.h"
}
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "esp32_hardware.h"
//must be declared afterwards or gfxfont.h wont be loaded, which is a dependency
#include "assets/fonts/GFX/FreeSans9pt7b.h"
#include "assets/fonts/GFX/FreeSansBold18pt7b.h"
#include "assets/fonts/GFX/FreeSansBold24pt7b.h"

#if defined(CONFIG_IDF_TARGET_ESP32C3)
    // Backlight is wired directly to 3V3 on this board - no GPIO drive needed.
    #define PIN_SCK  D8
    #define PIN_MOSI D10
    #define PIN_CS   D1
    #define PIN_DC   D3
    #define PIN_RST  D0
    #define PIN_BL   D6

#elif defined(BOARD_XIAO_ESP32S3_PLUS)
    // RST and BL are driven through the MCP23017 here (A2/A3), NOT raw
    // ESP32 GPIO - D0/D6 are freed up for the rotary encoder's CLK/DT
    // instead. See esp32_hardware.h for the shared MCP instance.
    #define DISPLAY_RESET_VIA_MCP
    #define PIN_SCK  D8
    #define PIN_MOSI D10
    #define PIN_CS   D1
    #define PIN_DC   D3
/*
| VCC  | 3V3  |
| GND  | GND  |
| DIN  | D10  |
| CLK  | D8   |
| CS   | D1   |
| DC   | D3   |
| RST  | MCP A2 |
| BL   | MCP A3 |
*/

#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    #define PIN_SCK  6
    #define PIN_MOSI 7
    #define PIN_CS   4
    #define PIN_DC   5
    #define PIN_RST  8
    #define PIN_BL   9

#else
    #error "Unsupported target - add a pin block for this chip"
#endif

static Arduino_DataBus *bus = new Arduino_ESP32SPI(PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI, GFX_NOT_DEFINED);
#ifdef DISPLAY_RESET_VIA_MCP
    // RST not passed here - the library can't reach an I2C-expander pin
    // itself, so it's pulsed manually in display_init() below instead.
    static Arduino_GFX *panel = new Arduino_ST7789(bus, GFX_NOT_DEFINED, 0, true, 240, 280, 0, 20, 0, 20);
#else
    static Arduino_GFX *panel = new Arduino_ST7789(bus, PIN_RST, 0, true, 240, 280, 0, 20, 0, 20);
#endif
static Arduino_Canvas *gfx = new Arduino_Canvas(240, 280, panel);

static const GFXfont *selectFont(int fontSize) {
    if (fontSize >= 120) return &FreeSansBold24pt7b;
    if (fontSize >= 60)  return &FreeSansBold18pt7b;
    return &FreeSans9pt7b;
}

extern "C" bool display_init(Display *display, CyberwatchData *data) {
    display->width = 240;
    display->height = 280;
    display->backend = NULL;

    // PC's display.c sets these; the ESP32 path never did - tabCount
    // defaulting to 0 (zero-init) meant cycle_tab()'s "% tabCount" was a
    // modulo-by-zero the instant button 1 was pressed. That's the crash.

#ifdef DISPLAY_RESET_VIA_MCP
    // esp32_hardware_init() must have already run (called from
    // main_esp32.cpp before this function) - mcp is not valid otherwise.
    if (!mcpReady) {
        Serial.println("display_init: MCP23017 not ready - call esp32_hardware_init() first");
        return false;
    }
    mcp.digitalWrite(MCP_DISP_RST, LOW);
    delay(20);
    mcp.digitalWrite(MCP_DISP_RST, HIGH);
    delay(120); // ST7789 needs settle time after reset before accepting commands
    mcp.digitalWrite(MCP_DISP_BL, HIGH);
#else
    #ifdef PIN_BL
        pinMode(PIN_BL, OUTPUT);
        digitalWrite(PIN_BL, HIGH);
    #endif
#endif

    if (!gfx->begin()) {
        Serial.println("Arduino_GFX begin() failed");
        return false;
    }
    gfx->fillScreen(0x0000); // black in RGB565

#ifdef DISPLAY_RESET_VIA_MCP
    // Must run here, after the display's own SPI setup - running SD's
    // SPI.begin() before this caused a hang. See chat history.
    esp32_sd_init();
#endif

    return true;
}

extern "C" void display_shutdown(Display *display) {
    (void) display;
}

extern "C" bool display_poll_events(Display *display, bool *running) {
    (void) display; (void) running;
    return true;
}

extern "C" DisplaySize display_get_size(Display *display) {
    return (DisplaySize) { display->width, display->height };
}

extern "C" void *display_load_image(Display *display, const char *path) {
    (void) display;
    if (strcmp(path, "batteryOutline") == 0) {
        return (void *) batteryIconData;
    }
    return NULL;
}

static uint16_t toRgb565(Clay_Color colour) {
    return gfx->color565((uint8_t) colour.r, (uint8_t) colour.g, (uint8_t) colour.b);
}

extern "C" void display_clear(Display *display, Clay_Color colour) {
    (void) display;
    gfx->fillScreen(toRgb565(colour));
}

extern "C" void display_fill_rect(Display *display, Clay_BoundingBox box, Clay_Color colour) {
    (void) display;
    gfx->fillRect((int) box.x, (int) box.y, (int) box.width, (int) box.height, toRgb565(colour));
}

extern "C" void display_draw_border(Display *display, Clay_BoundingBox box, Clay_BorderRenderData border) {
    (void) display;
    uint16_t colour = toRgb565(border.color);
    if (border.width.top > 0)    gfx->fillRect((int) box.x, (int) box.y, (int) box.width, (int) border.width.top, colour);
    if (border.width.bottom > 0) gfx->fillRect((int) box.x, (int) (box.y + box.height - border.width.bottom), (int) box.width, (int) border.width.bottom, colour);
    if (border.width.left > 0)   gfx->fillRect((int) box.x, (int) box.y, (int) border.width.left, (int) box.height, colour);
    if (border.width.right > 0)  gfx->fillRect((int) (box.x + box.width - border.width.right), (int) box.y, (int) border.width.right, (int) box.height, colour);
}

extern "C" void display_draw_text(Display *display, Clay_BoundingBox box, Clay_TextRenderData text) {
    (void) display;
    char buffer[128];
    int len = text.stringContents.length;
    if (len < 0) len = 0;
    if (len > 127) len = 127;
    memcpy(buffer, text.stringContents.chars, len);
    buffer[len] = '\0';

    gfx->setFont(selectFont(text.fontSize));
    gfx->setTextColor(toRgb565(text.textColor));

    int16_t x1, y1;
    uint16_t w, h;
    gfx->getTextBounds(buffer, 0, 0, &x1, &y1, &w, &h);
    gfx->setCursor((int) box.x, (int) box.y - y1);
    gfx->print(buffer);
}

extern "C" void display_draw_image(Display *display, Clay_BoundingBox box, Clay_ImageRenderData image) {
    (void) display;
    const uint16_t *pixels = (const uint16_t *) image.imageData;
    if (!pixels) return;
    gfx->draw16bitRGBBitmap((int) box.x, (int) box.y, (uint16_t *) pixels, 33, 16);
}

extern "C" void display_set_clip(Display *display, Clay_BoundingBox box) {
    (void) display;
}

extern "C" void display_clear_clip(Display *display) {
    (void) display;
}

extern "C" void display_present(Display *display) {
    (void) display;
    gfx->flush();
}

extern "C" Clay_Dimensions display_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    (void) userData;
    char buffer[128];
    int len = text.length;
    if (len < 0) len = 0;
    if (len > 127) len = 127;
    memcpy(buffer, text.chars, len);
    buffer[len] = '\0';

    gfx->setFont(selectFont(config->fontSize));
    int16_t x1, y1;
    uint16_t width, height;
    gfx->getTextBounds(buffer, 0, 0, &x1, &y1, &width, &height);
    return (Clay_Dimensions) { (float) width, (float) height };
}