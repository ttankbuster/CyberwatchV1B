#include "../clay_ui.h"
#include "../data.h"

static void render_timer(FrameContext *ctx, Clay_String timerString, bool timerActive) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
 
    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.4f) }
            }
        }) {}
 
        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65,ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }){
            if (timerActive){

                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_CLOCK,
                    .fontSize = 195,
                    .textColor = CLOCK_COLOUR
                }));
            } else {
                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_CLOCK,
                    .fontSize = 195,
                    .textColor = {80,80,80,255},
                }));

            }
        }
    }
}
 

Clay_RenderCommandArray clay_timer(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    FrameContext ctx = { .data = data, .debugOpacity = show_debug ? 255 : 0 };
 
    read_timer(data);
    Clay_String timerString = { .chars = data->timerChars, .length = strlen(data->timerChars), .isStaticallyAllocated = false };
 
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
        render_timer(&ctx, timerString, ctx.data->timerActive);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
}