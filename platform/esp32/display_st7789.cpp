//display_st7789.cpp
extern "C" {
    #include "../../cyan/data/display.h"
    #include "../../assets/icons/icon_battery.h"
    #include "../../assets/icons/icon_empty_tab.h"
    #include "../../assets/icons/icon_full_tab.h"
    #include "../../assets/icons/icon_cyan1.h"
    #include "../../cyan/data/data.h"
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

#define MAX_LOG_LINES 8
#define MAX_LOG_LINE_LEN 64
static char logLines[MAX_LOG_LINES][MAX_LOG_LINE_LEN];
static int logLineCount = 0;


static uint16_t *convertRgb888ToRgb565(const uint32_t *src, int width, int height) {
    uint16_t *out = (uint16_t *) malloc(width * height * sizeof(uint16_t));
    if (!out) return NULL;
    for (int i = 0; i < width * height; i++) {
        uint32_t rgb = src[i];
        uint8_t r = (rgb >> 16) & 0xFF, g = (rgb >> 8) & 0xFF, b = rgb & 0xFF;
        out[i] = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    }
    return out;
}

static IconHandle iconBatteryHandle;
static IconHandle iconFullTab;
static IconHandle iconEmptyTab;

static const GFXfont *selectFont(int fontSize) {
    if (fontSize >= 120) return &FreeSansBold24pt7b;
    if (fontSize >= 60)  return &FreeSansBold18pt7b;
    return &FreeSans9pt7b;
}

extern "C" bool display_init(Display *display, CyberwatchData *data) {
    display->width = 240;
    display->height = 280;
    display->backend = NULL;

    iconBatteryHandle.width = ICON_BATTERY_WIDTH;
    iconBatteryHandle.height = ICON_BATTERY_HEIGHT;
    iconBatteryHandle.pixels = convertRgb888ToRgb565(ICON_BATTERY, ICON_BATTERY_WIDTH, ICON_BATTERY_HEIGHT);
    data->batteryIcon = &iconBatteryHandle;

    iconEmptyTab.width = ICON_EMPTY_TAB_WIDTH;
    iconEmptyTab.height = ICON_EMPTY_TAB_HEIGHT;
    iconEmptyTab.pixels = convertRgb888ToRgb565(ICON_EMPTY_TAB, ICON_BATTERY_WIDTH, ICON_BATTERY_HEIGHT);
    data->tabs.tabIcons[0] = &iconEmptyTab;
    
    iconFullTab.width = ICON_FULL_TAB_WIDTH;
    iconFullTab.height = ICON_FULL_TAB_HEIGHT;
    iconFullTab.pixels = convertRgb888ToRgb565(ICON_FULL_TAB, ICON_BATTERY_WIDTH, ICON_BATTERY_HEIGHT);
    data->tabs.tabIcons[1] = &iconFullTab;

#ifdef DISPLAY_RESET_VIA_MCP
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
    // bus->beginWrite();
    // bus->writeC8D8(0x36, 0x08);
    // bus->endWrite();

    gfx->fillScreen(0x0000); // black in RGB565

#ifdef DISPLAY_RESET_VIA_MCP
    // Must run here, after the display's own SPI setup - running SD's
    // SPI.begin() before this caused a hang
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
        return (void *) ICON_BATTERY;
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
    IconHandle *handle = (IconHandle *) image.imageData;
    if (!handle || !handle->pixels) {
        cyan_log(VERBOSE_LOW, "[Display] FATAL, no draw image handle");
        return;
    }

    int destW = (int) box.width;
    int destH = (int) box.height;
    if (destW <= 0 || destH <= 0) return;

    for (int y = 0; y < destH; y++) {
        int srcY = (y * handle->height) / destH;
        for (int x = 0; x < destW; x++) {
            int srcX = (x * handle->width) / destW;
            uint16_t pixel = handle->pixels[srcY * handle->width + srcX];
            gfx->drawPixel((int) box.x + x, (int) box.y + y, pixel);
        }
    }
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


void display_loading_log_listener(VerbosityLevel level, const char *message) {
    (void) level;
    if (logLineCount < MAX_LOG_LINES) {
        snprintf(logLines[logLineCount], MAX_LOG_LINE_LEN, "%s", message);
        logLineCount++;
    } else {
        for (int i = 0; i < MAX_LOG_LINES - 1; i++) {
            memcpy(logLines[i], logLines[i + 1], MAX_LOG_LINE_LEN);
        }
        snprintf(logLines[MAX_LOG_LINES - 1], MAX_LOG_LINE_LEN, "%s", message);
    }
}

static void drawScaledIcon(int destX, int destY, int destW, int destH) {
    for (int y = 0; y < destH; y++) {
        int srcY = (y * ICON_CYAN1_HEIGHT) / destH;
        for (int x = 0; x < destW; x++) {
            int srcX = (x * ICON_CYAN1_WIDTH) / destW;
            uint32_t rgb = ICON_CYAN1[srcY * ICON_CYAN1_WIDTH + srcX];
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >> 8) & 0xFF;
            uint8_t b = rgb & 0xFF;
            uint16_t pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            gfx->drawPixel(destX + x, destY + y, pixel);
        }
    }
}
static int countWrappedLines(Display *display, const char *text, int availableWidth, int fontSize) {
    Clay_StringSlice slice = { .length = (int32_t) strlen(text), .chars = text };
    Clay_TextElementConfig config = { .fontSize = fontSize };
    Clay_Dimensions dims = display_measure_text(slice, &config, display);
    if (dims.width <= availableWidth) return 1;
    int lines = (int) ceilf(dims.width / (float) availableWidth);
    return lines < 1 ? 1 : lines;
}

extern "C" void display_loading_screen(Display *display, float progress) {
    display_clear(display, (Clay_Color){0,0,0,255});
    DisplaySize size = display_get_size(display);

    float scaleX = (float) size.width / (float) ICON_CYAN1_WIDTH;
    float scaleY = (float) size.height / (float) ICON_CYAN1_HEIGHT;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int drawWidth = (int) (ICON_CYAN1_WIDTH * scale);
    int drawHeight = (int) (ICON_CYAN1_HEIGHT * scale);
    int drawX = (size.width - drawWidth) / 2;
    int drawY = (size.height - drawHeight) / 2;

    int lineHeight = 18;
    int bottomMargin = 10;
    int leftMargin = 10;
    int availableWidth = size.width - (leftMargin * 2);
    Clay_Color logColor = {150, 150, 150, 255};

    int y = size.height - bottomMargin - lineHeight;
    for (int i = logLineCount - 1; i >= 0; i--) {
        if (y < 0) break;
        char *line = logLines[i];

        int linesUsed = countWrappedLines(display, line, availableWidth, 20);

        // This entry needs `linesUsed` rows total — move up first to make
        // room for the extra wrapped rows below the point we draw at,
        // since text renders downward from its top edge.
        y -= (linesUsed - 1) * lineHeight;
        if (y < 0) break;

        Clay_TextRenderData textData = {
            .stringContents = { .length = (int32_t) strlen(line), .chars = line },
            .textColor = logColor,
            .fontSize = 20
        };
        display_draw_text(display, (Clay_BoundingBox){ .x = (float) leftMargin, .y = (float) y, .width = 0, .height = 0 }, textData);

        y -= lineHeight; // advance for the next (older) entry
    }
    drawScaledIcon(drawX, drawY, drawWidth, drawHeight);

    // TODO: draw a progress bar/text here using progress (0.0 - 1.0)

    display_present(display);
}