/* XenonJS : Compiler Driver
 * Copyright(c) 2017-2018 y2c2 */

#ifdef _WIN32
# include <io.h>
# include <fcntl.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ec_algorithm.h>
#include <ec_string.h>
#include <ec_encoding.h>
#include <ec_alloc.h>
#include "sourcelist.h"
#include "xjs.h"
#include "print_error.h"
#include "utils.h"
#include "argsparse.h"

#define XJC_VERSION "0.0.1"

#define XJC_STDLIB_FILENAME "stdlib.js"

#ifdef PATH_MAX
#define XJSCD_PATH_MAX PATH_MAX
#else
#define XJSCD_PATH_MAX 255
#endif

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
        "xjc " XJC_VERSION "\n";
    printf("%s", info);
}

static void show_targets(void)
{
    const char *info = ""
    "Targets:\n"
    "  raw\n"
    "  windows-x86\n"
    "  windows-amd64\n"
    "  linux-x86\n"
    "  linux-amd64\n"
    "";
    puts(info);
}

static void show_help(void)
{
    const char *info = ""
        "Usage: xjc [options] <input>\n"
        "\n"
        "  -c                           Only compile\n"
        "  -g                           Generate debugging symbols\n"
        "  -o        <dest.x|dest.s>    Output file\n"
        "  -O<num>                      Optimization Level\n"
        "  -S                           Output assembly language\n"
        "  -x<lang>                     Specify explicitly the language\n"
        "\n"
        "  --target  <target>           Specify target\n"
        "  --show-targets               Show targets\n"
        "  --entry   <entry.[js|mjs|x]> Specify entry of linking\n"
        "  --no-stdlib                  Ignore standard library while linking\n"
        "  --ret-zero                   Return 0 when compilation failed\n"
        "  --ast-dump                   Dump abstract syntax tree\n"
        "\n"
        "  --list-sys-libs              List available system libraries\n"
        "\n"
        "  -h, --help                   Show help information\n"
        "  --version                    Show version information\n"
        "";
    puts(info);
}

static void list_sys_libs(void)
{
    char **filenames = NULL;
    char buf_pwd[256];
    int filenames_count, i;
    int buf_pwd_len;

    memset(buf_pwd, 0, 256);
    if (current_working_path(buf_pwd, 256) != 0)
    {
        fprintf(stderr, "error: failed to list libraries\n");
        return;
    }
    buf_pwd_len = (int)strlen(buf_pwd);
    buf_pwd[buf_pwd_len] = OS_SEP;
    buf_pwd_len++;
    strcat(buf_pwd, "lib");

    if (list_dir(&filenames, &filenames_count, buf_pwd) != 0)
    {
        fprintf(stderr, "error: failed to list libraries\n");
        return;
    }

    for (i = 0; i < filenames_count; i++)
    {
        fputs(filenames[i], stdout);
        fputc('\n', stdout);

        free(filenames[i]);
    }
    free(filenames);
}

typedef enum
{
    xjc_output_format_x = 0,
    xjc_output_format_s = 1,
} xjc_output_format;

typedef enum
{
    xjc_work_mode_compile = 0,
    xjc_work_mode_link = 1,
    xjc_work_mode_ast_dump = 2,
} xjc_work_mode;

static int gen_ast(xjs_error_ref err, const char *dst, const xjs_ast_program_ref ast)
{
    int ret = 0;
    ec_string *readable_ir_source = NULL;

    /* Print AST */
    if (xjs_astprinter_start(err, &readable_ir_source, ast) != 0)
    { ret = -1; goto fail; }

    /* Append a EOL */
    ec_string_append_c_str(readable_ir_source, "\n");

    {
        char *encoded_result;
        ec_size_t encoded_result_len;

        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, readable_ir_source);

        if (write_file(encoded_result, encoded_result_len, dst) != 0)
        { fprintf(stderr, "error: write to destination file failed\n"); ret = -1; goto fail; }

        ec_free(encoded_result);
    }

fail:
    ec_delete(readable_ir_source);
    return ret;
}

