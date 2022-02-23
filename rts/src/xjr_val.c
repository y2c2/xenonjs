/* XenonJS : Runtime Time System : Value
 * Copyright(c) 2017 y2c2 */

#include <stdarg.h>
#include "xjr_dt.h"
#include "xjr_libc.h"
#include "xjr_env.h"
#include "xjr_val.h"
#include "xjr_mbuf.h"

#define MP_ALLOC(_mp, _t) \
    (xjr_mp_malloc_managed(_mp, sizeof(_t)))

#define MP_ALLOCN(_mp, _t, _n) \
    (xjr_mp_malloc_managed(_mp, sizeof(_t) * (_n)))

xjr_bool xjr_val_eq2(xjr_mp_t *mp, xjr_val lhs, xjr_val rhs)
{
    if (XJR_VAL_IS_INTEGER(lhs) && XJR_VAL_IS_INTEGER(rhs))
    {
        int v_lhs = XJR_VAL_AS_INTEGER_UNTAG(lhs);
        int v_rhs = XJR_VAL_AS_INTEGER_UNTAG(rhs);
        return (v_lhs == v_rhs) ? xjr_true : xjr_false;
    }
    else if (XJR_VAL_IS_BOOLEAN_FALSE(lhs) && XJR_VAL_IS_BOOLEAN_FALSE(rhs))
    { return xjr_true; }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { return xjr_true; }
    else if (XJR_VAL_IS_BOOLEAN_TRUE(lhs) && XJR_VAL_IS_BOOLEAN_TRUE(rhs))
    { return xjr_true; }
    else if ((XJR_VAL_IS_NULL(lhs) || XJR_VAL_IS_UNDEFINED(lhs)) && \
            (XJR_VAL_IS_NULL(rhs) || XJR_VAL_IS_UNDEFINED(rhs)))
    { return xjr_true; }
    else if (XJR_VAL_IS_STRING(lhs) && XJR_VAL_IS_STRING(rhs))
    {
        xjr_size_t lhs_len = xjr_val_as_string_length(mp, lhs);
        xjr_size_t rhs_len = xjr_val_as_string_length(mp, rhs);
        char *lhs_body = xjr_val_as_string_body(mp, lhs);
        char *rhs_body = xjr_val_as_string_body(mp, rhs);
        if (lhs_len != rhs_len) { return xjr_false; }
        if (xjr_strncmp(lhs_body, rhs_body, lhs_len) != 0) { return xjr_false; }
        else { return xjr_true; }
    }
    else
    { return xjr_false; }
}

void xjr_val_properties_init(xjr_val_properties *props)
{
    props->begin = props->end = XJR_URID_INVALID;
    props->bltin[0] = XJR_VAL_MAKE_UNDEFINED();
    props->bltin[1] = XJR_VAL_MAKE_UNDEFINED();
}

int xjr_val_properties_get(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val *val_out, xjr_val_property_type *property_type_out, \
        xjr_val key)
{
    xjr_urid_t cur;
    xjr_val_property *p;

    /* Builtin properties */
    if (XJR_VAL_IS_STRING(key))
    {
        char *key_body = xjr_val_as_string_body(mp, key);
        xjr_size_t key_len = xjr_val_as_string_length(mp, key);
        if ((key_len == 9) && xjr_strncmp(key_body, "__proto__", 9) == 0)
        {
            *property_type_out = XJR_VAL_PROPERTY_TYPE_NORMAL;
            *val_out = xjr_val_properties_get_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO);
            return 0;
        }
        else if ((key_len == 9) && xjr_strncmp(key_body, "prototype", 9) == 0)
        {
            *property_type_out = XJR_VAL_PROPERTY_TYPE_NORMAL;
            *val_out = xjr_val_properties_get_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE);
            return 0;
        }
    }

    /* Normal properties */
    for (cur = props->begin; cur != XJR_URID_INVALID; cur = p->next)
    {
        p = xjr_mp_get_ptr(mp, cur);
        if (xjr_val_eq2(mp, p->key, key) == xjr_true)
        {
            *property_type_out = p->type;
            switch (p->type)
            {
                case XJR_VAL_PROPERTY_TYPE_NORMAL:
                    *val_out = p->u.value;
                    break;
                case XJR_VAL_PROPERTY_TYPE_GETTER:
                    *val_out = p->u.getter;
                    break;
                case XJR_VAL_PROPERTY_TYPE_SETTER:
                    *val_out = XJR_VAL_MAKE_UNDEFINED();
                    break;
            }
            return 0;
        }
    }

    return -1;
}


