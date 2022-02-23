/* XenonJS : Runtime Time System : Value
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_VAL_H
#define XJR_VAL_H

#include "xjr_dt.h"
#include "xjr_mp.h"
#include "xjr_x.h"
#include "xjr_vm_dt.h"

/* Value encoding
 * --------------
 * 
 * Each register holds a 32-bit integer. we use 3 bits as type tag.
 *
 * TypeTag  | Value
 * 000      | 0 or Positive Integer ()
 * 001      | Negative Integer
 * 010      | false | true | undefined | null (4???)
 * 011      | Float-Point Number
 * 100      | Reference String (str.length > 3)
 * 101      | Function (a???)
 * 110      | Object (c???)
 * 111      | Array
 */

/* Fundamental things */

#define XJR_TAG_POSINT 0U
#define XJR_TAG_NEGINT 1U
#define XJR_TAG_RESERVED_VAL 2U
#define XJR_TAG_FLOAT 3U
#define XJR_TAG_STRING 4U
#define XJR_TAG_FUNCTION 5U
#define XJR_TAG_OBJECT 6U
#define XJR_TAG_ARRAY 7U

#define XJR_VAL_MAKE(_tag, _body) ((xjr_val)(((_tag) << 29) | (_body)))
#define XJR_VAL_TAG(_v) (((_v) >> 29) & 0x7)
#define XJR_VAL_BODY(_v) ((_v) & 0x1FFFFFFF)

/* Integer */
/* int xjr_encode_integer(xjr_val *v, int value); */

#define XJR_VAL_MAKE_INTEGER(_v) \
    (((_v) >= 0) ? XJR_VAL_MAKE(XJR_TAG_POSINT, (xjr_u32)(_v)) : XJR_VAL_MAKE(XJR_TAG_NEGINT, (xjr_u32)(-(_v))))
#define XJR_VAL_IS_INTEGER(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_POSINT) || \
     (XJR_VAL_TAG(_v) == XJR_TAG_NEGINT))
#define XJR_VAL_AS_INTEGER_UNTAG(_v) \
    ((int)((XJR_VAL_TAG(_v) == XJR_TAG_POSINT) ? XJR_VAL_BODY(_v) : (-XJR_VAL_BODY(_v))))

/* Boolean */
#define XJR_VAL_MAKE_BOOLEAN_FALSE() \
    XJR_VAL_MAKE(XJR_TAG_RESERVED_VAL, 0)
#define XJR_VAL_MAKE_BOOLEAN_TRUE() \
    XJR_VAL_MAKE(XJR_TAG_RESERVED_VAL, 1)
#define XJR_VAL_MAKE_BOOLEAN(_cond) \
    XJR_VAL_MAKE(XJR_TAG_RESERVED_VAL, (_cond) ? 1 : 0)
#define XJR_VAL_MAKE_UNDEFINED() \
    XJR_VAL_MAKE(XJR_TAG_RESERVED_VAL, 2)
#define XJR_VAL_MAKE_NULL() \
    XJR_VAL_MAKE(XJR_TAG_RESERVED_VAL, 3)
#define XJR_VAL_IS_BOOLEAN_FALSE(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_RESERVED_VAL) && \
     (XJR_VAL_BODY(_v) == 0))
#define XJR_VAL_IS_BOOLEAN_TRUE(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_RESERVED_VAL) && \
     (XJR_VAL_BODY(_v) == 1))
#define XJR_VAL_IS_BOOLEAN(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_RESERVED_VAL) && \
     ((XJR_VAL_BODY(_v) == 0) || (XJR_VAL_BODY(_v) == 1)))
#define XJR_VAL_IS_UNDEFINED(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_RESERVED_VAL) && \
     (XJR_VAL_BODY(_v) == 2))
#define XJR_VAL_IS_NULL(_v) \
    ((XJR_VAL_TAG(_v) == XJR_TAG_RESERVED_VAL) && \
     (XJR_VAL_BODY(_v) == 3))

xjr_bool xjr_val_eq2(xjr_mp_t *mp, xjr_val val_lhs, xjr_val val_rhs);

/* Properties */

#define XJR_VAL_PROPERTIES_BLTIN_PROTO 0
#define XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE 1
#define XJR_VAL_PROPERTIES_BLTIN_COUNT (XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE + 1)

typedef enum
{
    XJR_VAL_PROPERTY_TYPE_NORMAL,
    XJR_VAL_PROPERTY_TYPE_GETTER,
    XJR_VAL_PROPERTY_TYPE_SETTER,
} xjr_val_property_type;

typedef struct
{
    xjr_val key;

    xjr_val_property_type type;
    union {
        xjr_val value;
        xjr_val getter;
        xjr_val setter;
    } u;

    xjr_urid_t next;
} xjr_val_property;

