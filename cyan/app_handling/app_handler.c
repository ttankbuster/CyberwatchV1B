// app_handler.c
#include "app_handler.h"
#include "../data/surface.h"
#include <string.h>

#define APP_MANIFEST ".cyan_app.lua"

static int app_handler_print(lua_State* L) {
    const char* appName = (const char*)lua_touserdata(L, lua_upvalueindex(1));
    int n = lua_gettop(L);
    printf("AppHandler[%s]> ", appName ? appName : "?");
    for (int i = 1; i <= n; i++) {
        size_t len;
        const char* str = luaL_tolstring(L, i, &len);
        fwrite(str, 1, len, stdout);
        if (i < n)
            putchar('\t');
        lua_pop(L, 1);
    }
    putchar('\n');
    return 0;
}

static void push_event_constants(lua_State* L) {
    lua_newtable(L);

#define EVENT_CONST(name)                                                                          \
    lua_pushinteger(L, name);                                                                      \
    lua_setfield(L, -2, #name + 6);

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
static void register_sandbox_api(lua_State* L, const char* appName) {
    lua_pushlightuserdata(L, (void*)appName);
    lua_pushcclosure(L, app_handler_print, 1);
    lua_setglobal(L, "print");

    push_event_constants(L);
}

static int app_handler_draw_rect(lua_State* L) {
    Surface* surface = (Surface*)lua_touserdata(L, lua_upvalueindex(1));
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    int w = (int)luaL_checknumber(L, 3);
    int h = (int)luaL_checknumber(L, 4);
    int r = (int)luaL_optnumber(L, 5, 255);
    int g = (int)luaL_optnumber(L, 6, 255);
    int b = (int)luaL_optnumber(L, 7, 255);
    int a = (int)luaL_optnumber(L, 8, 255);
    surface_push_rect(surface, x, y, w, h, (Clay_Color){(float)r, (float)g, (float)b, (float)a});
    return 0;
}

static int app_handler_draw_width(lua_State* L) {
    Surface* surface = (Surface*)lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, surface->width);
    return 1;
}

static int app_handler_draw_height(lua_State* L) {
    Surface* surface = (Surface*)lua_touserdata(L, lua_upvalueindex(1));
    lua_pushinteger(L, surface->height);
    return 1;
}

static int app_handler_draw_text(lua_State* L) {
    Surface* surface = (Surface*)lua_touserdata(L, lua_upvalueindex(1));
    int x = (int)luaL_checknumber(L, 1);
    int y = (int)luaL_checknumber(L, 2);
    const char* text = luaL_checkstring(L, 3);
    int fontSize = (int)luaL_optnumber(L, 4, 16);
    int r = (int)luaL_optnumber(L, 5, 255);
    int g = (int)luaL_optnumber(L, 6, 255);
    int b = (int)luaL_optnumber(L, 7, 255);
    surface_push_text(
        surface, x, y, text, 0, fontSize, (Clay_Color){(float)r, (float)g, (float)b, 255}
    );
    return 0;
}

static void push_draw_closure(lua_State* L, lua_CFunction fn, Surface* surface, Display* display) {
    lua_pushlightuserdata(L, surface);
    lua_pushlightuserdata(L, display);
    lua_pushcclosure(L, fn, 2);
}

static void register_draw_api(lua_State* L, Surface* surface, Display* display) {
    lua_newtable(L);

    push_draw_closure(L, app_handler_draw_rect, surface, display);
    lua_setfield(L, -2, "rect");

    push_draw_closure(L, app_handler_draw_text, surface, display);
    lua_setfield(L, -2, "text");

    push_draw_closure(L, app_handler_draw_width, surface, display);
    lua_setfield(L, -2, "width");

    push_draw_closure(L, app_handler_draw_height, surface, display);
    lua_setfield(L, -2, "height");

    lua_setglobal(L, "draw");
}

static bool load_app_manifest(AppEntry* app, bool load_dev) {
    char manifestPath[MAX_FILE_PATH];
    snprintf(manifestPath, sizeof(manifestPath), "%s/%s", app->path, APP_MANIFEST);

    char resolvedPath[512];
    platform_store_resolved_path(manifestPath, resolvedPath, sizeof(resolvedPath));

    lua_State* L = luaL_newstate();
    if (!L)
        return false;

    if (luaL_dofile(L, resolvedPath) != LUA_OK) {
        fprintf(
            stderr, "[AppHandler] failed to load manifest '%s': %s\n", resolvedPath,
            lua_tostring(L, -1)
        );
        lua_close(L);
        return false;
    }

    if (!lua_istable(L, -1)) {
        fprintf(stderr, "[AppHandler] manifest '%s' did not return a table\n", resolvedPath);
        lua_close(L);
        return false;
    }

#define READ_STRING_FIELD(field, dest)                                                             \
    lua_getfield(L, -1, field);                                                                    \
    if (lua_isstring(L, -1)) {                                                                     \
        snprintf(dest, sizeof(dest), "%s", lua_tostring(L, -1));                                   \
    }                                                                                              \
    lua_pop(L, 1);

#define READ_BOOL_FIELD(field, dest)                                                               \
    lua_getfield(L, -1, field);                                                                    \
    if (lua_isboolean(L, -1)) {                                                                    \
        dest = lua_toboolean(L, -1);                                                               \
    }                                                                                              \
    lua_pop(L, 1);

    bool dev = false;
    READ_BOOL_FIELD("dev", dev)
    if (dev && !load_dev) {
        lua_close(L);
        return false;
    }
    READ_STRING_FIELD("name", app->name)
    READ_STRING_FIELD("icon", app->icon)

    READ_STRING_FIELD("script", app->script)
#undef READ_STRING_FIELD
#undef READ_BOOL_FIELD

    lua_close(L);
    return true;
}

static bool load_app_icon(AppEntry* app, Display* display) {
    if (app->icon[0] == '\0') {
        return true;
    }

    char iconPath[MAX_FILE_PATH];
    snprintf(iconPath, sizeof(iconPath), "%s/%s", app->path, app->icon);
    if (load_image(display, iconPath, &app->iconHandle)) {
        return true;
    }

    fprintf(stderr, "[AppHandler] failed to load icon '%s', using fallback\n", iconPath);
    if (load_image(display, "assets/icons/test.png", &app->iconHandle)) {
        return true;
    }

    fprintf(stderr, "[AppHandler] failed to load fallback icon\n");
    return false;
}

void app_handler_show_apps(AppHandler* app_handler, bool show_apps) {
    cyan_log(
        VERBOSE_MED, "[AppHandler] indexed %d app(s)%s", app_handler->appCount, show_apps ? ":" : ""
    );
    if (!show_apps) {
        return;
    }
    for (int i = 0; i < app_handler->appCount; i++) {
        cyan_log(
            VERBOSE_MED, ">    %d: %s (script: %s, icon: %s, path: %s)", i,
            app_handler->apps[i].name, app_handler->apps[i].script, app_handler->apps[i].icon,
            app_handler->apps[i].path
        );
    }
}

static void app_handler_index(AppHandler* app_handler, char* path, Display* display) {
    FolderList folders = scan_folder(path);

    app_handler->appCount = 0;
    for (int i = 0; i < folders.count && app_handler->appCount < MAX_APPS; i++) {
        AppEntry app = {0};
        snprintf(app.name, sizeof(app.name), "%s", folders.names[i]);
        int pathLen = snprintf(app.path, sizeof(app.path), "%s/%s", path, folders.names[i]);
        if (pathLen < 0 || (size_t)pathLen >= sizeof(app.path)) {
            cyan_log(
                VERBOSE_LOW, "[AppHandler] app folder name too long, skipping: %s", folders.names[i]
            );
            continue;
        }

        if (load_app_manifest(&app, app_handler->devmode)) {
            load_app_icon(&app, display);
            app_handler->apps[app_handler->appCount] = app;
            app_handler->appCount++;
        }
    }
    app_handler_show_apps(app_handler, false);
}

bool app_handler_init(AppHandler* app_handler, Display* display) {
    app_handler->appCount = 0;
    app_handler->appLua = NULL;
    app_handler->devmode = true;
    app_handler_index(app_handler, "apps", display);

    return true;
}

bool app_handler_launch(AppHandler* app_handler, int id, Display* display) {

    if (id < 0 || id > app_handler->appCount - 1) {
        cyan_log(VERBOSE_LOW, "[AppHandler] incorrect app ID.\n");
        return false;
    }

    if (app_handler->appLua) {
        app_handler_unload(app_handler);
    }
    AppEntry* app = &app_handler->apps[id];
    cyan_log(VERBOSE_LOW, "[AppHandler] launching app [%d]: %s\n", id, app->name);

    // Create the runtime FIRST - L doesn't exist before this line.
    app_handler->appLua = luaL_newstate();
    if (!app_handler->appLua) {
        cyan_log(VERBOSE_LOW, "[AppHandler] failed to create runtime for '%s'\n", app->name);
        return false;
    }
    lua_State* L = app_handler->appLua;

    DisplaySize size = display_get_size(display);
    surface_init(&app_handler->surface, 0, 0, size.width, size.height);

    luaL_requiref(L, "_G", luaopen_base, 1);
    lua_pop(L, 1);
    register_sandbox_api(L, app->name);
    register_draw_api(L, &app_handler->surface, display);
    char scriptRelPath[MAX_FILE_PATH];
    snprintf(scriptRelPath, sizeof(scriptRelPath), "%s/%s", app->path, app->script);
    char resolvedPath[512];
    platform_store_resolved_path(scriptRelPath, resolvedPath, sizeof(resolvedPath));

    if (luaL_dofile(L, resolvedPath) != LUA_OK) {
        fprintf(
            stderr, "[AppHandler] failed to load script '%s': %s\n", resolvedPath,
            lua_tostring(L, -1)
        );
        lua_pop(L, 1);
        lua_close(L);
        app_handler->appLua = NULL;
        return false;
    }

    lua_getglobal(L, "on_load");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[AppHandler] on_load error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
    return true;
}

void app_handler_run_frame(AppHandler* app_handler, Display* display, float dt) {
    if (!app_handler->appLua) {
        cyan_log(VERBOSE_MED, "ERROR: no AppHandler app to run frame for.\n");
        return;
    }
    lua_State* L = app_handler->appLua;
    lua_getglobal(L, "on_update");
    if (lua_isfunction(L, -1)) {
        lua_pushnumber(L, dt);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "[AppHandler] on_update error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_getglobal(L, "on_draw");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[AppHandler] on_draw error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }
}

void app_handler_dispatch_events(AppHandler* app_handler, EventQueue* queue) {
    if (!app_handler->appLua) {
        printf("ERROR: no AppHandler app to dispatch events to.\n");
        return;
    }
    lua_State* L = app_handler->appLua;
    for (int i = 0; i < queue->len; i++) {
        Event* ev = &queue->events[i];
        lua_getglobal(L, "on_event");
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue;
        }

        lua_pushinteger(L, ev->type);
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            fprintf(stderr, "[AppHandler] on_event error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
}

void app_handler_unload(AppHandler* app_handler) {
    if (!app_handler->appLua)
        return;
    lua_State* L = app_handler->appLua;

    lua_getglobal(L, "on_unload");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[AppHandler] on_unload error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        lua_pop(L, 1);
    }

    lua_close(L);
    app_handler->appLua = NULL;
}

void app_handler_shutdown(AppHandler* AppHandler) {
    if (AppHandler->appLua) {
        lua_close(AppHandler->appLua);
        AppHandler->appLua = NULL;
    }
}