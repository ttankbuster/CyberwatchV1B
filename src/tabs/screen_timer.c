#include "../clay_ui.h"
#include "../data.h"

static void render_spinbox(int number, char *textChars, bool active) {
    Clay_Color bgColor = active ? (Clay_Color){255, 255, 255, 255} : (Clay_Color){55, 55, 55, 255};
    Clay_Color textColor = active ? (Clay_Color){55, 55, 55, 255} : (Clay_Color){255, 255, 255, 255};
    Clay_String textStr = { .length = (int)strlen(textChars), .chars = textChars };
    snprintf(textChars, sizeof(textChars), "%d", number);
    CLAY_AUTO_ID({
        .layout = { .padding = { .bottom = active ? 0 : 2 } },
        .backgroundColor = {22, 22, 22, 255}
    }) {
        CLAY_AUTO_ID({ 
            .layout = { 
                .padding = {16, 16, 12, 12}, 
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER } 
            },
            .backgroundColor = bgColor
        }) {
            CLAY_TEXT(textStr, CLAY_TEXT_CONFIG({
                .fontId = FONT_LARGE,
                .fontSize = 24,
                .textColor = textColor
            }));
        }
    }
}




static void render_timer(FrameContext *ctx, Clay_String timerString, bool timerActive) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
 
    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,ctx->debugOpacity},
            .layout = {
                .childGap = 10,
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.4f) }
            }
        }) {
            // render_spinbox(int number, char *textChars, bool active)
            render_spinbox(ctx->data->timerHspinbox, ctx->data->timerHspinboxChars, true);
            render_spinbox(ctx->data->timerMspinbox, ctx->data->timerMspinboxChars, false);
            render_spinbox(ctx->data->timerSspinbox, ctx->data->timerSspinboxChars, false);
        }
 
        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65, ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }){
            if (timerActive){
                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = CLOCK_COLOUR
                }));
            } else {
                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = {80,80,80,255},
                }));

            }
        }
    }
}
 

Clay_RenderCommandArray clay_timer(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    FrameContext ctx = { .data = data, .debugOpacity = show_debug ? 100 : 0 };
 
    read_timer(data);
    Clay_String timerString = { .chars = data->timerChars, .length = strlen(data->timerChars), .isStaticallyAllocated = false };
 
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
        render_header_bar(&ctx, width, headerHeight);
        render_timer(&ctx, timerString, ctx.data->timerActive);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
}