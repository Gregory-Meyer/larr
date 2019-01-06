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

typedef struct String {
    const char *str;
    size_t len;
} String;

static void check_typeinfo(lua_State *L, int arg, Typeinfo *tinfo);

static size_t type_size(const Typeinfo *tinfo);

int l_Array_new(lua_State *L) {
    Typeinfo type;
    TypedArray *arr;

    assert(L);

    check_typeinfo(L, 1, &type);

    arr = (TypedArray*) lua_newuserdata(L, sizeof(TypedArray));

    Array_new(&arr->array, type_size(&type));
    arr->type = type;

    luaL_setmetatable(L, "larr.Array");
    /*lua_pushvalue(L, 2);
    lua_setfield(L, 2, "__index");*/

    return 1;
}

static size_t check_size_t(lua_State *L, int arg);

int l_Array_with_capacity(lua_State *L) {
    Typeinfo tinfo;
    size_t capacity;
    TypedArray *arr;

    assert(L);

    check_typeinfo(L, 1, &tinfo);
    capacity = check_size_t(L, 2);

    arr = (TypedArray*) lua_newuserdata(L, sizeof(TypedArray));

    if (Array_with_capacity(&arr->array, type_size(&tinfo), (size_t) capacity) != LARR_OK) {
        return luaL_error(L, "couldn't allocate space for %ld elements", (long) capacity);
    }

    arr->type = tinfo;

    luaL_setmetatable(L, "larr.Array");

    return 1;
}

static TypedArray* check_array(lua_State *L);

int l_Array_meta_gc(lua_State *L) {
    TypedArray *arr;

    assert(L);

    arr = check_array(L);

    Array_delete(&arr->array);

    return 0;
}

static void push_size_t(lua_State *L, size_t value);

int l_Array_capacity(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    arr = check_array(L);

    push_size_t(L, Array_capacity(&arr->array));

    return 1;
}

int l_Array_meta_len(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    arr = check_array(L);

    push_size_t(L, Array_len(&arr->array));

    return 1;
}

int l_Array_is_empty(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    arr = check_array(L);

    lua_pushboolean(L, Array_is_empty(&arr->array));

    return 1;
}

static void push_elem(lua_State *L, const Typeinfo *tinfo, const void *elem);

int l_Array_first(lua_State *L) {
    const TypedArray *arr;
    const void *elem;

    assert(L);

    arr = check_array(L);

    elem = Array_first(&arr->array);

    if (!elem) {
        lua_pushnil(L);

        return 1;
    }

    push_elem(L, &arr->type, elem);

    return 1;
}

int l_Array_last(lua_State *L) {
    const TypedArray *arr;
    const void *elem;

    assert(L);

    arr = check_array(L);

    elem = Array_last(&arr->array);

    if (!elem) {
        lua_pushnil(L);

        return 1;
    }

    push_elem(L, &arr->type, elem);

    return 1;
}

static size_t to_size_t(lua_State *L, int arg, int *is_size_t);

int l_Array_meta_index(lua_State *L) {
    const TypedArray *arr;
    size_t index;
    int is_integer;
    const void *elem;

    assert(L);

    arr = check_array(L);
    index = to_size_t(L, 2, &is_integer) - 1;

    if (!is_integer) {
        const char *const str = lua_tostring(L, 2);
        luaL_getmetafield(L, 1, str);

        return 1;
    }

    elem = Array_get(&arr->array, index);

    if (!elem) {
        lua_pushnil(L);

        return 1;
    }

    push_elem(L, &arr->type, elem);

    return 1;
}

int l_Array_meta_newindex(lua_State *L) {
    return 0;

    /*
    TypedArray *arr;
    size_t index;

    assert(L);

    check_arity(L, 3);
    arr = check_array(L);
    index = check_size_t(L, 2);

    set_elem()

    if (!elem) {
        lua_pushnil(L);

        return 1;
    }

    push_elem(L, &arr->type, elem);

    return 1;
    */
}

typedef struct TypedArrayVtable {
    void (*number_fn)(lua_State*, Array*);
    void (*integer_fn)(lua_State*, Array*);
    void (*boolean_fn)(lua_State*, Array*);
    void (*string_fn)(lua_State*, Array*);
    void (*table_fn)(lua_State*, Array*);
    void (*function_fn)(lua_State*, Array*);
    void (*userdata_fn)(lua_State*, Array*, const char*);
    void (*thread_fn)(lua_State*, Array*);
    void (*light_userdata_fn)(lua_State*, Array*);
} TypedArrayVtable;

