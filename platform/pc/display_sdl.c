//display_sdl.c
#include "../../cyan/data/data.h"
#include "../../cyan/data/display.h"
#include "../../cyan/clay_ui.h"
#include "../../assets/icons/icon_battery.h"
#include "../../assets/icons/icon_empty_tab.h"
#include "../../assets/icons/icon_full_tab.h"
#include "../../assets/icons/icon_cyan1.h"


#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>
// #include <SDL3_ttf/SDL_ttf.h>
#include "gfx_text_pc.h"

#include <SDL3_image/SDL_image.h>   

static SDL_Texture *loadingIconTexture = NULL;
#define MAX_LOG_LINES 32
#define MAX_LOG_LINE_LEN 64
static char logLines[MAX_LOG_LINES][MAX_LOG_LINE_LEN];
static int logLineCount = 0;
 
#define PC_DISPLAY_SCALE_FACTOR 3


typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} SdlBackend;

static SDL_Color toSdlColor(Clay_Color colour) {
    return (SDL_Color) { (Uint8) colour.r, (Uint8) colour.g, (Uint8) colour.b, (Uint8) colour.a };
}



bool display_init(Display *display, CyberwatchData *data) {
    SdlBackend *backend = calloc(1, sizeof(SdlBackend));
    
    display->width = 240*PC_DISPLAY_SCALE_FACTOR;
    display->height = 280*PC_DISPLAY_SCALE_FACTOR;
    if (!backend) return false;
    display->backend = backend;
 
    if (!SDL_CreateWindowAndRenderer("Watch", display->width, display->height, SDL_WINDOW_RESIZABLE, &backend->window, &backend->renderer)) {
        return false;
    }

    SDL_Texture *battery_tex = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STATIC, ICON_BATTERY_WIDTH, ICON_BATTERY_HEIGHT);
    SDL_UpdateTexture(battery_tex, NULL, ICON_BATTERY, ICON_BATTERY_WIDTH * sizeof(uint32_t));
    SDL_SetTextureScaleMode(battery_tex, SDL_SCALEMODE_NEAREST);
    data->batteryIcon = battery_tex;
    SDL_Texture *empty_tab_tex = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STATIC, ICON_EMPTY_TAB_WIDTH, ICON_EMPTY_TAB_HEIGHT);
    SDL_UpdateTexture(empty_tab_tex, NULL, ICON_EMPTY_TAB, ICON_EMPTY_TAB_WIDTH * sizeof(uint32_t));
    SDL_SetTextureScaleMode(empty_tab_tex, SDL_SCALEMODE_NEAREST);
    data->tabs.tabIcons[0] = empty_tab_tex;
    SDL_Texture *full_tab_tex = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STATIC, ICON_FULL_TAB_WIDTH, ICON_FULL_TAB_HEIGHT);
    SDL_UpdateTexture(full_tab_tex, NULL, ICON_FULL_TAB, ICON_FULL_TAB_WIDTH * sizeof(uint32_t));
    SDL_SetTextureScaleMode(full_tab_tex, SDL_SCALEMODE_NEAREST);
    data->tabs.tabIcons[1] = full_tab_tex;
    return true;
}
 
void display_shutdown(Display *display) {
    cyan_log(VERBOSE_MED, "[Display] Shutting down SDL...");
    SdlBackend *backend = display->backend;
    if (!backend) return;
    if (backend->renderer) SDL_DestroyRenderer(backend->renderer);
    if (backend->window) SDL_DestroyWindow(backend->window);
    free(backend);
    display->backend = NULL;
    cyan_log(VERBOSE_MED, "[Display] SDL shutdown");
}

DisplaySize display_get_size(Display *display) {
    return (DisplaySize) { display->width, display->height };
}