typedef struct
{
    xjr_urid_t begin, end;

    /* Built-in properties */
    xjr_val bltin[XJR_VAL_PROPERTIES_BLTIN_COUNT];
} xjr_val_properties;

void xjr_val_properties_init(xjr_val_properties *props);
int xjr_val_properties_get(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val *val_out, xjr_val_property_type *property_type_out, \
        xjr_val key);
int xjr_val_properties_get_by_name(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val *val_out, xjr_val_property_type *property_type_out, \
        const char *name, const xjr_size_t len);
xjr_val xjr_val_properties_get_bltin(xjr_val_properties *props, int index);
int xjr_val_properties_set(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val_property_type type, \
        xjr_val key, xjr_val value);
int xjr_val_properties_set_by_name(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val_property_type type, \
        const char *name, const xjr_size_t len, \
        xjr_val value);
void xjr_val_properties_set_bltin(xjr_val_properties *props, int index, xjr_val val);

/* Float-point number */

typedef enum
{
    xjr_val_float_type_f32,
    xjr_val_float_type_f64,
} xjr_val_float_type;

typedef struct
{
    xjr_val_float_type type;

    union
    {
        xjr_f32 as_f32;
        xjr_f64 as_f64;
    } u;
} xjr_val_float;

xjr_val xjr_val_make_f32(xjr_mp_t *mp, xjr_f32 value);
xjr_val xjr_val_make_f64(xjr_mp_t *mp, xjr_f64 value);
xjr_val_float_type xjr_val_as_float_type(xjr_mp_t *mp, xjr_val v);
xjr_f32 xjr_val_as_float_extract_f32(xjr_mp_t *mp, xjr_val v);
xjr_f64 xjr_val_as_float_extract_f64(xjr_mp_t *mp, xjr_val v);
#define XJR_VAL_IS_FLOAT(_v) \
    (XJR_VAL_TAG(_v) == XJR_TAG_FLOAT)

/* Number */
#define XJR_VAL_IS_NUMBER(_v) \
    (XJR_VAL_IS_INTEGER(_v) || (XJR_VAL_IS_FLOAT(_v)))
xjr_f64 xjr_val_extract_f64(xjr_mp_t *mp, xjr_val v);

/* Object */

typedef void (*xjr_val_object_attached_data_mark_cb)(void *attached_data);
typedef void (*xjr_val_object_attached_data_sweep_cb)(void *attached_data);

typedef struct
{
    xjr_val_properties props;

    struct
    {
        void *data;
        xjr_val_object_attached_data_mark_cb cb_mark;
        xjr_val_object_attached_data_sweep_cb cb_sweep;
    } attached_data;
} xjr_val_object;

xjr_val xjr_val_make_object(xjr_mp_t *mp);
xjr_val_object *xjr_val_as_object_extract(xjr_mp_t *mp, xjr_val v);
xjr_val_properties *xjr_val_as_object_property_get(xjr_mp_t *mp, xjr_val v);
xjr_val xjr_val_as_object_get_by_path(xjr_mp_t *mp, xjr_val v, ...);

void xjr_val_object_set_attached_data(xjr_val_object *obj, \
        void *data, \
        xjr_val_object_attached_data_mark_cb cb_mark, \
        xjr_val_object_attached_data_sweep_cb cb_sweep);
void *xjr_val_object_get_attached_data(xjr_val_object *obj);

#define XJR_VAL_IS_OBJECT(_v) \
    (XJR_VAL_TAG(_v) == XJR_TAG_OBJECT)

/* Array */

typedef struct
{
    xjr_val element;
    xjr_urid_t prev, next;
} xjr_val_array_item;
xjr_urid_t xjr_val_make_array_item(xjr_mp_t *mp, xjr_val element);

typedef struct
{
    xjr_urid_t begin, end;
    xjr_size_t size;

    xjr_val_properties props;
} xjr_val_array;

xjr_val xjr_val_make_array(xjr_mp_t *mp);
xjr_val_array *xjr_val_as_array_extract(xjr_mp_t *mp, xjr_val v);
xjr_val_properties *xjr_val_as_array_property_get(xjr_mp_t *mp, xjr_val v);
int xjr_val_as_array_push(xjr_mp_t *mp, xjr_val v, xjr_val elem);
int xjr_val_as_array_unshift(xjr_mp_t *mp, xjr_val v, xjr_val elem);
xjr_val xjr_val_as_array_pop(xjr_mp_t *mp, xjr_val v);
xjr_val xjr_val_as_array_shift(xjr_mp_t *mp, xjr_val v);
xjr_val xjr_val_as_array_reverse(xjr_mp_t *mp, xjr_val v);
/* int xjr_val_as_array_sort(xjr_mp_t *mp, xjr_val v); */
xjr_val xjr_val_as_array_join(xjr_mp_t *mp, xjr_val v, xjr_val separator);
xjr_val xjr_val_as_array_get_by_idx(xjr_mp_t *mp, xjr_val v, int idx);
xjr_val xjr_val_as_array_set_by_idx(xjr_mp_t *mp, xjr_val v, int idx, xjr_val new_elment);
xjr_size_t xjr_val_as_array_size(xjr_mp_t *mp, xjr_val v);

