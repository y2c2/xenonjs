/* XenonJS : C4 : Text Section : Instruction Translate
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C4_TEXTSEC_INST_H
#define XJS_C4_TEXTSEC_INST_H

#include "xjs_types.h"
#include "xjs_c4_ctx.h"

typedef int (*xjs_c4_annotate_handler)(xjs_c4_func_ctx *func_ctx, xjs_ir_text_item_ref text_item);

xjs_c4_annotate_handler xjs_c4_text_section_annotate_get_handler(xjs_ir_text_item_ref text_item);

/* Measure the size of each IR text item */
int xjs_c4_text_item_instrument_size( \
        xjs_c4_ctx *ctx, xjs_ir_text_item_ref item);

#endif

