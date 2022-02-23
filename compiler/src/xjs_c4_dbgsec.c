/* XenonJS : C4 : Debug Section
 * Copyright(c) 2017 y2c2 */

#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include <ec_algorithm.h>
#include <ec_map.h>
#include <ec_list.h>
#include "xjs_helper.h"
#include "xjs_xformat.h"
#include "xjs_c4_helper.h"
#include "xjs_c4_regalloc.h"
#include "xjs_c4_ctx.h"
#include "xjs_c4_textsec_inst.h"
#include "xjs_c4_dbgsec.h"

#define WRITE_S8(_to, _x) \
    do { xjs_bitwriter_write_s8(_to, (xjs_s8)(_x)); } while (0)
#define WRITE_S16(_to, _x) \
    do { xjs_bitwriter_write_s16(_to, (xjs_s16)(_x)); } while (0)
#define WRITE_S32(_to, _x) \
    do { xjs_bitwriter_write_s32(_to, (xjs_s32)(_x)); } while (0)
#define WRITE_U8(_to, _x) \
    do { xjs_bitwriter_write_u8(_to, (xjs_u8)(_x)); } while (0)
#define WRITE_U16(_to, _x) \
    do { xjs_bitwriter_write_u16(_to, (xjs_u16)(_x)); } while (0)
#define WRITE_U32(_to, _x) \
    do { xjs_bitwriter_write_u32(_to, (xjs_u32)(_x)); } while (0)
#define WRITE_F64(_to, _x) \
    do { xjs_bitwriter_write_f64(_to, (xjs_f64)(_x)); } while (0)
#define WRITE_STR(_to, _s) \
    do { \
        ec_encoding_t enc; \
        ec_byte_t *encoded_bytes = NULL; \
        ec_size_t encoded_bytes_len; \
        ec_encoding_utf8_init(&enc); \
        ec_encoding_encode(&enc, &encoded_bytes, &encoded_bytes_len, _s); \
        xjs_bitwriter_append(_to, (const char *)encoded_bytes, encoded_bytes_len); \
        WRITE_S8(_to, 0); \
        ec_free(encoded_bytes); \
    } while (0)

int xjs_c4_write_dbg0(xjs_c4_ctx *ctx)
{
    xjs_size_t dbg0_offset = 0;

    /* Section signature */
    xjs_bitwriter_append(ctx->bw_dbg0, \
            XJS_BC_SECTION_SIGNATURE_DBG0, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Section signature */
    xjs_bitwriter_append(ctx->bw_dbi0, \
            XJS_BC_SECTION_SIGNATURE_DBI0, XJS_BC_SECTION_SIGNATURE_LEN);

    /* Functions */
    {
        ect_iterator(annotated_function_list) it_func;
        ect_for(annotated_function_list, ctx->annotated_functions, it_func)
        {
            annotated_function_ref func = ect_deref(annotated_function_ref, it_func);
            ect_iterator(annotated_text_section_item_list) it_ti;
            ect_for(annotated_text_section_item_list, func->items, it_ti)
            {
                annotated_text_section_item_ref ti = ect_deref(annotated_text_section_item_ref, it_ti);

                /* PC offset */
                WRITE_U32(ctx->bw_dbi0, ti->offset);
                /* dbg0 session offset */
                WRITE_U32(ctx->bw_dbi0, dbg0_offset);

                if ((ti->filename == NULL) || \
                        (ec_string_length(ti->filename) == 0))
                {
                    WRITE_U32(ctx->bw_dbg0, 0);
                    WRITE_S8(ctx->bw_dbg0, 0);
                    dbg0_offset += 5;
                }
                else
                {
                    ec_encoding_t enc;
                    ec_byte_t *encoded_bytes = NULL;
                    ec_size_t encoded_bytes_len;
                    ec_encoding_utf8_init(&enc);
                    ec_encoding_encode(&enc, &encoded_bytes, &encoded_bytes_len, ti->filename);
                    WRITE_U32(ctx->bw_dbg0, (int)(encoded_bytes_len));
                    xjs_bitwriter_append(ctx->bw_dbg0, (const char *)encoded_bytes, encoded_bytes_len);
                    WRITE_S8(ctx->bw_dbg0, 0);
                    ec_free(encoded_bytes);
                    dbg0_offset += 4 + encoded_bytes_len + 1;
                }
                WRITE_S32(ctx->bw_dbg0, ti->loc.start.ln);
                WRITE_S32(ctx->bw_dbg0, ti->loc.start.col);
                WRITE_S32(ctx->bw_dbg0, ti->loc.end.ln);
                WRITE_S32(ctx->bw_dbg0, ti->loc.end.col);
                WRITE_S32(ctx->bw_dbg0, ti->range.start);
                WRITE_S32(ctx->bw_dbg0, ti->range.end);
                dbg0_offset += 6 * 4;
            }
        }
    }

    return 0;
}

