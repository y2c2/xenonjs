/* XenonJS : Runtime Time System : X File
 * Copyright(c) 2017 y2c2 */

#include "xjr_xformat.h"
#include "xjr_x.h"

/* Initialize xfile */
void xjr_xfile_init(xjr_xfile *f)
{
    f->bc = xjr_nullptr;
    f->len = 0;
    f->sections.data = 0;
    f->sections.text = 0;
    f->sections.attr = 0;
    f->sections.expr = 0;
    f->sections.impr = 0;
    f->sections.dbg0 = 0;
    f->sections.dbi0 = 0;
}

/* If the signature is valid
 * return 0: OK, 1: failed */
static int xjr_verify_signature(xjr_xfile *f)
{
    if ((f->len < 4) || \
            (f->bc[0] != 0x78) || \
            (f->bc[1] != 0x6a) || \
            (f->bc[2] != 0x73) || \
            (f->bc[3] != 0x00))
    { return -1; }
    return 0;
}

/* If the platform is valid
 * return 0: OK, 1: failed */
static int xjr_verify_platform(xjr_xfile *f)
{
    if ((f->len < 6) || \
            (f->bc[4] != 0x01) || (f->bc[5] != 0x00))
    { return -1; }
    return 0;
}

/* If the SDK version is valid
 * return 0: OK, 1: failed */
static int xjr_verify_sdk_version(xjr_xfile *f)
{
    if ((f->len < 8) || \
            (f->bc[6] != 0x01) || (f->bc[7] != 0x00))
    { return -1; }
    return 0;
}

/* Load bytecode */
int xjr_xfile_load(xjr_xfile *f, char *bc, xjr_size_t len)
{
    f->bc = bc;
    f->len = len;

    /* Signature */
    if ((xjr_verify_signature(f) != 0) || \
            (xjr_verify_platform(f) != 0) || \
            (xjr_verify_sdk_version(f) != 0))
    { goto fail; }

    /* Index of Sections */
    {
        xjr_u32 sections_count, i, offset_section_index, offset_section_data;
        if (xjr_xfile_read_u32(f, &sections_count, 8) != 0) { goto fail; }
        offset_section_index = XJR_BC_SIZE_HEADER + \
                              XJR_BC_SIZE_OFFSET_COUNT;
        offset_section_data = XJR_BC_SIZE_HEADER + \
                              XJR_BC_SIZE_OFFSET_COUNT + \
                              (XJR_BC_SIZE_ADDR * 2 * sections_count);
        for (i = 0; i != sections_count; i++)
        {
            xjr_u32 offset_section, size_section;
            if (xjr_xfile_read_u32(f, &offset_section, \
                        offset_section_index + (XJR_BC_SIZE_ADDR + XJR_BC_SIZE_ADDR) * i) != 0)
            { return -1; }
            if (xjr_xfile_read_u32(f, &size_section, \
                        offset_section_index + (XJR_BC_SIZE_ADDR + XJR_BC_SIZE_ADDR) * i + XJR_BC_SIZE_ADDR) != 0)
            { return -1; }
            {
                xjr_u32 section_type;
                if (xjr_xfile_read_u32(f, &section_type, \
                            offset_section_data + offset_section) != 0)
                { return -1; }
                switch (section_type)
                {
                    case 0x61746164: /* data */
                        f->sections.data = offset_section_data + offset_section;
                        break;
                    case 0x74786574: /* text */
                        f->sections.text = offset_section_data + offset_section;
                        break;
                    case 0x72747461: /* attr */
                        f->sections.attr = offset_section_data + offset_section;
                        break;
                    case 0x74707865: /* expr */
                        f->sections.expr = offset_section_data + offset_section;
                        break;
                    case 0x74706d69: /* impr */
                        f->sections.impr = offset_section_data + offset_section;
                        break;
                    case 0x30676264: /* dbg0 */
                        f->sections.dbg0 = offset_section_data + offset_section;
                        break;
                    case 0x30696264: /* dbi0 */
                        f->sections.dbi0 = offset_section_data + offset_section;
                        break;
                    default:
                        /* Unrecognized */
                        break;
                }
            }
        }
    }

    /* attr */
    {
        xjr_u32 attrs_count, i, offset_attr_index, offset_attr_data;
        if (xjr_xfile_read_u32(f, &attrs_count, f->sections.attr + 4) != 0) return -1;
        offset_attr_index = f->sections.attr + 4 + XJR_BC_SIZE_OFFSET_COUNT;
        offset_attr_data = f->sections.attr + 4 + XJR_BC_SIZE_OFFSET_COUNT + (XJR_BC_SIZE_ADDR * attrs_count);
        for (i = 0; i != attrs_count; i++)
        {
            xjr_u32 offset_attr;
            if (xjr_xfile_read_u32(f, &offset_attr, \
                        offset_attr_index + XJR_BC_SIZE_ADDR * i) != 0)
            { return -1; }
            {
                xjr_u16 attr_type;
                if (xjr_xfile_read_u16(f, &attr_type, \
                            offset_attr_data + offset_attr) != 0)
                { return -1; }
                switch (attr_type)
                {
                    case XJR_BC_ATTR_TYPE_TOPLEVEL:
                        if (xjr_xfile_read_u32(f, &f->init_pc, \
                                    offset_attr_data + offset_attr + 2) != 0)
                        { return -1; }
                        break;
                }
            }
        }
    }

    return 0;
fail:
    xjr_xfile_init(f);
    return -1;
}

