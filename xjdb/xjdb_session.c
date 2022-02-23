/* XenonJS : Debugger : Session
 * Copyright(c) 2018 y2c2 */

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ec_alloc.h>
#include <ec_string.h>
#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include "ipparse.h"
#include "thread.h"
#include "hbuf.h"
#include "xjdb_conn.h"
#include "xjdb_protocol.h"
#include "xjdb_utils.h"
#include "xjdb_session.h"

typedef enum
{
    xjdb_session_state_idle,
    xjdb_session_state_connected,
} xjdb_session_state_t;

struct xjdb_opaque_session
{
    char *bytecode;
    xjdb_size_t bytecode_len;
    ipparse_t remote_target;

    xjdb_session_log_level level;
    ec_string *err_desc;

    xjdb_conn_t *conn;
    hbuf_t *recv_buf;

    xjdb_session_state_t state;

    xjdb_thread_t thd;
    xjdb_bool halt_flag;
    xjdb_bool halted;
};

#define XJDB_UPDATE_ERROR_BUF_SIZE 1024

static int xjdb_update_error_puts(xjdb_session_t *sess, \
        xjdb_session_log_level level, const char *s)
{
    ec_encoding_t enc;
    ec_string *u_s;
    ec_encoding_utf8_init(&enc);
    if (ec_encoding_decode(&enc, &u_s, (const ec_byte_t *)s, strlen(s)) != 0)
    { return -1; }
    ec_delete(sess->err_desc);
    sess->err_desc = u_s;
    sess->level = level;
    return 0;
}

static int xjdb_update_error_printf(xjdb_session_t *sess, \
        xjdb_session_log_level level, const char *fmt, ...)
{
    va_list ap;
    char buf[XJDB_UPDATE_ERROR_BUF_SIZE + 1];
    int size;

    va_start(ap, fmt);
    size = vsnprintf(buf, XJDB_UPDATE_ERROR_BUF_SIZE, fmt, ap);
    va_end(ap);
    if (size <= 0) return -1;

    return xjdb_update_error_puts(sess, level, buf);
}

ec_string *xjdb_session_get_error_desc(xjdb_session_t *sess)
{
    return sess->err_desc;
}

xjdb_session_log_level xjdb_session_get_error_level(xjdb_session_t *sess)
{
    return sess->level;
}

void xjdb_session_clear_error(xjdb_session_t *sess)
{
    ec_delete(sess->err_desc);
    sess->err_desc = NULL;
    sess->level = xjdb_session_log_none;
}

