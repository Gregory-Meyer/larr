#ifndef UTIL_H
#define UTIL_H

#include "vec.h"

#include <stddef.h>
#include <stdint.h>

#include <lua.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct String {
    const char *str;
    size_t len;
} String;

/* neat little trick inspired by ZCM */
#define TYPES \
    X(TP_NUM, lua_Number, num, "number") \
    X(TP_INT, lua_Integer, int, "integer") \
    X(TP_BOOL, uint8_t, bool, "boolean") \
    X(TP_STR, String, str, "string") \
    X(TP_TBL, int, tbl, "table") \
    X(TP_FN, int, fn, "function") \
    X(TP_USERDATA, int, userdata, "userdata") \
    X(TP_THREAD, lua_State*, thread, "thread") \
    X(TP_LIGHT_USERDATA, void*, light_userdata, "light_userdata")

typedef enum Type {
    #define X(name, type, nickname, string) name,
    TYPES
    #undef X
} Type;

typedef struct Typeinfo {
    int type; /* one of Type */
    const char *name;
} Typeinfo;

typedef struct TypeVec TypeVec;

typedef struct Vtbl {
    void (*push)(TypeVec*, lua_State*);
    void (*insert)(TypeVec*, size_t, lua_State*);
    void (*clear)(TypeVec*, lua_State*);
    void (*set_elem)(TypeVec*, size_t, lua_State*);
    void (*first)(const TypeVec*, lua_State*);
    void (*last)(const TypeVec*, lua_State*);
    void (*push_elem)(const TypeVec*, size_t, lua_State*);
} Vtbl;

struct TypeVec {
    Vec vec;
    Typeinfo typeinfo;
    const Vtbl *vtbl;
};

size_t sizeof_type_repr(int type);

size_t check_size_t(lua_State *L, int arg);

void push_size_t(lua_State *L, size_t x);

size_t to_size_t(lua_State *L, int arg, int *is_size_t);

Typeinfo check_typeinfo(lua_State *L, int arg);

const TypeVec* check_tv(lua_State *L, int arg);

TypeVec* check_tv_mut(lua_State *L, int arg);

const Vtbl* get_vtbl(int type);

const Vtbl* num_vtbl(void);

const Vtbl* int_vtbl(void);

const Vtbl* bool_vtbl(void);

const Vtbl* str_vtbl(void);

const Vtbl* tbl_vtbl(void);

const Vtbl* fn_vtbl(void);

const Vtbl* userdata_vtbl(void);

const Vtbl* thread_vtbl(void);

const Vtbl* light_userdata_vtbl(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
