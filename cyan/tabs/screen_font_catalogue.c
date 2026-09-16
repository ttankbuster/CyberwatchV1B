// screen_watchface.c
#include "../clay_ui.h"
// #include "../data/data.h"
#include "../data/surface.h"
#include <math.h>

Clay_RenderCommandArray
clay_font_catalogue(CyanData* data, int width, int height, bool analogue, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;
    int headerHeight = (int)(height * 0.1f);
    int footerHeight = (int)(height * 0.1f);
    Clay_Sizing expand = {.width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)};

    Clay_SetLayoutDimensions((Clay_Dimensions){(float)width, (float)height});
    Clay_BeginLayout();

    CLAY(
        CLAY_ID("Display"),
        {.layout = {
             .childAlignment = {.x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER},
             .layoutDirection = CLAY_TOP_TO_BOTTOM,
             .sizing = expand
         }}
    ) {
        render_header_bar(data, debugOpacity, width, headerHeight);
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}