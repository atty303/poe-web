#include <assert.h>
#include <emscripten.h>
#include "image.h"
#include "lua.h"

static const char *IMAGE_HANDLE_TYPE = "ImageHandle";

typedef struct {
    int handle;
    int width;
    int height;
} ImageHandle;

static int st_next_handle = 0;

static int is_user_data(lua_State *L, int index, const char *type) {
    if (lua_type(L, index) != LUA_TUSERDATA) {
        return 0;
    }

    if (lua_getmetatable(L, index) == 0) {
        return 0;
    }

    lua_getfield(L, LUA_REGISTRYINDEX, type);
    int result = lua_rawequal(L, -2, -1);
    lua_pop(L, 2);

    return result;
}

static ImageHandle *get_image_handle(lua_State *L) {
    assert(is_user_data(L, 1, IMAGE_HANDLE_TYPE));
    ImageHandle *image_handle = lua_touserdata(L, 1);
    lua_remove(L, 1);
    return image_handle;
}

static int NewImageHandle(lua_State *L) {
    ImageHandle *image_handle = lua_newuserdata(L, sizeof(ImageHandle));
    image_handle->handle = ++st_next_handle;
    image_handle->width = 1;
    image_handle->height = 1;

    lua_pushvalue(L, lua_upvalueindex(1));
    lua_setmetatable(L, -2);

    return 1;
}

static int ImageHandle_Load(lua_State *L) {
    ImageHandle *image_handle = get_image_handle(L);

    int n = lua_gettop(L);
    assert(n >= 1);
    assert(lua_isstring(L, 1));

    const char *filename = lua_tostring(L, 1);

    // TODO: lookup image size

    EM_ASM({
        Module.imageLoad($0, UTF8ToString($1));
    }, image_handle->handle, filename);

    return 0;
}

static int ImageHandle_ImageSize(lua_State *L) {
    ImageHandle *image_handle = get_image_handle(L);

    lua_pushinteger(L, image_handle->width);
    lua_pushinteger(L, image_handle->height);

    return 2;
}

void image_init(lua_State *L) {
    // Image handles
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_pushcclosure(L, NewImageHandle, 1);
    lua_setglobal(L, "NewImageHandle");

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, ImageHandle_Load);
    lua_setfield(L, -2, "Load");

    lua_pushcfunction(L, ImageHandle_ImageSize);
    lua_setfield(L, -2, "ImageSize");

    lua_setfield(L, LUA_REGISTRYINDEX, IMAGE_HANDLE_TYPE);
}
