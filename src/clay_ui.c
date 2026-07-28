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

void render_battery(BatteryData battery, Clay_Dimensions dimensions){
    int battery_outline = (int) (dimensions.width * 0.06f);

    CLAY(CLAY_ID("BatteryDisplay"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_LEFT, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { .width = CLAY_SIZING_FIXED(dimensions.width), .height = CLAY_SIZING_FIXED(dimensions.height) }
        }
    }) {
        CLAY(CLAY_ID("BatteryOutlineImage"), {
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } },
            .image = { .imageData = battery.icon },
            .floating = { .attachTo = CLAY_ATTACH_TO_PARENT, .zIndex = 1 }
        }) {}

        CLAY(CLAY_ID("BatteryFill"), {
            .backgroundColor = {20,244,200,120},
            .layout = {
                .sizing = {
                    .width = CLAY_SIZING_FIXED((battery.charge * (dimensions.width - (battery_outline * 2))) + battery_outline),
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
    snprintf(data->timeChars, sizeof(data->timeChars),"%02d:%02d",data->time.tm_hour,data->time.tm_min);
    snprintf(data->dateChars, sizeof(data->dateChars),"%02d/%02d/%02d %.3s",data->time.tm_mday,data->time.tm_mon,data->time.tm_year%100, WEEKDAYS[data->time.tm_wday]);
}

void read_stopwatch(CyberwatchData* data){
    if (data->stopwatchActive) {
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        if (data->stopwatchLastUpdated.tm_year >= 70) {
            time_t last = mktime(&data->stopwatchLastUpdated);
            if (last != (time_t)-1 && now > last) {
                int elapsed = (int)difftime(now, last);
                int totalSeconds = data->stopwatchH * 3600 + data->stopwatchM * 60 + data->stopwatchS + elapsed;
                data->stopwatchH = totalSeconds / 3600;
                data->stopwatchM = (totalSeconds % 3600) / 60;
                data->stopwatchS = totalSeconds % 60;
            }
        }
        if (now_tm) {
            data->stopwatchLastUpdated = *now_tm;
        }
    }
    snprintf(data->stopwatchChars, sizeof(data->stopwatchChars), "%02d:%02d:%02d", data->stopwatchH, data->stopwatchM, data->stopwatchS);
}

void read_timer(CyberwatchData* data){
    if (data->timerActive) {
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        if (data->timerLastUpdated.tm_year >= 70) {
            time_t last = mktime(&data->timerLastUpdated);
            if (last != (time_t)-1 && now > last) {
                int elapsed = (int)difftime(now, last);
                int totalSeconds = data->timerH * 3600 + data->timerM * 60 + data->timerS + elapsed;
                data->timerH = totalSeconds / 3600;
                data->timerM = (totalSeconds % 3600) / 60;
                data->timerS = totalSeconds % 60;
            }
        }
        if (now_tm) {
            data->timerLastUpdated = *now_tm;
        }
    }
    snprintf(data->timerChars, sizeof(data->timerChars), "%02d:%02d:%02d", data->timerH, data->timerM, data->timerS);
}

void widget_temperature(FrameContext *ctx, Clay_Sizing sizing) {
    CLAY(CLAY_ID("TemperatureDisplay"), {
        .backgroundColor = {100,32,24,ctx->debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = sizing
        }
    }) {
        CLAY_TEXT(CLAY_STRING("14*c"), CLAY_TEXT_CONFIG({
            .fontId = FONT_CLOCK,
            .fontSize = 42,
            .textColor = INFO_COLOUR
        }));
    }
}
 
void widget_battery(FrameContext *ctx, Clay_Sizing sizing, Clay_Dimensions iconDimensions) {
    CLAY(CLAY_ID("BatteryDisplayContext"), {
        .backgroundColor = {48,100,24, ctx->debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = sizing
        }
    }) {
        render_battery(ctx->data->battery, iconDimensions);
    }
}
 
void render_header_bar(FrameContext *ctx, int headerHeight) {
    float batteryHeight = headerHeight * 0.45f;
    float batteryWidth = batteryHeight * (33.0f / 16.0f);
    Clay_Dimensions batteryDimensions = (Clay_Dimensions) { batteryWidth, batteryHeight };
    Clay_Sizing widgetSizing = { .height = CLAY_SIZING_FIXED(headerHeight), .width = CLAY_SIZING_GROW() };
 
    CLAY(CLAY_ID("HeaderBar"), {
        .layout = { .sizing = { .height = CLAY_SIZING_FIXED(headerHeight), .width = CLAY_SIZING_GROW() } }
    }) {
        widget_temperature(ctx, widgetSizing);
        widget_battery(ctx, widgetSizing, batteryDimensions);
    }
}
  
void render_footer(FrameContext *ctx, int footerHeight) {
    CLAY(CLAY_ID("Footer"), {
        .backgroundColor = {47,24,24,ctx->debugOpacity},
        .layout = { .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }, .childGap = 20, .layoutDirection = CLAY_LEFT_TO_RIGHT, .sizing = { .height = CLAY_SIZING_FIXED(footerHeight), .width = CLAY_SIZING_GROW() } }
    }) {
        int tabIndex = ctx->data->tabIndex;
        int tabIconSize = 20;
        for (int i = 0; i < ctx->data->tabCount; i++) {
            if (i == tabIndex){
                if (i < TAB_ICON_COUNT-2){ // TAB_ICON_COUNT-2 because the first two icons are always generic: empty and full
                    CLAY(CLAY_IDI("TabIcon", i), {
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                        .image = { .imageData = ctx->data->tabIcons[i+2] },
                    }) {}
                } else {
                    CLAY(CLAY_IDI("TabIcon", i), {
                        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                        .image = { .imageData = ctx->data->tabIcons[1] },
                    }) {}
                }
            } else {
                CLAY(CLAY_IDI("TabIcon", i), {
                    .layout = { .sizing = { .width = CLAY_SIZING_FIXED(tabIconSize), .height = CLAY_SIZING_FIXED(tabIconSize) } },
                    .image = { .imageData = ctx->data->tabIcons[0] },
                }) {}
            }
        }
    }
}

Clay_RenderCommandArray clay_battery_only(BatteryData *battery, int width, int height) {
    float deltaTime = get_delta();
    int debugOpacity = 255;
    Clay_Dimensions batteryDimensions = (Clay_Dimensions) { (float) width/1.5, (float) height/1.5 };
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();
    CLAY(CLAY_ID("BatteryDisplayContext"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = expand
        }
    }) {
        render_battery(*battery, batteryDimensions);
    }
    return Clay_EndLayout(deltaTime);
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