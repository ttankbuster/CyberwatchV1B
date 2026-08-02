//clay_ui.c
#define CLAY_IMPLEMENTATION
#include "clay_ui.h"

void handleClayErrors(Clay_ErrorData errorData) {
    if (errorData.errorText.chars) {
        printf("%.*s\n", (int) errorData.errorText.length, errorData.errorText.chars);
    }
}

bool clay_ui_init(uint32_t max_elems, Clay_Dimensions (*measureTextFunction)(Clay_StringSlice, Clay_TextElementConfig *, void *), void *measureTextUserData, int width, int height) {
    Clay_SetMaxElementCount(max_elems);
    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    printf("Clay memory requirement: %" PRIu64 " bytes\n", clayRequiredMemory);
    void *memory = malloc(clayRequiredMemory);
    if (!memory) {
        return false;
    }
    Clay_Arena clayMemory = (Clay_Arena) { .memory = memory, .capacity = clayRequiredMemory };
    Clay_Initialize(clayMemory, (Clay_Dimensions) { (float) width, (float) height }, (Clay_ErrorHandler) { handleClayErrors });
    Clay_SetMeasureTextFunction(measureTextFunction, measureTextUserData);
    return true;
}

void render_battery(float chargePercent, void *icon, Clay_Dimensions dimensions){
    int battery_outline = (int) (dimensions.width * 0.06f);

    CLAY(CLAY_ID("BatteryDisplay"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { .width = CLAY_SIZING_FIXED(dimensions.width), .height = CLAY_SIZING_FIXED(dimensions.height) }
        }
    }) {
        CLAY(CLAY_ID("BatteryOutlineImage"), {
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } },
            .image = { .imageData = icon },
            .floating = { .attachTo = CLAY_ATTACH_TO_PARENT, .zIndex = 1 }
        }) {}

        CLAY(CLAY_ID("BatteryFill"), {
            .backgroundColor = {20,244,200,120},
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED((chargePercent * (dimensions.width - (battery_outline * 2))) + battery_outline),
                    .height = CLAY_SIZING_FIXED(dimensions.height - (battery_outline * 2))
                }
            },
            .floating = {
                .attachTo = CLAY_ATTACH_TO_PARENT,
                .zIndex = 0,
                .attachPoints = { .element = CLAY_ATTACH_POINT_LEFT_CENTER, .parent = CLAY_ATTACH_POINT_LEFT_CENTER }
            }
        }) {}
    }
}

void read_time_date(CyberwatchData* data){
    WatchfaceData *wf = &data->watchface;
    snprintf(wf->timeChars, sizeof(wf->timeChars),"%02d:%02d",wf->time.tm_hour,wf->time.tm_min);
    snprintf(wf->dateChars, sizeof(wf->dateChars),"%02d/%02d/%02d %.3s",wf->time.tm_mday,wf->time.tm_mon,wf->time.tm_year%100, WEEKDAYS[wf->time.tm_wday]);
}


void widget_temperature(CyberwatchData *data, int debugOpacity, Clay_Sizing sizing) {
    CLAY(CLAY_ID("TemperatureDisplay"), {
        .backgroundColor = {100,32,24,debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = sizing
        }
    }) {
        CLAY_TEXT(CLAY_STRING("14*c"), CLAY_TEXT_CONFIG({
            .fontId = FONT_LARGE,
            .fontSize = 42,
            .textColor = INFO_COLOUR
        }));
    }
}

void widget_battery(CyberwatchData *data, int debugOpacity, Clay_Sizing sizing, Clay_Dimensions iconDimensions) {
    float chargePercent = 0.0f;
    Service *s = services_find(&data->services, "Battery");
    if (s && s->available && s->available()) {
        BatteryService *battery = (BatteryService *) s;
        chargePercent = battery->charge_percent();
    }

    CLAY(CLAY_ID("BatteryDisplayContext"), {
        .backgroundColor = {48,100,24, debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = sizing
        }
    }) {
        render_battery(chargePercent, data->batteryIcon, iconDimensions);
    }
}

void shutdown_bar(float progress, int headerWidth, int headerHeight){
    CLAY(CLAY_ID("ShutdownProgress"), {
        .backgroundColor = {60, 60, 60, 1},
        .layout = {
            .sizing = {.width = CLAY_SIZING_FIXED(headerWidth*progress), .height = CLAY_SIZING_FIXED(headerHeight)}
        },
        .floating = {
            .attachTo = CLAY_ATTACH_TO_PARENT,
            .zIndex = -1
        }
    }) {}
}

