//screen_stopwatch.c
#include "../clay_ui.h"
#include "../data/data.h"

static void render_stopwatch(CyanData *data, int debugOpacity, Clay_String stopwatchString, bool stopwatchActive) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.4f) }
            }
        }) {}

        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65, debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }){
            if (stopwatchActive){
                CLAY_TEXT(stopwatchString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = ACCENT_COLOUR
                }));
            } else {
                CLAY_TEXT(stopwatchString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = {80,80,80,255},
                }));
            }
        }
    }
}

Clay_RenderCommandArray clay_stopwatch(CyanData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;

    read_stopwatch(data);
    Clay_String stopwatchString = { .chars = data->stopwatch.chars, .length = strlen(data->stopwatch.chars), .isStaticallyAllocated = false };

    int headerHeight = (int) (height * 0.1f);
    int footerHeight = (int) (height * 0.1f);
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Display"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = expand
        }
    }) {
        render_header_bar(data, debugOpacity, width, headerHeight);
        render_stopwatch(data, debugOpacity, stopwatchString, data->stopwatch.active);
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}

void stopwatch_toggle(CyanData *data){
    data->stopwatch.active = !data->stopwatch.active;
}

void stopwatch_reset(CyanData *data){
    data->stopwatch.h = 0;
    data->stopwatch.m = 0;
    data->stopwatch.s = 0;
}

void read_stopwatch(CyanData* data){
    StopwatchData *sw = &data->stopwatch;
    if (sw->active) {
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        if (sw->lastUpdated.tm_year >= 70) {
            time_t last = mktime(&sw->lastUpdated);
            if (last != (time_t)-1 && now > last) {
                int elapsed = (int) difftime(now, last);
                int totalSeconds = sw->h * 3600 + sw->m * 60 + sw->s + elapsed;
                sw->h = totalSeconds / 3600;
                sw->m = (totalSeconds % 3600) / 60;
                sw->s = totalSeconds % 60;
            }
        }
        if (now_tm) {
            sw->lastUpdated = *now_tm;
        }
    }
    snprintf(sw->chars, sizeof(sw->chars), "%02d:%02d:%02d", sw->h, sw->m, sw->s);
}