int xjr_val_properties_get_by_name(xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val *val_out, xjr_val_property_type *property_type_out, \
        const char *name, const xjr_size_t len)
{
    xjr_val key;

    if ((len == 9) && (xjr_strncmp(name, "__proto__", 9) == 0))
    { *val_out = xjr_val_properties_get_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO); return 0; }
    else if ((len == 9) && (xjr_strncmp(name, "prototype", 9) == 0))
    { *val_out = xjr_val_properties_get_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE); return 0; }

    /* TODO: Optimize */
    if (XJR_VAL_IS_UNDEFINED(key = xjr_val_make_string_from_heap(mp, name, len)))
    { return -1; }

    return xjr_val_properties_get(mp, props, val_out, property_type_out, key);
}

xjr_val xjr_val_properties_get_bltin(xjr_val_properties *props, int index)
{
    return props->bltin[index];
}

int xjr_val_properties_set(xjr_mp_t *mp, \
        xjr_val_properties *props, \
        xjr_val_property_type type, \
        xjr_val key, xjr_val value)
{
    xjr_urid_t cur;
    xjr_val_property *p;

    if (XJR_VAL_IS_STRING(key))
    {
        char *key_body = xjr_val_as_string_body(mp, key);
        xjr_size_t key_len = xjr_val_as_string_length(mp, key);
        if ((key_len == 9) && xjr_strncmp(key_body, "__proto__", 9) == 0)
        { xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, value); return 0; }
        else if ((key_len == 9) && xjr_strncmp(key_body, "prototype", 9) == 0)
        { xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE, value); return 0; }
    }

    for (cur = props->begin; cur != XJR_URID_INVALID; cur = p->next)
    {
        p = xjr_mp_get_ptr(mp, cur);
        if (xjr_val_eq2(mp, p->key, key) == xjr_true)
        {
            p->type = type;
            switch (type)
            {
                case XJR_VAL_PROPERTY_TYPE_NORMAL:
                    p->u.value = value;
                    break;
                case XJR_VAL_PROPERTY_TYPE_GETTER:
                    p->u.getter = value;
                    break;
                case XJR_VAL_PROPERTY_TYPE_SETTER:
                    p->u.setter = value;
                    break;
            }
            return 0;
        }
    }

    {
        xjr_urid_t u;
        xjr_val_property *p_new_item;
        if ((u = MP_ALLOC(mp, xjr_val_property)) == XJR_URID_INVALID)
        { return -1; }
        p_new_item = xjr_mp_get_ptr(mp, u);
        p_new_item->key = key;
        {
            p_new_item->type = type;
            switch (type)
            {
                case XJR_VAL_PROPERTY_TYPE_NORMAL:
                    p_new_item->u.value = value;
                    break;
                case XJR_VAL_PROPERTY_TYPE_GETTER:
                    p_new_item->u.getter = value;
                    break;
                case XJR_VAL_PROPERTY_TYPE_SETTER:
                    p_new_item->u.setter = value;
                    break;
            }
        }
        p_new_item->next = XJR_URID_INVALID;

        {
            xjr_val_property *p_final_item;
            if (props->begin == XJR_URID_INVALID)
            {
                props->begin = u;
                props->end = u;
            }
            else
            {
                p_final_item = xjr_mp_get_ptr(mp, props->end);
                p_final_item->next = u;
                props->end = u;
            }
        }
    }

    return 0;
}

int xjr_val_properties_set_by_name( \
        xjr_mp_t *mp, xjr_val_properties *props, \
        xjr_val_property_type type, \
        const char *name, const xjr_size_t len, \
        xjr_val value)
{
    xjr_val key;

    if ((len == 9) && (xjr_strncmp(name, "__proto__", 9) == 0))
    { xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, value); return 0; }
    else if ((len == 9) && (xjr_strncmp(name, "prototype", 9) == 0))
    { xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTOTYPE, value); return 0; }

    /* TODO: Optimize */
    if (XJR_VAL_IS_UNDEFINED(key = xjr_val_make_string_from_heap(mp, name, len)))
    { return -1; }

    return xjr_val_properties_set(mp, props, type, key, value);
}

void xjr_val_properties_set_bltin(xjr_val_properties *props, \
        int index, xjr_val val)
{
    props->bltin[index] = val;
}

/*
int xjr_encode_integer(xjr_val *v, int value)
{
    if (value == 0) { *v = 0; }
    else if (value > 0)
    {
        if (value > 0x1FFFFFFF) { return -1; }
        else
        {
            *v = XJR_VAL_MAKE(XJR_TAG_POSINT, (xjr_u32)value);
        }
    }
    else
    {
        if (value < -0x1FFFFFFF) { return -1; }
        else
        {
            *v = XJR_VAL_MAKE(XJR_TAG_NEGINT, (xjr_u32)(-value));
        }
    }

    return 0;
}
*/

