/* XenonJS
 * Copyright(c) 2017-2018 y2c2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argsparse.h"
#include "sourcelist.h"
#include "xjs.h"
#include "xjr.h"

#define XJS_VERSION "0.0.1"

static void init_ec(void)
{
    ec_allocator_set_malloc(malloc);
    ec_allocator_set_calloc(calloc);
    ec_allocator_set_free(free);
    ec_allocator_set_memset(memset);
    ec_allocator_set_memcpy(memcpy);

    xjs_allocator_set_malloc(malloc);
    xjs_allocator_set_calloc(calloc);
    xjs_allocator_set_free(free);
    xjs_allocator_set_memset(memset);
    xjs_allocator_set_memcpy(memcpy);
}

static void show_version(void)
{
    const char *info = ""
        "xjc " XJS_VERSION "\n";
    printf("%s", info);
}

static void show_help(void)
{
    const char *info = ""
        "Usage: xjs [options] <script.js>\n"
        "\n"
        "  -h, --help                   Show help information\n"
        "  --version                    Show version information\n"
        "";
    puts(info);
}

static int launch_script(const char *script)
{
    int ret = 0;
    (void)script;
    return ret;
}

static int launch_repl(void)
{
    int ret = 0;
    fprintf(stderr, "error: REPL current not supported\n");
    return ret;
}

typedef enum
{
    work_mode_script,
    work_mode_repl,
} work_mode_t;

int main(int argc, char *argv[])
{
    int ret = 0;
    argsparse_t argsparse;
    const char *source_path = NULL;
    work_mode_t work_mode = work_mode_repl;

    init_ec();
    argsparse_init(&argsparse, argc, argv);

    if (argsparse_available(&argsparse) == 0)
    { fprintf(stderr, "error: no input file\n"); ret = -1; goto fail; }

    while (argsparse_available(&argsparse) != 0)
    {
        if (argsparse_match_str(&argsparse, "--help") || \
                argsparse_match_str(&argsparse, "-h"))
        { show_help(); goto done; }
        if (argsparse_match_str(&argsparse, "--version"))
        { show_version(); goto done; }
        else
        {
            source_path = argsparse_fetch(&argsparse);
            if (source_path != NULL)
            { fprintf(stderr, "error: multiple source\n"); ret = -1; goto fail; }
            argsparse_next(&argsparse);
        }
    }

    if (source_path == NULL) { work_mode = work_mode_repl; }
    else { work_mode = work_mode_script; }

    switch (work_mode)
    {
        case work_mode_repl:
            ret = launch_repl();
            break;

        case work_mode_script:
            ret = launch_script(source_path);
            break;
    }

fail:
done:
    return ret;
}

