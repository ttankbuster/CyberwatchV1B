#ifndef SURFACE_H
#define SURFACE_H

#include "../external/clay/clay.h"
#include "../src/display.h"

#define MAX_SURFACE_COMMANDS 128
#define MAX_SURFACE_TEXT 64

typedef enum {
    SURFACE_CMD_RECT,
    SURFACE_CMD_TEXT,
    SURFACE_CMD_IMAGE,
} SurfaceCommandType;

typedef struct {
    SurfaceCommandType type;
    int x, y, w, h;
    Clay_Color color;
    char text[MAX_SURFACE_TEXT];
    int fontId;
    int fontSize;
    void *imageHandle;
} SurfaceCommand;

typedef struct {
    int originX, originY;
    int width, height;
    SurfaceCommand commands[MAX_SURFACE_COMMANDS];
    int count;
} Surface;

void surface_init(Surface *surface, int originX, int originY, int width, int height);
void surface_push_rect(Surface *surface, int x, int y, int w, int h, Clay_Color color);
void surface_push_text(Surface *surface, int x, int y, const char *text, int fontId, int fontSize, Clay_Color color);
void surface_push_image(Surface *surface, int x, int y, int w, int h, void *imageHandle);
void surface_render(Display *display, Surface *surface);
void surface_set_region(Surface *surface, int originX, int originY, int width, int height);
#endif