xjr_f64 xjr_val_extract_f64(xjr_mp_t *mp, xjr_val v)
{
    if (XJR_VAL_IS_INTEGER(v))
    {
        return (xjr_f64)(XJR_VAL_AS_INTEGER_UNTAG(v));
    }
    else if (XJR_VAL_IS_FLOAT(v))
    {
        xjr_f64 v_f64 = 0.0;
        xjr_val_float_type ft = xjr_val_as_float_type(mp, v);
        switch (ft)
        {
            case xjr_val_float_type_f32:
                v_f64 = (xjr_f64)xjr_val_as_float_extract_f32(mp, v);
                break;
            case xjr_val_float_type_f64:
                v_f64 = xjr_val_as_float_extract_f64(mp, v);
                break;
        }
        return v_f64;
    }

    return 0.0;
}

xjr_val xjr_val_make_object(xjr_mp_t *mp)
{
    xjr_val v;
    xjr_urid_t u;

    if ((u = MP_ALLOC(mp, xjr_val_object)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    {
        xjr_val_object *obj = xjr_mp_get_ptr(mp, u);
        xjr_val_properties_init(&obj->props);

        obj->attached_data.data = xjr_nullptr;
        obj->attached_data.cb_mark = xjr_nullptr;
        obj->attached_data.cb_sweep = xjr_nullptr;
    }
    v = XJR_VAL_MAKE(XJR_TAG_OBJECT, u);

    return v;
}

xjr_val_object *xjr_val_as_object_extract(xjr_mp_t *mp, xjr_val v)
{
    xjr_urid_t u = (xjr_urid_t)XJR_VAL_BODY(v);
    return xjr_mp_get_ptr(mp, u);
}

xjr_val_properties *xjr_val_as_object_property_get(xjr_mp_t *mp, xjr_val v)
{
    return &xjr_val_as_object_extract(mp, v)->props;
}

xjr_val xjr_val_as_object_get_by_path(xjr_mp_t *mp, xjr_val v, ...)
{
    va_list ap;
    xjr_val cur = v;

    va_start(ap, v);
    for (;;)
    {
        char *name;
        xjr_size_t name_len;

        if ((name = va_arg(ap, char *)) == xjr_nullptr) break;
        if (!XJR_VAL_IS_OBJECT(cur)) { cur = XJR_VAL_MAKE_UNDEFINED(); break; }
        name_len = xjr_strlen(name);
        {
            xjr_val v_property;
            xjr_val_property_type property_type;

            xjr_val_properties *props = xjr_val_as_object_property_get(mp, cur);
            if (xjr_val_properties_get_by_name(mp, props, \
                    &v_property, &property_type, \
                    name, name_len) != 0)
            { cur = XJR_VAL_MAKE_UNDEFINED(); break; }

            cur = v_property;
        }
    }
    va_end(ap);

    return cur;
}

void xjr_val_object_set_attached_data(xjr_val_object *obj, \
        void *data, \
        xjr_val_object_attached_data_mark_cb cb_mark, \
        xjr_val_object_attached_data_sweep_cb cb_sweep)
{
    obj->attached_data.data = data;
    obj->attached_data.cb_mark = cb_mark;
    obj->attached_data.cb_sweep = cb_sweep;
}

void *xjr_val_object_get_attached_data(xjr_val_object *obj)
{
    return obj->attached_data.data;
}

/* Float-point */

xjr_val xjr_val_make_f32(xjr_mp_t *mp, xjr_f32 value)
{
    xjr_urid_t u;
    if ((u = MP_ALLOC(mp, xjr_val_float)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }
    {
        xjr_val_float *f = xjr_mp_get_ptr(mp, u);
        f->type = xjr_val_float_type_f32;
        f->u.as_f32 = value;
    }
    return XJR_VAL_MAKE(XJR_TAG_FLOAT, u);
}

xjr_val xjr_val_make_f64(xjr_mp_t *mp, xjr_f64 value)
{
    xjr_urid_t u;
    if ((u = MP_ALLOC(mp, xjr_val_float)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }
    {
        xjr_val_float *f = xjr_mp_get_ptr(mp, u);
        f->type = xjr_val_float_type_f64;
        f->u.as_f64 = value;
    }
    return XJR_VAL_MAKE(XJR_TAG_FLOAT, u);
}

xjr_val_float_type xjr_val_as_float_type(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_float *f = xjr_mp_get_ptr(mp, (xjr_urid_t)XJR_VAL_BODY(v));
    return f->type;
}

xjr_f32 xjr_val_as_float_extract_f32(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_float *f = xjr_mp_get_ptr(mp, (xjr_urid_t)XJR_VAL_BODY(v));
    return f->u.as_f32;
}

xjr_f64 xjr_val_as_float_extract_f64(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_float *f = xjr_mp_get_ptr(mp, (xjr_urid_t)XJR_VAL_BODY(v));
    return f->u.as_f64;
}

/* Array */

xjr_urid_t xjr_val_make_array_item(xjr_mp_t *mp, xjr_val element)
{
    xjr_urid_t u;

    if ((u = MP_ALLOC(mp, xjr_val_array_item)) == XJR_URID_INVALID)
    { return XJR_URID_INVALID; }
    {
        xjr_val_array_item *arr_item = xjr_mp_get_ptr(mp, u);
        arr_item->prev = arr_item->next = XJR_URID_INVALID;
        arr_item->element = element;
    }

    return u;
}

xjr_val xjr_val_make_array(xjr_mp_t *mp)
{
    xjr_val v;
    xjr_urid_t u;

    if ((u = MP_ALLOC(mp, xjr_val_array)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    {
        xjr_val_array *arr = xjr_mp_get_ptr(mp, u);
        arr->begin = XJR_URID_INVALID;
        arr->end = XJR_URID_INVALID;
        arr->size = 0;
        xjr_val_properties_init(&arr->props);
    }
    v = XJR_VAL_MAKE(XJR_TAG_ARRAY, u);

    return v;
}

xjr_val_array *xjr_val_as_array_extract(xjr_mp_t *mp, xjr_val v)
{
    xjr_urid_t u = (xjr_urid_t)XJR_VAL_BODY(v);
    return xjr_mp_get_ptr(mp, u);
}

xjr_val_properties *xjr_val_as_array_property_get(xjr_mp_t *mp, xjr_val v)
{
    return &xjr_val_as_array_extract(mp, v)->props;
}

xjr_val xjr_val_as_array_shift(xjr_mp_t *mp, xjr_val v)
{
    xjr_val ret;
    xjr_val_array_item *item;
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);

    if (arr->size == 0) return XJR_VAL_MAKE_UNDEFINED();
    item = (xjr_val_array_item *)xjr_mp_get_ptr(mp, arr->begin);
    ret = item->element;

    if (item->next == XJR_URID_INVALID)
    {
        arr->begin = arr->end = XJR_URID_INVALID;
    }
    else
    {
        xjr_val_array_item *item_next = (xjr_val_array_item *)xjr_mp_get_ptr(mp, item->next);
        item_next->prev = XJR_URID_INVALID;
        arr->begin = item->next;
    }
    arr->size--;

    return ret;
}

int xjr_val_as_array_unshift(xjr_mp_t *mp, xjr_val v, xjr_val elem)
{
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);
    xjr_urid_t u_elem_new;

    u_elem_new = xjr_val_make_array_item(mp, elem);
    if (u_elem_new == XJR_URID_INVALID) return -1;

    if (arr->begin == XJR_URID_INVALID)
    {
        arr->begin = arr->end = u_elem_new;
    }
    else
    {
        xjr_val_array_item *item_first;
        item_first = xjr_mp_get_ptr(mp, arr->begin);
        item_first->prev = u_elem_new;
        ((xjr_val_array_item *)(xjr_mp_get_ptr(mp, u_elem_new)))->next = arr->begin;
        arr->begin = u_elem_new;
    }
    arr->size++;

    return 0;
}

int xjr_val_as_array_push(xjr_mp_t *mp, xjr_val v, xjr_val elem)
{
    xjr_val_array *arr;
    xjr_urid_t u_elem_new;
    xjr_val_array_item *item_last;

    arr = xjr_val_as_array_extract(mp, v);
    u_elem_new = xjr_val_make_array_item(mp, elem);
    if (u_elem_new == XJR_URID_INVALID) return -1;

    if (arr->begin == XJR_URID_INVALID)
    {
        arr->begin = arr->end = u_elem_new;
    }
    else
    {
        item_last = xjr_mp_get_ptr(mp, arr->end);
        item_last->next = u_elem_new;
        ((xjr_val_array_item *)(xjr_mp_get_ptr(mp, u_elem_new)))->prev = arr->end;
        arr->end = u_elem_new;
    }
    arr->size++;

    return 0;
}

xjr_val xjr_val_as_array_pop(xjr_mp_t *mp, xjr_val v)
{
    xjr_val ret;
    xjr_val_array_item *item;
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);

    if (arr->size == 0) return XJR_VAL_MAKE_UNDEFINED();
    item = (xjr_val_array_item *)xjr_mp_get_ptr(mp, arr->end);
    ret = item->element;

    if (item->prev == XJR_URID_INVALID)
    {
        arr->begin = arr->end = XJR_URID_INVALID;
    }
    else
    {
        xjr_val_array_item *item_prev = (xjr_val_array_item *)xjr_mp_get_ptr(mp, item->prev);
        item_prev->next = XJR_URID_INVALID;
        arr->end = item->prev;
    }
    arr->size--;

    return ret;
}

xjr_val xjr_val_as_array_reverse(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);
    xjr_urid_t u_elem_cur;
    xjr_urid_t t;

    if (arr->begin == XJR_URID_INVALID) return v;

    u_elem_cur = arr->begin;
    while (u_elem_cur != XJR_URID_INVALID)
    {
        xjr_val_array_item *item_cur = xjr_mp_get_ptr(mp, u_elem_cur);;
        /* Swap */
        t = item_cur->next;
        item_cur->next = item_cur->prev;
        item_cur->prev = t;
        /* Move on */
        u_elem_cur = t;
    }

    /* Swap begin and end */
    t = arr->begin;
    arr->begin = arr->end;
    arr->end = t;

    return v;
}

/*
int xjr_val_as_array_sort(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);
    xjr_urid_t u1, u2;

    if (arr->begin == XJR_URID_INVALID) return 0;

    u1 = arr->begin;
    while (u1 != XJR_URID_INVALID)
    {
        xjr_val_array_item *item1 = xjr_mp_get_ptr(mp, u1);
        u2 = item1->next;
        while (u2 != XJR_URID_INVALID)
        {
            xjr_val_array_item *item2 = xjr_mp_get_ptr(mp, u2);
            if (XJR_VAL_IS_INTEGER(item1->element) && \
                    XJR_VAL_IS_INTEGER(item2->element) && \
                    (XJR_VAL_AS_INTEGER_UNTAG(item1->element) > XJR_VAL_AS_INTEGER_UNTAG(item2->element)))
            {
                xjr_val t = item1->element;
                item1->element = item2->element;
                item2->element = t;
            }
            u2 = item2->next;
        }
        u1 = item1->next;
    }

    return 0;
}
*/

static void *xjr_mbuf_malloc_by_mp(void *data, xjr_mbuf_size_t size)
{
    xjr_urid_t urid = xjr_mp_malloc((xjr_mp_t *)data, size);
    return xjr_mp_get_ptr(data, urid);
}

static void xjr_mbuf_free_by_mp(void *data, void *ptr)
{
    xjr_mp_free((xjr_mp_t *)data, xjr_mp_get_urid((xjr_mp_t *)data, ptr));
}

xjr_val xjr_val_as_array_join(xjr_mp_t *mp, xjr_val v, xjr_val separator)
{
    xjr_val ret;
    char *separator_body = xjr_nullptr;
    xjr_size_t separator_len = 0;
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);
    xjr_urid_t cur;
    xjr_size_t len = 0;
    xjr_bool first;
    xjr_mbuf_t mbuf;

    separator_len = xjr_val_as_string_length(mp, separator);
    separator_body = xjr_val_as_string_body(mp, separator);

    /* Get total length */
    first = xjr_true;
    cur = arr->begin;
    while (cur != XJR_URID_INVALID)
    {
        xjr_val_array_item *cur_item = xjr_mp_get_ptr(mp, cur);

        if (!XJR_VAL_IS_STRING(cur_item->element))
        { return XJR_VAL_MAKE_UNDEFINED(); }

        if (first == xjr_true) first = xjr_false; else len += separator_len;

        len += xjr_val_as_string_length(mp, cur_item->element);

        cur = cur_item->next;
    }

    /* Create a concated string */
    if (xjr_mbuf_init(&mbuf, mp, \
                xjr_mbuf_malloc_by_mp, \
                xjr_mbuf_free_by_mp, \
                xjr_memcpy) != 0)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    first = xjr_true;
    cur = arr->begin;
    while (cur != XJR_URID_INVALID)
    {
        xjr_size_t element_len;
        char *element_body;
        xjr_val_array_item *cur_item = xjr_mp_get_ptr(mp, cur);

        if (first == xjr_true) first = xjr_false;
        else
        {
            xjr_mbuf_append(&mbuf, separator_body, separator_len);
        }

        element_len = xjr_val_as_string_length(mp, cur_item->element);
        element_body = xjr_val_as_string_body(mp, cur_item->element);
        xjr_mbuf_append(&mbuf, element_body, element_len);

        cur = cur_item->next;
    }

    ret = xjr_val_make_string_from_heap(mp, xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));

    xjr_mbuf_uninit(&mbuf);

    return ret;
}

