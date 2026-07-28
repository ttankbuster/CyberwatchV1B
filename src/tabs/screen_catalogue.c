#include "../clay_ui.h"
#include "../data.h"
#include "../../cyan/cyan.h"



Clay_RenderCommandArray render_cyan_catalogue(CyberwatchData* data, Cyan *cyan, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity;
    if (show_debug){
        debugOpacity = 255;
    } else {
        debugOpacity = 0;
    }


    CLAY(CLAY_ID("CyanDisplay"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
        }
    }) {
        CLAY(CLAY_ID("CyanApps"), {
            .backgroundColor = BG_COLOR,
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
        }) {
            for (int i = 0; i < cyan->appCount; i++) {
                CyanApp *app = &cyan->apps[i];
                CLAY(CLAY_IDI("AppCard", i), {
                    .backgroundColor = {48,32,24,debugOpacity},
                    .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
                }) {
                    Clay_String appNameString = {.isStaticallyAllocated = false, .length = (int32_t) strlen(app->name), .chars = app->name };
                    
                    CLAY(CLAY_IDI("AppIcon", i), {
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(100), .height = CLAY_SIZING_FIXED(100) } },
                        .image = { .imageData = app->iconHandle }
                    }) {};

                    CLAY_TEXT(appNameString, 
                        CLAY_TEXT_CONFIG({
                            .fontId = FONT_INFO,
                            .fontSize = 42,
                            .textColor = INFO_COLOUR
                        })
                    );
                }
            }
        }
    }
}

Clay_RenderCommandArray clay_cyan_catalogue(CyberwatchData* data, Cyan *cyan,int width, int height, bool show_debug) {
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
        render_cyan_catalogue(ctx.data, cyan, show_debug);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
}

