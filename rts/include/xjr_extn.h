/* XenonJS : Runtime Time System : External things
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_EXTN_H
#define XJR_EXTN_H

#include "xjr_dt.h"

typedef int (*xjr_extn_cb_write)(const char *data, const xjr_size_t len);

typedef struct
{
    xjr_extn_cb_write cb_write;
} xjr_extn;

void xjr_extn_init(xjr_extn *extn);


#endif

