/* XenonJS : Runtime Time System : Builtin Functions : Helper
 * Copyright(c) 2017 y2c2 */

#include "xjr_bltin_helper.h"

void *xjr_mbuf_malloc_by_mp(void *data, xjr_mbuf_size_t size)
{
    xjr_urid_t urid = xjr_mp_malloc((xjr_mp_t *)data, size);
    return xjr_mp_get_ptr(data, urid);
}

void xjr_mbuf_free_by_mp(void *data, void *ptr)
{
    xjr_mp_free((xjr_mp_t *)data, xjr_mp_get_urid((xjr_mp_t *)data, ptr));
}

int xjr_mbuf_write_cb( \
        void *data, char *s, xjr_size_t len)
{
    xjr_mbuf_t *mbuf = data;
    xjr_mbuf_append(mbuf, s, len);
    return 0;
}