void display_clear(Display *display, Clay_Color colour) {
    SdlBackend *backend = display->backend;
    SDL_Color c = toSdlColor(colour);
    SDL_SetRenderDrawColor(backend->renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(backend->renderer);
}

void display_fill_rect(Display *display, Clay_BoundingBox box, Clay_Color colour) {
    SdlBackend *backend = display->backend;
    SDL_Color c = toSdlColor(colour);
    SDL_SetRenderDrawColor(backend->renderer, c.r, c.g, c.b, c.a);
    SDL_FRect rect = { box.x, box.y, box.width, box.height };
    SDL_RenderFillRect(backend->renderer, &rect);
}

void display_draw_border(Display *display, Clay_BoundingBox box, Clay_BorderRenderData border) {
    SdlBackend *backend = display->backend;
    SDL_Color c = toSdlColor(border.color);
    SDL_SetRenderDrawColor(backend->renderer, c.r, c.g, c.b, c.a);

    if (border.width.top > 0) {
        SDL_FRect top = { box.x, box.y, box.width, (float) border.width.top };
        SDL_RenderFillRect(backend->renderer, &top);
    }
    if (border.width.bottom > 0) {
        SDL_FRect bottom = { box.x, box.y + box.height - border.width.bottom, box.width, (float) border.width.bottom };
        SDL_RenderFillRect(backend->renderer, &bottom);
    }
    if (border.width.left > 0) {
        SDL_FRect left = { box.x, box.y, (float) border.width.left, box.height };
        SDL_RenderFillRect(backend->renderer, &left);
    }
    if (border.width.right > 0) {
        SDL_FRect right = { box.x + box.width - border.width.right, box.y, (float) border.width.right, box.height };
        SDL_RenderFillRect(backend->renderer, &right);
    }
}

void display_draw_text(Display *display, Clay_BoundingBox box, Clay_TextRenderData text) {
    SdlBackend *backend = display->backend;
    SDL_Color c = toSdlColor(text.textColor);
    gfx_text_draw(backend->renderer, text.fontSize, text.stringContents.chars, text.stringContents.length, (int) box.x, (int) box.y, c);
}

void display_draw_image(Display *display, Clay_BoundingBox box, Clay_ImageRenderData image) {
    SdlBackend *backend = display->backend;
    SDL_Texture *texture = image.imageData;
    if (!texture) return;
    SDL_FRect rect = { box.x, box.y, box.width, box.height };
    SDL_RenderTexture(backend->renderer, texture, NULL, &rect);
}

void display_set_clip(Display *display, Clay_BoundingBox box) {
    SdlBackend *backend = display->backend;
    SDL_Rect rect = { (int) box.x, (int) box.y, (int) box.width, (int) box.height };
    SDL_SetRenderClipRect(backend->renderer, &rect);
}

void display_clear_clip(Display *display) {
    SdlBackend *backend = display->backend;
    SDL_SetRenderClipRect(backend->renderer, NULL);
}

void display_present(Display *display) {
    SdlBackend *backend = display->backend;
    SDL_RenderPresent(backend->renderer);
}

Clay_Dimensions display_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {
    int width, height;
    gfx_text_measure(config->fontSize, text.chars, text.length, &width, &height);
    return (Clay_Dimensions) { (float) width, (float) height };
}

void display_loading_log_listener(VerbosityLevel level, const char *message) {
    (void) level;
    if (logLineCount < MAX_LOG_LINES) {
        snprintf(logLines[logLineCount], MAX_LOG_LINE_LEN, "%s", message);
        logLineCount++;
    } else {
        for (int i = 0; i < MAX_LOG_LINES - 1; i++) {
            memcpy(logLines[i], logLines[i + 1], MAX_LOG_LINE_LEN);
        }
        snprintf(logLines[MAX_LOG_LINES - 1], MAX_LOG_LINE_LEN, "%s", message);
    }
}

 
void display_loading_screen(Display *display, float progress) {
    SdlBackend *backend = display->backend;

    if (!loadingIconTexture) {
        loadingIconTexture = SDL_CreateTexture(backend->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STATIC, ICON_CYAN1_WIDTH, ICON_CYAN1_HEIGHT);
        SDL_UpdateTexture(loadingIconTexture, NULL, ICON_CYAN1, ICON_CYAN1_WIDTH * sizeof(uint32_t));
        SDL_SetTextureScaleMode(loadingIconTexture, SDL_SCALEMODE_NEAREST);
    }

    
    display_clear(display, (Clay_Color){0,0,0,255});
    DisplaySize size = display_get_size(display);

    float scaleX = (float) size.width / (float) ICON_CYAN1_WIDTH;
    float scaleY = (float) size.height / (float) ICON_CYAN1_HEIGHT;
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int drawWidth = (int) (ICON_CYAN1_WIDTH * scale);
    int drawHeight = (int) (ICON_CYAN1_HEIGHT * scale);
    int drawX = (size.width - drawWidth) / 2;
    int drawY = (size.height - drawHeight) / 2;

    int lineHeight = 18;
    int bottomMargin = 10;
    SDL_Color logColor = { 150, 150, 150, 255 };

    int y = size.height - bottomMargin - lineHeight;
    for (int i = logLineCount - 1; i >= 0; i--) {
        if (y < 0) break; // cull
        char *line = logLines[i];
        gfx_text_draw(backend->renderer, 20, line, (int) strlen(line), 10, y, logColor);
        y -= lineHeight;
    }
    display_draw_image(display, (Clay_BoundingBox){ .x = (float) drawX, .y = (float) drawY, .width = (float) drawWidth, .height = (float) drawHeight }, (Clay_ImageRenderData){ .imageData = loadingIconTexture });

    // TODO: draw a progress bar/text here using progress (0.0 - 1.0)

    display_present(display);
}