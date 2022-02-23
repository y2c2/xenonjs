/* Stream Buffer */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "streambuf.h"

struct sb
{
    char *buf;
    xjr_size_t size;
    xjr_size_t capacity;
};

sb_t *sb_new(void)
{
    sb_t *new_rstr = (sb_t *)malloc(sizeof(sb_t));
    if (new_rstr == NULL) return NULL;
    new_rstr->buf = (char *)malloc(sizeof(char) * (SB_INIT_CAPACITY + 1));
    if (new_rstr->buf == NULL) 
    { free(new_rstr); return NULL; }
    new_rstr->size = 0;
    new_rstr->capacity = SB_INIT_CAPACITY;
    return new_rstr;
}


void sb_destroy(sb_t *hb)
{
    if (hb->buf != NULL) free(hb->buf);
    free(hb);
}

void sb_clear(sb_t *hb)
{
    hb->buf[0] = '\0';
    hb->size = 0;
}

/* Append data in the tail of the string */ 

int sb_append(sb_t *hb, const char *s, xjr_size_t len)
{
    char *new_buf = NULL;
    unsigned int new_capacity;

    if (hb->size + len + 1 >= hb->capacity)
    {
        /* Extend */
        new_capacity = hb->size + len + 1 + SB_EXTRA_CAPACITY;
        new_buf = (char *)malloc(sizeof(char) * new_capacity);
        if (new_buf == NULL) return -1;
        memcpy(new_buf, hb->buf, hb->size);
        memcpy(new_buf + hb->size, s, len);
        hb->size = hb->size + len;
        new_buf[hb->size] = '\0';
        free(hb->buf);
        hb->buf = new_buf;
        hb->capacity = new_capacity;
    }
    else
    {
        memcpy(hb->buf + hb->size, s, len);
        hb->size += len;
        hb->buf[hb->size] = '\0';
    }

    return 0;
}

int sb_append_char(sb_t *hb, char ch)
{
    return sb_append(hb, &ch, 1);
}

char *sb_buf(sb_t *hb)
{
    return hb->buf;
}

xjr_size_t sb_size(sb_t *hb)
{
    return hb->size;
}

void sb_left_strip(sb_t *hb, char ch)
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
        sb_clear(hb);
        return;
    }
    else if (offset == 0)
    {
        return;
    }
    {
        unsigned int len_to_copy = hb->size - offset;
        char *dst_p = hb->buf, *src_p = hb->buf + offset;
        memcpy(dst_p, src_p, len_to_copy);
        hb->size = len_to_copy;
    }
}

void sb_shift(sb_t *hb, xjr_size_t n)
{
    if (hb->size == 0) return;
    if (hb->size < n) return;

    {
        xjr_size_t len_to_copy = hb->size - n;
        char *dst_p = hb->buf, *src_p = hb->buf + n;
        memcpy(dst_p, src_p, len_to_copy);
        hb->size = len_to_copy;
    }
}

void sb_shift_one(sb_t *hb)
{
    sb_shift(hb, 1);
}

int sb_find(sb_t *hb, char ch)
{
    xjr_size_t i;

    for (i = 0; i != hb->size; i++)
    {
        if (hb->buf[i] == ch)
        {
            return (int)i;
        }
    }

    return -1;
}

xjr_u8 checksum(sb_t *hb, xjr_size_t len)
{
    xjr_u8 sum = 0;
    xjr_size_t i;

    for (i = 0; i != len; i++)
    {
        sum += (xjr_u8)hb->buf[i];
    }

    return sum;
}

