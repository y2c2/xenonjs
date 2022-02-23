#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ec_string.h>
#include <ec_encoding.h>
#include <ec_alloc.h>
#include "xjs.h"
#include "print_error.h"
#include "test_lexer_serialize.h"
#include "test_c0_serialize.h"
#include "test_c2_serialize.h"
#include "test_lexer.h"
#include "test_parser.h"
#include "test_c0.h"
#include "test_c0m.h"
#include "test_c2.h"
#include "argsparse.h"

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

static void show_help(void)
{
    const char *info = ""
        "test [options] <filename.js>\n"
        "\n"
        "  -f, --file         <script.js>      evaluate script file\n"
        "  -e, --eval         <script>         evaluate script\n"
        "  -p, --print        <script>         evaluate script and print result\n"
        "  -i, --repl                          interactive, REPL\n"
        "\n"
        "  --lex              <script>         tokenize script\n"
        "  --parse            <script>         parse script\n"
        "  --parsem           <module>         parse module\n"
        "  -c0                <script>         extract CFG (script)\n"
        "  -c0m               <module>         extract CFG (module)\n"
        "  -c2                <script>         generate IR (script)\n"
        "  -c2m               <module>         generate IR (module)\n"
        "  -S                 <script>         print readable IR (script)\n"
        "  -Sm                <module>         print readable IR (module)\n"
        "\n"
        "  --test-lexer       <testcases.txt>  perform lexer tests\n"
        "  --test-parse       <testcases.txt>  perform parse tests (script)\n"
        "  --test-parsem      <testcases.txt>  perform parse tests (module)\n"
        "  --test-c0          <testcases.txt>  perform c0 tests (script)\n"
        "  --test-c0m         <testcases.txt>  perform c0 tests (module)\n"
        "  --test-c2          <testcases.txt>  perform c2 tests (script)\n"
        "  --test-c2m         <testcases.txt>  perform c2 tests (module)\n"
        "  -h, --help                          show help information\n"
        "";
    puts(info);
}


static void lex(const char *script)
{
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Serialize */
    {
        char *s = xjs_token_list_serialize(tokens);
        if (s == NULL)
        { fprintf(stderr, "error: serialize failed\n"); goto fail; }
        printf("%s\n", s);
        fflush(stdout);
        free(s);
    }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
}

static void parse(const char *script, const xjs_bool is_module)
{
    xjs_vm_options opts;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    ec_string *readable_ir_source = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, is_module)) == NULL)
    { goto fail; }

    /* Serialize */
    {
        /* Print IR */
        if (xjs_astprinter_start(&err, &readable_ir_source, ast) != 0)
        {
            goto fail;
        }

        {
            char *encoded_result;
            ec_size_t encoded_result_len;

            ec_encoding_t enc;
            ec_encoding_utf8_init(&enc);
            ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, readable_ir_source);

            fwrite(encoded_result, encoded_result_len, 1, stdout);
            fflush(stdout);

            ec_free(encoded_result);
        }
    }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }
    xjs_error_uninit(&err);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
}

static void c0(const char *script, const xjs_bool is_module)
{
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    if ((vm = xjs_vm_new(&opts)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, is_module)) == NULL)
    { goto fail; }

    /* C0 */
    if ((cfg = xjs_c0_start(&err, ast)) == NULL)
    { goto fail; }

    {
        char *s = xjs_cfg_serialize(cfg);
        if (s == NULL)
        { fprintf(stderr, "error: serialize failed\n"); goto fail; }
        printf("%s\n", s);
        fflush(stdout);
        free(s);
    }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
    ec_delete(cfg);
}

static void c2(const char *script, const xjs_bool is_module)
{
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_ir_ref ir = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    if ((vm = xjs_vm_new(&opts)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, is_module)) == NULL)
    { goto fail; }

    /* C0 */
    if ((cfg = xjs_c0_start(&err, ast)) == NULL)
    { goto fail; }

    /* Generate IR */
    if ((ir = xjs_c2_start(&err, cfg)) == NULL)
    { goto fail; }

    {
        char *s = xjs_ir_serialize(ir);
        if (s == NULL)
        { fprintf(stderr, "error: serialize failed\n"); goto fail; }
        printf("%s\n", s);
        fflush(stdout);
        free(s);
    }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
    ec_delete(cfg);
    ec_delete(ir);
}

static void generate_readable_ir(const char *script)
{
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_ir_ref ir = NULL;
    ec_string *readable_ir_source = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    if ((vm = xjs_vm_new(&opts)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, xjs_false)) == NULL)
    { goto fail; }

    /* C0 */
    if ((cfg = xjs_c0_start(&err, ast)) == NULL)
    { goto fail; }

    /* Generate IR */
    if ((ir = xjs_c2_start(&err, cfg)) == NULL)
    { goto fail; }

    {
        /* Print IR */
        if (xjs_irprinter_start(&err, &readable_ir_source, ir) != 0)
        {
            goto fail;
        }

        {
            char *encoded_result;
            ec_size_t encoded_result_len;

            ec_encoding_t enc;
            ec_encoding_utf8_init(&enc);
            ec_encoding_encode(&enc, (ec_byte_t **)&encoded_result, &encoded_result_len, readable_ir_source);

            fwrite(encoded_result, encoded_result_len, 1, stdout);
            fflush(stdout);

            ec_free(encoded_result);
        }
    }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
    ec_delete(cfg);
    ec_delete(ir);
    ec_delete(readable_ir_source);
}

