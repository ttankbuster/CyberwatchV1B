//cyan.c
#include "cyan.h"
#include <string.h>

static int cyan_print(lua_State *L) {
    int n = lua_gettop(L);
    printf("CYAN> ");
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

// Registers a global "Event" table so app scripts can write
// `if type == Event.BUTTON1_DOWN then ... end` rather than magic numbers.
// Keep this list in sync with the EventType enum in data.h by hand for now —
// worth code-generating later once there are more event types.
static void push_event_constants(lua_State *L) {
    lua_newtable(L);

    #define EVENT_CONST(name) \
        lua_pushinteger(L, name); \
        lua_setfield(L, -2, #name + 6); // strips the "EVENT_" prefix

    EVENT_CONST(EVENT_NONE)
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

bool cyan_init(Cyan *cyan, const char *appScriptPath) {
    char resolvedPath[512];
    platform_resolve_path(appScriptPath, resolvedPath, sizeof(resolvedPath));

    cyan->mode = CYAN_MODE_RUNNING_APP;
    cyan->selectedApp = -1;
    cyan->appCount = 0;

    cyan->lua = luaL_newstate();
    if (!cyan->lua) return false;
    lua_State *L = cyan->lua;

    // Deliberately minimal, not luaL_openlibs — apps are sandboxed, they
    // don't get filesystem/os access unless explicitly granted later.
    luaL_requiref(L, "_G", luaopen_base, 1);
    lua_pop(L, 1);

    lua_pushcfunction(L, cyan_print);
    lua_setglobal(L, "print");

    push_event_constants(L);

    lua_gc(L, LUA_GCSETSTEPMUL, 300);
    lua_gc(L, LUA_GCSETPAUSE, 100);

    // Runs the script's top-level code once — this is where the app
    // defines its on_event() (and later on_draw(), etc.) functions.
    // It must NOT block (no while-true loops in the script itself);
    // the actual run loop lives in C, driven by cyan_dispatch_events.
    if (luaL_dofile(L, resolvedPath) != LUA_OK) {
        fprintf(stderr, "Cyan: failed to load '%s': %s\n", resolvedPath, lua_tostring(L, -1));
        lua_pop(L, 1);
        lua_close(L);
        cyan->lua = NULL;
        return false;
    }

    return true;
}

// Calls the app's global on_event(type) for every event
// queued this frame. Missing on_event is fine — treated as "app doesn't
// care about input," not an error.
void cyan_dispatch_events(Cyan *cyan, EventQueue *queue) {
    if (!cyan->lua) return;
    lua_State *L = cyan->lua;

    for (int i = 0; i < queue->len; i++) {
        Event *ev = &queue->events[i];

        lua_getglobal(L, "on_event");
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 1);
            continue; // app hasn't defined a handler — not an error
        }

        lua_pushinteger(L, ev->type);

        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            fprintf(stderr, "Cyan: on_event error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    lua_gc(L, LUA_GCSTEP, 0); // incremental collection, not a full GC every frame
}

void cyan_shutdown(Cyan *cyan) {
    if (cyan->lua) {
        lua_close(cyan->lua);
        cyan->lua = NULL;
    }
}