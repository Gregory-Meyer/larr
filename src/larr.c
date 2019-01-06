#include <larr/larr.h>

#include "util.h"
#include "vec.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <lauxlib.h>
#include <luaconf.h>

#ifdef __cplusplus
} // extern "C"
#endif

int l_Vec_new(lua_State *L) {
    Typeinfo typeinfo;
    TypeVec *tv;

    assert(L);

    typeinfo = check_typeinfo(L, -1);

    tv = (TypeVec*) lua_newuserdata(L, sizeof(TypeVec));

    Vec_new(&tv->vec, sizeof_type_repr(typeinfo.type));
    tv->typeinfo = typeinfo;
    tv->vtbl = get_vtbl(typeinfo.type);

    luaL_setmetatable(L, "larr.Vec");
    /*lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");*/

    return 1;
}

int l_Vec_with_capacity(lua_State *L) {
    Typeinfo typeinfo;
    size_t capacity;
    TypeVec *tv;

    assert(L);

    typeinfo = check_typeinfo(L, -2);
    capacity = check_size_t(L, -1);

    tv = (TypeVec*) lua_newuserdata(L, sizeof(TypeVec));

    if (Vec_with_capacity(&tv->vec, sizeof_type_repr(typeinfo.type),
                          (size_t) capacity) != LARR_OK) {
        return luaL_error(L, "couldn't allocate space for %ld elements", (long) capacity);
    }

    tv->typeinfo = typeinfo;
    tv->vtbl = get_vtbl(typeinfo.type);

    luaL_setmetatable(L, "larr.Vec");

    return 1;
}

int l_Vec_meta_gc(lua_State *L) {
    TypeVec *tv;

    assert(L);

    tv = check_tv_mut(L, -1);

    tv->vtbl->clear(tv, L);
    Vec_delete(&tv->vec);

    return 0;
}

int l_Vec_capacity(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = check_tv(L, -1);

    push_size_t(L, Vec_capacity(&tv->vec));

    return 1;
}

int l_Vec_meta_len(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = check_tv(L, -1);

    push_size_t(L, Vec_len(&tv->vec));

    return 1;
}

int l_Vec_is_empty(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = check_tv(L, -1);

    lua_pushboolean(L, Vec_is_empty(&tv->vec));

    return 1;
}

int l_Vec_first(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = check_tv(L, -1);
    tv->vtbl->first(tv, L);

    return 1;
}

int l_Vec_last(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = check_tv(L, -1);
    tv->vtbl->last(tv, L);

    return 1;
}

int l_Vec_meta_index(lua_State *L) {
    const TypeVec *tv;
    size_t index;
    int is_size_t;

    assert(L);

    tv = check_tv(L, -2);
    index = to_size_t(L, -1, &is_size_t);

    if (!is_size_t) {
        const char *const str = lua_tostring(L, -1);
        luaL_getmetafield(L, 1, str);

        return 1;
    }

    tv->vtbl->push_elem(tv, index - 1, L);

    return 1;
}

int l_Vec_meta_newindex(lua_State *L) {
    TypeVec *tv;
    size_t index;

    assert(L);

    tv = check_tv_mut(L, -3);
    index = check_size_t(L, -2);

    tv->vtbl->set_elem(tv, index - 1, L);

    return 1;
}


int l_Vec_push(lua_State *L) {
    TypeVec *tv;

    assert(L);

    tv = check_tv_mut(L, -2);
    tv->vtbl->push(tv, L);

    return 0;
}

int l_Vec_pop(lua_State *L) {
    TypeVec *tv;

    assert(L);

    tv = check_tv_mut(L, -2);

    if (Vec_pop(&tv->vec) != LARR_OK) {
        return luaL_error(L, "Vec is empty");
    }

    return 0;
}

int l_Vec_insert(lua_State *L) {
    TypeVec *tv;
    size_t index;

    assert(L);

    tv = check_tv_mut(L, -3);
    index = check_size_t(L, -2);

    tv->vtbl->insert(tv, index - 1, L);

    return 0;
}

int l_Vec_remove(lua_State *L) {
    TypeVec *tv;
    size_t index;

    assert(L);

    tv = check_tv_mut(L, -2);
    index = check_size_t(L, -1);

    Vec_remove(&tv->vec, index - 1);

    return 0;
}

int l_Vec_clear(lua_State *L) {
    TypeVec *tv;

    assert(L);

    tv = check_tv_mut(L, -1);

    tv->vtbl->clear(tv, L);
    Vec_clear(&tv->vec);

    return 0;
}

int l_Vec_meta_tostring(lua_State *L) {
    const TypeVec *tv;

    assert(L);

    tv = (const TypeVec*) check_tv(L, -1);

    if (Vec_is_empty(&tv->vec)) {
        lua_pushliteral(L, "{}");
    } else {
        size_t i;
        int first = 1;

        luaL_Buffer buf;
        luaL_buffinit(L, &buf);
        luaL_addchar(&buf, '{');

        for (i = 0; i < Vec_len(&tv->vec); ++i) {
            if (!first) {
                luaL_addlstring(&buf, ", ", 2);
            } else {
                first = 0;
            }

            tv->vtbl->push_elem(tv, i, L);
            luaL_addvalue(&buf);
        }

        luaL_addchar(&buf, '}');
        luaL_pushresult(&buf);
    }

    return 1;
}

int luaopen_liblarr(lua_State *L) {
    static const luaL_Reg funcs[] = {
        { "new", l_Vec_new },
        { "with_capacity", l_Vec_with_capacity },
        { "__gc", l_Vec_meta_gc },
        { "capacity", l_Vec_capacity },
        { "__len", l_Vec_meta_len },
        { "is_empty", l_Vec_is_empty },
        { "first", l_Vec_first },
        { "last", l_Vec_last },
        { "__index", l_Vec_meta_index },
        { "__newindex", l_Vec_meta_newindex },
        { "push", l_Vec_push },
        { "pop", l_Vec_pop },
        { "insert", l_Vec_insert },
        { "remove", l_Vec_remove },
        { "clear", l_Vec_clear },
        { "__tostring", l_Vec_meta_tostring },
        { NULL, NULL }
    };

    assert(L);

    lua_newtable(L);

    luaL_newmetatable(L, "larr.Vec");
    luaL_setfuncs(L, funcs, 0);

    lua_setfield(L, -2, "Vec");

    return 1;
}
