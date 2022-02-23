/* XenonJS : Runtime Time System : External things
 * Copyright(c) 2017 y2c2 */

#include "xjr_dt.h"
#include "xjr_extn.h"

void xjr_extn_init(xjr_extn *extn)
{
    extn->cb_write = xjr_nullptr;
}