xjr_val xjr_val_as_array_get_by_idx(xjr_mp_t *mp, xjr_val v, int idx)
{
    xjr_urid_t u;
    xjr_val_array_item *item;
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);

    if (idx < 0 || idx >= (int)arr->size) return XJR_VAL_MAKE_UNDEFINED();

    u = arr->begin;
    item = xjr_mp_get_ptr(mp, u);
    while (idx != 0)
    {
        u = item->next;
        item = xjr_mp_get_ptr(mp, u);
        idx--;
    }

    return item->element;
}

xjr_val xjr_val_as_array_set_by_idx(xjr_mp_t *mp, xjr_val v, int idx, xjr_val new_elment)
{
    xjr_urid_t u;
    xjr_val_array_item *item;
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);

    if (idx < 0 || idx >= (int)arr->size) return XJR_VAL_MAKE_UNDEFINED();

    u = arr->begin;
    item = xjr_mp_get_ptr(mp, u);
    while (idx != 0)
    {
        u = item->next;
        item = xjr_mp_get_ptr(mp, u);
        idx--;
    }

    item->element = new_elment;
    return new_elment;
}

xjr_size_t xjr_val_as_array_size(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_array *arr = xjr_val_as_array_extract(mp, v);
    return arr->size;
}

static xjr_val xjr_val_make_normal_function_raw(xjr_mp_t *mp, \
        xjr_urid_t env_parent, xjr_offset_t offset, xjr_u32 opts)
{
    xjr_val v;
    xjr_val_function *f;
    {
        xjr_urid_t u;
        if ((u = MP_ALLOC(mp, xjr_val_function)) == XJR_URID_INVALID)
        { return XJR_VAL_MAKE_UNDEFINED(); }
        f = xjr_mp_get_ptr(mp, u);
        v = XJR_VAL_MAKE(XJR_TAG_FUNCTION, u);
    }
    f->type = xjr_val_function_type_normal;
    f->u.as_normal.offset = offset;
    f->opts = opts;
    xjr_val_properties_init(&f->props);
    if ((f->env = xjr_vm_env_new(mp, env_parent)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }
    return v;
}

xjr_val xjr_val_make_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_offset_t offset)
{
    return xjr_val_make_normal_function_raw(mp, env_parent, offset, 0);
}

