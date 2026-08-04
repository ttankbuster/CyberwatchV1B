#ifndef CLAY_UI_H
#define CLAY_UI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>

#include "../external/clay/clay.h"
#include "data/display.h"
#include "data/data.h"

static const uint32_t FONT_LARGE = 0;
static const uint32_t FONT_INFO = 1;
static const uint32_t FONT_MED = 2;
static const uint32_t FONT_SMALL = 3;

static const Clay_Color ACCENT_COLOUR = {215,125,69,255};
static const Clay_Color INFO_COLOUR = {255,255,255,255};
static const Clay_Color SECONDARY_COLOUR = {150,150,150,255};
static const Clay_Color BG_COLOR = {4,15,24,255};
static const char* WEEKDAYS[] = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

typedef struct AppHandler AppHandler;

void handleClayErrors(Clay_ErrorData errorData);
bool clay_ui_init(uint32_t max_elems, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice, Clay_TextElementConfig *, void *), void *measureTextUserData, int width, int height);
void clay_render(Display *display, Clay_RenderCommandArray *commands, bool debug_out);
Clay_RenderCommandArray clay_watchface(CyberwatchData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_AppHandler_catalogue(CyberwatchData* data, AppHandler *app_handler,int width, int height, bool show_debug);
Clay_RenderCommandArray clay_timer(CyberwatchData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_stopwatch(CyberwatchData* data, int width, int height, bool show_debug);

Clay_RenderCommandArray clay_AppHandler_app(CyberwatchData* data, AppHandler *app_handler, int width, int height, bool show_debug, bool show_header);

void render_footer(CyberwatchData *data, int debugOpacity, int footerHeight);
void render_header_bar(CyberwatchData *data, int debugOpacity, int headerWidth, int headerHeight);
void widget_temperature(CyberwatchData *data, int debugOpacity, Clay_Sizing sizing);
void widget_battery(CyberwatchData *data, int debugOpacity, Clay_Sizing sizing, Clay_Dimensions iconDimensions);
void render_battery(float chargePercent, void *icon, Clay_Dimensions dimensions);

void read_time_date(CyberwatchData* data);
void read_timer(CyberwatchData* data);
void read_stopwatch(CyberwatchData* data);

void AppHandler_catalogue_move(AppCatalogue *catalogue, AppHandler *app_handler, int delta);
#endif