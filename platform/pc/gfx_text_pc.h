#ifndef GFX_TEXT_PC_H
#define GFX_TEXT_PC_H

#include <SDL3/SDL.h>

void gfx_text_measure(int fontSize, const char *text, int len, int *outWidth, int *outHeight);
void gfx_text_draw(SDL_Renderer *renderer, int fontSize, const char *text, int len, int x, int y, SDL_Color color);

#endif