xjr_val xjr_val_make_arrow_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_offset_t offset)
{
    return xjr_val_make_normal_function_raw(mp, env_parent, offset, XJR_VAL_FUNCTION_OPTS_ARROW);
}

xjr_val xjr_val_make_native_function(xjr_mp_t *mp, xjr_urid_t env_parent, xjr_native_fn_cb_t cb)
{
    xjr_val v;
    xjr_val_function *f;
    {
        xjr_urid_t u;
        if ((u = MP_ALLOC(mp, xjr_val_function)) == XJR_URID_INVALID)
        { return XJR_VAL_MAKE_UNDEFINED(); }
        f = xjr_mp_get_ptr(mp, u);
        v = XJR_VAL_MAKE(XJR_TAG_FUNCTION, u);
    }
    f->type = xjr_val_function_type_native;
    f->u.as_native.cb = cb;
    f->props.begin = f->props.end = XJR_URID_INVALID;
    if ((f->env = xjr_vm_env_new(mp, env_parent)) == XJR_URID_INVALID)
    { return XJR_VAL_MAKE_UNDEFINED(); }
    return v;
}

xjr_val_function *xjr_val_as_function_extract(xjr_mp_t *mp, xjr_val v)
{
    xjr_urid_t urid = (xjr_urid_t)(XJR_VAL_BODY(v));
    xjr_val_function *f = xjr_mp_get_ptr(mp, urid);
    return f;
}

