/* XenonJS : C4 : Import Section
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_c4_importsec.h"

int xjs_c4_write_import(xjs_c4_ctx *ctx)
{
    /* Section Signature */
    xjs_bitwriter_append(ctx->bw_import, \
            XJS_BC_SECTION_SIGNATURE_IMPT, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Item Count */
    xjs_bitwriter_write_u32(ctx->bw_import, (xjs_u32)ect_list_size(xjs_ir_import_list, ctx->ir->imported));
    {
        ect_iterator(xjs_ir_import_list) it;
        ect_for(xjs_ir_import_list, ctx->ir->imported, it)
        {
            xjs_ir_import_item_ref import_symbol = ect_deref(xjs_ir_import_item_ref, it);
            xjs_bitwriter_write_u32(ctx->bw_import, import_symbol->local);
            xjs_bitwriter_write_u32(ctx->bw_import, import_symbol->imported);
            xjs_bitwriter_write_u32(ctx->bw_import, import_symbol->source);
        }
    }

    return 0;
}

