/* XenonJS : C4 : Bytecode Source Mapping
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_BSMAP_H
#define XJS_C4_BSMAP_H

#include <ec_list.h>
#include "xjs_dt.h"

typedef struct
{
    xjs_offset_t offset;
    struct
    {
        int ln, col;
    } start, end;
} xjs_c4_bs_item;
typedef xjs_c4_bs_item *xjs_c4_bs_item_ref;

xjs_c4_bs_item_ref xjs_c4_bs_item_new(void);

ect_list_declare(xjs_c4_bs_item_list, xjs_c4_bs_item_ref);
typedef xjs_c4_bs_item_list *xjs_c4_bs_item_list_ref;

#endif