xjr_val_properties *xjr_val_as_function_property_get(xjr_mp_t *mp, xjr_val v)
{
    return &xjr_val_as_function_extract(mp, v)->props;
}

xjr_bool xjr_val_as_function_e3(xjr_mp_t *mp, xjr_val v1, xjr_val v2)
{
    xjr_val_function *f1 = xjr_val_as_function_extract(mp, v1);
    xjr_val_function *f2 = xjr_val_as_function_extract(mp, v2);
    if (f1->type == xjr_val_function_type_native && f2->type == xjr_val_function_type_native)
    {
        if (f1->u.as_native.cb == f2->u.as_native.cb) return xjr_true;
    }
    else if (f1->type == xjr_val_function_type_normal && f2->type == xjr_val_function_type_normal)
    {
        if (f1->u.as_normal.offset == f2->u.as_normal.offset) return xjr_true;
    }
    return xjr_false;
}

xjr_bool xjr_val_as_function_e2(xjr_mp_t *mp, xjr_val v1, xjr_val v2)
{
    return xjr_val_as_function_e3(mp, v1, v2);
}

xjr_val xjr_val_make_string_from_datasec(xjr_xfile *f, xjr_mp_t *mp, xjr_offset_t offset)
{
    xjr_val v;
    char *body;
    xjr_size_t len;

    if (xjr_xfile_read_string(f, &body, &len, offset) != 0)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    {
        xjr_urid_t u;
        xjr_val_string *s;
        if ((u = MP_ALLOC(mp, xjr_val_string)) == XJR_URID_INVALID)
        { return XJR_VAL_MAKE_UNDEFINED(); }
        s = xjr_mp_get_ptr(mp, u);
        v = XJR_VAL_MAKE(XJR_TAG_STRING, u);
        s->type = xjr_val_string_type_datasec;
        s->u.as_datasec.body = body;
        s->u.as_datasec.len = len;
        xjr_val_properties_init(&s->props);
    }

    return v;
}

