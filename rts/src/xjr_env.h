/* XenonJS : Runtime Time System : Environment
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_ENV_H
#define XJR_ENV_H

#include "xjr_dt.h"
#include "xjr_mp.h"


/* Variable */

typedef struct xjr_vm_var_opaque xjr_vm_var;
struct xjr_vm_var_opaque
{
    struct
    {
        char *body;
        xjr_size_t len;
    } name;
    xjr_val value;

    xjr_urid_t next;
};


/* Environment */

typedef struct xjr_vm_env_opaque xjr_vm_env;
struct xjr_vm_env_opaque
{
    struct
    {
        xjr_urid_t first, last;
    } vars;
    xjr_urid_t parent;
};

xjr_urid_t xjr_vm_env_new(xjr_mp_t *mp, xjr_urid_t parent);

xjr_urid_t xjr_vm_var_new(xjr_mp_t *mp, \
        char *name, xjr_size_t len, \
        xjr_val value);

xjr_vm_var *xjr_vm_env_get_var(xjr_mp_t *mp, \
        xjr_urid_t env, \
        const char *name, const xjr_size_t len);

int xjr_vm_env_set_var(xjr_mp_t *mp, \
        xjr_urid_t env, \
        char *name, xjr_size_t len, \
        xjr_val value);

#endif

