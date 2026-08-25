//surface.c
#include "surface.h"
#include <string.h>
#include <stdio.h>

void surface_init(Surface *surface, int originX, int originY, int width, int height) {
    surface->originX = originX;
    surface->originY = originY;
    surface->width = width;
    surface->height = height;
    surface->count = 0;
}

void surface_set_region(Surface *surface, int originX, int originY, int width, int height) {
    surface->originX = originX;
    surface->originY = originY;
    surface->width = width;
    surface->height = height;
}

static SurfaceCommand *push_command(Surface *surface) {
    if (surface->count >= MAX_SURFACE_COMMANDS) {
        printf("Surface: command overflow, dropping draw call\n");
        return NULL;
    }
    SurfaceCommand *cmd = &surface->commands[surface->count++];
    memset(cmd, 0, sizeof(SurfaceCommand));
    return cmd;
}

void surface_push_rect(Surface *surface, int x, int y, int w, int h, Clay_Color color) {
    SurfaceCommand *cmd = push_command(surface);
    if (!cmd) return;
    cmd->type = SURFACE_CMD_RECT;
    cmd->x = x; cmd->y = y; cmd->w = w; cmd->h = h;
    cmd->color = color;
}

void surface_push_quad(Surface *surface, int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, Clay_Color color) {
    SurfaceCommand *cmd = push_command(surface);
    if (!cmd) return;
    cmd->type = SURFACE_CMD_QUAD;
    cmd->quad[0][0] = x1; cmd->quad[0][1] = y1;
    cmd->quad[1][0] = x2; cmd->quad[1][1] = y2;
    cmd->quad[2][0] = x3; cmd->quad[2][1] = y3;
    cmd->quad[3][0] = x4; cmd->quad[3][1] = y4;
    cmd->color = color;
}

void surface_push_text(Surface *surface, int x, int y, const char *text, int fontId, int fontSize, Clay_Color color) {
    SurfaceCommand *cmd = push_command(surface);
    if (!cmd) return;
    cmd->type = SURFACE_CMD_TEXT;
    cmd->x = x; cmd->y = y;
    snprintf(cmd->text, sizeof(cmd->text), "%s", text);
    cmd->fontId = fontId;
    cmd->fontSize = fontSize;
    cmd->color = color;
}

void surface_push_image(Surface *surface, int x, int y, int w, int h, void *imageHandle) {
    SurfaceCommand *cmd = push_command(surface);
    if (!cmd) return;
    cmd->type = SURFACE_CMD_IMAGE;
    cmd->x = x; cmd->y = y; cmd->w = w; cmd->h = h;
    cmd->imageHandle = imageHandle;
}

static Clay_BoundingBox clamp_to_surface(Surface *surface, int x, int y, int w, int h) {
    int x1 = x;
    int y1 = y;
    int x2 = x + w;
    int y2 = y + h;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 > surface->width) x2 = surface->width;
    if (y2 > surface->height) y2 = surface->height;

    int clampedW = x2 - x1;
    int clampedH = y2 - y1;
    if (clampedW < 0) clampedW = 0;
    if (clampedH < 0) clampedH = 0;

    return (Clay_BoundingBox) {
        (float) (surface->originX + x1),
        (float) (surface->originY + y1),
        (float) clampedW,
        (float) clampedH
    };
}

void surface_render(Display *display, Surface *surface) {
    for (int i = 0; i < surface->count; i++) {
        SurfaceCommand *cmd = &surface->commands[i];

        switch (cmd->type) {
            case SURFACE_CMD_RECT: {
                Clay_BoundingBox box = clamp_to_surface(surface, cmd->x, cmd->y, cmd->w, cmd->h);
                if (box.width <= 0 || box.height <= 0) continue;
                display_fill_rect(display, box, cmd->color);
                break;
            }

            case SURFACE_CMD_QUAD: {
                display_fill_quad(display,
                    surface->originX + cmd->quad[0][0], surface->originY + cmd->quad[0][1],
                    surface->originX + cmd->quad[1][0], surface->originY + cmd->quad[1][1],
                    surface->originX + cmd->quad[2][0], surface->originY + cmd->quad[2][1],
                    surface->originX + cmd->quad[3][0], surface->originY + cmd->quad[3][1],
                    cmd->color);
                break;
            }

            case SURFACE_CMD_TEXT: {
                if (cmd->x < 0 || cmd->y < 0 || cmd->x >= surface->width || cmd->y >= surface->height) continue;

                Clay_BoundingBox box = {
                    (float) (surface->originX + cmd->x),
                    (float) (surface->originY + cmd->y),
                    0, 0
                };
                Clay_TextRenderData text = {0};
                text.stringContents.chars = cmd->text;
                text.stringContents.length = (int32_t) strlen(cmd->text);
                text.textColor = cmd->color;
                text.fontId = cmd->fontId;
                text.fontSize = cmd->fontSize;
                display_draw_text(display, box, text);
                break;
            }

            case SURFACE_CMD_IMAGE: {
                Clay_BoundingBox box = clamp_to_surface(surface, cmd->x, cmd->y, cmd->w, cmd->h);
                if (box.width <= 0 || box.height <= 0) continue;
                Clay_ImageRenderData image = {0};
                image.imageData = cmd->imageHandle;
                display_draw_image(display, box, image);
                break;
            }
        }
    }
    surface->count = 0;
}