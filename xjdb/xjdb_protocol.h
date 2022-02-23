/* XenonJS : Debugger : Debugging Protocol
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_PROTOCOL_H
#define XJDB_PROTOCOL_H

#include <stdarg.h>
#include "hbuf.h"
#include "xjdb_conn.h"

int xjdb_protocol_send(xjdb_conn_t *conn, const char *data, xjdb_size_t len);

int xjdb_protocol_send_empty(xjdb_conn_t *conn);
int xjdb_protocol_send_format(xjdb_conn_t *conn, const char *fmt, ...);
int xjdb_protocol_send_list(xjdb_conn_t *conn);

typedef enum
{
    XJDB_PROTOCOL_DECODE_OK,
    XJDB_PROTOCOL_DECODE_DEFER,
    XJDB_PROTOCOL_DECODE_ERROR,
} xjdb_protocol_decode_result_t;

xjdb_protocol_decode_result_t xjdb_protocol_try_decode(hbuf_t *buf);

#endif

