#ifndef LARR_LARR_H
#define LARR_LARR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>

int larr_Array_new(lua_State *L);

int larr_Array_with_capacity(lua_State *L);

int larr_Array_meta_gc(lua_State *L);

int larr_Array_capacity(lua_State *L);

int larr_Array_meta_len(lua_State *L);

int larr_Array_is_empty(lua_State *L);

int larr_Array_first(lua_State *L);

int larr_Array_last(lua_State *L);

int larr_Array_meta_index(lua_State *L);

int larr_Array_meta_newindex(lua_State *L);

int larr_Array_push(lua_State *L);

int larr_Array_pop(lua_State *L);

int larr_Array_insert(lua_State *L);

int larr_Array_remove(lua_State *L);

int larr_Array_clear(lua_State *L);

int luaopen_liblarr(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif
