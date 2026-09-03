// app_handler.h
#ifndef APP_HANDLER_H
#define APP_HANDLER_H

#include "../../external/lua/lua-5.4.7/lauxlib.h"
#include "../../external/lua/lua-5.4.7/lua.h"
#include "../../external/lua/lua-5.4.7/lualib.h"
#include "../data/data.h"
#include "../data/display.h"
#include "../data/surface.h"
#include <stdbool.h>

#define MAX_APPS 32

typedef struct Display Display;

typedef struct {
    char name[MAX_FILE_NAME];
    char path[MAX_FILE_NAME];
    char icon[MAX_FILE_NAME];
    char entry[MAX_FILE_NAME];
    char script[MAX_FILE_NAME];
    void* iconHandle;
} AppEntry;

typedef struct AppHandler {
    AppEntry apps[MAX_APPS];
    int appCount;
    lua_State* appLua;
    Surface surface;
    bool devmode;
} AppHandler;

bool app_handler_init(AppHandler* AppHandler, Display* display);
bool app_handler_launch(AppHandler* AppHandler, int id, Display* display);
void app_handler_unload(AppHandler* AppHandler);
void app_handler_run_frame(AppHandler* AppHandler, Display* display, float dt);
void app_handler_dispatch_events(AppHandler* AppHandler, EventQueue* queue);
void app_handler_shutdown(AppHandler* AppHandler);

#endif