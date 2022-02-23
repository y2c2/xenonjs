/* XenonJS : Debugger : Session
 * Copyright(c) 2018 y2c2 */

#ifndef XJDB_SESSION_H
#define XJDB_SESSION_H

#include <ec_string.h>
#include "xjdb_dt.h"

struct xjdb_opaque_session;
typedef struct xjdb_opaque_session xjdb_session_t;

typedef enum
{
    xjdb_session_log_none,
    xjdb_session_log_info,
    xjdb_session_log_warning,
    xjdb_session_log_error,
} xjdb_session_log_level;

xjdb_session_t *xjdb_session_new(void);
void xjdb_session_destroy(xjdb_session_t *sess);

ec_string *xjdb_session_get_error_desc(xjdb_session_t *sess);
xjdb_session_log_level xjdb_session_get_error_level(xjdb_session_t *sess);
void xjdb_session_clear_error(xjdb_session_t *sess);

int xjdb_session_prepare(xjdb_session_t *sess, \
        const char *remote_target, \
        const char *xfile_pathname);

int xjdb_session_fork_loop(xjdb_session_t *sess);
int xjdb_session_loop_halt(xjdb_session_t *sess);

int xjdb_session_start(xjdb_session_t *sess);
int xjdb_session_continue(xjdb_session_t *sess);
int xjdb_session_step_in(xjdb_session_t *sess);
int xjdb_session_step_over(xjdb_session_t *sess);
int xjdb_session_finish(xjdb_session_t *sess);
int xjdb_session_backtrace(xjdb_session_t *sess);
int xjdb_session_list(xjdb_session_t *sess);

#endif

