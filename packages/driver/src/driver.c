#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <assert.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "draw.h"
#include "image.h"

extern const char *boot_lua;
static lua_State *GL;

// メモリアロケーション関数
static void *my_alloc(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud;  (void)osize;  /* 未使用の引数 */
    if (nsize == 0) {
        free(ptr);
        return NULL;
    }
    else
        return realloc(ptr, nsize);
}

static int SetDrawColor(lua_State *L) {
    int n = lua_gettop(L);
    assert(n >= 1);
    if (lua_type(L, 1) == LUA_TSTRING) {
//        draw_set_color_escape(lua_tostring(L, 1));
    } else {
        assert(n >= 3);

        float alpha = 1.0f;
        if (n >= 4 && !lua_isnil(L, 4)) {
            alpha = lua_tonumber(L, 4);
        }
        draw_set_color(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), alpha);
    }
    return 0;
}

static int DrawImage(lua_State *L) {
    int n = lua_gettop(L);
    if (n > 5) {
    } else {
        draw_image(0, lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4), lua_tonumber(L, 5));
    }
    return 0;
}

#define ADD_LUA_FUNC(name) \
    lua_pushcclosure(L, name, 0); \
    lua_setglobal(L, #name);

EMSCRIPTEN_KEEPALIVE
int init() {
    GL = lua_newstate(my_alloc, NULL);
    lua_State *L = GL;

    luaL_openlibs(GL);  // 標準ライブラリを開く

    image_init(L);

    // Rendering
    ADD_LUA_FUNC(SetDrawColor);
    ADD_LUA_FUNC(DrawImage);

    if (luaL_dostring(L, boot_lua) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
        return 1;
    }

    return 0;
}

EM_JS(void, draw_commit, (const void *buffer, size_t size), {
    Module.drawCommit(buffer, size);
});

EMSCRIPTEN_KEEPALIVE
int on_frame() {
    draw_begin();

    lua_getglobal(GL, "runCallback");
    lua_pushstring(GL, "OnFrame");
    if (lua_pcall(GL, 1, 0, 0) != LUA_OK) {
        fprintf(stderr, "Error: %s\n", lua_tostring(GL, -1));
        return 1;
    }

    void *buffer;
    size_t size;
    draw_get_buffer(&buffer, &size);
    draw_commit(buffer, size);

    draw_end();

    return 0;
}
