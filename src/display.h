#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>
#include "clay.h"

typedef struct CyberwatchData CyberwatchData;

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

bool display_init(Display *display, CyberwatchData *data);
void display_shutdown(Display *display);

DisplaySize display_get_size(Display *display);

void display_clear(Display *display, Clay_Color colour);
void display_fill_rect(Display *display, Clay_BoundingBox box, Clay_Color colour);
void display_draw_border(Display *display, Clay_BoundingBox box, Clay_BorderRenderData border);
void display_draw_text(Display *display, Clay_BoundingBox box, Clay_TextRenderData text);
void display_draw_image(Display *display, Clay_BoundingBox box, Clay_ImageRenderData image);
void display_set_clip(Display *display, Clay_BoundingBox box);
void display_clear_clip(Display *display);
void display_present(Display *display);
float get_delta(void);

Clay_Dimensions display_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData);

#endif
