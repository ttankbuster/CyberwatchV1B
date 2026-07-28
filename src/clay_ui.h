//clay_ui.h
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
#include "display.h"
#include "data.h"



static const uint32_t FONT_CLOCK = 0;
static const uint32_t FONT_INFO = 1;

static const Clay_Color CLOCK_COLOUR = {215,125,69,255};
static const Clay_Color INFO_COLOUR = {255,255,255,255};
static const Clay_Color SECONDARY_COLOUR = {150,150,150,255};
static const Clay_Color BG_COLOR = {4,15,24,255};
static const char* WEEKDAYS[] = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

typedef struct Cyan Cyan;

typedef struct {
    CyberwatchData *data;
    int debugOpacity;
} FrameContext;
  

void handleClayErrors(Clay_ErrorData errorData);
bool clay_ui_init(uint32_t max_elems, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice, Clay_TextElementConfig *, void *), void *measureTextUserData, int width, int height);
void clay_render(Display *display, Clay_RenderCommandArray *commands, bool debug_out);
Clay_RenderCommandArray clay_watchface(CyberwatchData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_timer(CyberwatchData* data, int width, int height, bool show_debug);

Clay_RenderCommandArray clay_battery_only(BatteryData *battery, int width, int height);
Clay_RenderCommandArray clay_cyan_catalogue(CyberwatchData* data, Cyan *cyan,int width, int height, bool show_debug);
Clay_RenderCommandArray clay_cyan_app(CyberwatchData* data, Cyan *cyan, int width, int height, bool show_debug, bool show_header);

void render_footer(FrameContext *ctx, int footerHeight);
void render_header_bar(FrameContext *ctx, int headerHeight);
void widget_temperature(FrameContext *ctx, Clay_Sizing sizing);
void widget_battery(FrameContext *ctx, Clay_Sizing sizing, Clay_Dimensions iconDimensions);

void read_time_date(CyberwatchData* data);
void read_timer(CyberwatchData* data);
void read_stopwatch(CyberwatchData* data);

#endif