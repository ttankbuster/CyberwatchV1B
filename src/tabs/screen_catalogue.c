//screen_catalogue.c
#include "../clay_ui.h"
#include "../data.h"
#include "../../cyan/cyan.h"



void render_cyan_catalogue(CyberwatchData* data, Cyan *cyan, int width, int height, bool show_debug) {
    int debugOpacity = show_debug ? 255 : 0;

    const int targetCardSize = 100; // rough desired size — actual size is derived below
    int columns = width / targetCardSize;
    if (columns < 1) columns = 1;
    int cardWidth = width / columns; // guarantees columns * cardWidth == width exactly
    int cardHeight = cardWidth;      // square cards, matching your original .aspectRatio = 1

    int rows = (cyan->appCount + columns - 1) / columns; // ceil division
    int contentHeight = rows * cardHeight;

    // --- auto-scroll: keep the highlighted card inside the visible viewport ---
    int highlightedRow = cyan->highlightedApp / columns;
    int cardTop = highlightedRow * cardHeight;
    int cardBottom = cardTop + cardHeight;
    if (cardTop < cyan->catalogueScrollY) {
        cyan->catalogueScrollY = cardTop;
    } else if (cardBottom > cyan->catalogueScrollY + height) {
        cyan->catalogueScrollY = cardBottom - height;
    }
    if (cyan->catalogueScrollY < 0) cyan->catalogueScrollY = 0;
    int maxScroll = contentHeight - height;
    if (maxScroll < 0) maxScroll = 0;
    if (cyan->catalogueScrollY > maxScroll) cyan->catalogueScrollY = maxScroll;

    CLAY(CLAY_ID("CyanDisplay"), {
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
    }) {
        CLAY(CLAY_ID("CyanAppsViewport"), {
            .backgroundColor = BG_COLOR,
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } },
            .clip = { .vertical = true, .childOffset = { 0, (float) -cyan->catalogueScrollY } }
        }) {
            CLAY(CLAY_ID("CyanAppsContent"), {
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = { .width = CLAY_SIZING_GROW() }
                }
            }) {
                for (int row = 0; row < rows; row++) {
                    CLAY(CLAY_IDI("AppRow", row), {
                        .layout = { 
                            .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_FIXED(cardHeight) },
                            .childGap = 10
                        }
                    }) {
                        for (int col = 0; col < columns; col++) {
                            int i = row * columns + col;
                            if (i >= cyan->appCount) break;
                            CyanApp *app = &cyan->apps[i];
                            bool highlighted = (i == cyan->highlightedApp);

                            CLAY(CLAY_IDI("AppCard", i), {
                                .backgroundColor = highlighted ? (Clay_Color){90,90,90,255} : (Clay_Color){50,50,50,255},
                                .layout = {
                                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                                    .sizing = { .width = CLAY_SIZING_FIXED(cardWidth), .height = CLAY_SIZING_FIXED(cardHeight) }
                                }
                            }) {
                                CLAY(CLAY_IDI("AppIcon", i), {
                                    .layout = { .sizing = { .width = CLAY_SIZING_FIXED(50), .height = CLAY_SIZING_FIXED(50) } },
                                    .aspectRatio = 1,
                                    .image = { .imageData = app->iconHandle }
                                }) {};

                                Clay_String appNameString = { .isStaticallyAllocated = false, .length = (int32_t) strlen(app->name), .chars = app->name };
                                CLAY_TEXT(appNameString, CLAY_TEXT_CONFIG({
                                    .fontId = FONT_SMALL,
                                    .fontSize = 20,
                                    .textColor = INFO_COLOUR
                                }));
                            }
                        }
                    }
                }
            }
        }
    }
}

Clay_RenderCommandArray clay_cyan_catalogue(CyberwatchData* data, Cyan *cyan,int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;

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
        render_cyan_catalogue(data, cyan, width, height, show_debug);
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}

void cyan_catalogue_move(Cyan *cyan, int delta) {
    if (cyan->appCount == 0) return;
    cyan->highlightedApp = (cyan->highlightedApp + delta + cyan->appCount) % cyan->appCount;
}