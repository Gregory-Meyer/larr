#include <larr/larr.h>

#include "array.h"

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

typedef enum Type {
    TYPE_NUMBER, /* store lua_Number */
    TYPE_INTEGER, /* store lua_Integer */
    TYPE_BOOLEAN, /* store uint8_t */
    TYPE_STRING, /* store const char* */
    TYPE_TABLE, /* store int (reference) */
    TYPE_FUNCTION, /* store int (reference) */
    TYPE_USERDATA, /* store int (reference) */
    TYPE_THREAD, /* store lua_State* */
    TYPE_LIGHT_USERDATA /* store void* */
} Type;

typedef struct Typeinfo {
    int type;
    const char *typename;
} Typeinfo;

typedef struct TypedArray {
    Array array;
    Typeinfo type;
} TypedArray;

static int check_arity(lua_State *L, int arity);

static void check_typeinfo(lua_State *L, int arg, Typeinfo *tinfo);

static size_t type_size(const Typeinfo *tinfo);

int larr_Array_new(lua_State *L) {
    Typeinfo type;
    TypedArray *arr;

    assert(L);

    check_arity(L, 1);

    check_typeinfo(L, 1, &type);

    lua_pop(L, 1);

    arr = (TypedArray*) lua_newuserdata(L, sizeof(Array));

    Array_new(&arr->array, type_size(&type));
    arr->type = type;

    luaL_setmetatable(L, "larr.TypedArray");

    return 1;
}

static size_t check_size_t(lua_State *L, int arg);

int larr_Array_with_capacity(lua_State *L) {
    Typeinfo tinfo;
    size_t capacity;
    TypedArray *arr;

    assert(L);

    check_arity(L, 2);

    check_typeinfo(L, 1, &tinfo);
    capacity = check_size_t(L, 2);

    lua_pop(L, 2);

    arr = (TypedArray*) lua_newuserdata(L, sizeof(Array));

    if (Array_with_capacity(&arr->array, type_size(&tinfo), (size_t) capacity) != LARR_OK) {
        return luaL_error(L, "couldn't allocate space for %ld elements", (long) capacity);
    }

    arr->type = tinfo;

    luaL_setmetatable(L, "larr.Array");

    return 1;
}

static TypedArray* check_array(lua_State *L);

int larr_Array_meta_gc(lua_State *L) {
    TypedArray *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    Array_delete(&arr->array);

    return 0;
}

static void push_size_t(lua_State *L, size_t value);

int larr_Array_capacity(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    push_size_t(L, Array_capacity(&arr->array));

    return 1;
}

int larr_Array_meta_len(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    push_size_t(L, Array_len(&arr->array));

    return 1;
}

int larr_Array_is_empty(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);

    lua_pushboolean(L, Array_is_empty(&arr->array));

    return 1;
}

int larr_Array_first(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    check_arity(L, 1);
    arr = check_array(L);
    lua_pop(L, 1);
}

int larr_Array_last(lua_State *L);

int larr_Array_meta_index(lua_State *L);

int larr_Array_meta_newindex(lua_State *L);

int larr_Array_push(lua_State *L);

int larr_Array_pop(lua_State *L);

int larr_Array_insert(lua_State *L);

int larr_Array_remove(lua_State *L);

int larr_Array_clear(lua_State *L);

