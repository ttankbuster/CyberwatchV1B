//display.c
#include "../../src/display.h"
#include "../../external/clay/clay.h"

#include <stdlib.h>
#include <stdio.h>

#define NUM_FONTS 2


bool display_init(Display *display) {

}

void display_shutdown(Display *display) {

}

bool display_poll_events(Display *display, bool *running) {

}

DisplaySize display_get_size(Display *display) {
    return (DisplaySize) { display->width, display->height };
}


void display_clear(Display *display, Clay_Color colour) {
}

void display_fill_rect(Display *display, Clay_BoundingBox box, Clay_Color colour) {
}

void display_draw_border(Display *display, Clay_BoundingBox box, Clay_BorderRenderData border) {
}

void display_draw_text(Display *display, Clay_BoundingBox box, Clay_TextRenderData text) {
}

void display_draw_image(Display *display, Clay_BoundingBox box, Clay_ImageRenderData image) {

}

void display_set_clip(Display *display, Clay_BoundingBox box) {

}

void display_clear_clip(Display *display) {

}

void display_present(Display *display) {

}

Clay_Dimensions display_measure_text(Clay_StringSlice text, Clay_TextElementConfig *config, void *userData) {

}