void render_header_bar(CyberwatchData *data, int debugOpacity, int headerWidth, int headerHeight) {
    float batteryHeight = headerHeight * 0.45f;
    float batteryWidth = batteryHeight * (33.0f / 16.0f);
    Clay_Dimensions batteryDimensions = (Clay_Dimensions) { batteryWidth, batteryHeight };
    Clay_Sizing widgetSizing = { .height = CLAY_SIZING_FIXED(headerHeight), .width = CLAY_SIZING_GROW() };

    CLAY(CLAY_ID("HeaderBar"), {
        .layout = { .sizing = { .height = CLAY_SIZING_FIXED(headerHeight), .width = CLAY_SIZING_FIXED(headerWidth) } }
    }) {
        widget_temperature(data, debugOpacity, widgetSizing);
        widget_battery(data, debugOpacity, widgetSizing, batteryDimensions);
        shutdown_bar(data->shutdown.progress, headerWidth, headerHeight);
    }
}

void render_footer(CyberwatchData *data, int debugOpacity, int footerHeight) {
    CLAY(CLAY_ID("Footer"), {
        .backgroundColor = {47,24,24,debugOpacity},
        .layout = { .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, .childGap = 20, .layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = { .height = CLAY_SIZING_FIXED(footerHeight), .width = CLAY_SIZING_GROW() } }
    }) {
        int tabIndex = data->tabs.tabIndex;
        int tabIconSize = 30;
        for (int i = 0; i < data->tabs.tabCount; i++) {
            if (i == tabIndex){
                if (i < TAB_ICON_COUNT-2){
                    CLAY(CLAY_IDI("TabIcon", i), {
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                        .image = { .imageData = data->tabs.tabIcons[i+2] },
                    }) {}
                } else {
                    CLAY(CLAY_IDI("TabIcon", i), {
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                        .image = { .imageData = data->tabs.tabIcons[1] },
                    }) {}
                }
            } else {
                CLAY(CLAY_IDI("TabIcon", i), {
                    .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                    .image = { .imageData = data->tabs.tabIcons[0] },
                }) {}
            }
        }
    }
}

#define MAX_OVERLAY_STACK 16

static Clay_Color overlayStack[MAX_OVERLAY_STACK];
static int overlayStackDepth = 0;

static Clay_Color applyOverlay(Clay_Color colour) {
    if (overlayStackDepth == 0) return colour;
    Clay_Color overlay = overlayStack[overlayStackDepth - 1];
    float alpha = overlay.a / 255.0f;
    return (Clay_Color) {
        colour.r * (1 - alpha) + overlay.r * alpha,
        colour.g * (1 - alpha) + overlay.g * alpha,
        colour.b * (1 - alpha) + overlay.b * alpha,
        colour.a
    };
}

void clay_render(Display *display, Clay_RenderCommandArray *commands, bool debug_out) {
    overlayStackDepth = 0;
    if (debug_out){
        printf("Frame has %d render commands\n", commands->length);
    }
    for (int i = 0; i < commands->length; i++) {
        Clay_RenderCommand *cmd = Clay_RenderCommandArray_Get(commands, i);
        Clay_BoundingBox box = cmd->boundingBox;
        if (debug_out) {
            printf("cmd %d:type=%d box=(%.0f,%.0f,%.0f,%.0f)\n", i, cmd->commandType, cmd->boundingBox.x, cmd->boundingBox.y, cmd->boundingBox.width, cmd->boundingBox.height);
        }
        switch (cmd->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_NONE:
                break;
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                Clay_Color colour = applyOverlay(cmd->renderData.rectangle.backgroundColor);
                display_fill_rect(display, box, colour);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                Clay_BorderRenderData border = cmd->renderData.border;
                border.color = applyOverlay(border.color);
                display_draw_border(display, box, border);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                Clay_TextRenderData text = cmd->renderData.text;
                if (debug_out){
                    printf("len=%d text='", (int)text.stringContents.length);
                    fwrite(text.stringContents.chars, 1, text.stringContents.length, stdout);
                    printf("'\n");
                }
                display_draw_text(display, box, text);
                break;
            }
            case CLAY_RENDER_COMMAND_TYPE_IMAGE:
                display_draw_image(display, box, cmd->renderData.image);
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                display_set_clip(display, box);
                break;
            case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                display_clear_clip(display);
                break;
            case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
                break;
        }
    }
}