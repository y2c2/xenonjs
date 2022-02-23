/* XenonJS : Runtime Time System : Builtin Functions : Helper
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_BLTIN_HELPER_H
#define XJR_BLTIN_HELPER_H

#include "xjr_val.h"
#include "xjr_extn.h"
#include "xjr_err.h"
#include "xjr_mbuf.h"
#include "xjr_libc.h"
#include "xjr_nativefn.h"
#include "xjr_env.h"
#include "xjr_vm_inspect.h"

#define V_OOM(_x) \
    do { \
        if (!(_x)) { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); goto fail; } \
    } while (0)

#define XJR_BLTIN_CREATE_OBJ(_to, _make, _proto, _prototype) \
    do { \
        V_OOM(!(XJR_VAL_IS_UNDEFINED((_to) = (_make)))); \
        xjr_val_properties_set_bltin(XJR_VAL_PROPERTY_GET(mp, _to), \
                XJR_VAL_PROPERTIES_BLTIN_PROTO, _proto); \
        xjr_val_properties_set_bltin(XJR_VAL_PROPERTY_GET(mp, _to), \
                XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE, _prototype); \
    } while (0)

#define OBJ_SET_PROP_RAW(_obj, _type, _key, _key_len, _val) \
    do { \
        if (xjr_val_properties_set_by_name(mp, \
                    XJR_VAL_PROPERTY_GET(mp, _obj), \
                    _type, \
                    _key, _key_len, _val) != 0) \
        { XJR_ERR_UPDATE(&vm->err, XJR_ERR_OOM); goto fail; } \
    } while (0)

#define OBJ_SET_PROP(_obj, _key, _key_len, _val) \
    OBJ_SET_PROP_RAW(_obj, XJR_VAL_PROPERTY_TYPE_NORMAL, _key, _key_len, _val)

#define OBJ_SET_PROP_GETTER(_obj, _key, _key_len, _val) \
    OBJ_SET_PROP_RAW(_obj, XJR_VAL_PROPERTY_TYPE_GETTER, _key, _key_len, _val)

#define XJR_BLTIN_METHOD(_obj, _name, _name_len, _native_fn) \
        do { \
            xjr_val method; \
            XJR_BLTIN_CREATE_OBJ(method, xjr_val_make_native_function(mp, env, _native_fn), \
                    vm->fundamental.global_function_prototype, XJR_VAL_MAKE_UNDEFINED()); \
            OBJ_SET_PROP(_obj, _name, _name_len, method); \
        } while (0)

#define XJR_BLTIN_GETTER(_obj, _name, _name_len, _native_fn) \
        do { \
            xjr_val method; \
            XJR_BLTIN_CREATE_OBJ(method, xjr_val_make_native_function(mp, env, _native_fn), \
                    vm->fundamental.global_function_prototype, XJR_VAL_MAKE_UNDEFINED()); \
            OBJ_SET_PROP_GETTER(_obj, _name, _name_len, method); \
        } while (0)

void *xjr_mbuf_malloc_by_mp(void *data, xjr_mbuf_size_t size);
void xjr_mbuf_free_by_mp(void *data, void *ptr);
int xjr_mbuf_write_cb( \
        void *data, char *s, xjr_size_t len);

#endif

