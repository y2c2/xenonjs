/* XenonJS : Runtime Time System : GC
 * Copyright(c) 2017 y2c2 */

#include <stdio.h>
#include "xjr_dt.h"
#include "xjr_mp.h"
#include "xjr_val.h"
#include "xjr_env.h"
#include "xjr_vm_stkmap.h"
#include "xjr_gc.h"

static xjr_bool xjr_gc_mark_urid(xjr_vm *vm, xjr_urid_t u);
static xjr_bool xjr_gc_mark_val(xjr_vm *vm, xjr_val v);
static xjr_bool xjr_gc_mark_env(xjr_vm *vm, xjr_urid_t u);

#define MARK_URID(u) do { xjr_gc_mark_urid(vm, u); } while (0)
#define MARK_VAL(v) do { xjr_gc_mark_val(vm, v); } while (0)
#define MARK_REG(r) MARK_VAL(XJR_VM_GET_VREGV(vm, r))

static void xjr_gc_clean(xjr_vm *vm)
{
    xjr_mp_clear(vm->rts.rheap.mp);
}

static xjr_bool xjr_gc_mark_urid(xjr_vm *vm, xjr_urid_t u)
{
    return xjr_mp_mark(vm->rts.rheap.mp, u);
}

static xjr_bool xjr_gc_mark_properties(xjr_vm *vm, xjr_val_properties *props)
{
    xjr_urid_t cur = props->begin;
    xjr_val_property *p;
    while (cur != XJR_URID_INVALID)
    {
        MARK_URID(cur);
        p = xjr_mp_get_ptr(vm->rts.rheap.mp, cur);

        xjr_gc_mark_val(vm, p->key);
        switch (p->type)
        {
            case XJR_VAL_PROPERTY_TYPE_NORMAL:
                xjr_gc_mark_val(vm, p->u.value);
                break;
            case XJR_VAL_PROPERTY_TYPE_GETTER:
                xjr_gc_mark_val(vm, p->u.getter);
                break;
            case XJR_VAL_PROPERTY_TYPE_SETTER:
                xjr_gc_mark_val(vm, p->u.setter);
                break;
        }
        cur = p->next;
    }
    {
        xjr_size_t i;
        for (i = 0; i != XJR_VAL_PROPERTIES_BLTIN_COUNT; i++)
        {
            xjr_gc_mark_val(vm, props->bltin[i]);
        }
    }
    return xjr_false;
}

static xjr_bool xjr_gc_mark_val(xjr_vm *vm, xjr_val v)
{
    switch (XJR_VAL_TAG(v))
    {
        case XJR_TAG_POSINT:
        case XJR_TAG_NEGINT:
        case XJR_TAG_RESERVED_VAL:
            break;
        case XJR_TAG_FLOAT:
            if (xjr_gc_mark_urid(vm, (xjr_urid_t)XJR_VAL_BODY(v)) == xjr_true)
            { return xjr_true; }
            break;
        case XJR_TAG_STRING:
            if (xjr_gc_mark_urid(vm, (xjr_urid_t)XJR_VAL_BODY(v)) == xjr_true)
            { return xjr_true; }
            {
                xjr_val_string *s = xjr_mp_get_ptr(vm->rts.rheap.mp, (xjr_urid_t)XJR_VAL_BODY(v));
                if (s->type == xjr_val_string_type_heap)
                { xjr_gc_mark_urid(vm, s->u.as_heap.u); }
            }
            xjr_gc_mark_properties(vm, xjr_val_as_string_property_get(vm->rts.rheap.mp, v));
            break;
        case XJR_TAG_ARRAY:
            if (xjr_gc_mark_urid(vm, (xjr_urid_t)XJR_VAL_BODY(v)) == xjr_true)
            { return xjr_true; }
            xjr_gc_mark_properties(vm, xjr_val_as_array_property_get(vm->rts.rheap.mp, v));
            {
                xjr_urid_t u_cur;
                xjr_val_array *p_obj = xjr_val_as_array_extract(vm->rts.rheap.mp, v);
                u_cur = p_obj->begin;
                while (u_cur != XJR_URID_INVALID)
                {
                    xjr_val_array_item *u_item = xjr_mp_get_ptr(vm->rts.rheap.mp, u_cur);
                    xjr_gc_mark_urid(vm, u_cur);
                    xjr_gc_mark_val(vm, u_item->element);
                    u_cur = u_item->next;
                }
            }
            break;
        case XJR_TAG_OBJECT:
            if (xjr_gc_mark_urid(vm, (xjr_urid_t)XJR_VAL_BODY(v)) == xjr_true)
            { return xjr_true; }
            xjr_gc_mark_properties(vm, xjr_val_as_object_property_get(vm->rts.rheap.mp, v));
            {
                xjr_urid_t u_cur;
                xjr_val_object *p_obj = xjr_val_as_object_extract(vm->rts.rheap.mp, v);
                u_cur = p_obj->props.begin;
                while (u_cur != XJR_URID_INVALID)
                {
                    xjr_val_property *u_item = xjr_mp_get_ptr(vm->rts.rheap.mp, u_cur);
                    xjr_gc_mark_urid(vm, u_cur);
                    xjr_gc_mark_val(vm, u_item->key);
                    switch (u_item->type)
                    {
                        case XJR_VAL_PROPERTY_TYPE_NORMAL:
                            xjr_gc_mark_val(vm, u_item->u.value);
                            break;
                        case XJR_VAL_PROPERTY_TYPE_GETTER:
                            xjr_gc_mark_val(vm, u_item->u.getter);
                            break;
                        case XJR_VAL_PROPERTY_TYPE_SETTER:
                            xjr_gc_mark_val(vm, u_item->u.setter);
                            break;
                    }
                    u_cur = u_item->next;
                }
            }
            break;
        case XJR_TAG_FUNCTION:
            if (xjr_gc_mark_urid(vm, (xjr_urid_t)XJR_VAL_BODY(v)) == xjr_true)
            { return xjr_true; }
            xjr_gc_mark_properties(vm, xjr_val_as_function_property_get(vm->rts.rheap.mp, v));
            {
                xjr_val_function *f = xjr_val_as_function_extract(vm->rts.rheap.mp, v);
                xjr_gc_mark_env(vm, f->env);
            }
            break;
    }
    return xjr_false;
}

