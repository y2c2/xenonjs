#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hbuf.h"

struct hbuf
{
    unsigned char *buf;
    unsigned int size;
    unsigned int capacity;
};

hbuf_t *hbuf_new(void)
{
    hbuf_t *new_rstr = (hbuf_t *)malloc(sizeof(hbuf_t));
    if (new_rstr == HBUF_NULL) return HBUF_NULL;
    new_rstr->buf = (unsigned char *)malloc(sizeof(unsigned char) * (HBUF_INIT_CAPACITY + 1));
    if (new_rstr->buf == HBUF_NULL) 
    { free(new_rstr); return HBUF_NULL; }
    new_rstr->size = 0;
    new_rstr->capacity = HBUF_INIT_CAPACITY;
    return new_rstr;
}


void hbuf_destroy(hbuf_t *str)
{
    if (str->buf != HBUF_NULL) free(str->buf);
    free(str);
}

void hbuf_clear(hbuf_t *hb)
{
    hb->buf[0] = '\0';
    hb->size = 0;
}

/* Append data in the tail of the string */ 

int hbuf_append(hbuf_t *str, const unsigned char *s, unsigned int len)
{
    unsigned char *new_buf = HBUF_NULL;
    unsigned int new_capacity;

    if (str->size + len + 1 >= str->capacity)
    {
        /* Extend */
        new_capacity = str->size + len + 1 + HBUF_EXTRA_CAPACITY;
        new_buf = (unsigned char *)malloc(sizeof(unsigned char) * new_capacity);
        if (new_buf == HBUF_NULL) return -1;
        memcpy(new_buf, str->buf, str->size);
        memcpy(new_buf + str->size, s, len);
        str->size = str->size + len;
        new_buf[str->size] = '\0';
        free(str->buf);
        str->buf = new_buf;
        str->capacity = new_capacity;
    }
    else
    {
        memcpy(str->buf + str->size, s, len);
        str->size += len;
        str->buf[str->size] = '\0';
    }

    return 0;
}

int hbuf_append_char(hbuf_t *str, unsigned char ch)
{
    return hbuf_append(str, &ch, 1);
}

unsigned char *hbuf_buf(hbuf_t *str)
{
    return str->buf;
}

unsigned int hbuf_size(hbuf_t *str)
{
    return str->size;
}

void hbuf_left_strip(hbuf_t *hb, unsigned char ch)
{
    unsigned int offset = 0;

    if (hb->size == 0) return;

    /* Locate to ch */
    for (offset = 0; offset != hb->size; offset++)
    {
        if (hb->buf[offset] == ch)
        { break; }
    }
    if (offset == hb->size)
    {
        /* Not found, we should clear it? */
        hbuf_clear(hb);
        return;
    }
    else if (offset == 0)
    {
        return;
    }
    {
        unsigned int len_to_copy = hb->size - offset;
        unsigned char *dst_p = hb->buf, *src_p = hb->buf + offset;
        memcpy(dst_p, src_p, len_to_copy);
        hb->size = len_to_copy;
    }
}

void hbuf_shift(hbuf_t *hb, unsigned int n)
{
    if (hb->size == 0) return;
    if (hb->size < n) return;

    {
        unsigned int len_to_copy = hb->size - n;
        unsigned char *dst_p = hb->buf, *src_p = hb->buf + n;
        memcpy(dst_p, src_p, len_to_copy);
        hb->size = len_to_copy;
    }
}

void hbuf_shift_one(hbuf_t *hb)
{
    hbuf_shift(hb, 1);
}

int hbuf_find(hbuf_t *hb, unsigned char ch)
{
    unsigned int i;

    for (i = 0; i != hb->size; i++)
    {
        if (hb->buf[i] == ch) return ((int)i);
    }

    return -1;
}