xjr_val xjr_val_make_string_from_heap(xjr_mp_t *mp, \
        const char *body, const xjr_size_t len)
{
    xjr_val v;
    {
        xjr_urid_t u;
        xjr_val_string *s;
        if ((u = MP_ALLOC(mp, xjr_val_string)) == XJR_URID_INVALID)
        { return XJR_VAL_MAKE_UNDEFINED(); }
        s = xjr_mp_get_ptr(mp, u);
        v = XJR_VAL_MAKE(XJR_TAG_STRING, u);
        s->type = xjr_val_string_type_heap;
        {
            char *p;
            if ((s->u.as_heap.u = MP_ALLOCN(mp, char, (len + 1))) == XJR_URID_INVALID)
            { return XJR_VAL_MAKE_UNDEFINED(); }
            p = xjr_mp_get_ptr(mp, s->u.as_heap.u);
            xjr_memcpy(p, body, len);
            p[len] = '\0';
        }
        s->u.as_heap.len = len;
        xjr_val_properties_init(&s->props);
    }
    return v;
}

#define XJR_VAL_MAKE_STRING_FROM_INT_BUF_SIZE 16
xjr_val xjr_val_make_string_from_int(xjr_mp_t *mp, const int v)
{
    xjr_val ret;
    xjr_mbuf_t mbuf;

    if (xjr_mbuf_init(&mbuf, mp, \
                xjr_mbuf_malloc_by_mp, \
                xjr_mbuf_free_by_mp, \
                xjr_memcpy) != 0)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    {
        char buf_integer[XJR_VAL_MAKE_STRING_FROM_INT_BUF_SIZE + 1], *buf_integer_p;
        int value_integer = (int)v;
        xjr_bool negative = xjr_false;

        if (value_integer < 0)
        {
            negative = xjr_true;
            value_integer = -value_integer;
        }

        buf_integer_p = buf_integer + XJR_VAL_MAKE_STRING_FROM_INT_BUF_SIZE;
        *buf_integer_p = '\0'; buf_integer_p--;

        if (value_integer == 0) { xjr_mbuf_append(&mbuf, "0", 1); }
        else
        {
            while (value_integer != 0)
            {
                *buf_integer_p-- = '0' + (value_integer % 10);
                value_integer /= 10;
            }
            if (negative == xjr_true)
            {
                *buf_integer_p-- = '-';
            }
            buf_integer_p++;
            xjr_mbuf_append(&mbuf, buf_integer_p, \
                    (xjr_size_t)(buf_integer + XJR_VAL_MAKE_STRING_FROM_INT_BUF_SIZE - buf_integer_p));
        }
    }

    ret = xjr_val_make_string_from_heap(mp, xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));

    xjr_mbuf_uninit(&mbuf);

    return ret;
}

xjr_val_string *xjr_val_as_string_extract(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_string *ret = xjr_nullptr;

    if (XJR_VAL_IS_STRING(v))
    {
        xjr_urid_t u = (xjr_urid_t)XJR_VAL_BODY(v);
        xjr_val_string *s = xjr_mp_get_ptr(mp, u);
        ret = s;
    }
    else
    { ret = xjr_nullptr; }

    return ret;
}

char *xjr_val_as_string_body(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_string *s = xjr_val_as_string_extract(mp, v);
    char *ret = xjr_nullptr;

    switch (s->type)
    {
        case xjr_val_string_type_datasec:
            ret = s->u.as_datasec.body;
            break;
        case xjr_val_string_type_heap:
            ret = xjr_mp_get_ptr(mp, s->u.as_heap.u);
            break;
    }
    
    return ret;
}

