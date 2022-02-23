/* XenonJS : Runtime Time System : VM : Inspect
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_VM_INSPECT_H
#define XJR_VM_INSPECT_H

#include "xjr_dt.h"
#include "xjr_mp.h"

typedef int (*xjr_vm_inspect_output_callback)( \
        void *data, char *s, xjr_size_t len);

typedef enum
{
    XJR_VM_INSPECT_MODE_JSON_PRINT,
    XJR_VM_INSPECT_MODE_JSON_STRINGIFY,
} xjr_vm_inspect_mode;

#define CIRCULAR_DET_BUF_SIZE 64

typedef struct
{
    xjr_size_t used;
    xjr_val items[CIRCULAR_DET_BUF_SIZE];
} circular_det;

void xjr_vm_inspect_in( \
        xjr_mp_t *mp, \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        xjr_val v, xjr_bool in_obj, \
        xjr_vm_inspect_mode mode, \
        circular_det *cdet);

void xjr_vm_inspect( \
        xjr_mp_t *mp, \
        void *data, xjr_vm_inspect_output_callback output_cb, \
        xjr_val v, \
        xjr_vm_inspect_mode mode);

#endif