static void virtual_call(lua_State *L, TypedArray *tarr, const TypedArrayVtable *vtbl);

static void push_number(lua_State *L, Array *arr);

static void push_integer(lua_State *L, Array *arr);

static void push_boolean(lua_State *L, Array *arr);

static void push_string(lua_State *L, Array *arr);

static void push_table(lua_State *L, Array *arr);

static void push_function(lua_State *L, Array *arr);

static void push_userdata(lua_State *L, Array *arr, const char *typename);

static void push_thread(lua_State *L, Array *arr);

static void push_light_userdata(lua_State *L, Array *arr);

int l_Array_push(lua_State *L) {
    static const TypedArrayVtable vtbl = {
        push_number, push_integer, push_boolean, push_string, push_table, push_function,
        push_userdata, push_thread, push_light_userdata
    };

    TypedArray *arr;

    assert(L);

    arr = check_array(L);
    virtual_call(L, arr, &vtbl);

    return 0;
}

int l_Array_pop(lua_State *L) {
    TypedArray *arr;

    assert(L);

    arr = check_array(L);

    if (Array_pop(&arr->array) != LARR_OK) {
        return luaL_error(L, "cannot pop from an empty Array!");
    }

    return 0;
}

int l_Array_insert(lua_State *L) {
    return 0;
}

int l_Array_remove(lua_State *L) {
    return 0;
}

int l_Array_clear(lua_State *L) {
    return 0;
}

int l_Array_meta_tostring(lua_State *L) {
    const TypedArray *arr;

    assert(L);

    arr = (const TypedArray*) check_array(L);

    if (Array_is_empty(&arr->array)) {
        lua_pushliteral(L, "{}");
    } else {
        size_t i;
        int first = 1;

        luaL_Buffer buf;
        luaL_buffinit(L, &buf);
        luaL_addchar(&buf, '{');

        for (i = 0; i < Array_len(&arr->array); ++i) {
            const void *elem;

            if (!first) {
                luaL_addlstring(&buf, ", ", 2);
            } else {
                first = 0;
            }

            elem = Array_get(&arr->array, i);
            push_elem(L, &arr->type, elem);
            luaL_addvalue(&buf);
        }

        luaL_addchar(&buf, '}');
        luaL_pushresult(&buf);
    }

    return 1;
}

