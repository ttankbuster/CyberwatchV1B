#ifndef CYAN_H
#define CYAN_H

#include <stdbool.h>
#include "../external/lua/lua-5.4.7/lua.h"
#include "../external/lua/lua-5.4.7/lualib.h"
#include "../external/lua/lua-5.4.7/lauxlib.h"
#include "../../src/display.h"
#include "../../src/data.h"
#include "clay.h"

#define MAX_APPS 32

typedef struct {
    char name[MAX_FILE_NAME];   // folder name, may be overridden by manifest
    char path[MAX_FILE_NAME];   // relative path to the app's folder
    char icon[MAX_FILE_NAME];   // relative path to icon asset, loaded later
    char entry[MAX_FILE_NAME];  // relative path to the entry script
    void *iconHandle;
} CyanApp;



typedef enum {
    CYAN_MODE_HOME,
    CYAN_MODE_APP_MENU,
    CYAN_MODE_RUNNING_APP,
} CyanMode;

typedef struct {
    CyanMode mode;
    int selectedApp;
    CyanApp apps[MAX_APPS];
    int appCount;
    lua_State *lua;
} Cyan;

void cyan_index_apps(Cyan* cyan, Display* display, char* path);
bool cyan_init(Cyan *cyan, const char *appScriptPath);
void cyan_dispatch_events(Cyan *cyan, EventQueue *queue);
void cyan_shutdown(Cyan *cyan);

#endif