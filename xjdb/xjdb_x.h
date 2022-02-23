/* XenonJS : Debugger : X File
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_X_H
#define XJDB_X_H

#include "xjdb_dt.h"

typedef struct
{
    /* Bytecode Data */
    char *bc;
    xjdb_size_t len;

    /* Sections */
    struct
    {
        xjdb_offset_t text;
        xjdb_offset_t data;
        xjdb_offset_t attr;
        xjdb_offset_t expr;
        xjdb_offset_t impr;
        xjdb_offset_t dbg0;
    } sections;

    xjdb_offset_t init_pc;
} xjdb_xfile;

/* Initialize xfile */
void xjdb_xfile_init(xjdb_xfile *f);

/* Load bytecode */
int xjdb_xfile_load(xjdb_xfile *f, char *bc, xjdb_size_t len);

/* Basic interface for reading data from X
 * @param[in]   f         X File
 * @param[out]  v_out     Output value
 * @param[in]   offset    Offset
 * @return      0         Success
 * @return      -1        Fail */
int xjdb_xfile_read_u32(xjdb_xfile *f, xjdb_u32 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_s32(xjdb_xfile *f, xjdb_s32 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_u16(xjdb_xfile *f, xjdb_u16 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_s16(xjdb_xfile *f, xjdb_s16 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_u8(xjdb_xfile *f, xjdb_u8 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_s8(xjdb_xfile *f, xjdb_s8 *v_out, xjdb_offset_t offset);
int xjdb_xfile_read_f64(xjdb_xfile *f, xjdb_f64 *v_out, xjdb_offset_t offset);

/* Higher-level of interface for reading data from X
 * @param[in]   f         X File
 * @param[out]  body_out  Output data
 * @param[out]  len_out   Output length
 * @param[in]   offset    Offset of the item
 * @return      0         Success
 * @return      -1        Fail */
int xjdb_xfile_read_symbol(xjdb_xfile *f, \
        char **body_out, xjdb_size_t *len_out, \
        xjdb_offset_t offset);
int xjdb_xfile_read_string(xjdb_xfile *f, \
        char **body_out, xjdb_size_t *len_out, \
        xjdb_offset_t offset);

#endif

