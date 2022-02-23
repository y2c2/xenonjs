/* XenonJS : Runtime Time System : VM : Inspect
 * Copyright(c) 2017-2019 y2c2 */

/* TODO: Circular detection
 * e.g.
 * var a = {"n": 1};
 * a.x = a;
 * console.log(a.x);
 */

#include "xjr_nativefn.h"
#include "xjr_val.h"
#include "xjr_vm_inspect.h"

#define FLOAT_INTEGER_PART_BUF_LEN 16

static void circular_det_add(circular_det *cdet, xjr_val v)
{
    if (cdet->used == CIRCULAR_DET_BUF_SIZE) return;
    cdet->items[cdet->used] = v;
    cdet->used++;
}

static xjr_bool circular_det_detect(circular_det *cdet, xjr_val v)
{
    xjr_size_t i;
    for (i = 0; i != cdet->used; i++)
    {
        if (cdet->items[i] == v) return xjr_true;
    }
    return xjr_false;
}

static int xjr_vm_inspect_int( \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        int value)
{
    int ret = 0;
    char buf_integer[FLOAT_INTEGER_PART_BUF_LEN + 1], *buf_integer_p;
    int value_integer = (int)value;
    xjr_bool negative = xjr_false;

    if (value_integer < 0)
    {
        negative = xjr_true;
        value_integer = -value_integer;
    }

    buf_integer_p = buf_integer + FLOAT_INTEGER_PART_BUF_LEN;
    *buf_integer_p = '\0'; buf_integer_p--;

    /* Integer part */
    if (value_integer == 0) { output_cb(data, "0", 1); }
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
        output_cb(data, buf_integer_p, (xjr_size_t)(buf_integer + FLOAT_INTEGER_PART_BUF_LEN - buf_integer_p));
    }

    return ret;
}

static int xjr_vm_inspect_float( \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        xjr_f64 value)
{
    int ret = 0;
    char buf_integer[FLOAT_INTEGER_PART_BUF_LEN + 1], *buf_integer_p;
    int value_integer = (int)value;
    xjr_bool negative = xjr_false;

    if (value_integer < 0)
    {
        negative = xjr_true;
        value_integer = -value_integer;
    }

    buf_integer_p = buf_integer + FLOAT_INTEGER_PART_BUF_LEN;
    *buf_integer_p = '\0'; buf_integer_p--;
    value -= (double)value_integer;

    /* Integer part */
    if (value_integer == 0) { output_cb(data, "0", 1); }
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
        output_cb(data, buf_integer_p, (xjr_size_t)(buf_integer + FLOAT_INTEGER_PART_BUF_LEN - buf_integer_p));
    }

    /* Fractal part */
    if (value != 0.0)
    {
        /* const double double_epsilon = 2.22045e-016; */
        /* const double double_epsilon = 0.000000000000001; /1* Magic? *1/ */
        const double double_epsilon = 0.0000000001; /* Magic? */
        int limit = 16; /* Magic? */
        output_cb(data, ".", 1);
        while ((value >= double_epsilon) && (limit-- >= 0))
        {
            int d;
            value *= 10;
            d = ((int)value % 10);
            static char *decimal_digits = "0123456789";
            output_cb(data, decimal_digits + d, 1);
            value = value - (double)d;
        }
    }

    return ret;
}

