/* Debug Stub */

#ifndef DEBUGSTUB_H
#define DEBUGSTUB_H

#include "xjr.h"
#include "streambuf.h"

struct debugstub;
typedef struct debugstub debugstub_t;

typedef enum
{
    debugstub_program_state_pause,
    debugstub_program_state_running,
} debugstub_program_state_t;

typedef enum
{
    debugstub_client_state_waiting,
    debugstub_client_state_established,
} debugstub_client_state_t;

struct debugstub
{
    int fd_server, fd_client;
    debugstub_program_state_t program_state;
    debugstub_client_state_t client_state;

    sb_t *buf_recv, *buf_retrans;

    /* Break points */
};

int debugstub_init(debugstub_t *debugstub);
int debugstub_start(debugstub_t *debugstub, const char *host, const int port);
void debugstub_uninit(debugstub_t *debugstub);

int debugstub_cb(xjr_debugger_ctx *ctx);

#endif

