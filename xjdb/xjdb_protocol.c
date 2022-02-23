/* XenonJS : Debugger : Debugging Protocol
 * Copyright(c) 2018 y2c2 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xjdb_conn.h"
#include "xjdb_protocol.h"

static xjdb_u8 checksum(const char *data, const xjdb_size_t len)
{
    xjdb_u8 sum = 0;
    xjdb_size_t i;

    for (i = 0; i != len; i++) { sum += data[i]; }

    return sum;
}

static int hexlit_to_raw(xjdb_u8 *dst, const char *src, const xjdb_size_t len)
{
    xjdb_size_t i;
    xjdb_u8 *dst_p = dst;
    const char *src_p = src;

    if ((len & 1) != 0) { return -1; }

    for (i = 0; i != len; i += 2)
    {
        xjdb_u8 t_hi, t_lo;

#define _x(_d, _s) \
        do { \
            if ('0' <= _s && _s <= '9') { _d = (xjdb_u8)(_s - '0'); } \
            else if ('a' <= _s && _s <= 'f') { _d = (xjdb_u8)(_s - 'a' + 10); } \
            else if ('A' <= _s && _s <= 'F') { _d = (xjdb_u8)(_s - 'A' + 10); } \
            else return -1; \
        } while (0)
        _x(t_hi, *src_p); src_p++;
        _x(t_lo, *src_p); src_p++;
#undef _x

        *dst_p++ = (xjdb_u8)((t_hi << 4) | (t_lo));
    }

    return (int)(dst_p - dst);
}

static int raw_to_hexlit(char *dst, const xjdb_u8 *src, xjdb_size_t len)
{
    const xjdb_u8 *src_p = src;
    char *dst_p = dst;

    if (len == 0) return -1;

    while (len-- != 0)
    {
#define _x(_s) \
        do { *dst_p++ = "0123456789ABCDEF"[_s]; } while (0)

        _x((*src_p) >> 4);
        _x((*src_p) & 0xF);
        src_p++;

#undef _x
    }
    *dst_p = '\0';

    return (int)(dst_p - dst);
}

int xjdb_protocol_send(xjdb_conn_t *conn, const char *data, xjdb_size_t len)
{
    int ret = 0;
    char *frame = NULL;
    xjdb_size_t frame_len = len + 5;
    xjdb_u8 cs;

    /* $<body>#<checksum> */
    if ((frame = (char *)malloc(sizeof(char) * (frame_len))) == NULL) { return -1; }
    frame[0] = '$';
    memcpy(frame + 1, data, len);
    frame[1 + len] = '#';
    cs = checksum(data, len);
    raw_to_hexlit(frame + 1 + len, (const xjdb_u8 *)&cs, 1);

    /* Send */
    if (xjdb_conn_send(conn, frame, frame_len) < (int)frame_len)
    { ret = -1; }

    free(frame);
    return ret;
}

int xjdb_protocol_send_empty(xjdb_conn_t *conn)
{
    return xjdb_protocol_send(conn, "", 0);
}

int xjdb_protocol_send_format(xjdb_conn_t *conn, const char *fmt, ...)
{
    char buffer[1024];
    va_list ap;
    int size;

    va_start(ap, fmt);
    size = vsnprintf(buffer, 1024, fmt, ap);
    va_end(ap);

    return xjdb_protocol_send(conn, buffer, (xjdb_size_t)size);
}

int xjdb_protocol_send_list(xjdb_conn_t *conn)
{
    return xjdb_protocol_send(conn, "", 0);
}

xjdb_protocol_decode_result_t xjdb_protocol_try_decode(hbuf_t *buf)
{
    char *body = (char *)hbuf_buf(buf);
    int loc_sharp;

    /* First byte should be '$' */
    if (body[0] != '$') return XJDB_PROTOCOL_DECODE_ERROR;

    /* Should contains '#' */
    loc_sharp = hbuf_find(buf, '#');
    if (loc_sharp == -1) return XJDB_PROTOCOL_DECODE_DEFER;
    if ((int)hbuf_size(buf) - loc_sharp < 3) return XJDB_PROTOCOL_DECODE_DEFER;

    /* Checksum */
    {
        xjdb_u8 cs1, cs2;
        if (hexlit_to_raw(&cs1, body + loc_sharp + 1, 2) < 1)
        { return XJDB_PROTOCOL_DECODE_ERROR; }
        cs2 = checksum(body + 1, (xjdb_size_t)(loc_sharp - 1));
        if (cs1 != cs2)
        { return XJDB_PROTOCOL_DECODE_ERROR; }
    }

    return XJDB_PROTOCOL_DECODE_OK;
}

