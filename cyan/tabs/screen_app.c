//screen_app.c
#include "../clay_ui.h"
#include "../data/data.h"

Clay_RenderCommandArray clay_AppHandler_app(CyanData* data, AppHandler *app_handler, int width, int height, bool show_debug, bool show_header) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;

    int headerHeight = show_header ? (int) (height * 0.1f) : 0;
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("AppHandlerDisplay"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
        }
    }) {
        if (show_header) {
            render_header_bar(data, debugOpacity, width, headerHeight);
        }

        CLAY(CLAY_ID("AppContent"), {
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
        }) {}
    }

    return Clay_EndLayout(deltaTime);
}