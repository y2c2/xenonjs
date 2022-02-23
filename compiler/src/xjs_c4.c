/* XenonJS : C4
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_list.h>
#include "xjs_types.h"
#include "xjs.h"
#include "xjs_ir.h"
#include "xjs_bitwriter.h"
#include "xjs_error.h"
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_ctx.h"
#include "xjs_c4_ant.h"
#include "xjs_c4_datasec.h"
#include "xjs_c4_textsec.h"
#include "xjs_c4_attrsec.h"
#include "xjs_c4_exportsec.h"
#include "xjs_c4_importsec.h"
#include "xjs_c4_dbgsec.h"
#include "xjs_c4_indexsec.h"
#include "xjs_c4.h"

static int xjs_c4_write_header( \
        xjs_c4_ctx *ctx)
{
    /* Signature */
    xjs_bitwriter_write_u32(ctx->bw_header, XJS_BC_SIGNATURE);
    /* Platform (Not specified) */
    xjs_bitwriter_write_u16(ctx->bw_header, XJS_BC_PLATFORM); 
    /* SDK Version (Not specified) */
    xjs_bitwriter_write_u16(ctx->bw_header, XJS_BC_SDKVER);

    return 0;
}

/* C4 (IR -> bytecode) */
int xjs_c4_start_ex( \
        xjs_error_ref err, \
        char **bytecode_out, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info)
{
    int ret = 0;
    xjs_c4_ctx ctx;
    xjs_bitwriter bw_header, bw_data, bw_text, bw_attr, bw_export, bw_import;
    xjs_bitwriter bw_dbg0, bw_dbi0;
    xjs_bitwriter bw_index;
    annotated_function_list_ref annotated_functions = NULL;

    bw_header.body = NULL;
    bw_index.body = NULL;
    bw_data.body = NULL;
    bw_text.body = NULL;
    bw_attr.body = NULL;
    bw_dbg0.body = NULL;
    bw_dbi0.body = NULL;

    ctx.err = err;
    ctx.ir = ir;
    ctx.offset_toplevel = 0;
    ctx.map_ir_dataid_to_offset = NULL;
    ctx.map_ir_function_to_offset = NULL;
    ctx.map_ir_functionid_to_offset = NULL;
    ctx.generate_debug_info = generate_debug_info;

    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_header), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_index), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_attr), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_data), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_text), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_export), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_import), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_dbg0), err);
    XJS_VEZ_ERROR_MEM(xjs_bitwriter_init(&bw_dbi0), err);

    ctx.bw_header = &bw_header;
    ctx.bw_index = &bw_index;
    ctx.bw_data = &bw_data;
    ctx.bw_text = &bw_text;
    ctx.bw_attr = &bw_attr;
    ctx.bw_export = &bw_export;
    ctx.bw_import = &bw_import;
    ctx.bw_dbg0 = &bw_dbg0;
    ctx.bw_dbi0 = &bw_dbi0;

    /* Generate header */
    if (xjs_c4_write_header(&ctx) != 0) { goto fail; }

    /* Generate data section */
    if (xjs_c4_write_data(&ctx) != 0) { goto fail; }

    /* Annotate */
    if ((annotated_functions = xjs_c4_text_section_annotate(&ctx)) == NULL)
    { goto fail; }
    ctx.annotated_functions = annotated_functions;

    /* Generate text section */
    if (xjs_c4_write_text(&ctx) != 0) { goto fail; }

    /* Generate attr section */
    if (xjs_c4_write_attr(&ctx) != 0) { goto fail; }

    /* Generate export section */
    if (xjs_c4_write_export(&ctx) != 0) { goto fail; }

    /* Generate import section */
    if (xjs_c4_write_import(&ctx) != 0) { goto fail; }

    /* Generate dbg0 and dbi0 section */
    if (generate_debug_info == xjs_true)
    {
        if (xjs_c4_write_dbg0(&ctx) != 0) { goto fail; }
    }

    /* Generate index section */
    if (xjs_c4_write_index(&ctx) != 0) { goto fail; }

    {
        /* Mergw all things wrote together */
        char *bytecode = NULL, *p;
        xjs_size_t len = 0;

        len += xjs_bitwriter_size(&bw_header);
        len += xjs_bitwriter_size(&bw_index);
        len += xjs_bitwriter_size(&bw_data);
        len += xjs_bitwriter_size(&bw_text);
        len += xjs_bitwriter_size(&bw_attr);
        len += xjs_bitwriter_size(&bw_export);
        len += xjs_bitwriter_size(&bw_import);
        len += xjs_bitwriter_size(&bw_dbg0);
        len += xjs_bitwriter_size(&bw_dbi0);

        XJS_VNZ_ERROR_MEM(bytecode = (char *)ec_malloc(sizeof(char) * len), err);
        p = bytecode;

        ec_memcpy(p, xjs_bitwriter_body(&bw_header), xjs_bitwriter_size(&bw_header));
        p += xjs_bitwriter_size(&bw_header);

        ec_memcpy(p, xjs_bitwriter_body(&bw_index), xjs_bitwriter_size(&bw_index));
        p += xjs_bitwriter_size(&bw_index);

        ec_memcpy(p, xjs_bitwriter_body(&bw_data), xjs_bitwriter_size(&bw_data));
        p += xjs_bitwriter_size(&bw_data);

        ec_memcpy(p, xjs_bitwriter_body(&bw_text), xjs_bitwriter_size(&bw_text));
        p += xjs_bitwriter_size(&bw_text);

        ec_memcpy(p, xjs_bitwriter_body(&bw_attr), xjs_bitwriter_size(&bw_attr));
        p += xjs_bitwriter_size(&bw_attr);

        ec_memcpy(p, xjs_bitwriter_body(&bw_export), xjs_bitwriter_size(&bw_export));
        p += xjs_bitwriter_size(&bw_export);

        ec_memcpy(p, xjs_bitwriter_body(&bw_import), xjs_bitwriter_size(&bw_import));
        p += xjs_bitwriter_size(&bw_import);

        ec_memcpy(p, xjs_bitwriter_body(&bw_dbg0), xjs_bitwriter_size(&bw_dbg0));
        p += xjs_bitwriter_size(&bw_dbg0);

        ec_memcpy(p, xjs_bitwriter_body(&bw_dbi0), xjs_bitwriter_size(&bw_dbi0));
        p += xjs_bitwriter_size(&bw_dbi0);

        (void)p;

        *bytecode_out = bytecode;
        *bytecode_len_out = len;
    }

    goto done;
fail:
    ret = -1;
done:
    xjs_bitwriter_uninit(&bw_header);
    xjs_bitwriter_uninit(&bw_index);
    xjs_bitwriter_uninit(&bw_data);
    xjs_bitwriter_uninit(&bw_text);
    xjs_bitwriter_uninit(&bw_attr);
    xjs_bitwriter_uninit(&bw_export);
    xjs_bitwriter_uninit(&bw_import);
    xjs_bitwriter_uninit(&bw_dbg0);
    xjs_bitwriter_uninit(&bw_dbi0);
    ec_delete(ctx.map_ir_dataid_to_offset);
    ec_delete(ctx.map_ir_function_to_offset);
    ec_delete(ctx.map_ir_functionid_to_offset);
    ec_delete(annotated_functions);

    return ret;
}

int xjs_c4_start( \
        xjs_error_ref err, \
        char **bytecode_out, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir)
{
    return xjs_c4_start_ex(err, bytecode_out, bytecode_len_out, ir, xjs_false);
}

