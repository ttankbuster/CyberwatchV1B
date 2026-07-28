#include "../clay_ui.h"

static void render_time(FrameContext *ctx, Clay_String timeString, Clay_String dateString) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
 
    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.6f) }
            }
        }) {
            CLAY_TEXT(timeString, CLAY_TEXT_CONFIG({
                .fontId = FONT_CLOCK,
                .fontSize = 195,
                .textColor = CLOCK_COLOUR
            }));
        }
 
        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65,ctx->debugOpacity},
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
 

Clay_RenderCommandArray clay_watchface(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    FrameContext ctx = { .data = data, .debugOpacity = show_debug ? 255 : 0 };
 
    read_time_date(data);
    Clay_String timeString = { .chars = data->timeChars, .length = strlen(data->timeChars), .isStaticallyAllocated = false };
    Clay_String dateString = { .chars = data->dateChars, .length = strlen(data->dateChars), .isStaticallyAllocated = false };
 
    int headerHeight = (int) (height * 0.155f);
    int footerHeight = (int) (height * 0.151f);
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
        render_header_bar(&ctx, headerHeight);
        render_time(&ctx, timeString, dateString);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
}