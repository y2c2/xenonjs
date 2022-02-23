/* XenonJS : Bit Writer
 * Copyright(c) 2017 y2c2 */

#include "xjs_alloc.h"
#include "xjs_libc.h"
#include "xjs_bitwriter.h"

#define MBUF_DEFAULT_INIT_SIZE 512
#define MBUF_DEFAULT_INC_SIZE 512

int xjs_bitwriter_init_conf( \
        xjs_bitwriter *bw, \
        xjs_size_t init_size, xjs_size_t inc_size, \
        xjs_bitwriter_malloc_cb_t malloc_cb, \
        xjs_bitwriter_free_cb_t free_cb, \
        xjs_bitwriter_memcpy_cb_t memcpy_cb)
{
    bw->malloc_cb = malloc_cb;
    bw->free_cb = free_cb;
    bw->memcpy_cb = memcpy_cb;
    bw->size = 0;
    bw->capacity = init_size;
    bw->init_size = init_size;
    bw->inc_size = inc_size;
    if ((bw->body = (char *)malloc_cb( \
                    sizeof(char) * init_size)) == ((void *)0))
    { return -1; }

    return 0;
}

int xjs_bitwriter_init( \
        xjs_bitwriter *bw)
{
    return xjs_bitwriter_init_conf( \
            bw, \
            MBUF_DEFAULT_INIT_SIZE, \
            MBUF_DEFAULT_INC_SIZE, \
            xjs_malloc, \
            xjs_free, \
            xjs_memcpy);
}

void xjs_bitwriter_uninit(xjs_bitwriter *bw)
{
    if (bw->body != (void *)0)
    {
        bw->free_cb(bw->body);
        bw->body = (void *)0;
    }
}

int xjs_bitwriter_append(xjs_bitwriter *bw, const char *s, const xjs_size_t len)
{
    char *new_buf = (void *)0;
    xjs_size_t new_capacity;

    if (bw->size + len + 1 >= bw->capacity)
    {
        /* Extend */
        new_capacity = bw->size + len + 1 + bw->inc_size;
        new_buf = (char *)bw->malloc_cb( \
                sizeof(char) * new_capacity);
        if (new_buf == (void *)0) return -1;
        if (bw->size > 0)
        {
            bw->memcpy_cb(new_buf, bw->body, bw->size);
        }
        bw->memcpy_cb(new_buf + bw->size, s, len);
        bw->size = bw->size + len;
        new_buf[bw->size] = '\0';
        bw->capacity = new_capacity;
        bw->free_cb(bw->body);
        bw->body = new_buf;
    }
    else
    {
        bw->memcpy_cb(bw->body + bw->size, s, len);
        bw->size += len;
        bw->body[bw->size] = '\0';
    }

    return 0;
}

int xjs_bitwriter_append_c_str(xjs_bitwriter *bw, const char *s)
{
    return xjs_bitwriter_append(bw, s, xjs_strlen(s));
}

int xjs_bitwriter_write_u32(xjs_bitwriter *bw, const xjs_u32 v)
{
    xjs_u8 buf[4];
    buf[0] = v & 0xFF;
    buf[1] = (v >> 8) & 0xFF;
    buf[2] = (v >> 16) & 0xFF;
    buf[3] = (v >> 24) & 0xFF;
    return xjs_bitwriter_append(bw, (char *)buf, 4);
}

int xjs_bitwriter_write_u16(xjs_bitwriter *bw, const xjs_u16 v)
{
    xjs_u8 buf[2];
    buf[0] = v & 0xFF;
    buf[1] = (v >> 8) & 0xFF;
    return xjs_bitwriter_append(bw, (char *)buf, 2);
}

int xjs_bitwriter_write_u8(xjs_bitwriter *bw, const xjs_u8 v)
{
    return xjs_bitwriter_append(bw, (const char *)&v, 1);
}

int xjs_bitwriter_write_s32(xjs_bitwriter *bw, const xjs_s32 v)
{
    return xjs_bitwriter_write_u32(bw, (xjs_u32)v);
}

int xjs_bitwriter_write_s16(xjs_bitwriter *bw, const xjs_s16 v)
{
    return xjs_bitwriter_write_u16(bw, (xjs_u16)v);
}

int xjs_bitwriter_write_s8(xjs_bitwriter *bw, const xjs_s8 v)
{
    return xjs_bitwriter_write_u8(bw, (xjs_u8)v);
}

int xjs_bitwriter_write_f64(xjs_bitwriter *bw, const xjs_f64 v)
{
    const xjs_u8 *buf = (const xjs_u8 *)(&v);
    xjs_size_t i;
    for (i = 0; i != 8; i++)
    { xjs_bitwriter_write_u8(bw, (xjs_u8)buf[i]); }
    return 0;
}

int xjs_bitwriter_shift(xjs_bitwriter *bw, const xjs_size_t len)
{
    if (len > bw->size) return -1;

    bw->memcpy_cb(bw->body, bw->body + len, bw->size - len);
    bw->size -= len;

    return 0;
}

int xjs_bitwriter_clear(xjs_bitwriter *bw)
{
    bw->free_cb(bw->body);
    if ((bw->body = (char *)bw->malloc_cb( \
                    sizeof(char) * bw->init_size)) == ((void *)0))
    { return -1; }
    bw->size = 0;
    bw->capacity = bw->init_size;

    return 0;
}

char *xjs_bitwriter_body(xjs_bitwriter *bw)
{
    return bw->body;
}

xjs_size_t xjs_bitwriter_size(xjs_bitwriter *bw)
{
    return bw->size;
}


