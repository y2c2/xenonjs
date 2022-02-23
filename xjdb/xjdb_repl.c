/* XenonJS : Debugger : REPL
 * Copyright(c) 2018 y2c2 */

#include <string.h>
#include <ec_alloc.h>
#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include "cmdparse.h"
#include "xjdb_repl.h"

#define CMD_BUF_SIZE (512)

static int strtrim(char *buf, int len)
{
    char *p = buf;

    while ((len != 0) && \
            ((*p == '\t') || (*p == ' ')))
    { p++; len--; }

    if (p != buf)
    {
        int i;
        for (i = 0; i != len; i++)
        { buf[i] = p[i]; }
        buf[len] = '\0';
    }

    p = buf + len - 1;
    while ((len != 0) && \
            ((*p == '\t') || (*p == ' ')))
    { p--; len--; }

    return len;
}

typedef struct
{
    xjdb_session_t *sess;
    char *cmd;
    int cmd_len;
    int *halt_flag;
} cmd_handler_ctx;

typedef int (*cmd_handler)(cmd_handler_ctx *ctx);

typedef struct
{
    const char *name;
    int argc_min;
    int argc_max;
    cmd_handler handler;
} cmd_item;

static int cmd_handler_start(cmd_handler_ctx *ctx)
{
    if (xjdb_session_start(ctx->sess) != 0) { return -1; }

    return 0;
}

static int cmd_handler_list(cmd_handler_ctx *ctx)
{
    if (xjdb_session_list(ctx->sess) != 0) { return -1; }

    return 0;
}

static int cmd_handler_continue(cmd_handler_ctx *ctx)
{
    if (xjdb_session_continue(ctx->sess) != 0) { return -1; }

    return 0;
}

static int cmd_handler_quit(cmd_handler_ctx *ctx)
{
    *ctx->halt_flag = 1;
    return 0;
}

static int cmd_process(xjdb_session_t *sess, \
        char *cmd, int cmd_len, \
        int *halt_flag)
{
    int ret = 0;
    cmdparse_t cmdparse;
    static const cmd_item items[] = {
        { "start", 1, 1, cmd_handler_start },
        { "list", 1, 1, cmd_handler_list },
        { "l", 1, 1, cmd_handler_list },
        { "continue", 1, 1, cmd_handler_continue },
        { "quit", 1, 1, cmd_handler_quit },
        { "q", 1, 1, cmd_handler_quit }
    };
    static const int items_count = sizeof(items) / sizeof(cmd_item);

    if (cmdparse_parse(&cmdparse, cmd, (int)cmd_len) != 0)
    {
        fprintf(stderr, "error: invalid command '%s'\n", cmd);
        goto fail;
    }

    {
        const cmd_item *target_item = NULL;
        int i;
        for (i = 0; i != items_count; i++)
        {
            if ((items[i].argc_min <= cmdparse.argc) && \
                    (cmdparse.argc <= items[i].argc_max) && \
                    (strcmp(items[i].name, cmdparse.argv[0].s) == 0))
            {
                target_item = &items[i];
                break;
            }
        }
        if (target_item == NULL)
        {
            fprintf(stderr, "error: unknown command '%s'\n", cmd);
            goto fail;
        }
        {
            cmd_handler_ctx ctx;
            ctx.sess = sess;
            ctx.cmd = cmd;
            ctx.cmd_len = cmd_len;
            ctx.halt_flag = halt_flag;
            ret = target_item->handler(&ctx);
            if (ret != 0) { goto fail; }
        }
    }

    goto done;
fail:
    {
        ec_string *desc = xjdb_session_get_error_desc(sess);
        if (desc != NULL)
        {
            ec_byte_t *encoded_desc;
            ec_size_t encoded_desc_len;
            ec_encoding_t enc;
            ec_encoding_utf8_init(&enc);
            ec_encoding_encode(&enc, (ec_byte_t **)&encoded_desc, &encoded_desc_len, desc);
            switch (xjdb_session_get_error_level(sess))
            {
                case xjdb_session_log_none:
                    ret = 0;
                    break;
                case xjdb_session_log_info:
                    fputs("info: ", stderr);
                    ret = 0;
                    break;
                case xjdb_session_log_warning:
                    fputs("warning: ", stderr);
                    ret = 0;
                    break;
                case xjdb_session_log_error:
                    fputs("error: ", stderr);
                    break;
            }
            fwrite(encoded_desc, encoded_desc_len, 1, stderr);
            fputc('\n', stderr);
            fflush(stderr);
            ec_free(encoded_desc);
            xjdb_session_clear_error(sess);
        }
    }
done:
    return ret;
}

int xjdb_repl_start(xjdb_session_t *sess)
{
    int ret = 0;
    int halt_flag = 0;

    while (halt_flag == 0)
    {
        char cmd[CMD_BUF_SIZE + 1];
        int cmd_len;

        printf("(xjdb) ");

        if (fgets(cmd, CMD_BUF_SIZE, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_len = (int)strlen(cmd);
        if (cmd_len == 0) {}
        else
        {
            if (cmd[cmd_len - 1] == '\n')
            { cmd[cmd_len - 1] = '\0'; cmd_len--; }
            if (cmd_len == 0) { continue; }
            cmd_len = strtrim(cmd, cmd_len);
            if (cmd_len == 0) { continue; }
        }

        if (cmd_process(sess, cmd, cmd_len, &halt_flag) != 0)
        { break; }
        if (halt_flag != 0) { break; }
    }

    return ret;
}