int xjr_xfile_read_u32(xjr_xfile *f, xjr_u32 *v_out, xjr_offset_t offset)
{
    if (offset + 4 > f->len) return -1;
    *v_out = (((xjr_u8 *)f->bc)[offset]) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 1] << 8) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 2] << 16) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 3] << 24);
    return 0;
}

int xjr_xfile_read_s32(xjr_xfile *f, xjr_s32 *v_out, xjr_offset_t offset)
{
    if (offset + 4 > f->len) return -1;
    xjr_u32 v = (((xjr_u8 *)f->bc)[offset]) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 1] << 8) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 2] << 16) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 3] << 24);
    *v_out = *((xjr_s32 *)(&v));
    return 0;
}

int xjr_xfile_read_u16(xjr_xfile *f, xjr_u16 *v_out, xjr_offset_t offset)
{
    if (offset + 2 > f->len) return -1;
    *v_out = (xjr_u16)((((xjr_u8 *)f->bc)[offset]) | 
        (xjr_u32)(((xjr_u8 *)f->bc)[offset + 1] << 8));
    return 0;
}

int xjr_xfile_read_s16(xjr_xfile *f, xjr_s16 *v_out, xjr_offset_t offset)
{
    if (offset + 2 > f->len) return -1;
    xjr_u16 v = (((xjr_u8 *)f->bc)[offset]) | 
        (xjr_u16)(((xjr_u8 *)f->bc)[offset + 1] << 8);
    *v_out = *((xjr_s16 *)(&v));
    return 0;
}

int xjr_xfile_read_u8(xjr_xfile *f, xjr_u8 *v_out, xjr_offset_t offset)
{
    if (offset + 1 > f->len) return -1;
    *v_out = (xjr_u8)(f->bc[offset]);
    return 0;
}

int xjr_xfile_read_s8(xjr_xfile *f, xjr_s8 *v_out, xjr_offset_t offset)
{
    if (offset + 1 > f->len) return -1;
    *v_out = (f->bc[offset]);
    return 0;
}

int xjr_xfile_read_f64(xjr_xfile *f, xjr_f64 *v_out, xjr_offset_t offset)
{
    if (offset + 8 > f->len) return -1;
    *v_out = *((xjr_f64 *)(&f->bc[offset]));
    return 0;
}

static int xjr_xfile_read_datasec_item(xjr_xfile *f, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset, \
        int opt)
{
    xjr_offset_t offset_item = f->sections.data + XJR_BC_SECTION_SIGNATURE_LEN + offset;
    xjr_u16 type;
    xjr_u32 len;

    /* Read data type */
    if (xjr_xfile_read_u16(f, &type, offset_item) != 0) { return -1; }
    offset_item += 2;
    /* It should be a symbol */ 
    if (type != opt) { return -1; }
    /* Length */
    if (xjr_xfile_read_u32(f, &len, offset_item) != 0) { return -1; }
    offset_item += 4;
    if (offset_item + len > f->len) { return -1; }
    /* Body */
    *len_out = (xjr_size_t)(len);
    *body_out = f->bc + offset_item;

    return 0;
}

int xjr_xfile_read_symbol(xjr_xfile *f, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset)
{
    return xjr_xfile_read_datasec_item(f, \
        body_out, len_out, offset, XJR_BC_DATASEC_ITEM_TYPE_UTF8SYM);
}

int xjr_xfile_read_string(xjr_xfile *f, \
        char **body_out, xjr_size_t *len_out, \
        xjr_offset_t offset)
{
    return xjr_xfile_read_datasec_item(f, \
        body_out, len_out, offset, XJR_BC_DATASEC_ITEM_TYPE_UTF8STR);
}

