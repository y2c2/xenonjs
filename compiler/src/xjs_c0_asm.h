/* XenonJS : C0
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C0_ASM_H
#define XJS_C0_ASM_H

#define xjs_asm_clone_loc_range(_dst) \
    do { \
        (_dst)->loc.start.ln = xjs_asm_block_ctx->loc.start.ln; \
        (_dst)->loc.start.col = xjs_asm_block_ctx->loc.start.col; \
        (_dst)->loc.end.ln = xjs_asm_block_ctx->loc.end.ln; \
        (_dst)->loc.end.col = xjs_asm_block_ctx->loc.end.col; \
        (_dst)->range.start = xjs_asm_block_ctx->range.start; \
        (_dst)->range.end = xjs_asm_block_ctx->range.end; \
    } while (0)

#define xjs_asm_block_decl(_block) \
    xjs_cfg_node_ref xjs_asm_block_ctx = _block; (void)xjs_asm_block_ctx

#define xjs_asm_var(_var) \
    xjs_cfg_var _var

#define xjs_asm_allocate_var(_var) \
    do { \
        xjs_cfg_node_ref node; \
        _var = xjs_cfgbuilder_allocate_var(ctx->cfgb); \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_alloca_new(ctx->cfgb, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_load_undefined(_var) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_literal_undefined_new(ctx->cfgb, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_load_int(_var, _val) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_literal_number_new(ctx->cfgb, _val, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_load_string(_var, _str) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_literal_string_new(ctx->cfgb, _str, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_declvar(_var, _name) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_declvar_new(ctx->cfgb, _name), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_load(_var, _name) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_load_new(ctx->cfgb, _name, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_add(_dst, _lhs, _rhs) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_binary_op_new(ctx->cfgb, XJS_CFG_BINARY_OP_TYPE_ADD, _dst, _lhs, _rhs), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_sub(_dst, _lhs, _rhs) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_binary_op_new(ctx->cfgb, XJS_CFG_BINARY_OP_TYPE_SUB, _dst, _lhs, _rhs), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_store(_var, _name) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_store_new(ctx->cfgb, _name, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_ret(_var) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_return_new(ctx->cfgb, _var), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_objset(_var_dst, _var_obj, _var_key, _var_value) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_object_set_new(ctx->cfgb, _var_dst, _var_obj, _var_key, _var_value), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_objget(_var_dst, _var_obj, _var_key) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_object_get_new(ctx->cfgb, _var_dst, _var_obj, _var_key), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_binop(_type, _var_dst, _var_lhs, _var_rhs) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_binary_op_new(ctx->cfgb, _type, _var_dst, _var_lhs, _var_rhs), ctx->err); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_jump(_dest) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_jump_new(ctx->cfgb), ctx->err); \
        xjs_cfgbuilder_jump_dest_set(node, _dest); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#define xjs_asm_branch(_cond, _consequent, _alternate) \
    do { \
        xjs_cfg_node_ref node; \
        XJS_VNZ_ERROR_MEM(node = xjs_cfgbuilder_branch_new(ctx->cfgb), ctx->err); \
        xjs_cfgbuilder_branch_cond_set(node, _cond); \
        xjs_cfgbuilder_branch_true_branch_set(node, _consequent); \
        xjs_cfgbuilder_branch_false_branch_set(node, _alternate); \
        xjs_asm_clone_loc_range(node); \
        xjs_cfgbuilder_block_push_back(xjs_asm_block_ctx, node); \
    } while (0)

#endif

