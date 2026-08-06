#include "gfxfont.h"

#include "../../assets/fonts/GFX/FreeSans9pt7b.h"
#include "../../assets/fonts/GFX/FreeSansBold18pt7b.h"
#include "../../assets/fonts/GFX/FreeSansBold24pt7b.h"


#include <SDL3/SDL.h>
#include <stdbool.h>

static const GFXfont *selectFont(int fontSize) {
    if (fontSize >= 120) return &FreeSansBold24pt7b;
    if (fontSize >= 60)  return &FreeSansBold18pt7b;
    return &FreeSans9pt7b;
}

static void measureGfxText(const GFXfont *font, const char *text, int len, int *outWidth, int *outMinY, int *outMaxY) {
    int minY = 0, maxY = 0, cursorX = 0;
    bool first = true;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char) text[i];
        if (c < font->first || c > font->last) continue;
        GFXglyph *g = &font->glyph[c - font->first];
        if (first) {
            minY = g->yOffset;
            maxY = g->yOffset + g->height;
            first = false;
        } else {
            if (g->yOffset < minY) minY = g->yOffset;
            if (g->yOffset + g->height > maxY) maxY = g->yOffset + g->height;
        }
        cursorX += g->xAdvance;
    }
    *outWidth = cursorX;
    *outMinY = minY;
    *outMaxY = maxY;
}

void gfx_text_measure(int fontSize, const char *text, int len, int *outWidth, int *outHeight) {
    const GFXfont *font = selectFont(fontSize);
    int width, minY, maxY;
    measureGfxText(font, text, len, &width, &minY, &maxY);
    *outWidth = width;
    *outHeight = maxY - minY;
}

void gfx_text_draw(SDL_Renderer *renderer, int fontSize, const char *text, int len, int x, int y, SDL_Color color) {
    const GFXfont *font = selectFont(fontSize);
    int width, minY, maxY;
    measureGfxText(font, text, len, &width, &minY, &maxY);

    int baselineY = y - minY;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int cursorX = x;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char) text[i];
        if (c < font->first || c > font->last) continue;
        GFXglyph *g = &font->glyph[c - font->first];

        const uint8_t *bitmap = font->bitmap + g->bitmapOffset;
        uint8_t bitBuf = 0;
        int bitPos = 0;

        for (int row = 0; row < g->height; row++) {
            for (int col = 0; col < g->width; col++) {
                if (bitPos == 0) {
                    bitBuf = *bitmap++;
                    bitPos = 8;
                }
                bitPos--;
                if (bitBuf & (1 << bitPos)) {
                    SDL_RenderPoint(renderer, cursorX + g->xOffset + col, baselineY + g->yOffset + row);
                }
            }
        }
        cursorX += g->xAdvance;
    }
}
