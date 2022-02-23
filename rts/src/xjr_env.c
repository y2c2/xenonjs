/* XenonJS : Runtime Time System : Environment
 * Copyright(c) 2017 y2c2 */

#include "xjr_libc.h"
#include "xjr_env.h"

#define MP_ALLOC(_mp, _t) \
    (xjr_mp_malloc_managed(_mp, sizeof(_t)))

xjr_urid_t xjr_vm_var_new(xjr_mp_t *mp, \
        char *name, xjr_size_t len, \
        xjr_val value)
{
    xjr_urid_t u;
    if ((u = MP_ALLOC(mp, xjr_vm_var)) == XJR_URID_INVALID)
    { return XJR_URID_INVALID; }
    {
        xjr_vm_var *var = xjr_mp_get_ptr(mp, u);
        var->name.body = name;
        var->name.len = len;
        var->value = value;
        var->next = XJR_URID_INVALID;
    }
    return u;
}

xjr_vm_var *xjr_vm_env_get_var(xjr_mp_t *mp, \
        xjr_urid_t env, \
        const char *name, const xjr_size_t len)
{
    xjr_vm_env *e = xjr_mp_get_ptr(mp, env);
    xjr_urid_t var;
    xjr_vm_var *v;

    var = e->vars.first;
    while (var != XJR_URID_INVALID)
    {
        v = xjr_mp_get_ptr(mp, var);
        if ((v->name.len == len) && \
                (xjr_strncmp(v->name.body, name, len) == 0))
        { return v; }

        var = v->next;
    }

    return xjr_nullptr;
}

int xjr_vm_env_set_var(xjr_mp_t *mp, \
        xjr_urid_t env, \
        char *name, xjr_size_t len, \
        xjr_val value)
{
    xjr_vm_var *var = xjr_vm_env_get_var(mp, env, name, len);
    if (var != xjr_nullptr) { var->value = value; return 0; }

    {
        xjr_vm_env *e = xjr_mp_get_ptr(mp, env);
        if (e->vars.last == XJR_URID_INVALID)
        {
            e->vars.first = e->vars.last = xjr_vm_var_new(mp, name, len, value);
        }
        else
        {
            xjr_vm_var *last_var = xjr_mp_get_ptr(mp, e->vars.last);
            xjr_urid_t new_var = xjr_vm_var_new(mp, name, len, value);
            if (new_var == XJR_URID_INVALID) { return -1; }
            last_var->next = new_var;
            e->vars.last = new_var;
        }
    }

    return 0;
}


xjr_urid_t xjr_vm_env_new(xjr_mp_t *mp, xjr_urid_t parent)
{
    xjr_urid_t u_env;
    xjr_vm_env *env;
    if ((u_env = MP_ALLOC(mp, xjr_vm_env)) == XJR_URID_INVALID)
    { return XJR_URID_INVALID; }
    env = xjr_mp_get_ptr(mp, u_env);
    env->parent = parent;
    env->vars.first = XJR_URID_INVALID;
    env->vars.last = XJR_URID_INVALID;
    return u_env;
}