static int gen_s(xjs_error_ref err, const char *dst, const xjs_ir_ref ir, \
        const xjs_bool generate_debug_info)
{
    int ret = 0;
    ec_string *readable_ir_source = NULL;

    /* Print IR */
    if (xjs_irprinter_start_ex(err, &readable_ir_source, ir, generate_debug_info) != 0)
    { ret = -1; goto fail; }

    {
        char *encoded_result;
        ec_size_t encoded_result_len;

        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, readable_ir_source);

        if (write_file(encoded_result, encoded_result_len, dst) != 0)
        { fprintf(stderr, "error: write to destination file failed\n"); ret = -1; goto fail; }

        ec_free(encoded_result);
    }

fail:
    ec_delete(readable_ir_source);
    return ret;
}

static int gen_x(xjs_error_ref err, const char *dst, const xjs_ir_ref ir, \
        const xjs_bool generate_debug_info)
{
    int ret = 0;
    char *bytecode = NULL;
    xjs_size_t bytecode_len;

    if (xjs_c4_start_ex(err, &bytecode, &bytecode_len, ir, \
                generate_debug_info) != 0)
    { ret = -1; goto fail; }

    if (write_file(bytecode, bytecode_len, dst) != 0)
    {
        fprintf(stderr, "error: write to destination file failed\n");
        free(bytecode);
        bytecode = NULL;
        goto fail;
    }

fail:
    if (bytecode != NULL) free(bytecode);
    return ret;
}

static int xjs_load_sys_lib_cb( \
        char **data_out, xjs_size_t *size_out, \
        char **lib_fullpath_out, xjs_size_t *lib_fullpath_len_out, \
        const char *name, const xjs_size_t name_len, \
        xjs_u32 opts)
{
    int ret = 0;
    char exe_path[PATH_MAX];
    char lib_path[PATH_MAX];
    char *p;
    char *source_data = NULL;
    size_t source_len = 0;
    char *lib_fullpath = NULL;
    xjs_size_t lib_fullpath_len = 0;

    (void)opts;
    (void)name_len;

    if (current_program_path(exe_path, PATH_MAX) != 0) { goto fail; }
    p = strrchr(exe_path, OS_SEP);
    if (p == NULL) { goto fail; }
    *p = '\0';
    lib_fullpath_len = (xjs_size_t)snprintf(lib_path, PATH_MAX, "%s%clib%c%s.mjs", exe_path, OS_SEP, OS_SEP, name);

    if ((lib_fullpath = (char *)ec_malloc(sizeof(char) * (lib_fullpath_len + 1))) == NULL)
    { goto fail; }
    memcpy(lib_fullpath, lib_path, lib_fullpath_len);
    lib_fullpath[lib_fullpath_len] = '\0';

    if (read_file(&source_data, &source_len, lib_path) != 0) { goto fail; }

    *data_out = source_data;
    *size_out = source_len;
    *lib_fullpath_out = lib_fullpath;
    *lib_fullpath_len_out = lib_fullpath_len;

    goto done;
fail:
    ret = -1;
    if (lib_fullpath != NULL) ec_free(lib_fullpath);
done:
    return ret;
}

