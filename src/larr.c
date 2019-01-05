#include <larr/larr.h>

#include "array.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lauxlib.h>
#include <luaconf.h>

#ifdef __cplusplus
} // extern "C"
#endif

static int larr_Array_new(lua_State *L);

static int larr_Array_with_capacity(lua_State *L);

static int larr_Array_meta_gc(lua_State *L);

static int larr_Array_capacity(lua_State *L);

static int larr_Array_meta_len(lua_State *L);

static int larr_Array_is_empty(lua_State *L);

static int larr_Array_first(lua_State *L);

static int larr_Array_last(lua_State *L);

static int larr_Array_meta_index(lua_State *L);

static int larr_Array_meta_newindex(lua_State *L);

static int larr_Array_push(lua_State *L);

static int larr_Array_pop(lua_State *L);

static int larr_Array_insert(lua_State *L);

static int larr_Array_remove(lua_State *L);

static int larr_Array_clear(lua_State *L);

int luaopen_liblarr(lua_State *L) {
    int registered;

    assert(L);

    registered = luaL_newmetatable(L, "larr.Array");
    assert(registered);

    lua_createtable(L, 0, 1);
    lua_createtable(L, 0, 15);

    lua_pushcfunction(L, larr_Array_new);
    lua_setfield(L, -2, "new");

    lua_pushcfunction(L, larr_Array_with_capacity);
    lua_setfield(L, -2, "with_capacity");

    lua_pushcfunction(L, larr_Array_meta_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, larr_Array_capacity);
    lua_setfield(L, -2, "capacity");

    lua_pushcfunction(L, larr_Array_meta_len);
    lua_setfield(L, -2, "__len");

    lua_pushcfunction(L, larr_Array_is_empty);
    lua_setfield(L, -2, "is_empty");

    lua_pushcfunction(L, larr_Array_first);
    lua_setfield(L, -2, "first");

    lua_pushcfunction(L, larr_Array_last);
    lua_setfield(L, -2, "last");

    lua_pushcfunction(L, larr_Array_meta_index);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, larr_Array_meta_newindex);
    lua_setfield(L, -2, "__newindex");

    lua_pushcfunction(L, larr_Array_push);
    lua_setfield(L, -2, "push");

    lua_pushcfunction(L, larr_Array_pop);
    lua_setfield(L, -2, "pop");

    lua_pushcfunction(L, larr_Array_insert);
    lua_setfield(L, -2, "insert");

    lua_pushcfunction(L, larr_Array_remove);
    lua_setfield(L, -2, "remove");

    lua_pushcfunction(L, larr_Array_clear);
    lua_setfield(L, -2, "clear");

    lua_setfield(L, -2, "Array");

    return 1;
}

static int check_arity(lua_State *L, int arity);

static int larr_Array_new(lua_State *L) {
    Array *arr;

    assert(L);

    check_arity(L, 0);

    arr = (Array*) lua_newuserdata(L, sizeof(Array));
    Array_new(arr);
    luaL_setmetatable(L, "larr.Array");

    return 1;
}

static size_t check_size_t(lua_State *L, int arg);

static int larr_Array_with_capacity(lua_State *L) {
    size_t capacity;
    Array *arr;

    assert(L);

    check_arity(L, 1);

    capacity = check_size_t(L, 1);
    lua_pop(L, 1);

    arr = (Array*) lua_newuserdata(L, sizeof(Array));

    if (Array_with_capacity(arr, (size_t) capacity) != LARR_OK) {
        return luaL_error(L, "couldn't allocate space for %ld elements", (long) capacity);
    }

    luaL_setmetatable(L, "larr.Array");

    return 1;
}

static Array* check_array(lua_State *L);

static int larr_Array_meta_gc(lua_State *L) {
    Array *arr;

    assert(L);

    check_arity(L, 1);

    arr = check_array(L);
    lua_pop(L, 1);

    Array_delete(arr);

    return 0;
}

static void push_size_t(lua_State *L, size_t value);

static int larr_Array_capacity(lua_State *L) {
    const Array *arr;

    assert(L);

    check_arity(L, 1);

    arr = check_array(L);
    lua_pop(L, 1);

    push_size_t(L, Array_capacity(arr));

    return 1;
}

static int larr_Array_meta_len(lua_State *L) {
    const Array *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    push_size_t(L, Array_len(arr));

    return 1;
}

static int larr_Array_is_empty(lua_State *L) {
    const Array *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    lua_pushboolean(L, Array_is_empty(arr));

    return 1;
}

static int larr_Array_first(lua_State *L);

static int larr_Array_last(lua_State *L);

static int larr_Array_meta_index(lua_State *L);

static int larr_Array_meta_newindex(lua_State *L);

static int larr_Array_push(lua_State *L);

static int larr_Array_pop(lua_State *L);

static int larr_Array_insert(lua_State *L);

static int larr_Array_remove(lua_State *L);

static int larr_Array_clear(lua_State *L);

static int check_arity(lua_State *L, int arity) {
    int num_elements;

    assert(L);
    assert(arity >= 0);

    num_elements = lua_gettop(L);

    if (num_elements != arity) {
        return luaL_error(L, "invalid number of arguments (%d)", num_elements);
    }

    return arity;
}

static Array* check_array(lua_State *L) {
    assert(L);

    return (Array*) luaL_checkudata(L, 1, "larr.Array");
}

static int can_cast_to_size_t(int x);

static size_t check_size_t(lua_State *L, int arg) {
    lua_Integer value;

    assert(L);

    value = luaL_checkinteger(L, arg);
    luaL_argcheck(L, can_cast_to_size_t(value), arg, "not representable by size_t");

    return (size_t) value;
}

static int can_cast_to_Integer(size_t x);

static void push_size_t(lua_State *L, size_t value) {
    assert(L);

    if (!can_cast_to_Integer(value)) {
        luaL_error(L, "cannot represent %lu as an Integer", (unsigned long) value);
    }

    lua_pushinteger(L, (lua_Integer) value);
}

static int can_cast_to_size_t(lua_Integer x) {
    if (x < 0) {
        return 0;
    }

    if (sizeof(lua_Integer) > sizeof(size_t)) {
        return x < (lua_Integer) SIZE_MAX;
    } else {
        return (size_t) x < SIZE_MAX;
    }
}

static int can_cast_to_Integer(size_t x) {
    if (sizeof(lua_Integer) > sizeof(size_t)) {
        return (lua_Integer) x < LUA_MAXINTEGER;
    } else {
        return x < (size_t) LUA_MAXINTEGER;
    }
}