xjr_size_t xjr_val_as_string_length(xjr_mp_t *mp, xjr_val v)
{
    xjr_val_string *s = xjr_val_as_string_extract(mp, v);
    xjr_size_t ret = 0;

    switch (s->type)
    {
        case xjr_val_string_type_datasec:
            ret = s->u.as_datasec.len;
            break;
        case xjr_val_string_type_heap:
            ret = s->u.as_heap.len;
            break;
    }
    
    return ret;
}

xjr_bool xjr_val_as_string_e3(xjr_mp_t *mp, xjr_val v1, xjr_val v2)
{
    if (v1 == v2) { return xjr_true; }

    if (xjr_val_as_string_length(mp, v1) != xjr_val_as_string_length(mp, v2))
    { return xjr_false; }

    return (xjr_strncmp( \
                xjr_val_as_string_body(mp, v1), \
                xjr_val_as_string_body(mp, v2), \
                xjr_val_as_string_length(mp, v2)) == 0) ? xjr_true : xjr_false;
}

xjr_bool xjr_val_as_string_e2(xjr_mp_t *mp, xjr_val v1, xjr_val v2)
{
    return xjr_val_as_string_e3(mp, v1, v2);
}

int xjr_val_as_string_indexof(xjr_mp_t *mp, xjr_val v, xjr_val v_search_value, xjr_val v_from_index)
{
    char *body = xjr_val_as_string_body(mp, v);
    int len = (int)xjr_val_as_string_length(mp, v);
    char *search_value_body;
    int search_value_len;
    int from_idx = 0;

    if (XJR_VAL_IS_STRING(v_search_value))
    {
        search_value_body = xjr_val_as_string_body(mp, v_search_value);
        search_value_len = (int)xjr_val_as_string_length(mp, v_search_value);
    }
    else
    {
        return -1;
    }

    if (XJR_VAL_IS_INTEGER(v_from_index))
    {
        from_idx = XJR_VAL_AS_INTEGER_UNTAG(v_from_index);
        if (from_idx < 0)
        { from_idx = 0; }
        else if (from_idx >= len)
        { from_idx = len; }
    }
    else
    {
        from_idx = 0;
    }

    while (from_idx + search_value_len <= len)
    {
        xjr_bool matched = xjr_true;
        int i, j;
        i = from_idx; j = 0;

        while (j < search_value_len)
        {
            if (body[i] != search_value_body[j])
            { matched = xjr_false; break; }
            i++; j++;
        }

        if (matched == xjr_true)
        {
            return from_idx;
        }

        from_idx++;
    }

    return -1;
}

xjr_val_properties *xjr_val_as_string_property_get(xjr_mp_t *mp, xjr_val v)
{
    return &xjr_val_as_string_extract(mp, v)->props;
}

xjr_val xjr_val_as_string_get_by_idx(xjr_mp_t *mp, xjr_val v, int idx)
{
    char *body = xjr_val_as_string_body(mp, v);
    int len = (int)xjr_val_as_string_length(mp, v);
    if (idx < 0) return XJR_VAL_MAKE_UNDEFINED();
    if (idx >= len) return XJR_VAL_MAKE_UNDEFINED();
    return xjr_val_make_string_from_heap(mp, body + idx, 1);
}

void xjr_val_properties_install(xjr_vm *vm, xjr_val v)
{
    xjr_val_properties *props;
    switch (XJR_VAL_TAG(v))
    {
        case XJR_TAG_POSINT:
        case XJR_TAG_NEGINT:
        case XJR_TAG_RESERVED_VAL:
        case XJR_TAG_FLOAT:
            break;
        case XJR_TAG_STRING:
            props = xjr_val_as_string_property_get(vm->rts.rheap.mp, v); 
            xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, vm->fundamental.global_string_prototype);
            break;
        case XJR_TAG_FUNCTION:
            props = xjr_val_as_function_property_get(vm->rts.rheap.mp, v); 
            xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, vm->fundamental.global_function_prototype);
            break;
        case XJR_TAG_OBJECT:
            props = xjr_val_as_object_property_get(vm->rts.rheap.mp, v); 
            xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, vm->fundamental.global_object_prototype);
            break;
        case XJR_TAG_ARRAY:
            props = xjr_val_as_array_property_get(vm->rts.rheap.mp, v); 
            xjr_val_properties_set_bltin(props, XJR_VAL_PROPERTIES_BLTIN_PROTO, vm->fundamental.global_array_prototype);
            break;
    }
}

