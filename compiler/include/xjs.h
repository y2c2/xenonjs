/* XenonJS
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_H
#define XJS_H

#include "xjs_private.h"

/* Memory callbacks */
void xjs_allocator_set_malloc(xjs_malloc_cb_t cb);
void xjs_allocator_set_calloc(xjs_calloc_cb_t cb);
void xjs_allocator_set_free(xjs_free_cb_t cb);
void xjs_allocator_set_memcpy(xjs_memcpy_cb_t cb);
void xjs_allocator_set_memset(xjs_memset_cb_t cb);

/* Error */
void xjs_error_init(xjs_error_ref e);
void xjs_error_uninit(xjs_error_ref e);

/* Lexer */
xjs_token_list_ref xjs_lexer_start_ex( \
        xjs_error_ref err, \
        ec_string *src_code, \
        const char *filename);
xjs_token_list_ref xjs_lexer_start( \
        xjs_error_ref err, \
        ec_string *src_code);
xjs_token_type xjs_token_type_get(xjs_token_ref token);
ec_string *xjs_token_value_get(xjs_token_ref token);

/* Parser */
xjs_ast_program_ref xjs_parser_start_ex( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode, \
        const char *filename, \
        const xjs_bool loc);
xjs_ast_program_ref xjs_parser_start( \
        xjs_error_ref err, \
        const xjs_token_list_ref tokens, \
        const xjs_bool module_mode);

/* AST Printer */
int xjs_astprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ast_program_ref ast);

/* C0 (AST -> CFG) */
xjs_cfg_ref xjs_c0_start_ex( \
        xjs_error_ref err, \
        xjs_ast_program_ref program, \
        const char *filename);
xjs_cfg_ref xjs_c0_start( \
        xjs_error_ref err, \
        xjs_ast_program_ref program);

/* C1 (CFG Oriented Optimization) */
xjs_cfg_ref xjs_c1_start( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg, \
        xjs_c1_options_ref opts);

/* C2 (CFG -> IR) */
xjs_ir_ref xjs_c2_start_ex( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg, \
        const char *filename);
xjs_ir_ref xjs_c2_start( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg);

/* IR Manipulate */
void xjs_ir_module_name_set( \
        xjs_ir_ref ir, const char *name, const xjs_size_t name_len);
void xjs_ir_module_fullpath_set( \
        xjs_ir_ref ir, const char *fullpath, const xjs_size_t fullpath_len);

/* IR Printer */
int xjs_irprinter_start_ex( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info);
int xjs_irprinter_start( \
        xjs_error_ref err, \
        ec_string **result_out, \
        xjs_ir_ref ir);

/* C4 (IR -> bytecode) */
int xjs_c4_start_ex( \
        xjs_error_ref err, \
        char **bytecode, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir, \
        const xjs_bool generate_debug_info);
int xjs_c4_start( \
        xjs_error_ref err, \
        char **bytecode, xjs_size_t *bytecode_len_out, \
        xjs_ir_ref ir);

/* L0 (bytecode -> IR) */
int xjs_l0_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        char *bytecode, xjs_size_t bytecode_len);

/* L1 ([IR] -> IR) */
int xjs_l1_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        xjs_ir_ref *ir_in, xjs_size_t ir_in_count, \
        const char *entry, \
        const xjs_bool no_stdlib, \
        const char *stdlib_source_data, \
        const xjs_size_t stdlib_source_len, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb);

/* VM */
void xjs_vm_options_init(xjs_vm_options_ref opts);
xjs_vm_ref xjs_vm_new(xjs_vm_options_ref opts);
void xjs_vm_delete(xjs_vm_ref vm);
xjs_val_ref xjs_vm_eval( \
        xjs_vm_ref vm, \
        char *bytecode, xjs_size_t bytecode_len);
void xjs_vm_println( \
        xjs_val_ref val);

#endif

