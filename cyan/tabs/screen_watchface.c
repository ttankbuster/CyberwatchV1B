//screen_watchface.c
#include "../clay_ui.h"
// #include "../data/data.h"
#include "../data/surface.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static void render_time_digital(CyanData *data, int debugOpacity, Clay_String timeString, Clay_String dateString, bool mmddyy) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

    CLAY(CLAY_ID("Watchface"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("WatchTime"), {
            .backgroundColor = {20,46,65,debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.6f) }
            }
        }) {
            CLAY_TEXT(timeString, CLAY_TEXT_CONFIG({
                .fontId = FONT_LARGE,
                .fontSize = 190,
                .textColor = ACCENT_COLOUR
            }));
        }

        CLAY(CLAY_ID("WatchDate"), {
            .backgroundColor = {23,56,65,debugOpacity},
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

static void draw_hand(Surface *surface, int centre_x, int centre_y, float angle, float length, float thickness, Clay_Color color) {
    float half_thickness = thickness / 2.0f;
    float direction_x = sinf(angle);
    float direction_y = -cosf(angle);
    float perpendicular_x = -direction_y * half_thickness;
    float perpendicular_y = direction_x * half_thickness;

    int x1 = (int) (centre_x - perpendicular_x);
    int y1 = (int) (centre_y - perpendicular_y);
    int x2 = (int) (centre_x + perpendicular_x);
    int y2 = (int) (centre_y + perpendicular_y);
    int x3 = (int) (centre_x + direction_x * length + perpendicular_x);
    int y3 = (int) (centre_y + direction_y * length + perpendicular_y);
    int x4 = (int) (centre_x + direction_x * length - perpendicular_x);
    int y4 = (int) (centre_y + direction_y * length - perpendicular_y);

    surface_push_quad(surface, x1, y1, x2, y2, x3, y3, x4, y4, color);
}

static void render_time_analogue(CyanData *data, int debugOpacity, float H, float M, float S) { // [H = hour, M = minute, S = second] measured in radians
    CLAY(CLAY_ID("Watchface"), {
        .backgroundColor = {20,46,65,debugOpacity},
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
        }
    }) {
        Surface *surface = &data->watchface.analogueSurface;
        int centre_x = surface->width / 2;
        int centre_y = surface->height / 2;
        float hand_length = (surface->width < surface->height ? surface->width : surface->height) / 2.0f - 20.0f;

        draw_hand(surface, centre_x, centre_y, H, hand_length * 0.5f, 14.0f, INFO_COLOUR);
        draw_hand(surface, centre_x, centre_y, M, hand_length * 0.75f, 10.0f, INFO_COLOUR);
        draw_hand(surface, centre_x, centre_y, S, hand_length * 0.9f, 4.0f, ACCENT_COLOUR);

        int pivot_radius = 5;
        surface_push_rect(surface, centre_x - pivot_radius, centre_y - pivot_radius, pivot_radius * 2, pivot_radius * 2, ACCENT_COLOUR);
    }
}

void read_time_date(CyanData* data){
    WatchfaceData *wf = &data->watchface;
    snprintf(wf->timeChars, sizeof(wf->timeChars),"%02d:%02d",wf->time.tm_hour,wf->time.tm_min);
    snprintf(wf->dateChars, sizeof(wf->dateChars),"%02d/%02d/%02d %.3s",wf->time.tm_mday,wf->time.tm_mon,wf->time.tm_year%100, WEEKDAYS[wf->time.tm_wday]);
}

Clay_RenderCommandArray clay_watchface(CyanData* data, int width, int height, bool analogue, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;
    read_time_date(data);
    Clay_String timeString = { .chars = data->watchface.timeChars, .length = strlen(data->watchface.timeChars), .isStaticallyAllocated = false };
    Clay_String dateString = { .chars = data->watchface.dateChars, .length = strlen(data->watchface.dateChars), .isStaticallyAllocated = false };

    int headerHeight = (int) (height * 0.1f);
    int footerHeight = (int) (height * 0.1f);
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
        render_header_bar(data, debugOpacity, width, headerHeight);
        if (analogue) {
            surface_init(&data->watchface.analogueSurface, 0, headerHeight, width, height - headerHeight - footerHeight);
            float hourAngle = (data->watchface.time.tm_hour % 12) * (M_PI / 6.0f) + data->watchface.time.tm_min * (M_PI / 360.0f);
            float minuteAngle = data->watchface.time.tm_min * (M_PI / 30.0f);
            float secondAngle = data->watchface.time.tm_sec * (M_PI / 30.0f);
            render_time_analogue(data, debugOpacity, hourAngle, minuteAngle, secondAngle);
        } else {
            render_time_digital(data, debugOpacity, timeString, dateString, false);
        }
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}