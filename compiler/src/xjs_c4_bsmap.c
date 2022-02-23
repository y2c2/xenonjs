/* XenonJS : C4 : Bytecode Source Mapping
 * Copyright(c) 2017 y2c2 */

#include <ec_alloc.h>
#include "xjs_c4_bsmap.h"

static void xjs_c4_bs_item_ctor(void *data)
{
    xjs_c4_bs_item_ref r = data;
    r->offset = 0;
    r->start.ln = -1;
    r->start.col = -1;
    r->end.ln = -1;
    r->end.col = -1;
}

xjs_c4_bs_item_ref xjs_c4_bs_item_new(void)
{
    xjs_c4_bs_item_ref r = ec_newcd(xjs_c4_bs_item, \
            xjs_c4_bs_item_ctor, NULL);
    return r;
}

static void xjs_ast_parameterlist_node_dtor(xjs_c4_bs_item_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_c4_bs_item_list, xjs_c4_bs_item_ref, xjs_ast_parameterlist_node_dtor);
