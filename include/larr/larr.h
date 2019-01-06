#ifndef LARR_LARR_H
#define LARR_LARR_H

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_LIB
#include <lua.h>

int l_Array_new(lua_State *L);

int l_Array_with_capacity(lua_State *L);

int l_Array_meta_gc(lua_State *L);

int l_Array_capacity(lua_State *L);

int l_Array_meta_len(lua_State *L);

int l_Array_is_empty(lua_State *L);

int l_Array_first(lua_State *L);

int l_Array_last(lua_State *L);

int l_Array_meta_index(lua_State *L);

int l_Array_meta_newindex(lua_State *L);

int l_Array_push(lua_State *L);

int l_Array_pop(lua_State *L);

int l_Array_insert(lua_State *L);

int l_Array_remove(lua_State *L);

int l_Array_clear(lua_State *L);

int l_Array_meta_tostring(lua_State *L);

int luaopen_liblarr(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
