//app_handler.h
#ifndef APP_HANDLER_H
#define APP_HANDLER_H

#include <stdbool.h>
#include "../../external/lua/lua-5.4.7/lua.h"
#include "../../external/lua/lua-5.4.7/lualib.h"
#include "../../external/lua/lua-5.4.7/lauxlib.h"
#include "../data/data.h"
#include "../data/display.h"
#include "surface.h"

#define MAX_APPS 32

typedef struct Display Display;

typedef struct {
    char name[MAX_FILE_NAME];
    char path[MAX_FILE_NAME];
    char icon[MAX_FILE_NAME];
    char entry[MAX_FILE_NAME];
    char script[MAX_FILE_NAME];
    void *iconHandle;
} AppEntry;

typedef struct AppHandler {
    AppEntry apps[MAX_APPS];
    int appCount;
    lua_State *appLua;
    Surface surface;
    bool devmode;
} AppHandler;

bool AppHandler_init(AppHandler *AppHandler, Display *display);
bool AppHandler_launch(AppHandler *AppHandler, int id, Display *display);
void AppHandler_unload(AppHandler *AppHandler);
void AppHandler_update(AppHandler *AppHandler, Display *display, float dt);
void AppHandler_dispatch_events(AppHandler *AppHandler, EventQueue *queue);
void AppHandler_shutdown(AppHandler *AppHandler);

#endif