void xjr_vm_inspect_in( \
        xjr_mp_t *mp, \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        xjr_val v, xjr_bool in_obj, \
        xjr_vm_inspect_mode mode, \
        circular_det *cdet)
{
    if ((XJR_VAL_TAG(v) == XJR_TAG_OBJECT) || \
            (XJR_VAL_TAG(v) == XJR_TAG_ARRAY))
    {
        /* Circular detection */
        if (circular_det_detect(cdet, v) == xjr_true)
        {
            output_cb(data, "[Circular]", 10);
            return;
        }
        circular_det_add(cdet, v);
    }

    /* Type */
    switch (XJR_VAL_TAG(v))
    {
        case XJR_TAG_POSINT:
            xjr_vm_inspect_int( \
                    data, output_cb, XJR_VAL_BODY(v));
            break;
        case XJR_TAG_NEGINT:
            output_cb(data, "-", 1);
            xjr_vm_inspect_int( \
                    data, output_cb, XJR_VAL_BODY(v));
            break;
        case XJR_TAG_RESERVED_VAL:
            if (XJR_VAL_IS_BOOLEAN_FALSE(v))
            {
                output_cb(data, "false", 5);
            }
            else if (XJR_VAL_IS_BOOLEAN_TRUE(v))
            {
                output_cb(data, "true", 4);
            }
            else if (XJR_VAL_IS_UNDEFINED(v))
            {
                output_cb(data, "undefined", 9);
            }
            else if (XJR_VAL_IS_NULL(v))
            {
                output_cb(data, "null", 4);
            }
            else
            {
                output_cb(data, "unknown", 7);
            }
            break;
        case XJR_TAG_FLOAT:
            {
                xjr_val_float *f = xjr_mp_get_ptr(mp, ((xjr_urid_t)XJR_VAL_BODY(v)));
                switch (f->type)
                {
                    case xjr_val_float_type_f32:
                        xjr_vm_inspect_float( \
                                data, output_cb, (xjr_f64)f->u.as_f32);
                        break;
                    case xjr_val_float_type_f64:
                        xjr_vm_inspect_float( \
                                data, output_cb, f->u.as_f64);
                        break;
                }
            }
            break;
        case XJR_TAG_STRING:
            {
                if (mode == XJR_VM_INSPECT_MODE_JSON_STRINGIFY)
                {
                    output_cb(data, "\"", 1);
                    output_cb(data,  \
                            xjr_val_as_string_body(mp, v), 
                            xjr_val_as_string_length(mp, v));
                    output_cb(data, "\"", 1);
                }
                else
                {
                    if (in_obj) { output_cb(data, "'", 1); }
                    output_cb(data,  \
                            xjr_val_as_string_body(mp, v), 
                            xjr_val_as_string_length(mp, v));
                    if (in_obj) { output_cb(data, "'", 1); }
                }
            }
            break;
        case XJR_TAG_OBJECT:
            output_cb(data, "{", 1);
            {
                xjr_bool first = xjr_true;
                xjr_urid_t u_cur;
                xjr_val_object *p_obj = xjr_val_as_object_extract(mp, v);
                u_cur = p_obj->props.begin;
                while (u_cur != XJR_URID_INVALID)
                {
                    xjr_val_property *p = xjr_mp_get_ptr(mp, u_cur);
                    if (first == xjr_true) first = xjr_false; 
                    else output_cb(data, ",", 1);

                    switch (p->type)
                    {
                        case XJR_VAL_PROPERTY_TYPE_NORMAL:
                            xjr_vm_inspect_in(mp, data, output_cb, p->key, xjr_false, mode, cdet);
                            output_cb(data, ":", 1);
                            xjr_vm_inspect_in(mp, data, output_cb, p->u.value, xjr_true, mode, cdet);
                            break;
                        case XJR_VAL_PROPERTY_TYPE_GETTER:
                        case XJR_VAL_PROPERTY_TYPE_SETTER:
                            return;
                    }

                    u_cur = p->next;
                }
            }
            output_cb(data, "}", 1);
            break;
        case XJR_TAG_ARRAY:
            output_cb(data, "[", 1);
            {
                xjr_bool first = xjr_true;
                xjr_urid_t u_cur;
                xjr_val_array *p_obj = xjr_val_as_array_extract(mp, v);
                u_cur = p_obj->begin;
                while (u_cur != XJR_URID_INVALID)
                {
                    xjr_val_array_item *item = xjr_mp_get_ptr(mp, u_cur);
                    if (first == xjr_true) first = xjr_false;
                    else output_cb(data, ",", 1);

                    /* TODO: Circular detection */
                    xjr_vm_inspect_in(mp, data, output_cb, item->element, xjr_true, mode, cdet);

                    u_cur = item->next;
                }
            }
            output_cb(data, "]", 1);
            break;
    }
}

void xjr_vm_inspect( \
        xjr_mp_t *mp, \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        xjr_val v, \
        xjr_vm_inspect_mode mode)
{
    /* Place a record for circular detection */
    circular_det cdet;

    /* Clean */
    cdet.used = 0;

    xjr_vm_inspect_in(mp, data, output_cb, v, xjr_false, mode, &cdet);
}

