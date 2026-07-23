//data_pc.c
#include <stdio.h>
#include <time.h>
#include "../../src/data.h"
#include <SDL3/SDL.h>


static void append_event(EventQueue* queue, Event event){
    if (queue->len+1 < MAX_EVENTS){
        queue->events[queue->len] = event;
    } else {
        printf("EVENT OVERWHELM: SKIPPING\n");
    }
}

static void update_events(CyberwatchData* data, bool debug) {
    SDL_Event sdl_event;
    Event event;
    event.type = EVENT_NONE;
    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
            case SDL_EVENT_KEY_DOWN:
                    if (!sdl_event.key.repeat) { 
                        if (sdl_event.key.key == SDLK_1){
                            event.type = EVENT_BUTTON1_DOWN;
                            append_event(&data->eventQueue, event);
                        }
                        if (sdl_event.key.key == SDLK_2){
                            event.type = EVENT_BUTTON2_DOWN;
                            append_event(&data->eventQueue, event);
                        }
                        if (sdl_event.key.key == SDLK_3){
                            event.type = EVENT_BUTTON3_DOWN;
                            append_event(&data->eventQueue, event);
                        }
                        if (debug){printf("[EVENT] SDL_EVENT_KEY_DOWN: Key: %u | key: %d\n", sdl_event.key.key, sdl_event.key.key);}
                    }
                break;
            case SDL_EVENT_KEY_UP:
                if (sdl_event.key.key == SDLK_1){
                    event.type = EVENT_BUTTON1_UP;
                    append_event(&data->eventQueue, event);
                } else if (sdl_event.key.key == SDLK_2) {
                    event.type = EVENT_BUTTON2_UP;
                    append_event(&data->eventQueue, event);
                } else if (sdl_event.key.key == SDLK_3) {
                    event.type = EVENT_BUTTON3_UP;
                    append_event(&data->eventQueue, event);
                }
                if (debug){printf("[EVENT] SDL_EVENT_KEY_UP: Key: %u | key: %d\n", sdl_event.key.key, sdl_event.key.key);}
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                if (sdl_event.wheel.y > 0.0f) {
                    event.type = EVENT_SCROLL_UP;
                    append_event(&data->eventQueue, event);
                } else if (sdl_event.wheel.y < 0.0f) {
                    event.type = EVENT_SCROLL_DOWN;
                    append_event(&data->eventQueue, event);
                }
                if (debug) { printf("[EVENT] SDL_EVENT_MOUSE_WHEEL: y: %f\n", sdl_event.wheel.y); }
                break;
            default:
                break;
        }
    }
}

void platform_resolve_path(const char *relativePath, char *outBuffer, size_t bufferSize) {
    const char *basePath = SDL_GetBasePath();
    SDL_snprintf(outBuffer, bufferSize, "%s../../%s", basePath ? basePath : "", relativePath);
}

void update_data(CyberwatchData* data) {
    data->eventQueue.len = 0;
    time_t raw_time = time(NULL);
    struct tm *local_ptr = localtime(&raw_time);
    if (local_ptr != NULL) {
        data->time = *local_ptr;
    }
    update_events(data, false);
}