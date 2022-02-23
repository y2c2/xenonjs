/* XenonJS : Debugger : Connection
 * Copyright(c) 2018 y2c2 */

#include "xjdb_platform.h"
#include "xjdb_dt.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#elif defined(PLATFORM_FREEBSD)
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#elif defined(PLATFORM_WINDOWS)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# ifndef WINVER
# define WINVER 0x0501
# endif
# include <winsock2.h>
# include <windows.h>
# include <ws2tcpip.h>
#endif

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef WITHSSL
#include "ssladapter.h"
#endif

#include "xjdb_conn.h"

xjdb_conn_t *xjdb_conn_new(void)
{
    xjdb_conn_t *conn = (xjdb_conn_t *)malloc(sizeof(xjdb_conn_t));
    conn->type = xjdb_conn_none;
    return conn;
}

void xjdb_conn_destroy(xjdb_conn_t *conn)
{
    free(conn);
}

static int _writable(int fd, const int timeout)
{
    fd_set wfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    retval = select(fd + 1, NULL, &wfds, NULL, &tv);
    if (retval == -1) return -1;
    else if (retval == 0) return 0;
    /* retval >= 0 */

    return 1;
}

static int socket_set_nonblock(int fd)
{
#if defined(PLATFORM_WINDOWS)
# if defined(__MINGW64__) || defined(__MINGW32__)
    u_long imode = 0;
# else
    unsigned int imode = 0;
#endif
    int ret;
    ret = ioctlsocket(fd, FIONBIO, &imode);
    if (ret != NO_ERROR) return -1;
    return 0;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    /* Non-block */
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return 0;
#endif
}

static int xjdb_conn_connect2(xjdb_conn_t *conn, \
        const char *host, const xjdb_size_t port, const int timeout)
{
    int ret = 0;
    int fd = -1;
    struct sockaddr_in addr;
    struct addrinfo *servinfo = NULL;

    if (conn->type != xjdb_conn_none) { return -1; }

    /* Resolve the host */
    struct addrinfo hints;
    char service_buf[16 + 1];
    snprintf(service_buf, 16, "%d", (int)port);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(host, service_buf, &hints, &servinfo) != 0)
    { ret = -1; goto fail; }

    /* Fill address */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((xjdb_u16)port);
    memcpy(&addr.sin_addr, &((struct sockaddr_in *)servinfo->ai_addr)->sin_addr, sizeof(struct in_addr));

    /* Create socket */
    if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
    { ret = -1; goto fail; }

    if (timeout == 0)
    {
        /* No timeout */

        /* Connect */
        if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        { ret = -1; goto fail; }

        /* Non-block */
        socket_set_nonblock(fd);
    }
    else
    {
        /* Non-block */
        socket_set_nonblock(fd);

        /* Connect */
        if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
        {

            if (errno != EINPROGRESS) { ret = -1; goto fail; }
            if (!(_writable(fd, timeout) == 1)) { ret = -1; goto fail; }
            {
                socklen_t len = sizeof(int);
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
                int err;
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
                { ret = -1; goto fail; }
#elif defined(PLATFORM_WINDOWS)
                char err;
                if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
                { ret = -1; goto fail; }
#endif
                if (err != 0)
                { ret = -1; goto fail; }
            }
        }
    }


    conn->type = xjdb_conn_tcp;
    conn->u.as_tcp.fd = fd;

    goto done;
fail:
    if (fd != -1)
    {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
        close(fd);
#elif defined(PLATFORM_WINDOWS)
        closesocket(fd);
#endif
    }
done:
    if (servinfo != NULL)
    {
        freeaddrinfo(servinfo);
    }
    return ret;
}

int xjdb_conn_connect(xjdb_conn_t *conn, \
        const char *host, const xjdb_size_t port)
{
    return xjdb_conn_connect2(conn, host, port, 3);
}

xjdb_bool xjdb_conn_alive(xjdb_conn_t *conn)
{
    return conn->type == xjdb_conn_none ? xjdb_false : xjdb_true;
}

void xjdb_conn_close(xjdb_conn_t *conn)
{
    switch (conn->type)
    {
        case xjdb_conn_none:
            break;

        case xjdb_conn_tcp:
            if (conn->u.as_tcp.fd != -1)
            {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
                close(conn->u.as_tcp.fd);
#elif defined(PLATFORM_WINDOWS)
                closesocket(conn->u.as_tcp.fd);
#endif
            }
            conn->u.as_tcp.fd = -1;
            conn->type = xjdb_conn_none;
            break;

        case xjdb_conn_serial:
            conn->u.as_serial.fd = -1;
            conn->type = xjdb_conn_none;
            break;
    }
}


static int _readable(int fd)
{
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    retval = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (retval == -1) return -1;
    else if (retval) return 1;

    return 0;
}

/* Readable */
int xjdb_conn_readable(xjdb_conn_t *conn)
{
    int ret = 0;

    switch (conn->type)
    {
        case xjdb_conn_none:
            ret = -1;
            break;

        case xjdb_conn_tcp:
            ret = _readable(conn->u.as_tcp.fd);
            break;

        case xjdb_conn_serial:
            ret = -1;
            break;
    }

    return ret;
}

int xjdb_conn_recv(xjdb_conn_t *conn, void *buf, xjdb_size_t len)
{
    int ret = 0;

    switch (conn->type)
    {
        case xjdb_conn_none:
            ret = -1;
            break;

        case xjdb_conn_tcp:
#if defined(PLATFORM_LINUX)
            ret = (int)recv(conn->u.as_tcp.fd, buf, len, MSG_NOSIGNAL);
#elif defined(PLATFORM_WINDOWS) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
            ret = (int)recv(conn->u.as_tcp.fd, buf, len, 0);
#endif
            break;

        case xjdb_conn_serial:
            ret = -1;
            break;
    }

    return ret;
}

int xjdb_conn_send(xjdb_conn_t *conn, const void *buf, xjdb_size_t len)
{
    int ret = 0;

    switch (conn->type)
    {
        case xjdb_conn_none:
            ret = -1;
            break;

        case xjdb_conn_tcp:
            /* MSG_NOSIGNAL: SIGPIPE rised when the remote site closes the connection which
             * may causes the termination of the process */
#if defined(PLATFORM_LINUX)
            ret = (int)send( \
                    conn->u.as_tcp.fd, buf, len, MSG_NOSIGNAL);
#elif defined(PLATFORM_WINDOWS) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
            ret = (int)send( \
                    conn->u.as_tcp.fd, buf, len, 0);
#endif
            break;

        case xjdb_conn_serial:
            ret = -1;
            break;
    }

    return ret;
}

