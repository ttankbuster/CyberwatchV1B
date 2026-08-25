//display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "../external/clay/clay.h"
#include "log.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CyanData CyanData;

typedef struct Display {
    int width;
    int height;
    void *backend;
} Display;

typedef struct {
    int width;
    int height;
} DisplaySize;

typedef enum {
    DISPICON_BATTERY_OUTLINE,
} DisplayIcon;

bool display_init(Display *display, CyanData *data);
void display_shutdown(Display *display);

DisplaySize display_get_size(Display *display);

void display_clear(Display *display, Clay_Color colour);
void display_fill_rect(Display *display, Clay_BoundingBox box, Clay_Color colour);
void display_fill_quad(Display *display, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, Clay_Color colour);
void display_draw_border(Display *display, Clay_BoundingBox box, Clay_BorderRenderData border);
void display_draw_text(Display *display, Clay_BoundingBox box, Clay_TextRenderData text);
void display_draw_image(Display *display, Clay_BoundingBox box, Clay_ImageRenderData image);
void display_set_clip(Display *display, Clay_BoundingBox box);
void display_clear_clip(Display *display);
void display_present(Display *display);
void display_loading_log_listener(VerbosityLevel level, const char *message);
void display_loading_screen(Display *display, float progress);

Clay_Dimensions display_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);

#ifdef __cplusplus
}
#endif

#endif