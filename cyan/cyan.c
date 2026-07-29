//cyan.c
#include "cyan.h"
#include "surface.h"
#include <string.h>

static int cyan_print(lua_State *L) {
    const char *appName = (const char *) lua_touserdata(L, lua_upvalueindex(1));
    int n = lua_gettop(L);
    printf("CYAN[%s]> ", appName ? appName : "?");
    for (int i = 1; i <= n; i++) {
        size_t len;
        const char *str = luaL_tolstring(L, i, &len);
        fwrite(str, 1, len, stdout);
        if (i < n) putchar('\t');
        lua_pop(L, 1);
    }
    putchar('\n');
    return 0;
}

static void push_event_constants(lua_State *L) {
    lua_newtable(L);

    #define EVENT_CONST(name) lua_pushinteger(L, name); lua_setfield(L, -2, #name + 6);

    EVENT_CONST(EVENT_NONE)
    EVENT_CONST(EVENT_DISPLAY_ALTERED)
    EVENT_CONST(EVENT_BUTTON1_UP)
    EVENT_CONST(EVENT_BUTTON1_DOWN)
    EVENT_CONST(EVENT_BUTTON2_UP)
    EVENT_CONST(EVENT_BUTTON2_DOWN)
    EVENT_CONST(EVENT_BUTTON3_UP)
    EVENT_CONST(EVENT_BUTTON3_DOWN)
    EVENT_CONST(EVENT_SCROLL_UP)
    EVENT_CONST(EVENT_SCROLL_DOWN)

    #undef EVENT_CONST

    lua_setglobal(L, "Event");
}
static void register_sandbox_api(lua_State *L, const char *appName) {
    lua_pushlightuserdata(L, (void *) appName);
    lua_pushcclosure(L, cyan_print, 1);
    lua_setglobal(L, "print");

    push_event_constants(L);
}

static int lua_draw_rect(lua_State *L) {
    Surface *surface = (Surface *) lua_touserdata(L, lua_upvalueindex(1));
    int x = (int) luaL_checknumber(L, 1);
    int y = (int) luaL_checknumber(L, 2);
    int w = (int) luaL_checknumber(L, 3);
    int h = (int) luaL_checknumber(L, 4);
    int r = (int) luaL_optnumber(L, 5, 255);
    int g = (int) luaL_optnumber(L, 6, 255);
    int b = (int) luaL_optnumber(L, 7, 255);
    int a = (int) luaL_optnumber(L, 8, 255);
    surface_push_rect(surface, x, y, w, h, (Clay_Color){ (float) r, (float) g, (float) b, (float) a });
    return 0;
}

static int lua_draw_width(lua_State *L) {
    Surface *surface = (Surface *) lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, surface->width);
    return 1;
}

static int lua_draw_height(lua_State *L) {
    Surface *surface = (Surface *) lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, surface->height);
    return 1;
}

static int lua_draw_text(lua_State *L) {
    Surface *surface = (Surface *) lua_touserdata(L, lua_upvalueindex(1));
    int x = (int) luaL_checknumber(L, 1);
    int y = (int) luaL_checknumber(L, 2);
    const char *text = luaL_checkstring(L, 3);
    int fontSize = (int) luaL_optnumber(L, 4, 16);
    int r = (int) luaL_optnumber(L, 5, 255);
    int g = (int) luaL_optnumber(L, 6, 255);
    int b = (int) luaL_optnumber(L, 7, 255);
    surface_push_text(surface, x, y, text, 0, fontSize, (Clay_Color){ (float) r, (float) g, (float) b, 255 });
    return 0;
}


static void push_draw_closure(lua_State *L, lua_CFunction fn, Surface *surface, Display *display) {
    lua_pushlightuserdata(L, surface);
    lua_pushlightuserdata(L, display);
    lua_pushcclosure(L, fn, 2);
}

static void register_draw_api(lua_State *L, Surface *surface, Display *display) {
    lua_newtable(L);

    push_draw_closure(L, lua_draw_rect, surface, display);
    lua_setfield(L, -2, "rect");

    push_draw_closure(L, lua_draw_text, surface, display);
    lua_setfield(L, -2, "text");

    push_draw_closure(L, lua_draw_width, surface, display);
    lua_setfield(L, -2, "width");

    push_draw_closure(L, lua_draw_height, surface, display);
    lua_setfield(L, -2, "height");

    lua_setglobal(L, "draw");
}

static bool load_app_manifest(CyanApp *app, bool load_dev) {
    char manifestPath[MAX_FILE_PATH];
    snprintf(manifestPath, sizeof(manifestPath), "%s/.cyan_app.lua", app->path);

    char resolvedPath[512];
    platform_store_resolved_path(manifestPath, resolvedPath, sizeof(resolvedPath));

    lua_State *L = luaL_newstate();
    if (!L) return false;

    if (luaL_dofile(L, resolvedPath) != LUA_OK) {
        fprintf(stderr, "Cyan: failed to load manifest '%s': %s\n", resolvedPath, lua_tostring(L, -1));
        lua_close(L);
        return false;
    }

    if (!lua_istable(L, -1)) {
        fprintf(stderr, "Cyan: manifest '%s' did not return a table\n", resolvedPath);
        lua_close(L);
        return false;
    }

    #define READ_STRING_FIELD(field, dest) \
        lua_getfield(L, -1, field); \
        if (lua_isstring(L, -1)) { snprintf(dest, sizeof(dest), "%s", lua_tostring(L, -1)); } lua_pop(L, 1);
    
    #define READ_BOOL_FIELD(field, dest) \
        lua_getfield(L, -1, field); \
        if (lua_isboolean(L, -1)) { dest = lua_toboolean(L, -1); } lua_pop(L, 1);

    bool dev = false;
    READ_BOOL_FIELD("dev", dev)
    if (dev && !load_dev) { lua_close(L); return false; }
    READ_STRING_FIELD("name", app->name)
    READ_STRING_FIELD("icon", app->icon)

    READ_STRING_FIELD("script", app->script)
    #undef READ_STRING_FIELD
    #undef READ_BOOL_FIELD

    lua_close(L);
    return true;
}

