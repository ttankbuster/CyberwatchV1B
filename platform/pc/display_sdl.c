//display.c
#include "../../src/data.h"
#include "../../src/display.h"

#include <stdlib.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>   

#define NUM_FONTS 2

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_TextEngine *textEngine;
    TTF_Font *fonts[NUM_FONTS];
} SdlBackend;

static SDL_Color toSdlColor(Clay_Color colour) {
    return (SDL_Color) { (Uint8) colour.r, (Uint8) colour.g, (Uint8) colour.b, (Uint8) colour.a };
}

static TTF_Font *_loadFontRelative(const char *relativePath, int pointSize) {
    const char *basePath = SDL_GetBasePath();
    char fullPath[512];
    SDL_snprintf(fullPath, sizeof(fullPath), "%s../../%s", basePath ? basePath : "", relativePath);

    TTF_Font *font = TTF_OpenFont(fullPath, pointSize);
    if (!font) {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font '%s': %s", fullPath, SDL_GetError());
    }
    return font;
}

bool display_init(Display *display, CyberwatchData *data) {

    SdlBackend *backend = calloc(1, sizeof(SdlBackend));
    display->width = 240*3;
    display->height = 280*3;
    if (!backend) return false;
    display->backend = backend;
    
    if (!TTF_Init()) return false;
    
    if (!SDL_CreateWindowAndRenderer("Watch", display->width, display->height, SDL_WINDOW_RESIZABLE, &backend->window, &backend->renderer)) {
        return false;
    }
    
    backend->textEngine = TTF_CreateRendererTextEngine(backend->renderer);
    if (!backend->textEngine) return false;
    
    backend->fonts[0] = _loadFontRelative("assets/fonts/Lexend_Deca/static/LexendDeca-Bold.ttf", 42);
    backend->fonts[1] = _loadFontRelative("assets/fonts/Lexend_Deca/static/LexendDeca-Light.ttf", 80);
    if (!backend->fonts[0] || !backend->fonts[1]) return false;
    
    data->battery = (BatteryData){.charge=1.0f};
    char batteryIconPath[MAX_FILE_PATH];
    platform_resolve_path("assets\\icons\\battery.png", batteryIconPath, sizeof(batteryIconPath));
    data->battery.icon = IMG_LoadTexture(backend->renderer, batteryIconPath);
    printf("battery icon: %s\n", batteryIconPath);
    return true;
}

void display_shutdown(Display *display) {
    printf("shutting down SDL...\n");
    SdlBackend *backend = display->backend;
    if (!backend) return;

    for (int i = 0; i < NUM_FONTS; i++) {
        if (backend->fonts[i]) TTF_CloseFont(backend->fonts[i]);
    }
    if (backend->textEngine) TTF_DestroyRendererTextEngine(backend->textEngine);
    if (backend->renderer) SDL_DestroyRenderer(backend->renderer);
    if (backend->window) SDL_DestroyWindow(backend->window);
    free(backend);
    display->backend = NULL;

    TTF_Quit();
    printf("SDL shutdown\n");
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
    TTF_Font *font = backend->fonts[text.fontId];
    TTF_SetFontSize(font, text.fontSize);

    SDL_Color c = toSdlColor(text.textColor);
    TTF_Text *ttfText = TTF_CreateText(backend->textEngine, font, text.stringContents.chars, text.stringContents.length);
    if (!ttfText) return;
    TTF_SetTextColor(ttfText, c.r, c.g, c.b, c.a);
    TTF_DrawRendererText(ttfText, box.x, box.y);
    TTF_DestroyText(ttfText);
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
    Display *display = userData;
    SdlBackend *backend = display->backend;
    TTF_Font *font = backend->fonts[config->fontId];
    int width = 0, height = 0;
    TTF_SetFontSize(font, config->fontSize);
    TTF_GetStringSize(font, text.chars, text.length, &width, &height);
    return (Clay_Dimensions) { (float) width, (float) height };
}

