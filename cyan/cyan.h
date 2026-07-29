#ifndef CYAN_H
#define CYAN_H

#include <stdbool.h>
#include "../external/lua/lua-5.4.7/lua.h"
#include "../external/lua/lua-5.4.7/lualib.h"
#include "../external/lua/lua-5.4.7/lauxlib.h"
#include "../src/data.h"
#include "../src/display.h"
#include "surface.h"

#define MAX_APPS 32

typedef struct Display Display;


typedef struct {
    char name[MAX_FILE_NAME];   // folder name, may be overridden by manifest
    char path[MAX_FILE_NAME];   // relative path to the app's folder
    char icon[MAX_FILE_NAME];   // relative path to icon asset, loaded later
    char entry[MAX_FILE_NAME];  // relative path to the entry script
    char script[MAX_FILE_NAME]; // optional script value from manifest
    void *iconHandle;
} CyanApp;


typedef struct Cyan {
    int selectedApp;
    CyanApp apps[MAX_APPS];
    int appCount;
    lua_State *appLua;
    Surface surface;
    int highlightedApp;     // which app the dial cursor is currently on
    int catalogueScrollY;   // current scroll offset, managed manually (see above)
    bool devmode;
} Cyan;


void cyan_index_apps(Cyan* cyan, char* path, Display *display);
bool cyan_init(Cyan *cyan, Display *display);
bool cyan_launch_app(Cyan *cyan, int id, Display *display);
void cyan_unload_app(Cyan *cyan);
void cyan_run_frame(Cyan *cyan, Display *display, float dt);
void cyan_dispatch_events(Cyan *cyan, EventQueue *queue);
void cyan_shutdown(Cyan *cyan);

#endif