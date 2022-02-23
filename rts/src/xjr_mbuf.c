/* XenonJS : Runtime Time System : Mutable Buffer
 * Copyright(c) 2018 y2c2 */

#include "xjr_libc.h"
#include "xjr_mbuf.h"

#define MBUF_DEFAULT_INIT_SIZE 512
#define MBUF_DEFAULT_INC_SIZE 512

int xjr_mbuf_init_conf( \
        xjr_mbuf_t *mbuf, \
        xjr_mbuf_size_t init_size, xjr_mbuf_size_t inc_size, \
        void *data, \
        xjr_mbuf_malloc_cb_t malloc_cb, \
        xjr_mbuf_free_cb_t free_cb, \
        xjr_mbuf_memcpy_cb_t memcpy_cb)
{
    mbuf->data = data;
    mbuf->malloc_cb = malloc_cb;
    mbuf->free_cb = free_cb;
    mbuf->memcpy_cb = memcpy_cb;
    mbuf->size = 0;
    mbuf->capacity = init_size;
    mbuf->init_size = init_size;
    mbuf->inc_size = inc_size;
    if ((mbuf->body = (char *)malloc_cb( \
                    data, \
                    sizeof(char) * init_size)) == ((void *)0))
    { return -1; }

    return 0;
}

int xjr_mbuf_init( \
        xjr_mbuf_t *mbuf, \
        void *data, \
        xjr_mbuf_malloc_cb_t malloc_cb, \
        xjr_mbuf_free_cb_t free_cb, \
        xjr_mbuf_memcpy_cb_t memcpy_cb)
{
    return xjr_mbuf_init_conf( \
            mbuf, \
            MBUF_DEFAULT_INIT_SIZE, \
            MBUF_DEFAULT_INC_SIZE, \
            data, \
            malloc_cb, \
            free_cb, \
            memcpy_cb);
}

void xjr_mbuf_uninit(xjr_mbuf_t *mbuf)
{
    if (mbuf->body != (void *)0)
    {
        mbuf->free_cb(mbuf->data, mbuf->body);
        mbuf->body = (void *)0;
    }
}

int xjr_mbuf_append(xjr_mbuf_t *mbuf, const char *s, const xjr_mbuf_size_t len)
{
    char *new_buf = (void *)0;
    xjr_mbuf_size_t new_capacity;

    if (mbuf->size + len + 1 >= mbuf->capacity)
    {
        /* Extend */
        new_capacity = mbuf->size + len + 1 + mbuf->inc_size;
        new_buf = (char *)mbuf->malloc_cb( \
                mbuf->data, \
                sizeof(char) * new_capacity);
        if (new_buf == (void *)0) return -1;
        if (mbuf->size > 0)
        {
            mbuf->memcpy_cb(new_buf, mbuf->body, mbuf->size);
        }
        mbuf->memcpy_cb(new_buf + mbuf->size, s, len);
        mbuf->size = mbuf->size + len;
        new_buf[mbuf->size] = '\0';
        mbuf->capacity = new_capacity;
        mbuf->free_cb(mbuf->data, mbuf->body);
        mbuf->body = new_buf;
    }
    else
    {
        mbuf->memcpy_cb(mbuf->body + mbuf->size, s, len);
        mbuf->size += len;
        mbuf->body[mbuf->size] = '\0';
    }

    return 0;
}

int xjr_mbuf_append_c_str(xjr_mbuf_t *mbuf, const char *s)
{
    return xjr_mbuf_append(mbuf, s, xjr_strlen(s));
}

int xjr_mbuf_shift(xjr_mbuf_t *mbuf, const xjr_mbuf_size_t len)
{
    if (len > mbuf->size) return -1;

    mbuf->memcpy_cb(mbuf->body, mbuf->body + len, mbuf->size - len);
    mbuf->size -= len;

    return 0;
}

int xjr_mbuf_clear(xjr_mbuf_t *mbuf)
{
    mbuf->free_cb(mbuf->data, mbuf->body);
    if ((mbuf->body = (char *)mbuf->malloc_cb( \
                    mbuf->data, sizeof(char) * mbuf->init_size)) == ((void *)0))
    { return -1; }
    mbuf->size = 0;
    mbuf->capacity = mbuf->init_size;

    return 0;
}

char *xjr_mbuf_body(xjr_mbuf_t *mbuf)
{
    return mbuf->body;
}

xjr_mbuf_size_t xjr_mbuf_size(xjr_mbuf_t *mbuf)
{
    return mbuf->size;
}

