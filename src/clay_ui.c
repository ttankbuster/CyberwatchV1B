//clay_ui.c
#define CLAY_IMPLEMENTATION
#include "clay_ui.h"
#include "../cyan/cyan.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static const uint32_t FONT_CLOCK = 0;
static const uint32_t FONT_INFO = 1;

static const Clay_Color CLOCK_COLOUR = {215,125,69,255};
static const Clay_Color INFO_COLOUR = {255,255,255,255};
static const Clay_Color SECONDARY_COLOUR = {150,150,150,255};
static const Clay_Color BG_COLOR = {4,15,24,255};
const char* WEEKDAYS[] = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

typedef enum {
    ELEMENT_CLASS_BG,
    ELEMENT_CLASS_RECT,
} CwElementClass;



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

void showBattery(BatteryData battery, Clay_Dimensions dimensions){
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

typedef struct {
    CyberwatchData *data;
    int debugOpacity;
} FrameContext;
  
static void widget_temperature(FrameContext *ctx, Clay_Sizing sizing) {
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
 
static void widget_battery(FrameContext *ctx, Clay_Sizing sizing, Clay_Dimensions iconDimensions) {
    CLAY(CLAY_ID("BatteryDisplayContext"), {
        .backgroundColor = {48,100,24, ctx->debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = sizing
        }
    }) {
        showBattery(ctx->data->battery, iconDimensions);
    }
}
 
static void render_header_bar(FrameContext *ctx, int headerHeight) {
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
  
static void render_time(FrameContext *ctx, Clay_String timeString, Clay_String dateString) {
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
        }) {
            CLAY_TEXT(timeString, CLAY_TEXT_CONFIG({
                .fontId = FONT_CLOCK,
                .fontSize = 195,
                .textColor = CLOCK_COLOUR
            }));
        }
 
        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65,ctx->debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }) {
            CLAY_TEXT(dateString, CLAY_TEXT_CONFIG({
                .fontId = FONT_INFO,
                .fontSize = 80,
                .textColor = SECONDARY_COLOUR
            }));
        }
    }
}
 
 static void render_footer(FrameContext *ctx, int footerHeight) {
    CLAY(CLAY_ID("Footer"), {
        .backgroundColor = {47,24,24,ctx->debugOpacity},
        .layout = { .sizing = { .height = CLAY_SIZING_FIXED(footerHeight), .width = CLAY_SIZING_GROW() } }
    }) {}
}

Clay_RenderCommandArray clay_cyberwatch(CyberwatchData* data, int width, int height, bool show_debug) {
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
        render_time(&ctx, timeString, dateString);
        render_footer(&ctx, footerHeight);
    }
 
    return Clay_EndLayout(deltaTime);
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
        showBattery(*battery, batteryDimensions);
    }
    return Clay_EndLayout(deltaTime);
}
Clay_RenderCommandArray clay_cyan_catalogue(CyberwatchData* data, Cyan *cyan,int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity;
    if (show_debug){
        debugOpacity = 255;
    } else {
        debugOpacity = 0;
    }

    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();

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
                CLAY(CLAY_IDI("app", i), {
                    .backgroundColor = {48,32,24,debugOpacity},
                    .layout = { .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } }
                }) {
                    Clay_String appNameString = {
                        .isStaticallyAllocated = false,
                        .length = (int32_t) strlen(app->name),
                        .chars = app->name,
                    };
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

    return Clay_EndLayout(deltaTime);
}
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