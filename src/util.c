#include "util.h"

#include <assert.h>
#include <string.h>

#include <lauxlib.h>

size_t sizeof_type_repr(int type) {
    #define X(name, type, nickname, string) case name: return sizeof(type);

    switch ((Type) type) {
        TYPES
    }

    #undef X
}

static int can_cast_to_size_t(lua_Integer x);

size_t check_size_t(lua_State *L, int arg) {
    lua_Integer value;

    assert(L);

    value = luaL_checkinteger(L, arg);
    luaL_argcheck(L, can_cast_to_size_t(value), arg, "not representable by size_t");

    return (size_t) value;
}

static int can_cast_to_Integer(size_t x);

void push_size_t(lua_State *L, size_t x) {
    assert(L);

    if (!can_cast_to_Integer(x)) {
        luaL_error(L, "cannot represent %lu as an Integer", (unsigned long) x);
    }

    lua_pushinteger(L, (lua_Integer) x);
}

size_t to_size_t(lua_State *L, int arg, int *is_size_t) {
    lua_Integer value;
    int is_integer;

    assert(L);

    value = lua_tointegerx(L, arg, &is_integer);

    if (is_size_t) {
        *is_size_t = is_integer && can_cast_to_size_t(value);
    }

    return (size_t) value;
}

Typeinfo check_typeinfo(lua_State *L, int arg) {
    #define X(name, type, nickname, string) static const char name ## _STR[] = string;
    TYPES
    #undef X

    Typeinfo typeinfo;
    size_t length;

    typeinfo.name = luaL_checklstring(L, arg, &length);

    if (length == sizeof(TP_NUM_STR) - 1 && memcmp(typeinfo.name, TP_NUM_STR, length) == 0) {
        typeinfo.type = TP_NUM;
    } else if (length == sizeof(TP_INT_STR) - 1 && memcmp(typeinfo.name, TP_INT_STR, length) == 0) {
        typeinfo.type = TP_INT;
    } else if (length == sizeof(TP_BOOL_STR) - 1 && memcmp(typeinfo.name, TP_BOOL_STR, length) == 0) {
        typeinfo.type = TP_BOOL;
    } else if (length == sizeof(TP_STR_STR) - 1 && memcmp(typeinfo.name, TP_STR_STR, length) == 0) {
        typeinfo.type = TP_STR;
    } else if (length == sizeof(TP_TBL_STR) - 1 && memcmp(typeinfo.name, TP_TBL_STR, length) == 0) {
        typeinfo.type = TP_TBL;
    } else if (length == sizeof(TP_FN_STR) - 1 && memcmp(typeinfo.name, TP_FN_STR, length) == 0) {
        typeinfo.type = TP_FN;
    } else if (length == sizeof(TP_THREAD_STR) - 1
               && memcmp(typeinfo.name, TP_THREAD_STR, length) == 0) {
        typeinfo.type = TP_THREAD;
    } else if (length == sizeof(TP_LIGHT_USERDATA_STR) - 1
               && memcmp(typeinfo.name, TP_LIGHT_USERDATA_STR, length) == 0) {
        typeinfo.type = TP_LIGHT_USERDATA;
    } else {
        typeinfo.type = TP_USERDATA;
    }

    return typeinfo;
}

const TypeVec* check_tv(lua_State *L, int arg) {
    return (const TypeVec*) luaL_checkudata(L, arg, "larr.Vec");
}

TypeVec* check_tv_mut(lua_State *L, int arg) {
    return (TypeVec*) luaL_checkudata(L, arg, "larr.Vec");
}

const Vtbl* get_vtbl(int type) {
    #define X(name, type, nickname, string) case name: return nickname ## _vtbl();

    switch ((Type) type) {
        TYPES
    }

    #undef X
}

static void noop_clear(TypeVec *tv, lua_State*);

static void unref_clear(TypeVec *tv, lua_State *L);

static void num_push(TypeVec *tv, lua_State *L);

static void num_insert(TypeVec *tv, size_t index, lua_State *L);

static void num_set_elem(TypeVec *tv, size_t index, lua_State *L);

static void num_first(const TypeVec *tv, lua_State *L);

static void num_last(const TypeVec *tv, lua_State *L);

static void num_push_elem(const TypeVec *tv, size_t index, lua_State *L);

const Vtbl* num_vtbl(void) {
    static const Vtbl vtbl = {
        num_push,
        num_insert,
        noop_clear,
        num_set_elem,
        num_first,
        num_last,
        num_push_elem
    };

    return &vtbl;
}

static void int_push(TypeVec *tv, lua_State *L);

static void int_insert(TypeVec *tv, size_t index, lua_State *L);

static void int_set_elem(TypeVec *tv, size_t index, lua_State *L);

static void int_first(const TypeVec *tv, lua_State *L);

static void int_last(const TypeVec *tv, lua_State *L);

static void int_push_elem(const TypeVec *tv, size_t index, lua_State *L);

const Vtbl* int_vtbl(void) {
    static const Vtbl vtbl = {
        int_push,
        int_insert,
        noop_clear,
        int_set_elem,
        int_first,
        int_last,
        int_push_elem
    };

    return &vtbl;
}

const Vtbl* bool_vtbl(void) {
    return NULL;
}