static int compile( \
        const sourcelist_ref sources, const char *dst, \
        const xjc_output_format output_format, \
        const char *entry, \
        const xjc_work_mode work_mode, \
        const xjs_bool generate_debug_info, \
        const xjs_bool no_stdlib)
{
    int ret = 0;
    char decorated_dst[XJSCD_PATH_MAX + 1];

    xjs_ir_ref *irs = NULL;
    xjs_ir_ref ir_merged = NULL;
    ect_iterator(sourcelist) it_src;
    ec_size_t srcs_count = 0, i;
    xjs_error err;
    const char *source_first = NULL;
    xjs_ir_ref new_ir = NULL;

    xjs_error_init(&err);

    if (work_mode == xjc_work_mode_link)
    {
        srcs_count = ect_list_size(sourcelist, sources);
        if ((irs = (xjs_ir_ref *)malloc(sizeof(xjs_ir_ref) * srcs_count)) == NULL)
        { fprintf(stderr, "error: out of memory\n"); return -1; }
        for (i = 0; i != srcs_count; i++) { irs[i] = NULL; }
    }

    i = 0;
    ect_for(sourcelist, sources, it_src)
    {
        ec_string *uscript = NULL;
        char *source_data = NULL;
        size_t source_len;
        xjs_token_list_ref tokens = NULL;
        xjs_ast_program_ref ast = NULL;
        xjs_cfg_ref cfg = NULL;
        xjc_source_ref source = ect_deref(xjc_source_ref, it_src);

        if (source_first == NULL) source_first = source->source_path;

        /* Read source file */
        if (read_file(&source_data, &source_len, source->source_path) != 0)
        {
            fprintf(stderr, "error: failed to open file %s\n", source->source_path);
            goto fail;
        }

        if (source->filetype == xjc_filetype_auto)
        {
            /* Which format? */
            const char *p_end = source->source_path + strlen(source->source_path);
            const char *p_dot = strrchr(source->source_path, '.');
            if (p_dot == NULL)
            {
                fprintf(stderr, "error: file not recognized %s\n", source->source_path);
                free(source_data);
                goto fail;
            }
            if ((p_end - p_dot >= 4) && (strncmp(p_dot, ".mjs", 4) == 0))
            {
                source->filetype = xjc_filetype_mjs;
            }
            else if ((p_end - p_dot >= 3) && (strncmp(p_dot, ".js", 3) == 0))
            {
                source->filetype = xjc_filetype_js;
            }
            else if ((p_end - p_dot >= 2) && (strncmp(p_dot, ".x", 2) == 0))
            {
                source->filetype = xjc_filetype_x;
            }
            else
            {
                fprintf(stderr, "error: file not recognized %s\n", source->source_path);
                free(source_data);
                goto fail;
            }
        }

        if ((source->filetype == xjc_filetype_js) || (source->filetype == xjc_filetype_mjs))
        {
            /* Decode */
            {
                int tmpret;
                ec_encoding_t enc;
                ec_encoding_utf8_init(&enc);
                tmpret = ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)source_data, source_len);
                free(source_data); source_data = NULL;
                if (tmpret != 0)
                { fprintf(stderr, "error: invalid source file encoding (only supports UTF-8)\n"); goto fail; }
                if (uscript == NULL)
                { fprintf(stderr, "error: internal error on encoding convertion\n"); goto fail; }
            }

            /* Tokenize */
            tokens = xjs_lexer_start_ex(&err, uscript, \
                    source->source_path);
            ec_delete(uscript);
            if (tokens == NULL) { goto fail; }
            uscript = NULL;

            /* Parse */
            ast = xjs_parser_start_ex(&err, tokens, \
                    (source->filetype == xjc_filetype_mjs ? xjs_true : xjs_false), \
                    source->source_path, \
                    generate_debug_info ? xjs_true : xjs_false);
            ec_delete(tokens);
            if (ast == NULL) { goto fail; }
            tokens = NULL;

            if (work_mode == xjc_work_mode_ast_dump)
            {
                int retval = 0;
                if (dst == NULL)
                {
                    if (fork_path( \
                                decorated_dst, XJSCD_PATH_MAX, \
                                "ast", source->source_path) != 0)
                    { fprintf(stderr, "error: internal error\n"); goto fail; }
                    retval = gen_ast(&err, decorated_dst, ast);
                }
                else
                {
                    retval = gen_ast(&err, dst, ast);
                }
                ec_delete(ast);
                if(retval != 0) goto fail;
                goto done;
            }

            /* C0 */
            cfg = xjs_c0_start_ex(&err, ast, \
                    source->source_path);
            ec_delete(ast);
            if (cfg == NULL) { goto fail; }
            ast = NULL;

            /* Generate IR */
            new_ir = xjs_c2_start_ex(&err, cfg, \
                    source->source_path);
            ec_delete(cfg);
            if (new_ir == NULL) { goto fail; }
            cfg = NULL;
        }
        else if (source->filetype == xjc_filetype_x)
        {
            if (xjs_l0_start(&err, &new_ir, source_data, source_len) != 0)
            {
                free(source_data);
                goto fail;
            }
        }
        else
        { fprintf(stderr, "error: internal error\n"); goto fail; }

        if (work_mode == xjc_work_mode_compile)
        {
            /* Output */
            switch (output_format)
            {
                case xjc_output_format_s:
                    if (dst == NULL)
                    {
                        if (fork_path( \
                                decorated_dst, XJSCD_PATH_MAX, \
                                "s", source->source_path) != 0)
                        { fprintf(stderr, "error: internal error\n"); goto fail; }
                        if (gen_s(&err, decorated_dst, new_ir, generate_debug_info) != 0) { goto fail; }
                    }
                    else
                    {
                        if (gen_s(&err, dst, new_ir, generate_debug_info) != 0) { goto fail; }
                    }
                    break;
                case xjc_output_format_x:
                    if (dst == NULL)
                    {
                        if (fork_path( \
                                decorated_dst, XJSCD_PATH_MAX, \
                                "x", source->source_path) != 0)
                        { fprintf(stderr, "error: internal error\n"); goto fail; }
                        if (gen_x(&err, decorated_dst, new_ir, generate_debug_info) != 0) { goto fail; }
                    }
                    else
                    {
                        if (gen_x(&err, dst, new_ir, generate_debug_info) != 0) { goto fail; }
                    }
                    break;
            }
            ec_delete(new_ir); new_ir = NULL;
        }
        else if (work_mode == xjc_work_mode_link)
        {
            /* Module Full Path & Name */
            {
                char *module_fullpath;
                const char *module_name;
                size_t module_name_len;
                size_t module_fullpath_len;

                module_fullpath = realpath_get(source->source_path);
                if (module_fullpath == NULL)
                { fprintf(stderr, "error: out of memory\n"); goto fail; }
                module_fullpath_len = strlen(module_fullpath);

                if (mainname_get(&module_name, &module_name_len, module_fullpath, strlen(module_fullpath)) != 0)
                { free(module_fullpath); fprintf(stderr, "error: out of memory\n"); goto fail; }
                (void)module_name_len;

                /* TODO: module name duplicate checking */
                xjs_ir_module_fullpath_set(new_ir, module_fullpath, module_fullpath_len);
                xjs_ir_module_name_set(new_ir, module_name, module_name_len);

                free(module_fullpath);
            }

            irs[i] = new_ir; new_ir = NULL;
            i++;
        }
    }

    if (work_mode == xjc_work_mode_link)
    {
        char stdlib_path[PATH_MAX];
        char *stdlib_source_data = NULL;
        size_t stdlib_source_len = 0;

        if (no_stdlib == xjs_false)
        {
            char exe_path[PATH_MAX];
            {
                char *p;
                if (current_program_path(exe_path, PATH_MAX) != 0)
                { fprintf(stderr, "error: get executable path failed\n"); goto fail; }
                p = strrchr(exe_path, OS_SEP);
                if (p == NULL)
                { fprintf(stderr, "error: invalid executable path\n"); goto fail; }
                *p = '\0';
            }
            snprintf(stdlib_path, PATH_MAX, "%s%c%s", exe_path, OS_SEP, XJC_STDLIB_FILENAME);

            if (read_file(&stdlib_source_data, &stdlib_source_len, stdlib_path) != 0)
            {
                fprintf(stderr, "error: failed to open file %s\n", stdlib_path);
                goto fail;
            }
        }

        if (xjs_l1_start(&err, \
                    &ir_merged, \
                    irs, srcs_count, \
                    entry, no_stdlib, \
                    stdlib_source_data, stdlib_source_len, \
                    xjs_load_sys_lib_cb) != 0)
        {
            if (stdlib_source_data != NULL) { free(stdlib_source_data); stdlib_source_data = NULL; }
            goto fail;
        }
        if (stdlib_source_data != NULL) { free(stdlib_source_data); stdlib_source_data = NULL; }

        switch (output_format)
        {
            case xjc_output_format_s:
                if (dst == NULL)
                {
                    char buf[PATH_MAX + 1];
                    char buf2[PATH_MAX + 1];
                    if (current_working_path(buf, PATH_MAX) != 0)
                    { fprintf(stderr, "error: internal error\n"); goto fail; }
                    snprintf(buf2, PATH_MAX, "%s%ca.s", buf, OS_SEP);
                    if (gen_s(&err, buf2, ir_merged, generate_debug_info) != 0) { goto fail; }
                }
                else
                {
                    if (gen_s(&err, dst, ir_merged, generate_debug_info) != 0) { goto fail; }
                }
                break;
            case xjc_output_format_x:
                if (dst == NULL)
                {
                    char buf[PATH_MAX + 1];
                    char buf2[PATH_MAX + 1];
                    if (current_working_path(buf, PATH_MAX) != 0)
                    { fprintf(stderr, "error: internal error\n"); goto fail; }
                    snprintf(buf2, PATH_MAX, "%s%ca.x", buf, OS_SEP);
                    if (gen_x(&err, buf2, ir_merged, generate_debug_info) != 0) { goto fail; }
                }
                else
                {
                    if (gen_x(&err, dst, ir_merged, generate_debug_info) != 0) { goto fail; }
                }
                break;
        }
    }

    goto done;
fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }
    ret = -1;
done:
    xjs_error_uninit(&err);
    if (irs != NULL)
    {
        for (i = 0; i != srcs_count; i++)
        {
            if (irs[i] != NULL)
            { ec_delete(irs[i]); }
        }
        free(irs);
    }
    if (ir_merged != NULL) ec_delete(ir_merged);
    ec_delete(new_ir);
    return ret;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    argsparse_t argsparse;
    xjc_work_mode work_mode = xjc_work_mode_link;
    xjc_output_format output_format = xjc_output_format_x;
    const char *dst = NULL;
    char *entry = NULL, *entry_store = NULL;
    xjc_filetype filetype = xjc_filetype_auto;
    sourcelist_ref sources = NULL;
    xjs_bool generate_debug_info = xjs_false;
    int ret_zero = 0;
    xjs_bool no_stdlib = xjs_false;

    init_ec();
    argsparse_init(&argsparse, argc, argv);
    if ((sources = ect_list_new(sourcelist)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); ret = -1; goto fail; }

    if (argsparse_available(&argsparse) == 0)
    { fprintf(stderr, "error: no input file\n"); ret = -1; goto fail; }

    while (argsparse_available(&argsparse) != 0)
    {
        if (argsparse_match_str(&argsparse, "--help") || \
                argsparse_match_str(&argsparse, "-h"))
        { show_help(); goto done; }
        else if (argsparse_match_str(&argsparse, "--version"))
        { show_version(); goto done; }
        else if (argsparse_match_str(&argsparse, "--show-targets"))
        { show_targets(); goto done; }
        else if (argsparse_match_str(&argsparse, "--list-sys-libs"))
        { list_sys_libs(); goto done; }
        else if (argsparse_match_str(&argsparse, "--ret-zero"))
        {
            argsparse_next(&argsparse);
            ret_zero = 1;
        }
        else if (argsparse_match_str(&argsparse, "--no-stdlib"))
        {
            argsparse_next(&argsparse);
            no_stdlib = xjs_true;
        }
        else if (argsparse_match_str(&argsparse, "-c"))
        {
            argsparse_next(&argsparse);
            work_mode = xjc_work_mode_compile;
        }
        else if (argsparse_match_str(&argsparse, "--ast-dump"))
        {
            argsparse_next(&argsparse);
            work_mode = xjc_work_mode_ast_dump;
        }
        else if (argsparse_match_str(&argsparse, "-S"))
        {
            argsparse_next(&argsparse);
            output_format = xjc_output_format_s;
        }
        else if (argsparse_match_str(&argsparse, "-xmjs"))
        { argsparse_next(&argsparse); filetype = xjc_filetype_mjs; }
        else if (argsparse_match_str(&argsparse, "-xjs"))
        { argsparse_next(&argsparse); filetype = xjc_filetype_js; }
        else if (argsparse_match_str(&argsparse, "-xx"))
        { argsparse_next(&argsparse); filetype = xjc_filetype_x; }
        else if (argsparse_match_str(&argsparse, "-x"))
        { argsparse_next(&argsparse); filetype = xjc_filetype_auto; }
        else if (argsparse_match_str(&argsparse, "-o"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            { fprintf(stderr, "error: destination file expected\n"); ret = -1; goto fail; }
            dst = argsparse_fetch(&argsparse);
#ifdef _WIN32
            if ((strlen(dst) == 1) && (dst[0] == '-')) { setmode(fileno(stdout), O_BINARY); }
#endif
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "-g"))
        {
            argsparse_next(&argsparse);
            generate_debug_info = xjs_true;
        }
        else if (argsparse_match_str(&argsparse, "--entry"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            { fprintf(stderr, "error: entry file expected\n"); ret = -1; goto fail; }
            entry = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else
        {
            const char *source_path = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
            {
                xjc_source_ref new_xjc_source;
                if ((new_xjc_source = xjc_source_new(source_path, filetype)) == NULL)
                { fprintf(stderr, "error: out of memory\n"); ret = -1; goto fail; }
                ect_list_push_back(sourcelist, sources, new_xjc_source);
            }
        }
    }

    if (ect_list_size(sourcelist, sources) == 0)
    { fprintf(stderr, "error: no source file\n"); ret = -1; goto fail; }

    if (output_format == xjc_output_format_x)
    {
        ect_iterator(sourcelist) it_src;
        ect_for(sourcelist, sources, it_src)
        {
            const xjc_source_ref src = ect_deref(xjc_source_ref, it_src);
            size_t source_path_len = strlen(src->source_path);
            if ((dst != NULL) && \
                    (source_path_len == strlen(dst)) && (strncmp(src->source_path, dst, source_path_len) == 0))
            { fprintf(stderr, "error: source file is the same as destination file\n"); ret = -1; goto fail; }
        }
    }

    if ((ect_list_size(sourcelist, sources) > 1) && \
            ((work_mode == xjc_work_mode_compile)))
    { fprintf(stderr, "error: can not specify -o with -c or -S with multiple files\n"); ret = -1; goto fail; }

    if (work_mode == xjc_work_mode_compile && entry != NULL)
    { fprintf(stderr, "error: redundant entry specified\n"); ret = -1; goto fail; }
    else if (work_mode == xjc_work_mode_ast_dump && entry != NULL)
    { fprintf(stderr, "error: redundant entry specified\n"); ret = -1; goto fail; }
    else if ((work_mode == xjc_work_mode_link) && entry == NULL)
    {
        if (ect_list_size(sourcelist, sources) == 1)
        {
            const char *entry_filename = ect_list_front(sourcelist, sources)->source_path;
            const char *entry1;
            size_t entry1_len;
            if (mainname_get(&entry1, &entry1_len, entry_filename, strlen(entry_filename)) != 0)
            { fprintf(stderr, "error: invalid entry\n"); ret = -1; goto fail; }
            if ((entry_store = (char *)malloc(sizeof(char) * (entry1_len + 1))) == NULL)
            { fprintf(stderr, "error: out of memory\n"); ret = -1; goto fail; }
            memcpy(entry_store, entry1, entry1_len);
            entry_store[entry1_len] = '\0';
            entry = entry_store;
        }
        else
        { fprintf(stderr, "error: entry required\n"); ret = -1; goto fail; }
    }

    if ((ret = compile(sources, dst, \
                    output_format, \
                    entry, \
                    work_mode, \
                    generate_debug_info, \
                    no_stdlib)) != 0)
    { goto fail; }

fail:
done:
    ec_delete(sources);
    if (ret_zero != 0) { ret = 0; }
    if (entry_store != NULL) free(entry_store);
    return ret;
}