int luaopen_liblarr(lua_State *L) {
    int registered;

    assert(L);

    luaL_checkstack(L, 3, NULL);

    lua_createtable(L, 0, 1);
    registered = luaL_newmetatable(L, "larr.Array");
    assert(registered);

    lua_pushcfunction(L, larr_Array_new);
    lua_setfield(L, 2, "new");

    lua_pushcfunction(L, larr_Array_with_capacity);
    lua_setfield(L, 2, "with_capacity");

    lua_pushcfunction(L, larr_Array_meta_gc);
    lua_setfield(L, 2, "__gc");

    lua_pushcfunction(L, larr_Array_capacity);
    lua_setfield(L, 2, "capacity");

    lua_pushcfunction(L, larr_Array_meta_len);
    lua_setfield(L, 2, "__len");

    lua_pushcfunction(L, larr_Array_is_empty);
    lua_setfield(L, 2, "is_empty");

    lua_pushcfunction(L, larr_Array_first);
    lua_setfield(L, 2, "first");

    lua_pushcfunction(L, larr_Array_last);
    lua_setfield(L, 2, "last");

    lua_pushcfunction(L, larr_Array_meta_index);
    lua_setfield(L, 2, "__index");

    lua_pushcfunction(L, larr_Array_meta_newindex);
    lua_setfield(L, 2, "__newindex");

    lua_pushcfunction(L, larr_Array_push);
    lua_setfield(L, 2, "push");

    lua_pushcfunction(L, larr_Array_pop);
    lua_setfield(L, 2, "pop");

    lua_pushcfunction(L, larr_Array_insert);
    lua_setfield(L, 2, "insert");

    lua_pushcfunction(L, larr_Array_remove);
    lua_setfield(L, 2, "remove");

    lua_pushcfunction(L, larr_Array_clear);
    lua_setfield(L, 2, "clear");

    lua_copy(L, 2, 3);
    lua_setfield(L, 2, "__index");

    lua_setfield(L, 2, "Array");

    return 1;
}

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

static void check_typeinfo(lua_State *L, int arg, Typeinfo *tinfo) {
    static const char NUMBER[] = "number";
    static const char INTEGER[] = "integer";
    static const char BOOLEAN[] = "boolean";
    static const char STRING[] = "string";
    static const char TABLE[] = "table";
    static const char FUNCTION[] = "function";
    static const char THREAD[] = "thread";
    static const char LIGHT_USERDATA[] = "light_userdata";

    size_t length;
    const char *const typename = luaL_checklstring(L, arg, &length);

    if (length == sizeof(NUMBER) - 1 && memcmp(typename, NUMBER, length) == 0) {
        tinfo->type = TYPE_NUMBER;
        tinfo->typename = NUMBER;
    } else if (length == sizeof(INTEGER) - 1 && memcmp(typename, INTEGER, length) == 0) {
        tinfo->type = TYPE_INTEGER;
        tinfo->typename = INTEGER;
    } else if (length == sizeof(BOOLEAN) - 1 && memcmp(typename, BOOLEAN, length) == 0) {
        tinfo->type = TYPE_BOOLEAN;
        tinfo->typename = BOOLEAN;
    } else if (length == sizeof(STRING) - 1 && memcmp(typename, STRING, length) == 0) {
        tinfo->type = TYPE_STRING;
        tinfo->typename = STRING;
    } else if (length == sizeof(TABLE) - 1 && memcmp(typename, TABLE, length) == 0) {
        tinfo->type = TYPE_TABLE;
        tinfo->typename = TABLE;
    } else if (length == sizeof(FUNCTION) - 1 && memcmp(typename, FUNCTION, length) == 0) {
        tinfo->type = TYPE_FUNCTION;
        tinfo->typename = FUNCTION;
    } else if (length == sizeof(THREAD) - 1 && memcmp(typename, THREAD, length) == 0) {
        tinfo->type = TYPE_THREAD;
        tinfo->typename = THREAD;
    } else if (length == sizeof(LIGHT_USERDATA) - 1
               && memcmp(typename, LIGHT_USERDATA, length) == 0) {
        tinfo->type = TYPE_LIGHT_USERDATA;
        tinfo->typename = LIGHT_USERDATA;
    } else {
        tinfo->type = TYPE_USERDATA;
        tinfo->typename = typename;
    }
}

static TypedArray* check_array(lua_State *L) {
    assert(L);

    return (TypedArray*) luaL_checkudata(L, 1, "larr.Array");
}

static size_t type_size(const Typeinfo *tinfo) {
    assert(tinfo);

    switch (tinfo->type) {
        case TYPE_NUMBER:
            return sizeof(lua_Number);
        case TYPE_INTEGER:
            return sizeof(lua_Integer);
        case TYPE_BOOLEAN:
            return sizeof(int8_t);
        case TYPE_STRING:
            return sizeof(const char*);
        case TYPE_TABLE:
            return sizeof(int);
        case TYPE_FUNCTION:
            return sizeof(int);
        case TYPE_USERDATA:
            return sizeof(int);
        case TYPE_THREAD:
            return sizeof(lua_State*);
        default:
            assert(0 && "invalid type passed!");
    }
}

static int can_cast_to_size_t(lua_Integer x);

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
