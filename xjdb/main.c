/* XenonJS : Debugger
 * Copyright(c) 2018 y2c2 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ec_alloc.h>
#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include "argsparse.h"
#include "xjdb_session.h"
#include "xjdb_repl.h"
#include "xjdb_daemon.h"

static void show_help(void)
{
    const char *info = ""
        "usage: xjdb [options] <source.x>\n"
        "  --remote-target   <addr>           Debug remote program\n"
        "  --controller      <addr>           Listen address of RESTful controller\n"
        "    addr: serial://<serial-device>\n"
        "          tcp://<host:port>\n"
        "          udp://<host:port>\n"
        "    controller only allows TCP type of address\n"
        "  --repl                             Interactive shell\n"
        "  --daemon                           RESTful controller\n"
        "\n"
        "  --help                             Show help information\n"
        "\n";
    printf("%s", info);
}

typedef enum
{
    work_mode_repl,
    work_mode_daemon
} work_mode_t;

static int start_debugger( \
        const work_mode_t work_mode, \
        const char *remote_target, \
        const char *controller, \
        const char *xfile_pathname)
{
    int ret = 0;
    xjdb_session_t *sess = NULL;

    if (remote_target == NULL)
    {
        fprintf(stderr, "error: remote target missing\n");
        goto fail;
    }

    if ((sess = xjdb_session_new()) == NULL)
    {
        fprintf(stderr, "error: out of memory\n");
        goto fail;
    }

    if (xjdb_session_prepare(sess, \
                remote_target, xfile_pathname) != 0)
    { goto fail; }

    if (xjdb_session_fork_loop(sess) != 0)
    { goto fail; }

    switch (work_mode)
    {
        case work_mode_repl:
            ret = xjdb_repl_start(sess);
            break;

        case work_mode_daemon:
            if (controller == NULL)
            {
                fprintf(stderr, "error: controller missing\n");
                goto fail;
            }
            ret = xjdb_daemon_start(sess, controller);
            break;
    }

    xjdb_session_loop_halt(sess);

    goto done;
fail:
    if (sess != NULL)
    {
        ec_string *desc = xjdb_session_get_error_desc(sess);
        ec_byte_t *encoded_desc;
        ec_size_t encoded_desc_len;
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_desc, &encoded_desc_len, desc);
        fputs("error: ", stderr);
        fwrite(encoded_desc, encoded_desc_len, 1, stderr);
        fputc('\n', stderr);
        fflush(stderr);
        ec_free(encoded_desc);
    }
done:
    ec_delete(sess);
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    argsparse_t argsparse;
    work_mode_t work_mode = work_mode_repl;
    char *remote_target = NULL;
    char *controller = NULL;
    char *xfile_pathname = NULL;

    ec_allocator_set_malloc(malloc);
    ec_allocator_set_calloc(calloc);
    ec_allocator_set_free(free);
    ec_allocator_set_memset(memset);
    ec_allocator_set_memcpy(memcpy);

    argsparse_init(&argsparse, argc, argv);

    if (argsparse_available(&argsparse) == 0)
    {
        fprintf(stderr, "error: no input file\n");
        return -1;
    }

    while (argsparse_available(&argsparse) != 0)
    {
        if (argsparse_match_str(&argsparse, "--help") || \
                argsparse_match_str(&argsparse, "-h"))
        { show_help(); goto done; }
        else if (argsparse_match_str(&argsparse, "--remote-target"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            {
                fprintf(stderr, "error: no remote target\n");
                goto fail;
            }
            remote_target = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--controller"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            {
                fprintf(stderr, "error: no controller\n");
                goto fail;
            }
            controller = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--repl"))
        {
            argsparse_next(&argsparse);
            work_mode = work_mode_repl;
        }
        else if (argsparse_match_str(&argsparse, "--daemon"))
        {
            argsparse_next(&argsparse);
            work_mode = work_mode_daemon;
        }
        else
        {
            if (xfile_pathname != NULL)
            {
                fprintf(stderr, "error: multiple source file\n");
                goto fail;
            }
            else
            {
                xfile_pathname = argsparse_fetch(&argsparse);
                argsparse_next(&argsparse);
            }
        }
    }

    if (xfile_pathname == NULL)
    {
        fprintf(stderr, "error: no input file\n");
        goto fail;
    }

    ret = start_debugger( \
            work_mode, remote_target, controller, xfile_pathname);

    goto done;
fail:
done:
    return ret;
}

