/* XenonJS : Runtime Time System : X File
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_X_H
#define XJR_X_H

#include "xjr_dt.h"

typedef struct
{
    /* Bytecode Data */
    char *bc;
    xjr_size_t len;

    /* Sections */
    struct
    {
        xjr_offset_t text;
        xjr_offset_t data;
        xjr_offset_t attr;
        xjr_offset_t expr;
        xjr_offset_t impr;
        xjr_offset_t dbg0;
        xjr_offset_t dbi0;
    } sections;

    xjr_offset_t init_pc;
} xjr_xfile;

/* Initialize xfile */
void xjr_xfile_init(xjr_xfile *f);

/* Load bytecode */
int xjr_xfile_load(xjr_xfile *f, char *bc, xjr_size_t len);

/* Basic interface for reading data from X
 * @param[in]   f         X File
 * @param[out]  v_out     Output value
 * @param[in]   offset    Offset
 * @return      0         Success
 * @return      -1        Fail */
int xjr_xfile_read_u32(xjr_xfile *f, xjr_u32 *v_out, xjr_offset_t offset);
int xjr_xfile_read_s32(xjr_xfile *f, xjr_s32 *v_out, xjr_offset_t offset);
int xjr_xfile_read_u16(xjr_xfile *f, xjr_u16 *v_out, xjr_offset_t offset);
int xjr_xfile_read_s16(xjr_xfile *f, xjr_s16 *v_out, xjr_offset_t offset);
int xjr_xfile_read_u8(xjr_xfile *f, xjr_u8 *v_out, xjr_offset_t offset);
int xjr_xfile_read_s8(xjr_xfile *f, xjr_s8 *v_out, xjr_offset_t offset);
int xjr_xfile_read_f64(xjr_xfile *f, xjr_f64 *v_out, xjr_offset_t offset);

/* Higher-level of interface for reading data from X
 * @param[in]   f         X File
 * @param[out]  body_out  Output data
 * @param[out]  len_out   Output length
 * @param[in]   offset    Offset of the item
 * @return      0         Success
 * @return      -1        Fail */
int xjr_xfile_read_symbol(xjr_xfile *f, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset);
int xjr_xfile_read_string(xjr_xfile *f, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset);

#endif

