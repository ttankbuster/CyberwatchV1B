//data_pc.c
#include <stdio.h>
#include <time.h>
#include <SDL3/SDL.h>
#include "../../src/data.h"
#include "../../src/display.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


static void append_event(EventQueue* queue, Event event){
    if (queue->len+1 < MAX_EVENTS){
        queue->events[queue->len] = event;
        queue->len += 1;
    } else {
        printf("EVENT OVERWHELM: SKIPPING\n");
    }
}

static void update_events(CyberwatchData *data, Display *display, bool *running, bool debug) {
    SDL_Event sdl_event;
    Event event;
    event.type = EVENT_NONE;

    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
            case SDL_EVENT_QUIT:
                if (running) *running = false;
                if (debug) { printf("[EVENT] SDL_EVENT_QUIT\n"); }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                if (display) {
                    display->width = sdl_event.window.data1;
                    display->height = sdl_event.window.data2;

                    event.type = EVENT_DISPLAY_ALTERED;
                    append_event(&data->eventQueue, event);
                }
                if (debug) {
                    printf("[EVENT] SDL_EVENT_WINDOW_RESIZED: %d x %d\n", sdl_event.window.data1, sdl_event.window.data2);
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                if (!sdl_event.key.repeat) {
                    if (sdl_event.key.key == SDLK_1) {
                        event.type = EVENT_BUTTON1_DOWN;
                        append_event(&data->eventQueue, event);
                    }
                    if (sdl_event.key.key == SDLK_2) {
                        event.type = EVENT_BUTTON2_DOWN;
                        append_event(&data->eventQueue, event);
                    }
                    if (sdl_event.key.key == SDLK_3) {
                        event.type = EVENT_BUTTON3_DOWN;
                        append_event(&data->eventQueue, event);
                    }
                    if (debug) {
                        printf("[EVENT] SDL_EVENT_KEY_DOWN: Key: %u | key: %d\n", sdl_event.key.key, sdl_event.key.key);
                    }
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (sdl_event.key.key == SDLK_1) {
                    event.type = EVENT_BUTTON1_UP;
                    append_event(&data->eventQueue, event);
                } else if (sdl_event.key.key == SDLK_2) {
                    event.type = EVENT_BUTTON2_UP;
                    append_event(&data->eventQueue, event);
                } else if (sdl_event.key.key == SDLK_3) {
                    event.type = EVENT_BUTTON3_UP;
                    append_event(&data->eventQueue, event);
                }
                if (debug) {
                    printf("[EVENT] SDL_EVENT_KEY_UP: Key: %u | key: %d\n", sdl_event.key.key, sdl_event.key.key);
                }
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

void update_data(CyberwatchData *data, Display *display, bool *running) {
    data->eventQueue.len = 0;
    time_t raw_time = time(NULL);
    struct tm *local_ptr = localtime(&raw_time);
    if (local_ptr != NULL) {
        data->time = *local_ptr;
    }
    update_events(data, display, running, false);
}


FolderList scan_folder(char *path) {
    FolderList result = {0};

    char resolvedPath[512];
    platform_resolve_path(path, resolvedPath, sizeof(resolvedPath));

    DIR *dir = opendir(resolvedPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return result;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && result.count < MAX_FOLDERS) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", resolvedPath, entry->d_name);

        struct stat pathStat;
        if (stat(fullPath, &pathStat) == 0 && S_ISDIR(pathStat.st_mode)) {
            snprintf(result.names[result.count], MAX_FILE_NAME, "%s", entry->d_name);
            result.count++;
        }
    }

    closedir(dir);
    return result;
}