static xjr_bool xjr_gc_mark_env(xjr_vm *vm, xjr_urid_t u)
{
    for (;;)
    {
        xjr_vm_env *e; 
        xjr_urid_t var;

        if (u == XJR_URID_INVALID) break;
        if (xjr_gc_mark_urid(vm, u) == xjr_true) return xjr_true;
        e = xjr_mp_get_ptr(vm->rts.rheap.mp, u);
        var = e->vars.first;
        while (var != XJR_URID_INVALID)
        {
            xjr_vm_var *v = xjr_mp_get_ptr(vm->rts.rheap.mp, var);
            MARK_URID(var);
            MARK_VAL(v->value);

            /* Next variable */
            var = v->next;
        }

        u = e->parent;
    }

    return xjr_false;
}

static void xjr_gc_mark(xjr_vm *vm)
{
    xjr_u8 *stk = vm->rts.rstack.data;
    xjr_u32 sp = vm->rts.rstack.sp;
    xjr_offset_t sdp_mirror = vm->rts.rstack.sdp;

    for (;;)
    {
        xjr_gc_mark_env(vm, XJR_VM_GET_ENV(vm));
        MARK_VAL(XJR_VM_GET_CALLEE(vm));
        MARK_VAL(XJR_VM_GET_THIS(vm));
        {
            xjr_size_t argc = XJR_VM_GET_ARGC(vm), i;
            for (i = 0; i != argc; i++)
            { MARK_VAL(XJR_VM_GET_ARGV(vm, i)); }
        }
        {
            xjr_size_t vregc = XJR_VM_GET_VREGC(vm), i;
            for (i = 0; i != vregc; i++)
            {
                MARK_VAL(XJR_VM_GET_VREGV(vm, i));
            }
        }

        if (sp == vm->rts.rstack.size) break;
        vm->rts.rstack.sdp = *((xjr_offset_t *)(stk + sp));
        sp += sizeof(xjr_offset_t);
    }

    vm->rts.rstack.sdp = sdp_mirror;

    /* Mark external values */
    {
        xjr_size_t i;

        for (i = 0; i != vm->misc.external_vals.size; i++)
        {
            MARK_VAL(vm->misc.external_vals.body[i]);
        }
    }
}

static void xjr_gc_sweep(xjr_vm *vm)
{
    xjr_mp_sweep(vm->rts.rheap.mp);
}

int xjr_gc(xjr_vm *vm)
{
    xjr_gc_clean(vm);
    xjr_gc_mark(vm);
    xjr_gc_sweep(vm);
    return 0;
}

