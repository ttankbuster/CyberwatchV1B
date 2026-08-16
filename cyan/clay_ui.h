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
Clay_RenderCommandArray clay_watchface(CyanData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_AppHandler_catalogue(CyanData* data, AppHandler *app_handler,int width, int height, bool show_debug);
Clay_RenderCommandArray clay_timer(CyanData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_stopwatch(CyanData* data, int width, int height, bool show_debug);

Clay_RenderCommandArray clay_AppHandler_app(CyanData* data, AppHandler *app_handler, int width, int height, bool show_debug, bool show_header);

void render_footer(CyanData *data, int debugOpacity, int footerHeight);
void render_header_bar(CyanData *data, int debugOpacity, int headerWidth, int headerHeight);
void widget_temperature(CyanData *data, int debugOpacity, Clay_Sizing sizing);
void widget_battery(CyanData *data, int debugOpacity, Clay_Sizing sizing, Clay_Dimensions iconDimensions);
void render_battery(float chargePercent, void *icon, Clay_Dimensions dimensions);

void read_time_date(CyanData* data);
void read_timer(CyanData* data);
void read_stopwatch(CyanData* data);

void AppHandler_catalogue_move(AppCatalogue *catalogue, AppHandler *app_handler, int delta);
#endif