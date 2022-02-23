/* XenonJS : C4 : Index Section
 * Copyright(c) 2017 y2c2 */

#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_indexsec.h"

int xjs_c4_write_index(xjs_c4_ctx *ctx)
{
    ec_size_t offset = 0, size;
    ec_size_t sec_count = 0;

    /* Section Count */
    if (xjs_bitwriter_size(ctx->bw_data) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_text) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_attr) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_export) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_import) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_dbg0) != 0) sec_count++;
    if (xjs_bitwriter_size(ctx->bw_dbi0) != 0) sec_count++;

    /* Section Count (data + text + attr + export + import) */
    xjs_bitwriter_write_u32(ctx->bw_index, (xjs_u32)sec_count);

    /* Offset of data section */
#define WRITE_SECTION(_sec_name) \
    if (xjs_bitwriter_size(ctx->_sec_name) != 0) \
    { \
        size = xjs_bitwriter_size(ctx->_sec_name); \
        xjs_bitwriter_write_u32(ctx->bw_index, (xjs_u32)offset); \
        xjs_bitwriter_write_u32(ctx->bw_index, (xjs_u32)size); \
        offset += size; \
        (void)offset; \
    }

    WRITE_SECTION(bw_data);
    WRITE_SECTION(bw_text);
    WRITE_SECTION(bw_attr);
    WRITE_SECTION(bw_export);
    WRITE_SECTION(bw_import);
    WRITE_SECTION(bw_dbg0);
    WRITE_SECTION(bw_dbi0);

    (void)offset;

    return 0;
}

