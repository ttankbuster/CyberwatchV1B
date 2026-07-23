#ifndef CYAN_H
#define CYAN_H

#include <stdbool.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "display.h"
#include "data.h"
#include "clay.h"

typedef struct {
    char name[32];
    char author[32];
    char version[16];
    char script[64];
    char icon[64];
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
    CyanApp apps[16];
    int appCount;
    lua_State *lua;
} Cyan;

void cyan_index_apps(Cyan* cyan, Display* display, char* path);
bool cyan_init(Cyan *cyan, const char *appScriptPath);
void cyan_dispatch_events(Cyan *cyan, EventQueue *queue);
void cyan_shutdown(Cyan *cyan);

#endif