//screen_watchface.c
#include "../clay_ui.h"
#include "../data/data.h"

static void render_time(CyberwatchData *data, int debugOpacity, Clay_String timeString, Clay_String dateString) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.6f) }
            }
        }) {
            CLAY_TEXT(timeString, CLAY_TEXT_CONFIG({
                .fontId = FONT_LARGE,
                .fontSize = 190,
                .textColor = ACCENT_COLOUR
            }));
        }

        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65,debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }) {
            CLAY_TEXT(dateString, CLAY_TEXT_CONFIG({
                .fontId = FONT_INFO,
                .fontSize = 80,
                .textColor = SECONDARY_COLOUR
            }));
        }
    }
}

void read_time_date(CyberwatchData* data){
    WatchfaceData *wf = &data->watchface;
    snprintf(wf->timeChars, sizeof(wf->timeChars),"%02d:%02d",wf->time.tm_hour,wf->time.tm_min);
    snprintf(wf->dateChars, sizeof(wf->dateChars),"%02d/%02d/%02d %.3s",wf->time.tm_mday,wf->time.tm_mon,wf->time.tm_year%100, WEEKDAYS[wf->time.tm_wday]);
}

Clay_RenderCommandArray clay_watchface(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;

    read_time_date(data);
    Clay_String timeString = { .chars = data->watchface.timeChars, .length = strlen(data->watchface.timeChars), .isStaticallyAllocated = false };
    Clay_String dateString = { .chars = data->watchface.dateChars, .length = strlen(data->watchface.dateChars), .isStaticallyAllocated = false };

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
        render_time(data, debugOpacity, timeString, dateString);
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}