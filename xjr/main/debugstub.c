/* Debug Stub */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(linux)
# define DEBUGSTUB_TARGET_LINUX
#elif defined(__FreeBSD__)
# define DEBUGSTUB_TARGET_FREEBSD
#elif defined(__APPLE__)
# define DEBUGSTUB_TARGET_MACOS
#elif (defined(__GNUC__) && \
        (defined(__MINGW32__) || \
         defined(__MINGW64__) || \
         defined(__MSYS__) || \
         defined(__CYGWIN__))) || \
         defined(_MSC_VER)
# define DEBUGSTUB_TARGET_WINDOWS
#else
# define DEBUGSTUB_TARGET_UNKNOWN
#endif


#if defined(DEBUGSTUB_TARGET_LINUX) || defined(DEBUGSTUB_TARGET_FREEBSD) || defined(DEBUGSTUB_TARGET_MACOS)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#elif defined(DEBUGSTUB_TARGET_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

#endif

#include "debugstub.h"


static int set_reuse(int fd, int enabled)
{
    int optval;
    optval = enabled ? 1 : 0;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
    return 0;
}

static int set_blocking(int fd, int enabled)
{
#if defined(DEBUGSTUB_TARGET_LINUX) || defined(DEBUGSTUB_TARGET_FREEBSD) || defined(DEBUGSTUB_TARGET_MACOS)
   int flags = fcntl(fd, F_GETFL, 0);
   flags = enabled ? (flags &~ O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? 0 : -1;
#elif defined(DEBUGSTUB_TARGET_WINDOWS)
	ULONG NonBlock = enabled ? 0 : 1;
	if (ioctlsocket(fd, FIONBIO, &NonBlock) == SOCKET_ERROR) return -1;
	return 0;
#endif
}

static int is_readable(int fd, int timeout_usec)
{
    fd_set fd_read;
    struct timeval timeout = {0, timeout_usec};
    int retval;

    FD_ZERO(&fd_read);
    FD_SET(fd, &fd_read);

    retval = select(fd + 1, &fd_read, NULL, NULL, &timeout);
    return retval > 0 ? 1 : 0;
}

static int socket_close(int fd)
{
#if defined(DEBUGSTUB_TARGET_LINUX) || defined(DEBUGSTUB_TARGET_FREEBSD) || defined(DEBUGSTUB_TARGET_MACOS)
    close(fd);
#elif defined(DEBUGSTUB_TARGET_WINDOWS)
	closesocket(fd);
#endif
	return 0;
}

static int create_tcp_server(const char *host, const int port)
{
    int fd = -1;
    struct sockaddr_in serveraddr;

    /* Fill address */
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons((uint16_t)port);
    if (host == NULL || strlen(host) == 0)
    {
        serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        serveraddr.sin_addr.s_addr = inet_addr(host);
    }

    /* Create socket */
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    { goto fail; }
    set_reuse(fd, 1);
    set_blocking(fd, 0);

    /* Bind address */
    if (bind(fd, \
            (struct sockaddr *)&serveraddr, \
            sizeof(serveraddr)) != 0)
    { goto fail; }

    if (listen(fd, 5) < 0)
    { goto fail; }

    return fd;
fail:
    if (fd != -1) socket_close(fd);
    return -1;
}

int debugstub_init(debugstub_t *debugstub)
{
    debugstub->fd_server = -1;
    debugstub->fd_client = -1;
    debugstub->program_state = debugstub_program_state_pause;
    debugstub->client_state = debugstub_client_state_waiting;
    debugstub->buf_recv = NULL;
    debugstub->buf_retrans = NULL;
    return 0;
}

int debugstub_start(debugstub_t *debugstub, const char *host, const int port)
{
    int fd;

    if ((fd = create_tcp_server(host, port)) < 0)
    { return -1; }

    debugstub->fd_server = fd;

    if ((debugstub->buf_recv = sb_new()) == NULL) { return -1; }
    if ((debugstub->buf_retrans = sb_new()) == NULL) { return -1; }

    return 0;
}

void debugstub_uninit(debugstub_t *debugstub)
{
    if (debugstub->fd_server != -1)
    { socket_close(debugstub->fd_server); }

    if (debugstub->fd_client != -1)
    { socket_close(debugstub->fd_client); }

    if (debugstub->buf_recv != NULL)
    { sb_destroy(debugstub->buf_recv); }

    if (debugstub->buf_retrans != NULL)
    { sb_destroy(debugstub->buf_retrans); }
}

static int hexlit_to_raw(xjr_u8 *dst, const char *src, const xjr_size_t len)
{
    xjr_size_t i;
    xjr_u8 *dst_p = dst;
    const char *src_p = src;

    if ((len & 1) != 0) { return -1; }

    for (i = 0; i != len; i += 2)
    {
        xjr_u8 t_hi, t_lo;

#define _x(_d, _s) \
        do { \
            if ('0' <= _s && _s <= '9') { _d = (xjr_u8)(_s - '0'); } \
            else if ('a' <= _s && _s <= 'f') { _d = (xjr_u8)(_s - 'a' + 10); } \
            else if ('A' <= _s && _s <= 'F') { _d = (xjr_u8)(_s - 'A' + 10); } \
            else return -1; \
        } while (0)
        _x(t_hi, *src_p); src_p++;
        _x(t_lo, *src_p); src_p++;
#undef _x

        *dst_p++ = (xjr_u8)((t_hi << 4) | (t_lo));
    }

    return (int)(dst_p - dst);
}

static int debugstub_send_raw(debugstub_t *debugstub, \
        char *s, xjr_size_t len, xjr_bool cache)
{
    if (cache == xjr_true)
    {
        sb_clear(debugstub->buf_retrans);
        sb_append(debugstub->buf_retrans, s, len);
    }
#if defined(DEBUGSTUB_TARGET_LINUX)
    return send(debugstub->fd_client, s, len, MSG_NOSIGNAL) > 0 ? 0 : -1;
#elif defined(DEBUGSTUB_TARGET_FREEBSD) || defined(DEBUGSTUB_TARGET_MACOS)
    return send(debugstub->fd_client, s, len, 0) > 0 ? 0 : -1;
#else
    return send(debugstub->fd_client, s, len, 0) > 0 ? 0 : -1;
#endif
}

static int debugstub_send(debugstub_t *debugstub, \
        char *s, xjr_bool cache)
{
    return debugstub_send_raw(debugstub, s, (xjr_size_t)strlen(s), cache);
}

static int debugstub_send_ack(debugstub_t *debugstub)
{
    return debugstub_send(debugstub, "+", xjr_false);
}

static int debugstub_send_reply_empty(debugstub_t *debugstub)
{
    return debugstub_send(debugstub, "$#00", xjr_true);
}

static int debugstub_decode(debugstub_t *debugstub)
{
    sb_t *buf_recv = debugstub->buf_recv;
    /* sb_t *buf_retrans = debugstub->buf_retrans; */
    char *buf;

    for (;;)
    {
        if (sb_size(buf_recv) == 0) break;

        /* First */
        buf = sb_buf(buf_recv);
        if (buf[0] == '+')
        {
            /* Packet received correctly */

            /* Shift '+' out */
            sb_shift_one(buf_recv);

            /* Clear retransmission buf */
            sb_clear(debugstub->buf_retrans);
        }
        else if (buf[0] == '-')
        {
            /* Request retransmission */

            /* Shift '-' out */
            sb_shift_one(buf_recv);
        }
        else if (buf[0] == '$')
        {
            int loc_sharp;
            xjr_u8 sum1, sum2;

            /* Command */

            /* Shift '$' out */
            sb_shift_one(buf_recv);

            /* Find the tail of the command */
            if ((loc_sharp = sb_find(buf_recv, '#')) == -1)
            { return 0; }

            /* Checksum */
            if ((int)sb_size(buf_recv) - loc_sharp <= 2)
            { return 0; }
            sum1 = checksum(buf_recv, (xjr_size_t)loc_sharp);
            if (hexlit_to_raw(&sum2, sb_buf(buf_recv) + loc_sharp + 1, 2) < 0)
            {
                return -1;
            }
            if (sum1 != sum2)
            {
                sb_shift(buf_recv, (xjr_size_t)loc_sharp + 3);
                continue;
            }

            /* Ack */
            if (debugstub_send_ack(debugstub) != 0)
            {
                return -1;
            }

            /* No handler */
            if (debugstub_send_reply_empty(debugstub) != 0)
            {
                return -1;
            }

            /* Shift '#' out */
            sb_shift(buf_recv, (xjr_size_t)loc_sharp + 3);
        }
    }

    return 0;
}

static int debugstub_try_recv(debugstub_t *debugstub)
{
    int ret;
    char buffer[1024];

    switch (debugstub->client_state)
    {
        case debugstub_client_state_waiting:
            {
                socklen_t clientlen;
                struct sockaddr_in clientaddr;

                ret = is_readable(debugstub->fd_server, 500);
                if (ret == -1) return -1;
                if (ret == 0) return 0;
                debugstub->fd_client = accept(debugstub->fd_server, (struct sockaddr *)&clientaddr, &clientlen);
                if (debugstub->fd_client < 0)
                { return 0; }

                (void)clientlen;

                debugstub->client_state = debugstub_client_state_established;
            }
            break;

        case debugstub_client_state_established:
            {
                int recv_len;

                ret = is_readable(debugstub->fd_client, 500);
                if (ret == -1)
                {
                    socket_close(debugstub->fd_client);
                    debugstub->fd_client = -1;
                    debugstub->client_state = debugstub_client_state_waiting;
                }
                if (ret == 0) return 0;

#if defined(DEBUGSTUB_TARGET_LINUX)
                recv_len = (int)recv(debugstub->fd_client, buffer, 1024, MSG_NOSIGNAL);
#else
                recv_len = (int)recv(debugstub->fd_client, buffer, 1024, 0);
#endif
                if (recv_len <= 0)
                {
                    socket_close(debugstub->fd_client);
                    debugstub->fd_client = -1;
                    debugstub->client_state = debugstub_client_state_waiting;
                }

                sb_append(debugstub->buf_recv, buffer, (xjr_size_t)recv_len);

                if (debugstub_decode(debugstub) != 0)
                {
                    socket_close(debugstub->fd_client);
                    debugstub->fd_client = -1;
                    debugstub->client_state = debugstub_client_state_waiting;
                }
            }
            break;
    }

    return 0;
}

int debugstub_cb(xjr_debugger_ctx *ctx)
{
    debugstub_t *debugstub = xjr_debugger_stub_get(ctx);

    if (debugstub_try_recv(debugstub) != 0) { return -1; }

    return 0;
}