static bool load_app_icon(CyanApp *app, Display *display) {
    if (app->icon[0] == '\0') {
        return true;
    }

    char iconPath[MAX_FILE_PATH];
    snprintf(iconPath, sizeof(iconPath), "%s/%s", app->path, app->icon);
    if (load_image(display, iconPath, &app->iconHandle)) {
        return true;
    }

    fprintf(stderr, "Cyan: failed to load icon '%s', using fallback\n", iconPath);
    if (load_image(display, "assets/icons/test.png", &app->iconHandle)) {
        return true;
    }

    fprintf(stderr, "Cyan: failed to load fallback icon\n");
    return false;
}

void cyan_index_apps(Cyan *cyan, char *path, Display *display) {
    FolderList folders = scan_folder(path);

    cyan->appCount = 0;
    for (int i = 0; i < folders.count && cyan->appCount < MAX_APPS; i++) {
        CyanApp app = {0};
        snprintf(app.name, MAX_FILE_PATH, "%s", folders.names[i]);
        snprintf(app.path, MAX_FILE_PATH, "%s/%s", path, folders.names[i]);

        if (load_app_manifest(&app, cyan->devmode)) {
            load_app_icon(&app, display);
            cyan->apps[cyan->appCount] = app;
            cyan->appCount++;
        }
    }

    printf("Cyan: indexed %d app(s)\n", cyan->appCount);
    for (int i = 0; i < cyan->appCount; i++) {
        printf("  %d: %s (script: %s, icon: %s, path: %s)\n", i, cyan->apps[i].name, cyan->apps[i].script, cyan->apps[i].icon, cyan->apps[i].path);
    }
}

bool cyan_init(Cyan *cyan, Display *display) {
    cyan->selectedApp = -1;
    cyan->appCount = 0;
    cyan->appLua = NULL;
    cyan->devmode = true;
    cyan_index_apps(cyan, "cyan/apps", display);

    return true;
}

bool cyan_launch_app(Cyan *cyan, int id, Display *display) {

    if (id < 0 || id > cyan->appCount - 1) {
        printf("Cyan: incorrect app ID.\n");
        return false;
    }

    if (cyan->appLua) {
        cyan_unload_app(cyan);
    }

    CyanApp *app = &cyan->apps[id];
    printf("Cyan: launching app [%d]: %s\n", id, app->name);

    // Create the runtime FIRST — L doesn't exist before this line.
    cyan->appLua = luaL_newstate();
    if (!cyan->appLua) {
        printf("Cyan: failed to create runtime for '%s'\n", app->name);
        return false;
    }
    lua_State *L = cyan->appLua;

    DisplaySize size = display_get_size(display);
    surface_init(&cyan->surface, 0, 0, size.width, size.height);

    luaL_requiref(L, "_G", luaopen_base, 1);
    lua_pop(L, 1);
    register_sandbox_api(L, app->name);
    register_draw_api(L, &cyan->surface, display);
    char scriptRelPath[MAX_FILE_PATH];
    snprintf(scriptRelPath, sizeof(scriptRelPath), "%s/%s", app->path, app->script);
    char resolvedPath[512];
    platform_store_resolved_path(scriptRelPath, resolvedPath, sizeof(resolvedPath));

    if (luaL_dofile(L, resolvedPath) != LUA_OK) {
        fprintf(stderr, "Cyan: failed to load script '%s': %s\n", resolvedPath, lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_close(L);
        cyan->appLua = NULL;
        return false;
    }

    lua_getglobal(L, "on_load");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_load error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    cyan->selectedApp = id;
    return true;
}

void cyan_run_frame(Cyan *cyan, Display *display, float dt) {
    lua_State *L = cyan->appLua;

    lua_getglobal(L, "on_update");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_update error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_getglobal(L, "on_draw");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_draw error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
}

void cyan_dispatch_events(Cyan *cyan, EventQueue *queue) {
    if (!cyan->appLua) {
        printf("ERROR: no cyan app to dispatch events to.\n"); 
        return;
    }
    lua_State *L = cyan->appLua;
    for (int i = 0; i < queue->len; i++) {
        Event *ev = &queue->events[i];
        lua_getglobal(L, "on_event");
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue;
        }

        lua_pushinteger(L, ev->type);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_event error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

void cyan_unload_app(Cyan *cyan) {
    if (!cyan->appLua) return;
    lua_State *L = cyan->appLua;

    lua_getglobal(L, "on_unload");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_unload error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_close(L);
    cyan->appLua = NULL;
    cyan->selectedApp = -1;
}

void cyan_shutdown(Cyan *cyan) {
    if (cyan->appLua) {
        lua_close(cyan->appLua);
        cyan->appLua = NULL;
    }
}