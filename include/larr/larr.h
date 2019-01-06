#ifndef LARR_LARR_H
#define LARR_LARR_H

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_LIB
#include <lua.h>

int l_Vec_new(lua_State *L);

int l_Vec_with_capacity(lua_State *L);

int l_Vec_meta_gc(lua_State *L);

int l_Vec_capacity(lua_State *L);

int l_Vec_meta_len(lua_State *L);

int l_Vec_is_empty(lua_State *L);

int l_Vec_first(lua_State *L);

int l_Vec_last(lua_State *L);

int l_Vec_meta_index(lua_State *L);

int l_Vec_meta_newindex(lua_State *L);

int l_Vec_push(lua_State *L);

int l_Vec_pop(lua_State *L);

int l_Vec_insert(lua_State *L);

int l_Vec_remove(lua_State *L);

int l_Vec_clear(lua_State *L);

int l_Vec_meta_tostring(lua_State *L);

int luaopen_liblarr(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