const Vtbl* str_vtbl(void) {
    return NULL;
}

const Vtbl* tbl_vtbl(void) {
    return NULL;
}

const Vtbl* fn_vtbl(void) {
    return NULL;
}

const Vtbl* userdata_vtbl(void) {
    return NULL;
}

const Vtbl* thread_vtbl(void) {
    return NULL;
}

const Vtbl* light_userdata_vtbl(void) {
    return NULL;
}

static void noop_clear(TypeVec *tv, lua_State *L) { }

static void unref_clear(TypeVec *tv, lua_State *L) {
    const size_t length = Vec_len(&tv->vec);
    size_t i;

    for (i = 0; i < length; ++i) {
        const int ref = *(const int*) Vec_get(&tv->vec, i);
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
}

static void num_push(TypeVec *tv, lua_State *L) {
    lua_Number num;

    assert(tv);
    assert(L);

    num = luaL_checknumber(L, -1);
    Vec_push(&tv->vec, &num);
}

static void num_insert(TypeVec *tv, size_t index, lua_State *L) {
    lua_Number num;
    int ret;

    assert(tv);
    assert(L);

    num = luaL_checkinteger(L, -1);
    ret = Vec_insert(&tv->vec, index, &num);

    if (ret == LARR_NO_MEMORY) {
        luaL_error(L, "out of memory");
    } else if (ret == LARR_OUT_OF_RANGE) {
        luaL_error(L, "index %lu out of range", (unsigned long) index);
    }
}

static void num_set_elem(TypeVec *tv, size_t index, lua_State *L) {
    lua_Number num;
    lua_Number *num_ptr;

    assert(tv);
    assert(L);

    num = luaL_checkinteger(L, -1);
    num_ptr = (lua_Number*) Vec_get_mut(&tv->vec, index);

    if (num_ptr) {
        *num_ptr = num;
    } else {
        luaL_error(L, "index %lu out of range", (unsigned long) index);
    }
}

static void num_first(const TypeVec *tv, lua_State *L) {
    const lua_Number *first_ptr;

    assert(tv);
    assert(L);

    first_ptr = Vec_first(&tv->vec);

    if (first_ptr) {
        lua_pushnumber(L, *first_ptr);
    } else {
        lua_pushnil(L);
    }
}

static void num_last(const TypeVec *tv, lua_State *L) {
    const lua_Number *last_ptr;

    assert(tv);
    assert(L);

    last_ptr = Vec_last(&tv->vec);

    if (last_ptr) {
        lua_pushnumber(L, *last_ptr);
    } else {
        lua_pushnil(L);
    }
}

static void num_push_elem(const TypeVec *tv, size_t index, lua_State *L) {
    const lua_Number *num_ptr;

    assert(tv);
    assert(L);

    num_ptr = Vec_get(&tv->vec, index);

    if (num_ptr) {
        lua_pushnumber(L, *num_ptr);
    } else {
        lua_pushnil(L);
    }
}

static void int_push(TypeVec *tv, lua_State *L) {
    lua_Integer integer;

    assert(tv);
    assert(L);

    integer = luaL_checkinteger(L, -1);
    Vec_push(&tv->vec, &integer);
}

static void int_insert(TypeVec *tv, size_t index, lua_State *L) {
    lua_Integer integer;
    int ret;

    assert(tv);
    assert(L);

    integer = luaL_checkinteger(L, -1);
    ret = Vec_insert(&tv->vec, index, &integer);

    if (ret == LARR_NO_MEMORY) {
        luaL_error(L, "out of memory");
    } else if (ret == LARR_OUT_OF_RANGE) {
        luaL_error(L, "index %lu out of range", (unsigned long) index);
    }
}

static void int_set_elem(TypeVec *tv, size_t index, lua_State *L) {
    lua_Integer integer;
    lua_Integer *int_ptr;

    assert(tv);
    assert(L);

    index = check_size_t(L, -2);
    integer = luaL_checkinteger(L, -1);
    int_ptr = (lua_Integer*) Vec_get_mut(&tv->vec, index);

    if (int_ptr) {
        *int_ptr = integer;
    } else {
        luaL_error(L, "index %lu out of range", (unsigned long) index);
    }
}

static void int_first(const TypeVec *tv, lua_State *L) {
    const lua_Integer *first_ptr;

    assert(tv);
    assert(L);

    first_ptr = Vec_first(&tv->vec);

    if (first_ptr) {
        lua_pushinteger(L, *first_ptr);
    } else {
        lua_pushnil(L);
    }
}

static void int_last(const TypeVec *tv, lua_State *L) {
    const lua_Integer *last_ptr;

    assert(tv);
    assert(L);

    last_ptr = Vec_last(&tv->vec);

    if (last_ptr) {
        lua_pushinteger(L, *last_ptr);
    } else {
        lua_pushnil(L);
    }
}

static void int_push_elem(const TypeVec *tv, size_t index, lua_State *L) {
    const lua_Integer *int_ptr;

    assert(tv);
    assert(L);

    int_ptr = Vec_get(&tv->vec, index);

    if (int_ptr) {
        lua_pushinteger(L, *int_ptr);
    } else {
        lua_pushnil(L);
    }
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