#define XJR_VAL_IS_ARRAY(_v) \
    (XJR_VAL_TAG(_v) == XJR_TAG_ARRAY)

/* Function */

typedef enum
{
    xjr_val_function_type_native,
    xjr_val_function_type_normal,
} xjr_val_function_type;

struct xjr_opaque_native_fn_args;
typedef struct xjr_opaque_native_fn_args xjr_native_fn_args;
typedef void (*xjr_native_fn_cb_t)(xjr_native_fn_args *args);

#define XJR_VAL_FUNCTION_OPTS_ARROW (1 << 0)

typedef struct
{
    xjr_val_function_type type;
    union
    {
        struct
        {
            xjr_native_fn_cb_t cb;
        } as_native;
        struct
        {
            xjr_offset_t offset;
        } as_normal;
    } u;
    xjr_urid_t env;

    xjr_val_properties props;

    xjr_u32 opts;
} xjr_val_function;

xjr_val xjr_val_make_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_offset_t offset);
xjr_val xjr_val_make_arrow_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_offset_t offset);
xjr_val xjr_val_make_native_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_native_fn_cb_t cb);
xjr_val_function *xjr_val_as_function_extract(xjr_mp_t *mp, xjr_val v);
xjr_val_properties *xjr_val_as_function_property_get(xjr_mp_t *mp, xjr_val v);
xjr_bool xjr_val_as_function_e2(xjr_mp_t *mp, xjr_val v1, xjr_val v2);
xjr_bool xjr_val_as_function_e3(xjr_mp_t *mp, xjr_val v1, xjr_val v2);

#define XJR_VAL_IS_FUNCTION(_v) \
    (XJR_VAL_TAG(_v) == XJR_TAG_FUNCTION)

/* String */

typedef enum
{
    xjr_val_string_type_datasec,
    xjr_val_string_type_heap,
} xjr_val_string_type;

typedef struct
{
    xjr_val_string_type type;
    union
    {
        struct
        {
            char *body;
            xjr_size_t len;
        } as_datasec;
        struct
        {
            xjr_urid_t u;
            xjr_size_t len;
        } as_heap;
    } u;

    xjr_val_properties props;
} xjr_val_string;

xjr_val xjr_val_make_string_from_datasec(xjr_xfile *f, xjr_mp_t *mp, \
        xjr_offset_t offset);
xjr_val xjr_val_make_string_from_heap(xjr_mp_t *mp, \
        const char *body, const xjr_size_t len);
xjr_val xjr_val_make_string_from_int(xjr_mp_t *mp, const int v);
xjr_val_string *xjr_val_as_string_extract(xjr_mp_t *mp, xjr_val v);
char *xjr_val_as_string_body(xjr_mp_t *mp, xjr_val v);
xjr_size_t xjr_val_as_string_length(xjr_mp_t *mp, xjr_val v);
xjr_bool xjr_val_as_string_e3(xjr_mp_t *mp, xjr_val v1, xjr_val v2);
xjr_bool xjr_val_as_string_e2(xjr_mp_t *mp, xjr_val v1, xjr_val v2);
int xjr_val_as_string_indexof(xjr_mp_t *mp, xjr_val v, xjr_val v_search_value, xjr_val v_from_index);
xjr_val_properties *xjr_val_as_string_property_get(xjr_mp_t *mp, xjr_val v);
xjr_val xjr_val_as_string_get_by_idx(xjr_mp_t *mp, xjr_val v, int idx);


#define XJR_VAL_IS_STRING(_v) \
    (XJR_VAL_TAG(_v) == XJR_TAG_STRING)

#define XJR_VAL_HAS_PROPERTIES(_v) \
    ((XJR_VAL_IS_OBJECT(_v)) || \
     (XJR_VAL_IS_ARRAY(_v)) || \
     (XJR_VAL_IS_FUNCTION(_v)) || \
     (XJR_VAL_IS_STRING(_v)))

#define XJR_VAL_PROPERTY_GET(_mp, _v) \
    (XJR_VAL_IS_OBJECT(_v) ? xjr_val_as_object_property_get(_mp, _v) : \
     (XJR_VAL_IS_ARRAY(_v) ? xjr_val_as_array_property_get(_mp, _v) : \
      (XJR_VAL_IS_FUNCTION(_v) ? xjr_val_as_function_property_get(_mp, _v) : \
       (XJR_VAL_IS_STRING(_v) ? xjr_val_as_string_property_get(_mp, _v) : \
        xjr_nullptr))))

void xjr_val_properties_install(xjr_vm *vm, xjr_val v);

#endif