static void run_file(const char *script_file)
{
    FILE *fp = NULL;
    long len;
    char *script = NULL;
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_ir_ref ir = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script_file == NULL)
    { fprintf(stderr, "error: script file expected\n"); goto fail; }

    if ((fp = fopen(script_file, "rb")) == NULL)
    { fprintf(stderr, "error: failed to open script file %s\n", script_file); goto fail; }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if ((script = malloc(sizeof(char) * (size_t)len + 1)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    fread(script, (size_t)len, 1, fp);

    if ((vm = xjs_vm_new(&opts)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, xjs_false)) == NULL)
    { goto fail; }

    /* C0 */
    if ((cfg = xjs_c0_start(&err, ast)) == NULL)
    { goto fail; }

    /* Generate IR */
    if ((ir = xjs_c2_start(&err, cfg)) == NULL)
    { goto fail; }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
    ec_delete(cfg);
    ec_delete(ir);
    if (script != NULL) { free(script); }
    if (fp != NULL) fclose(fp);
}

static void eval(const char *script)
{
    xjs_vm_options opts;
    xjs_vm_ref vm = NULL;
    ec_string *uscript = NULL;
    xjs_token_list_ref tokens = NULL;
    xjs_ast_program_ref ast = NULL;
    xjs_cfg_ref cfg = NULL;
    xjs_ir_ref ir = NULL;
    xjs_error err;

    xjs_error_init(&err);
    xjs_vm_options_init(&opts);

    if (script == NULL)
    { fprintf(stderr, "error: script expected\n"); goto fail; }

    if ((vm = xjs_vm_new(&opts)) == NULL)
    { fprintf(stderr, "error: out of memory\n"); goto fail; }

    /* Decode */
    {
        ec_encoding_t enc;
        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &uscript, (const ec_byte_t *)script, strlen(script)) != 0)
        { fprintf(stderr, "error: decode script fail\n"); goto fail; }
    }

    /* Tokenize */
    if ((tokens = xjs_lexer_start(&err, uscript)) == NULL)
    { goto fail; }

    /* Parse */
    if ((ast = xjs_parser_start(&err, tokens, xjs_false)) == NULL)
    { goto fail; }

    /* C0 */
    if ((cfg = xjs_c0_start(&err, ast)) == NULL)
    { goto fail; }

    /* Generate IR */
    if ((ir = xjs_c2_start(&err, cfg)) == NULL)
    { goto fail; }

fail:
    if (err.error_no != 0)
    {
        print_error(&err);
    }

    ec_delete(vm);
    ec_delete(uscript);
    ec_delete(tokens);
    ec_delete(ast);
    ec_delete(cfg);
    ec_delete(ir);
}

int main(int argc, char *argv[])
{
    argsparse_t argsparse;

    init_ec();
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
        else if (argsparse_match_str(&argsparse, "--file") || \
                argsparse_match_str(&argsparse, "-f"))
        {
            argsparse_next(&argsparse);
            run_file(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--eval") || \
                argsparse_match_str(&argsparse, "-e"))
        {
            argsparse_next(&argsparse);
            eval(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--lex"))
        {
            argsparse_next(&argsparse);
            lex(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--parse"))
        {
            argsparse_next(&argsparse);
            parse(argsparse_fetch(&argsparse), xjs_false);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--parsem"))
        {
            argsparse_next(&argsparse);
            parse(argsparse_fetch(&argsparse), xjs_true);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--extract-cfg") ||
                argsparse_match_str(&argsparse, "--c0") ||
                argsparse_match_str(&argsparse, "-c0"))
        {
            argsparse_next(&argsparse);
            c0(argsparse_fetch(&argsparse), xjs_false);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--extract-cfg-module") ||
                argsparse_match_str(&argsparse, "--c0m") ||
                argsparse_match_str(&argsparse, "-c0m"))
        {
            argsparse_next(&argsparse);
            c0(argsparse_fetch(&argsparse), xjs_true);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--extract-ir") ||
                argsparse_match_str(&argsparse, "--c2") ||
                argsparse_match_str(&argsparse, "-c2"))
        {
            argsparse_next(&argsparse);
            c2(argsparse_fetch(&argsparse), xjs_false);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--extract-ir-module") ||
                argsparse_match_str(&argsparse, "--c2m") ||
                argsparse_match_str(&argsparse, "-c2m"))
        {
            argsparse_next(&argsparse);
            c2(argsparse_fetch(&argsparse), xjs_true);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "-S"))
        {
            argsparse_next(&argsparse);
            generate_readable_ir(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-lexer"))
        {
            argsparse_next(&argsparse);
            test_lexer(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-parse"))
        {
            argsparse_next(&argsparse);
            test_parse(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-parsem"))
        {
            argsparse_next(&argsparse);
            test_parsem(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-c0"))
        {
            argsparse_next(&argsparse);
            test_c0(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-c0m"))
        {
            argsparse_next(&argsparse);
            test_c0m(argsparse_fetch(&argsparse));
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-c2"))
        {
            argsparse_next(&argsparse);
            test_c2(argsparse_fetch(&argsparse), xjs_false);
            return 0;
        }
        else if (argsparse_match_str(&argsparse, "--test-c2m"))
        {
            argsparse_next(&argsparse);
            test_c2(argsparse_fetch(&argsparse), xjs_true);
            return 0;
        }
        else
        {
            fprintf(stderr, "error: unknown argument '%s'\n", \
                    argsparse_fetch(&argsparse));
            return -1;
        }
    }

done:
    return 0;
}

