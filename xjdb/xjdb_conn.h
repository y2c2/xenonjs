/* XenonJS : Debugger : Connection
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_CONN_H
#define XJDB_CONN_H

#include "xjdb_platform.h"
#include "xjdb_dt.h"

typedef enum
{
    xjdb_conn_none,
    xjdb_conn_tcp,
    xjdb_conn_serial,
} xjdb_conn_type_t;

struct xjdb_opaque_conn
{
    xjdb_conn_type_t type;
    union
    {
        struct
        {
            int fd;
        } as_tcp;
        struct
        {
            int fd;
        } as_udp;
        struct
        {
            int fd;
        } as_serial;
    } u;
};
typedef struct xjdb_opaque_conn xjdb_conn_t;

xjdb_conn_t *xjdb_conn_new(void);
void xjdb_conn_destroy(xjdb_conn_t *conn);

int xjdb_conn_connect(xjdb_conn_t *conn, \
        const char *host, const xjdb_size_t port);

xjdb_bool xjdb_conn_alive(xjdb_conn_t *conn);
void xjdb_conn_close(xjdb_conn_t *conn);
int xjdb_conn_readable(xjdb_conn_t *conn);
int xjdb_conn_recv(xjdb_conn_t *conn, void *buf, xjdb_size_t len);
int xjdb_conn_send(xjdb_conn_t *conn, const void *buf, xjdb_size_t len);

#endif

