//clay_ui.h
#ifndef CLAY_UI_H
#define CLAY_UI_H

#include <stdbool.h>
#include <time.h>
#include "../external/clay/clay.h"
#include "display.h"
#include "data.h"

void handleClayErrors(Clay_ErrorData errorData);
bool clay_ui_init(uint32_t max_elems, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice, Clay_TextElementConfig *, void *), void *measureTextUserData, int width, int height);
void clay_render(Display *display, Clay_RenderCommandArray *commands, bool debug_out);
Clay_RenderCommandArray clay_cyberwatch(CyberwatchData* data, int width, int height, bool show_debug);
Clay_RenderCommandArray clay_battery_only(BatteryData *battery, int width, int height);


#endif