int luaopen_liblarr(lua_State *L) {
    static const luaL_Reg funcs[] = {
        { "new", l_Array_new },
        { "with_capacity", l_Array_with_capacity },
        { "__gc", l_Array_meta_gc },
        { "capacity", l_Array_capacity },
        { "__len", l_Array_meta_len },
        { "is_empty", l_Array_is_empty },
        { "first", l_Array_first },
        { "last", l_Array_last },
        { "__index", l_Array_meta_index },
        { "__newindex", l_Array_meta_newindex },
        { "push", l_Array_push },
        { "pop", l_Array_pop },
        { "insert", l_Array_insert },
        { "remove", l_Array_remove },
        { "clear", l_Array_clear },
        { "__tostring", l_Array_meta_tostring },
        { NULL, NULL }
    };

    assert(L);

    lua_newtable(L);

    luaL_newmetatable(L, "larr.Array");
    luaL_setfuncs(L, funcs, 0);

    lua_setfield(L, -2, "Array");

    return 1;
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

static void push_number(lua_State *L, Array *arr) {
    const lua_Number num = luaL_checknumber(L, 2);
    Array_push(arr, &num);
}

static void push_integer(lua_State *L, Array *arr) {
    const lua_Integer integer = luaL_checkinteger(L, 2);
    Array_push(arr, &integer);
}

static void push_boolean(lua_State *L, Array *arr) {
    const uint8_t boolean = (lua_toboolean(L, 2) != 0);
    Array_push(arr, &boolean);
}

static void push_string(lua_State *L, Array *arr) {
    String str;

    str.str = lua_tolstring(L, 2, &str.len);
    Array_push(arr, &str);
}

static void push_table(lua_State *L, Array *arr) {
    int ref;

    luaL_checktype(L, 2, LUA_TTABLE);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    Array_push(arr, &ref);
}

static void push_function(lua_State *L, Array *arr) {
    int ref;

    luaL_checktype(L, 2, LUA_TFUNCTION);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    Array_push(arr, &ref);
}

static void push_userdata(lua_State *L, Array *arr, const char *typename) {
    int ref;

    luaL_checkudata(L, 2, typename);
    ref = luaL_ref(L, LUA_REGISTRYINDEX);
    Array_push(arr, &ref);
}

static void push_thread(lua_State *L, Array *arr) {
    lua_State *const thread = lua_tothread(L, 2);
    luaL_argcheck(L, thread, 2, "not a thread");

    Array_push(arr, &thread);
}

static void push_light_userdata(lua_State *L, Array *arr) {
    void *const userdata = lua_touserdata(L, 2);
    luaL_argcheck(L, userdata && lua_islightuserdata(L, 2), 2, "not light userdata");

    Array_push(arr, &userdata);
}

static size_t type_size(const Typeinfo *tinfo) {
    assert(tinfo);

    switch ((Type) tinfo->type) {
        case TYPE_NUMBER:
            return sizeof(lua_Number);
        case TYPE_INTEGER:
            return sizeof(lua_Integer);
        case TYPE_BOOLEAN:
            return sizeof(int8_t);
        case TYPE_STRING:
            return sizeof(String);
        case TYPE_TABLE:
            return sizeof(int);
        case TYPE_FUNCTION:
            return sizeof(int);
        case TYPE_USERDATA:
            return sizeof(int);
        case TYPE_THREAD:
            return sizeof(lua_State*);
        case TYPE_LIGHT_USERDATA:
            return sizeof(void*);
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

static size_t to_size_t(lua_State *L, int arg, int *is_size_t) {
    lua_Integer value;
    int is_integer;

    assert(L);

    value = lua_tointegerx(L, arg, &is_integer);

    if (is_size_t) {
        *is_size_t = is_integer && can_cast_to_size_t(value);
    }

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

static void push_elem(lua_State *L, const Typeinfo *tinfo, const void *elem) {
    assert(L);
    assert(tinfo);
    assert(elem);

    switch ((Type) tinfo->type) {
        case TYPE_NUMBER:
            lua_pushnumber(L, *(const lua_Number*) elem);

            break;
        case TYPE_INTEGER:
            lua_pushinteger(L, *(const lua_Integer*) elem);

            break;
        case TYPE_BOOLEAN:
            lua_pushboolean(L, *(const uint8_t*) elem);

            break;
        case TYPE_STRING: {
            const String *const str = (const String*) elem;

            lua_pushlstring(L, str->str, str->len);

            break;
        } case TYPE_TABLE:
        case TYPE_FUNCTION:
        case TYPE_USERDATA:
            lua_rawgeti(L, LUA_REGISTRYINDEX, *(const int*) elem);

            break;
        case TYPE_THREAD:
            lua_pushthread(*(lua_State *const *) elem);

            break;
        case TYPE_LIGHT_USERDATA:
            lua_pushlightuserdata(L, *(void *const *) elem);

            break;
    }
}

static void virtual_call(lua_State *L, TypedArray *tarr, const TypedArrayVtable *vtbl) {
    Array *arr;
    const Typeinfo *type;

    assert(L);
    assert(tarr);
    assert(vtbl);
    assert(vtbl->number_fn && vtbl->integer_fn && vtbl->boolean_fn && vtbl->string_fn
           && vtbl->table_fn && vtbl->function_fn && vtbl->userdata_fn && vtbl->thread_fn
           && vtbl->light_userdata_fn);

    arr = &tarr->array;
    type = &tarr->type;

    switch ((Type) type->type) {
        case TYPE_NUMBER:
            vtbl->number_fn(L, arr);

            break;
        case TYPE_INTEGER:
            vtbl->integer_fn(L, arr);

            break;
        case TYPE_BOOLEAN:
            vtbl->boolean_fn(L, arr);

            break;
        case TYPE_STRING:
            vtbl->string_fn(L, arr);

            break;
        case TYPE_TABLE:
            vtbl->table_fn(L, arr);

            break;
        case TYPE_FUNCTION:
            vtbl->function_fn(L, arr);

            break;
        case TYPE_USERDATA:
            vtbl->userdata_fn(L, arr, type->typename);

            break;
        case TYPE_THREAD:
            vtbl->thread_fn(L, arr);

            break;
        case TYPE_LIGHT_USERDATA:
            vtbl->light_userdata_fn(L, arr);

            break;
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
