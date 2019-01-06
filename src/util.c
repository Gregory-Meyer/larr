#include "util.h"

#include <assert.h>
#include <string.h>

#include <lauxlib.h>

size_t sizeof_type_repr(int type) {
    #define X(name, type, nickname, string) case name: return sizeof(type);

    switch ((Type) type) {
        TYPES
        default: assert(0 && "invalid argument passed");
    }

    #undef X
}

static int can_cast_to_size_t(lua_Integer x);

int String_cmp(const String *lhs, const String *rhs) {
    size_t rlen;
    int compare_res;

    assert(lhs);
    assert(rhs);

    rlen = (lhs->len < rhs->len) ? lhs->len : rhs->len;
    compare_res = memcmp(lhs->str, rhs->str, rlen);

    if (compare_res != 0) {
        return compare_res;
    } else if (lhs->len < rhs->len) {
        return -1;
    } else if (lhs->len > rhs->len) {
        return 1;
    } else {
        return 0;
    }
}

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
    #define X(name, type, nickname, string) static const String name ## _STR = { string, sizeof(string) - 1 };
    TYPES
    #undef X

    Typeinfo typeinfo;

    typeinfo.name.str = luaL_checklstring(L, arg, &typeinfo.name.len);

    if (String_cmp(&typeinfo.name, &TP_NUM_STR) == 0) {
        typeinfo.type = TP_NUM;
    } else if (String_cmp(&typeinfo.name, &TP_INT_STR) == 0) {
        typeinfo.type = TP_INT;
    } else if (String_cmp(&typeinfo.name, &TP_BOOL_STR) == 0) {
        typeinfo.type = TP_BOOL;
    } else if (String_cmp(&typeinfo.name, &TP_STR_STR) == 0) {
        typeinfo.type = TP_STR;
    } else if (String_cmp(&typeinfo.name, &TP_TBL_STR) == 0) {
        typeinfo.type = TP_TBL;
    } else if (String_cmp(&typeinfo.name, &TP_FN_STR) == 0) {
        typeinfo.type = TP_FN;
    } else if (String_cmp(&typeinfo.name, &TP_THREAD_STR) == 0) {
        typeinfo.type = TP_THREAD;
    } else if (String_cmp(&typeinfo.name, &TP_LIGHT_USERDATA_STR) == 0) {
        typeinfo.type = TP_LIGHT_USERDATA;
    } else {
        (void) TP_USERDATA_STR; /* suppress unused variable warning */

        typeinfo.type = TP_USERDATA;
    }

    return typeinfo;
}

const TypeVec* check_tv(lua_State *L, int arg) {
    assert(L);

    return (const TypeVec*) luaL_checkudata(L, arg, "larr.Vec");
}

TypeVec* check_tv_mut(lua_State *L, int arg) {
    assert(L);

    return (TypeVec*) luaL_checkudata(L, arg, "larr.Vec");
}

TypeVec* test_tv_mut(lua_State *L, int arg) {
    assert(L);

    return (TypeVec*) luaL_testudata(L, arg, "larr.Vec");
}

const Vtbl* get_vtbl(int type) {
    #define X(name, type, nickname, string) case name: return nickname ## _vtbl();

    switch ((Type) type) {
        TYPES
        default: assert(0 && "invalid argument passed");
    }

    #undef X
}

static void noop_clear(TypeVec *tv, lua_State*);

static void unref_clear(TypeVec *tv, lua_State *L);

static void simple_truncate(TypeVec *tv, size_t len, lua_State *L);

static void unref_truncate(TypeVec *tv, size_t len, lua_State *L);

static void num_push(TypeVec *tv, lua_State *L);

static int num_try_push(TypeVec *tv, lua_State *L);

static void num_insert(TypeVec *tv, size_t index, lua_State *L);

static void num_set_elem(TypeVec *tv, size_t index, lua_State *L);

static void num_first(const TypeVec *tv, lua_State *L);

static void num_last(const TypeVec *tv, lua_State *L);

