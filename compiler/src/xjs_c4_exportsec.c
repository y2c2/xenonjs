/* XenonJS : C4 : Export Section
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_c4_exportsec.h"

int xjs_c4_write_export(xjs_c4_ctx *ctx)
{
    /* Section Signature */
    xjs_bitwriter_append(ctx->bw_export, \
            XJS_BC_SECTION_SIGNATURE_EXPT, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Item Count */
    xjs_bitwriter_write_u32(ctx->bw_export, (xjs_u32)ect_list_size(xjs_ir_export_list, ctx->ir->exported));
    {
        ect_iterator(xjs_ir_export_list) it;
        ect_for(xjs_ir_export_list, ctx->ir->exported, it)
        {
            xjs_ir_export_item_ref export_symbol = ect_deref(xjs_ir_export_item_ref, it);
            xjs_bitwriter_write_u32(ctx->bw_export, export_symbol->exported);
            xjs_bitwriter_write_u32(ctx->bw_export, export_symbol->local);
        }
    }

    return 0;
}

