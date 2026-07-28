#include "../clay_ui.h"
#include "../data.h"

static void render_stopwatch(FrameContext *ctx, Clay_String stopwatchString) {
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
        }) {}
 
        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65,ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }){
            CLAY_TEXT(stopwatchString, CLAY_TEXT_CONFIG({
                .fontId = FONT_CLOCK,
                .fontSize = 195,
                .textColor = CLOCK_COLOUR
            }));
        }
    }
}
 

Clay_RenderCommandArray clay_stopwatch(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    FrameContext ctx = { .data = data, .debugOpacity = show_debug ? 255 : 0 };
 
    read_stopwatch(data);
    Clay_String stopwatchString = { .chars = data->stopwatchChars, .length = strlen(data->stopwatchChars), .isStaticallyAllocated = false };
 
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
        render_stopwatch(&ctx, stopwatchString);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
}