static void num_push_elem(const TypeVec *tv, size_t index, lua_State *L);

const Vtbl* num_vtbl(void) {
    static const Vtbl vtbl = {
        num_push,
        num_try_push,
        num_insert,
        noop_clear,
        num_set_elem,
        num_first,
        num_last,
        num_push_elem,
        simple_truncate
    };

    return &vtbl;
}

static void int_push(TypeVec *tv, lua_State *L);

static int int_try_push(TypeVec *tv, lua_State *L);

static void int_insert(TypeVec *tv, size_t index, lua_State *L);

static void int_set_elem(TypeVec *tv, size_t index, lua_State *L);

static void int_first(const TypeVec *tv, lua_State *L);

static void int_last(const TypeVec *tv, lua_State *L);

static void int_push_elem(const TypeVec *tv, size_t index, lua_State *L);

const Vtbl* int_vtbl(void) {
    static const Vtbl vtbl = {
        int_push,
        int_try_push,
        int_insert,
        noop_clear,
        int_set_elem,
        int_first,
        int_last,
        int_push_elem,
        simple_truncate
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
    size_t length;
    size_t i;

    assert(tv);
    assert(L);

    length = Vec_len(&tv->vec);

    for (i = 0; i < length; ++i) {
        const int ref = *(const int*) Vec_get(&tv->vec, i);
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
    }
}

static void simple_truncate(TypeVec *tv, size_t len, lua_State *L) {
    assert(tv);
    assert(L);

    Vec_truncate(&tv->vec, len);
}

static void unref_truncate(TypeVec *tv, size_t len, lua_State *L) {
    size_t current_length;

    assert(tv);
    assert(L);

    current_length = Vec_len(&tv->vec);

    if (len >= current_length) {
        unref_clear(tv, L);

        return;
    } else {
        size_t i;

        for (i = len; i < current_length; ++i) {
            const int ref = *(const int*) Vec_get(&tv->vec, i);
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
        }

        Vec_truncate(&tv->vec, len);
    }
}

static void num_push(TypeVec *tv, lua_State *L) {
    int res;

    assert(tv);
    assert(L);

    res = num_try_push(tv, L);

    if (res == PE_NO_MEMORY) {
        luaL_error(L, "out of memory");
    } else if (res == PE_INVALID_TYPE) {
        const char *const type = luaL_typename(L, 2);

        luaL_error(L, "bad argument #2 to 'l_Vec_push' (expected number, got %s)", type);
    }
}

static int num_try_push(TypeVec *tv, lua_State *L) {
    lua_Number num;
    int is_number;

    assert(tv);
    assert(L);

    num = lua_tonumberx(L, 2, &is_number);

    if (!is_number) {
        return PE_INVALID_TYPE;
    }

    if (Vec_push(&tv->vec, &num) != LARR_OK) {
        return PE_NO_MEMORY;
    }

    return PE_OK;
}

static void num_insert(TypeVec *tv, size_t index, lua_State *L) {
    lua_Number num;
    int ret;

    assert(tv);
    assert(L);

    num = luaL_checknumber(L, 3);
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
    int res;

    assert(tv);
    assert(L);

    res = int_try_push(tv, L);

    if (res == PE_NO_MEMORY) {
        luaL_error(L, "out of memory");
    } else if (res == PE_INVALID_TYPE) {
        const char *const type = luaL_typename(L, 2);

        luaL_error(L, "bad argument #2 to 'l_Vec_push' (expected integer, got %s)", type);
    }
}

static int int_try_push(TypeVec *tv, lua_State *L) {
    lua_Integer integer;
    int is_integer;

    assert(tv);
    assert(L);

    integer = lua_tointegerx(L, 2, &is_integer);

    if (!is_integer) {
        return PE_INVALID_TYPE;
    }

    if (Vec_push(&tv->vec, &integer) != LARR_OK) {
        return PE_NO_MEMORY;
    }

    return PE_OK;
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
