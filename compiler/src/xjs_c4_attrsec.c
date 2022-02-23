/* XenonJS : C4 : Attribute Section
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding_utf8.h>
#include <ec_encoding.h>
#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_attrsec.h"

int xjs_c4_write_attr(xjs_c4_ctx *ctx)
{
    int ret = 0;

    /* Section Signature */
    xjs_bitwriter_append(ctx->bw_attr, \
            XJS_BC_SECTION_SIGNATURE_ATTR, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Item Count */
    xjs_bitwriter_write_u32(ctx->bw_attr, 1);
    /* Index */
    xjs_bitwriter_write_u32(ctx->bw_attr, 0);
    /* Top Level */
    xjs_bitwriter_write_u16(ctx->bw_attr, XJS_BC_ATTRSEC_ITEM_TYPE_TOPLEVEL);
    xjs_bitwriter_write_u32(ctx->bw_attr, (xjs_u32)ctx->offset_toplevel);

    return ret;
}