static int read_file( \
        char **data_out, xjdb_size_t *len_out, \
        const char *filename)
{
    FILE *fp;
    char *data;
    long len;
    int from_terminal = ((strlen(filename) == 1) && (strncmp(filename, "-", 1) == 0));

    if (from_terminal)
    {
        fp = stdin;
    }
    else
    {
        if ((fp = fopen(filename, "rb")) == NULL)
        { return -1; }
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    if (len < 0)
    {
        fclose(fp);
        return -1;
    }
    fseek(fp, 0, SEEK_SET);

    if ((data = (char *)malloc(sizeof(char) * ((size_t)len + 1))) == NULL)
    { 
        if (!from_terminal) fclose(fp);
        return -1;
    }
    fread(data, (size_t)len, 1, fp);
    data[len] = '\0';

    *data_out = data;
    *len_out = (xjdb_size_t)len;

    if (!from_terminal) fclose(fp);

    return 0;
}

static void xjdb_session_ctor(void *data)
{
    xjdb_session_t *sess = data;
    sess->bytecode = NULL;
    sess->bytecode_len = 0;
    sess->err_desc = NULL;
    sess->level = xjdb_session_log_none;
    sess->state = xjdb_session_state_idle;
    sess->conn = xjdb_conn_new();
    sess->recv_buf = hbuf_new();
    sess->halt_flag = xjdb_false;
    sess->halted = xjdb_false;
}

static void xjdb_session_dtor(void *data)
{
    xjdb_session_t *sess = data;
    if (sess->bytecode != NULL) free(sess->bytecode);
    ec_delete(sess->err_desc);
    if (sess->conn != NULL) xjdb_conn_destroy(sess->conn);
    hbuf_destroy(sess->recv_buf);
}

xjdb_session_t *xjdb_session_new(void)
{
    xjdb_session_t *sess = ec_newcd(xjdb_session_t, \
            xjdb_session_ctor, 
            xjdb_session_dtor);
    return sess;
}

int xjdb_session_prepare(xjdb_session_t *sess, \
        const char *remote_target, \
        const char *xfile_pathname)
{
    int ret = 0;

    if (ipparse_parse(&sess->remote_target, remote_target, (int)strlen(remote_target)) != 0)
    {
        xjdb_update_error_puts(sess, xjdb_session_log_error, \
                "invalid remote target");
        goto fail;
    }

    ret = read_file(&sess->bytecode, &sess->bytecode_len, xfile_pathname);
    if (ret != 0)
    {
        xjdb_update_error_printf(sess, xjdb_session_log_error, \
                "failed to read data from %s", xfile_pathname);
        goto fail;
    }

fail:
    return ret;
}

static void *xjdb_session_loop(void *data)
{
    xjdb_session_t *sess = (xjdb_session_t *)data;
    (void)sess;

    while (sess->halt_flag == xjdb_false)
    {
        switch (sess->state)
        {
            case xjdb_session_state_idle:
                if (xjdb_conn_connect(sess->conn, \
                            sess->remote_target.u.as_network.host, \
                            (xjdb_size_t)sess->remote_target.u.as_network.port) == 0)
                { sess->state = xjdb_session_state_connected; }
                break;

            case xjdb_session_state_connected:
                if (xjdb_conn_alive(sess->conn) == xjdb_false)
                {
                    xjdb_conn_close(sess->conn);
                    sess->state = xjdb_session_state_idle;
                    hbuf_clear(sess->recv_buf);
                    break;
                }

                if (xjdb_conn_readable(sess->conn) == xjdb_false)
                { break; }

                {
                    char buf[512 + 1];
                    int len;
                    len = xjdb_conn_recv(sess->conn, buf, 512);
                    if (len <= 0)
                    {
                        xjdb_conn_close(sess->conn);
                        sess->state = xjdb_session_state_idle;
                        hbuf_clear(sess->recv_buf);
                    }
                    else
                    {
                        hbuf_append(sess->recv_buf, (unsigned char *)buf, (unsigned int)len);

                        hbuf_left_strip(sess->recv_buf, '$');
                        while (hbuf_size(sess->recv_buf) != 0)
                        {
                            /* Try decoding */
                            xjdb_protocol_decode_result_t result;

                            result = xjdb_protocol_try_decode(sess->recv_buf);
                            switch (result)
                            {
                                case XJDB_PROTOCOL_DECODE_OK:
                                    /* Decoded */
                                    printf("DECODED\n"); fflush(stdout);
                                    hbuf_shift_one(sess->recv_buf);
                                    hbuf_left_strip(sess->recv_buf, '$');
                                    break;
                                case XJDB_PROTOCOL_DECODE_DEFER:
                                    /* Nothing to be done right now*/
                                    break;
                                case XJDB_PROTOCOL_DECODE_ERROR:
                                    hbuf_shift_one(sess->recv_buf);
                                    hbuf_left_strip(sess->recv_buf, '$');
                                    break;
                            }
                            if (result == XJDB_PROTOCOL_DECODE_DEFER) { break; }
                        }
                    }
                }
                break;
        }

        xjdb_sleep(); 
    }

    sess->halted = xjdb_true;

    return NULL;
}

int xjdb_session_fork_loop(xjdb_session_t *sess)
{
    if (thread_init(&sess->thd, xjdb_session_loop, sess) != 0)
    {
        xjdb_update_error_printf(sess, xjdb_session_log_error, \
                "failed to init thread");
        return -1;
    }

    return 0;
}

int xjdb_session_loop_halt(xjdb_session_t *sess)
{
    void *retval;
    sess->halt_flag = xjdb_true;
    while (sess->halted == xjdb_false) { xjdb_sleep(); }
    thread_join(&(sess->thd), &retval);
    return 0;
}

int xjdb_session_start(xjdb_session_t *sess)
{
    /* int ret = 0; */

    /*
    switch (sess->state)
    {
        case xjdb_session_state_idle:
            if (xjdb_conn_connect(sess->conn, \
                    sess->remote_target.u.as_network.host, \
                    (xjdb_size_t)sess->remote_target.u.as_network.port) != 0)
            {
                xjdb_update_error_printf(sess, \
                        xjdb_session_log_warning, \
                        "connect to %s:%d failed", \
                        sess->remote_target.u.as_network.host, \
                        sess->remote_target.u.as_network.port);
                ret = -1;
            }
            else
            {
                sess->state = xjdb_session_state_connected;
            }
            break;

        case xjdb_session_state_connected:
            xjdb_update_error_printf(sess, \
                    xjdb_session_log_warning, \
                    "already connect to %s:%d", \
                    sess->remote_target.u.as_network.host, \
                    sess->remote_target.u.as_network.port);
            ret = -1;
            break;
    }
    */

    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'start' not implemented");
    return -1;
}

int xjdb_session_continue(xjdb_session_t *sess)
{
    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'continue' not implemented");
    return -1;
}

int xjdb_session_step_in(xjdb_session_t *sess)
{
    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'step in' not implemented");
    return -1;
}

int xjdb_session_step_over(xjdb_session_t *sess)
{
    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'step over' not implemented");
    return -1;
}

int xjdb_session_finish(xjdb_session_t *sess)
{
    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'finish' not implemented");
    return -1;
}

int xjdb_session_backtrace(xjdb_session_t *sess)
{
    xjdb_update_error_printf(sess, xjdb_session_log_warning, "'backtrace' not implemented");
    return -1;
}

int xjdb_session_list(xjdb_session_t *sess)
{
    int ret = 0;

    switch (sess->state)
    {
        case xjdb_session_state_idle:
            xjdb_update_error_puts(sess, \
                    xjdb_session_log_warning, \
                    "not connected");
            ret = -1;
            break;

        case xjdb_session_state_connected:
            ret = xjdb_protocol_send_list(sess->conn);
            break;
    }

    return ret;
}

