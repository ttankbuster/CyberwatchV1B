#include "../clay_ui.h"
#include "../data.h"

Clay_RenderCommandArray clay_cyan_app(CyberwatchData* data, Cyan *cyan, int width, int height, bool show_debug, bool show_header) {
    float deltaTime = get_delta();
    FrameContext ctx = { .data = data, .debugOpacity = show_debug ? 255 : 0 };

    int headerHeight = show_header ? (int) (height * 0.155f) : 0;

    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("CyanDisplay"), {
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
        }
    }) {
        if (show_header) {
            render_header_bar(&ctx, headerHeight);
        }

        CLAY(CLAY_ID("AppContent"), {
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
        }) {}
    }

    return Clay_EndLayout(deltaTime);
}