/* XenonJS : C0
 * Copyright(c) 2017 y2c2 */

#include <ec_string.h>
#include <ec_algorithm.h>
#include "xjs_types.h"
#include "xjs_ast.h"
#include "xjs_cfg.h"
#include "xjs_cfgbuilder.h"
#include "xjs_aux.h"
#include "xjs_helper.h"
#include "xjs_c0_asm.h"
#include "xjs_c0_ctx.h"
#include "xjs_c0.h"

#define C0_ERROR_PRINTF(_fmt, ...) \
    do { \
        xjs_error_update(ctx->err, __FILE__, __LINE__, XJS_ERRNO_COMPILE); \
        xjs_error_update_desc_printf(ctx->err, _fmt, __VA_ARGS__); \
        goto fail; \
    } while (0)


#define CFG_LOC_CLONE_START(_dst, _src) \
    do { \
        xjs_cfg_node_set_loc_start(_dst, (_src)->loc.start.ln, (_src)->loc.start.col); \
    } while (0)
#define CFG_LOC_CLONE_END(_dst, _src) \
    do { \
        xjs_cfg_node_set_loc_end(_dst, (_src)->loc.end.ln, (_src)->loc.end.col); \
    } while (0)
#define CFG_LOC_CLONE(_dst, _src) \
    do { \
        CFG_LOC_CLONE_START(_dst, _src); \
        CFG_LOC_CLONE_END(_dst, _src); \
    } while (0)
#define CFG_RANGE_CLONE(_dst, _src) \
    do { \
        xjs_cfg_node_set_range_start(_dst, (_src)->range.start); \
        xjs_cfg_node_set_range_end(_dst, (_src)->range.end); \
    } while (0)
#define CFG_LOC_RANGE_CLONE(_dst, _src) \
    do { \
        CFG_LOC_CLONE(_dst, _src); \
        CFG_RANGE_CLONE(_dst, _src); \
    } while (0)

/* Declarations */
static int
xjs_c0_statementlist( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_statementlist_ref statements);

static int 
xjs_c0_statementlistitem( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_statementlistitem_ref item);

static int
xjs_c0_expression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_expression_ref expr);

static xjs_cfg_node_ref 
xjs_c0_statementlist_function_scope_statementlistitem( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlistitem_ref statements, \
        ec_bool toplevel);

static xjs_cfg_node_ref 
xjs_c0_statementlist_function_scope_statementlist( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlist_ref statements, \
        ec_bool toplevel);

/* Variable Hoisting
 *
 * Move function-scope 'var' kind of variable declarations 
 * to the beginning of a function declaration;
 */

static int xjs_var_hoist_vardecl( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_variabledeclaration_ref vardecl)
{
    int ret = 0;
    xjs_ast_variabledeclaratorlist_ref r3;

    if (vardecl->kind != xjs_ast_variabledeclaration_kind_var) return 0;
    r3 = vardecl->declarations;
    {
        ect_iterator(xjs_ast_variabledeclaratorlist) it;
        ect_for(xjs_ast_variabledeclaratorlist, r3, it)
        {
            xjs_ast_variabledeclarator_ref declarator \
                = ect_deref(xjs_ast_variabledeclarator_ref, it);
            switch (declarator->id->tag)
            {
                case xjs_ast_variabledeclarator_id_identifier:
                    {
                        xjs_varname *new_varname = NULL;
                        XJS_VNZ_ERROR_MEM( \
                                new_varname = xjs_varname_new( \
                                    ec_string_clone(declarator->id->u.xjs_ast_variabledecorator_identifier->name)), ctx->err);
                        new_varname->loc.start.ln = declarator->loc.start.ln;
                        new_varname->loc.start.col = declarator->loc.start.col;
                        new_varname->loc.end.ln = declarator->loc.end.ln;
                        new_varname->loc.end.col = declarator->loc.end.col;
                        new_varname->range.start = declarator->range.start;
                        new_varname->range.end = declarator->range.end;
                        if (ect_set_count(xjs_varname_set, vars, new_varname) == 0)
                        {
                            xjs_varname_set_insert(vars, new_varname);
                        }
                        else
                        {
                            ec_delete(new_varname);
                        }
                    }
                    break;

                case xjs_ast_variabledeclarator_id_bindingpattern:
                    xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                    goto fail;
            }
        }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_var_hoist_declaration( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_declaration_ref decl)
{
    int ret = 0;
    xjs_ast_variabledeclaration_ref r2;

    if (decl->tag != xjs_ast_declaration_variabledeclaration) return 0;
    r2 = decl->u.xjs_ast_declaration_variabledeclaration;
    ret = xjs_var_hoist_vardecl(ctx, vars, r2);

    return ret;
}

static int xjs_var_hoist_statement( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_statement_ref stmt);
static int xjs_var_hoist_statementlistitem( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_statementlistitem_ref item);

static int xjs_var_hoist_statement_block( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_blockstatement_ref block_stmt)
{
    ect_iterator(xjs_ast_statementlist) it;
    ect_for(xjs_ast_statementlist, block_stmt->body, it)
    {
        xjs_ast_statementlistitem_ref item = ect_deref(xjs_ast_statementlistitem_ref, it);
        if (xjs_var_hoist_statementlistitem(ctx, vars, item) != 0) { return -1; }
    }

    return 0;
}

static int xjs_var_hoist_statement_for( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_forstatement_ref for_stmt)
{

    if (for_stmt->init.type == xjs_ast_forstatement_init_vardecl)
    {
        xjs_ast_variabledeclaration_ref vardecl = for_stmt->init.u.as_vardecl;
        if (xjs_var_hoist_vardecl(ctx, vars, vardecl) != 0)
        { return -1; }
    }

    if (xjs_var_hoist_statementlistitem(ctx, vars, for_stmt->body) != 0)
    { return -1; }

    return 0;
}

static int xjs_var_hoist_statement_if( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_ifstatement_ref if_stmt)
{
    if (xjs_var_hoist_statementlistitem(ctx, vars, \
                if_stmt->consequent) != 0)
    { return -1; }
    if (if_stmt->alternate != NULL)
    {
        if (xjs_var_hoist_statementlistitem(ctx, vars, \
                    if_stmt->alternate) != 0)
        { return -1; }
    }

    return 0;
}

static int xjs_var_hoist_statement_while( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_whilestatement_ref while_stmt)
{
    if (xjs_var_hoist_statementlistitem(ctx, vars, \
                while_stmt->body) != 0)
    { return -1; }

    return 0;
}

static int xjs_var_hoist_statement_do( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_dostatement_ref do_stmt)
{
    if (xjs_var_hoist_statementlistitem(ctx, vars, \
                do_stmt->body) != 0)
    { return -1; }

    return 0;
}

static int xjs_var_hoist_statement( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_statement_ref stmt)
{
    int ret = 0;

    switch (stmt->tag)
    {
        case xjs_ast_statement_expressionstatement:
            break;

        case xjs_ast_statement_emptystatement:
        case xjs_ast_statement_inspectstatement:
        case xjs_ast_statement_returnstatement:
        case xjs_ast_statement_breakstatement:
        case xjs_ast_statement_continuestatement:
            break;

        case xjs_ast_statement_forstatement:
            ret = xjs_var_hoist_statement_for(ctx, vars, stmt->u.xjs_ast_statement_forstatement);
            break;
        case xjs_ast_statement_blockstatement:
            ret = xjs_var_hoist_statement_block(ctx, vars, stmt->u.xjs_ast_statement_blockstatement);
            break;
        case xjs_ast_statement_ifstatement:
            ret = xjs_var_hoist_statement_if(ctx, vars, stmt->u.xjs_ast_statement_ifstatement);
            break;
        case xjs_ast_statement_whilestatement:
            ret = xjs_var_hoist_statement_while(ctx, vars, stmt->u.xjs_ast_statement_whilestatement);
            break;
        case xjs_ast_statement_dostatement:
            ret = xjs_var_hoist_statement_do(ctx, vars, stmt->u.xjs_ast_statement_dostatement);
            break;
    }

    return ret;
}

static int xjs_var_hoist_statementlistitem( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_statementlistitem_ref item)
{
    int ret = 0;

    switch (item->tag)
    {
        case xjs_ast_statementlistitem_declaration:
            ret = xjs_var_hoist_declaration( \
                    ctx, vars, item->u.xjs_ast_statementlistitem_declaration);
            break;

        case xjs_ast_statementlistitem_statement:
            ret = xjs_var_hoist_statement( \
                    ctx, vars, item->u.xjs_ast_statementlistitem_statement);
            break;
    }

    return ret;
}

static int xjs_var_hoist_moduleitem( \
        xjs_c0_ctx *ctx, xjs_varname_set_ref vars, \
        xjs_ast_moduleitem_ref item)
{
    int ret = 0;

    switch (item->tag)
    {
        case xjs_ast_moduleitem_statementlistitem:
            if (item->u.xjs_ast_moduleitem_statementlistitem->tag == \
                    xjs_ast_statementlistitem_declaration)
            {
                ret = xjs_var_hoist_declaration( \
                        ctx, vars, \
                        item->u.xjs_ast_moduleitem_statementlistitem->u.xjs_ast_statementlistitem_declaration);
            }
            break;
        case xjs_ast_moduleitem_exportdeclaration:
            break;
        case xjs_ast_moduleitem_importdeclaration:
            break;
    }

    return ret;
}

static xjs_varname_set_ref
xjs_var_hoist_collect_localvars( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlist_ref stmt)
{
    xjs_varname_set_ref vars = NULL;

    XJS_VNZ_ERROR_MEM(vars = xjs_varname_set_new(), ctx->err);
    {
        ect_iterator(xjs_ast_statementlist) it;
        ect_for(xjs_ast_statementlist, stmt, it)
        {
            xjs_ast_statementlistitem_ref item = ect_deref(xjs_ast_statementlistitem_ref, it);
            if (xjs_var_hoist_statementlistitem(ctx, vars, item) != 0)
            { goto fail; }
        }
    }

fail:
    return vars;
}

static xjs_varname_set_ref
xjs_var_hoist_collect_localvars1( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlistitem_ref item)
{
    xjs_varname_set_ref vars = NULL;

    XJS_VNZ_ERROR_MEM(vars = xjs_varname_set_new(), ctx->err);

    if (xjs_var_hoist_statementlistitem(ctx, vars, item) != 0)
    { goto fail; }

    goto done;
fail:
    ec_delete(vars);
    vars = NULL;
done:
    return vars;
}

static xjs_varname_set_ref
xjs_var_hoist_collect_localvars_from_module( \
        xjs_c0_ctx *ctx, \
        xjs_ast_moduleitemlist_ref moduleitemlist)
{
    xjs_varname_set_ref vars = NULL;

    XJS_VNZ_ERROR_MEM(vars = xjs_varname_set_new(), ctx->err);
    {
        ect_iterator(xjs_ast_moduleitemlist) it;
        ect_for(xjs_ast_moduleitemlist, moduleitemlist, it)
        {
            xjs_ast_moduleitem_ref item = ect_deref(xjs_ast_moduleitem_ref, it);
            if (xjs_var_hoist_moduleitem(ctx, vars, item) != 0)
            { goto fail; }
        }
    }

fail:
    return vars;
}

static xjs_cfg_node_ref
xjs_var_declare_localvars( \
        xjs_c0_ctx *ctx, \
        xjs_varname_set_ref varset)
{
    xjs_cfg_node_ref block_vardecl = NULL;
    xjs_cfg_node_ref vardecl = NULL;
    ec_string *new_name = NULL;

    /* Block to contain statements */
    XJS_VNZ_ERROR_MEM(block_vardecl = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);

    ect_iterator(xjs_varname_set) it;
    ect_for (xjs_varname_set, varset, it)
    {
        xjs_varname *varname = ect_deref(xjs_varname *, it);
        ec_string *name = varname->name;
        XJS_VNZ_ERROR_MEM(new_name = ec_string_clone(name), ctx->err);
        XJS_VNZ_ERROR_MEM(vardecl = xjs_cfgbuilder_declvar_new(ctx->cfgb, new_name), ctx->err);
        CFG_LOC_RANGE_CLONE(vardecl, varname);
        new_name = NULL;
        xjs_cfgbuilder_block_push_back(block_vardecl, vardecl);
        vardecl = NULL;
    }

    goto done;
fail:
    block_vardecl = NULL;
done:
    return block_vardecl;
}


/* IR Emit */

static int
xjs_c0_variabledeclaratorlist_var( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_variabledeclaratorlist_ref decllst)
{
    int ret = 0;
    ect_iterator(xjs_ast_variabledeclaratorlist) it;
    xjs_cfg_node_ref node_base = NULL;
    xjs_cfg_node_ref node_start = NULL;

    ect_for(xjs_ast_variabledeclaratorlist, decllst, it)
    {
        xjs_ast_variabledeclarator_ref declarator \
            = ect_deref(xjs_ast_variabledeclarator_ref, it);
        switch (declarator->id->tag)
        {
            case xjs_ast_variabledeclarator_id_identifier:
                if (declarator->init == NULL) { /* Empty */ }
                else
                {
                    ec_string *name = declarator->id->u.xjs_ast_variabledecorator_identifier->name;
                    xjs_cfg_node_ref init_value_in, init_value_out;
                    xjs_cfg_var var;

                    /* Compute the expression */
                    if (xjs_c0_expression(ctx, \
                                &var, &init_value_in, &init_value_out, \
                                declarator->init) != 0) { goto fail; }
                    /* Expose the entry of variable declaration */
                    if (node_start == NULL) node_start = init_value_in;
                    /* Link previous block with current one */
                    if (node_base != NULL)
                    { xjs_cfgbuilder_link(ctx->cfgb, node_base, init_value_in); }
                    /* Update base */
                    node_base = init_value_out;

                    XJS_V_ERROR_INTERNAL(node_base->type == XJS_CFG_NODE_TYPE_BLOCK, ctx->err);

                    /* Store */
                    {
                        ec_string *new_name = NULL;
                        xjs_cfg_node_ref new_store = NULL;
                        XJS_VNZ_ERROR_MEM(new_name = ec_string_clone(name), ctx->err);
                        XJS_VNZ_ERROR_MEM_OR(new_store = xjs_cfgbuilder_store_new(ctx->cfgb, new_name, var), \
                                ctx->err, ec_delete(new_name));
                        CFG_LOC_RANGE_CLONE(new_store, declarator);
                        new_name = NULL;
                        /* Assume node_base is a block */
                        xjs_cfgbuilder_block_push_back(node_base, new_store);
                    }
                }
                break;

            case xjs_ast_variabledeclarator_id_bindingpattern:
                xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                goto fail;
        }
    }

    /* No variable has been declared with an initial value */
    if (node_start == NULL)
    {
        XJS_VNZ_ERROR_MEM(node_start = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        *node_in = *node_out = node_start;
    }
    else
    {
        *node_in = node_start;
        *node_out = node_base;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_variabledeclaration( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_variabledeclaration_ref vardecl)
{
    int ret = 0;

    switch (vardecl->kind)
    {
        case xjs_ast_variabledeclaration_kind_var:
            /* Variable declaration has been hoisted,
             * do assignment */
            if ((ret = xjs_c0_variabledeclaratorlist_var( \
                            ctx, node_in, node_out, vardecl->declarations)) != 0)
            { goto fail; }
            break;

        case xjs_ast_variabledeclaration_kind_const:
        case xjs_ast_variabledeclaration_kind_let:
            /* Block scope variables */
            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
            goto fail;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_declaration( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_declaration_ref decl)
{
    int ret = 0;

    switch (decl->tag)
    {
        case xjs_ast_declaration_variabledeclaration:
            if (xjs_c0_variabledeclaration( \
                    ctx, node_in, node_out, \
                    decl->u.xjs_ast_declaration_variabledeclaration) != 0)
            { goto fail; }
            break;

        case xjs_ast_declaration_classdeclaration:
        case xjs_ast_declaration_functiondeclaration:
            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
            break;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_blockstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_blockstatement_ref block_stmt)
{
    int ret = 0;

    if ((ret = xjs_c0_statementlist( \
                    ctx, node_in, node_out, block_stmt->body)) != 0)
    { goto fail; }

fail:
    return ret;
}

static int
xjs_c0_expression_literal_number( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_lit_out, \
        xjs_cfg_var var_lit, \
        xjs_ast_literal_ref literal)
{
    int ret = 0;
    xjs_cfg_node_ref node_lit = NULL;

    /* Literal */
    XJS_VNZ_ERROR_MEM(node_lit = xjs_cfgbuilder_literal_number_new(ctx->cfgb,  literal->value.as_number, var_lit), ctx->err);

    /* Expose */
    *node_lit_out = node_lit;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_expression_literal( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_literal_ref literal)
{
    int ret = 0;
    xjs_cfg_var var_lit;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_lit = NULL;
    ec_string *value = literal->raw;
    ec_char_t ch;

    /* Alloca */
    var_lit = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_lit), ctx->err);
    
    /* Load Literal */
    if (xjs_aux_string_match_with(value, "undefined"))
    {
        XJS_VNZ_ERROR_MEM(node_lit = xjs_cfgbuilder_literal_undefined_new(ctx->cfgb, var_lit), ctx->err);
    }
    else if (xjs_aux_string_match_with(value, "null"))
    {
        XJS_VNZ_ERROR_MEM(node_lit = xjs_cfgbuilder_literal_null_new(ctx->cfgb, var_lit), ctx->err);
    }
    else if (xjs_aux_string_match_with(value, "false"))
    {
        XJS_VNZ_ERROR_MEM(node_lit = xjs_cfgbuilder_literal_false_new(ctx->cfgb, var_lit), ctx->err);
    }
    else if (xjs_aux_string_match_with(value, "true"))
    {
        XJS_VNZ_ERROR_MEM(node_lit = xjs_cfgbuilder_literal_true_new(ctx->cfgb, var_lit), ctx->err);
    }
    else
    {
        /* Invalid value */
        if (ec_string_length(value) == 0)
        { xjs_c0_ctx_error(ctx, XJS_ERRNO_INTERNAL); goto fail; }

        /* String */
        ch = ec_string_at(value, 0);
        if ((ch == '\"') || (ch == '\''))
        {
            XJS_VNZ_ERROR_MEM( \
                    node_lit = xjs_cfgbuilder_literal_string_new( \
                        ctx->cfgb, ec_string_clone(literal->value.as_string), var_lit), ctx->err);
        }
        else if ((('0' <= ch) && (ch <= '9')) || (ch == '.'))
        {
            if (xjs_c0_expression_literal_number(ctx, &node_lit, var_lit, literal) != 0)
            { goto fail; }
        }
        else
        { xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP); goto fail; }
    }

    /* Group alloca and literal loading */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, literal);
    CFG_LOC_RANGE_CLONE(node_lit, literal);
    CFG_LOC_RANGE_CLONE(node_block, literal);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_lit);

    *var_out = var_lit;
    *node_in = node_block;
    *node_out = node_block;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_expression_identifier( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_identifier_ref id)
{
    int ret = 0;
    xjs_cfg_node_ref node_block = NULL, node_alloca = NULL;
    xjs_cfg_node_ref node_load = NULL;
    ec_string *new_name = NULL;
    xjs_cfg_var var;

    /* Alloca */
    var = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var), ctx->err);

    /* Load */
    XJS_VNZ_ERROR_MEM(new_name = ec_string_clone(id->name), ctx->err);
    XJS_VNZ_ERROR_MEM(node_load = xjs_cfgbuilder_load_new(ctx->cfgb, new_name, var), ctx->err);
    new_name = NULL;

    /* Group alloca and load */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, id);
    CFG_LOC_RANGE_CLONE(node_load, id);
    CFG_LOC_RANGE_CLONE(node_block, id);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_load);

    *var_out = var;
    *node_in = node_block;
    *node_out = node_block;

    goto done;
fail:
    if (new_name != NULL) ec_delete(new_name);
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_expression_unaryexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_unaryexpression_ref unaryexpression)
{
    int ret = 0;
    xjs_cfg_unary_op_type op_type;

    if (xjs_aux_string_match_with(unaryexpression->op, "+"))
    {
        op_type = XJS_CFG_UNARY_OP_TYPE_ADD;
    }
    else if (xjs_aux_string_match_with(unaryexpression->op, "-"))
    {
        op_type = XJS_CFG_UNARY_OP_TYPE_SUB;
    }
    else if (xjs_aux_string_match_with(unaryexpression->op, "~"))
    {
        op_type = XJS_CFG_UNARY_OP_TYPE_BNOT;
    }
    else if (xjs_aux_string_match_with(unaryexpression->op, "!"))
    {
        op_type = XJS_CFG_UNARY_OP_TYPE_NOT;
    }
    else
    {
        xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
        goto fail;
    }

    {
        xjs_cfg_node_ref node_unary = NULL;
        xjs_cfg_var var_argument, var_unaryed;
        xjs_cfg_node_ref node_argument_in, node_argument_out;
        xjs_cfg_node_ref node_alloca, node_block;

        /* Argument */
        if (xjs_c0_expression(ctx, \
                    &var_argument, &node_argument_in, &node_argument_out, \
                    unaryexpression->argument) != 0)
        { goto fail; }

        /* Alloca */
        var_unaryed = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_unaryed), ctx->err);

        /* Unary */
        XJS_VNZ_ERROR_MEM( \
                node_unary = xjs_cfgbuilder_unary_op_new( \
                    ctx->cfgb, op_type, \
                    var_unaryed, var_argument), \
                ctx->err);

        /* Group alloca and unary */
        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, unaryexpression);
        CFG_LOC_RANGE_CLONE(node_unary, unaryexpression);
        CFG_LOC_RANGE_CLONE(node_block, unaryexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca);
        xjs_cfgbuilder_block_push_back(node_block, node_unary);

        /* Link argument to group */
        xjs_cfgbuilder_link(ctx->cfgb, node_argument_out, node_block);

        *node_in = node_argument_in;
        *node_out = node_block;
        *var_out = var_unaryed;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_binaryexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_binaryexpression_ref binaryexpression)
{
    int ret = 0;
    xjs_cfg_binary_op_type op_type;

    if (xjs_aux_string_match_with(binaryexpression->op, "+"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_ADD; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "-"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_SUB; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "*"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_MUL; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "/"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_DIV; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "%"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_MOD; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "=="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_E2; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "!="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_NE2; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "==="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_E3; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "!=="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_NE3; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "<"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_L; }
    else if (xjs_aux_string_match_with(binaryexpression->op, "<="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_LE; }
    else if (xjs_aux_string_match_with(binaryexpression->op, ">"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_G; }
    else if (xjs_aux_string_match_with(binaryexpression->op, ">="))
    { op_type = XJS_CFG_BINARY_OP_TYPE_GE; }
    else
    {
        xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
        goto fail;
    }

    {
        xjs_cfg_node_ref node_binary = NULL;
        xjs_cfg_var var_dst, var_src1, var_src2;
        xjs_cfg_node_ref node_src1_in, node_src1_out;
        xjs_cfg_node_ref node_src2_in, node_src2_out;
        xjs_cfg_node_ref node_alloca, node_block;

        /* Arguments */
        if (xjs_c0_expression( \
                    ctx, \
                    &var_src1, &node_src1_in, &node_src1_out, \
                    binaryexpression->left) != 0)
        { goto fail; }
        if (xjs_c0_expression( \
                    ctx, \
                    &var_src2, &node_src2_in, &node_src2_out, \
                    binaryexpression->right) != 0)
        { goto fail; }

        /* Alloca */
        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);

        /* Binary */
        XJS_VNZ_ERROR_MEM( \
                node_binary = xjs_cfgbuilder_binary_op_new( \
                    ctx->cfgb, \
                    op_type, var_dst, var_src1, var_src2), \
                ctx->err);

        /* Group alloca and binary */
        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, binaryexpression);
        CFG_LOC_RANGE_CLONE(node_binary, binaryexpression);
        CFG_LOC_RANGE_CLONE(node_block, binaryexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca);
        xjs_cfgbuilder_block_push_back(node_block, node_binary);

        /* Chain blocks in order */
        *node_in = node_src1_in;
        xjs_cfgbuilder_link(ctx->cfgb, node_src1_out, node_src2_in);
        xjs_cfgbuilder_link(ctx->cfgb, node_src2_out, node_block);
        *node_out = node_block;

        *var_out = var_dst;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_logicalexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_logicalexpression_ref logicalexpression)
{
    int ret = 0;
    xjs_cfg_binary_op_type op_type;

    if (xjs_aux_string_match_with(logicalexpression->op, "&&"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_AND; }
    else if (xjs_aux_string_match_with(logicalexpression->op, "||"))
    { op_type = XJS_CFG_BINARY_OP_TYPE_OR; }
    else
    {
        xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
        goto fail;
    }

    {
        xjs_cfg_node_ref node_binary = NULL;
        xjs_cfg_var var_dst, var_src1, var_src2;
        xjs_cfg_node_ref node_src1_in, node_src1_out;
        xjs_cfg_node_ref node_src2_in, node_src2_out;
        xjs_cfg_node_ref node_alloca, node_block;

        /* Arguments */
        if (xjs_c0_expression( \
                    ctx, \
                    &var_src1, &node_src1_in, &node_src1_out, \
                    logicalexpression->left) != 0)
        { goto fail; }
        if (xjs_c0_expression( \
                    ctx, \
                    &var_src2, &node_src2_in, &node_src2_out, \
                    logicalexpression->right) != 0)
        { goto fail; }

        /* Alloca */
        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);

        /* Binary */
        XJS_VNZ_ERROR_MEM( \
                node_binary = xjs_cfgbuilder_binary_op_new( \
                    ctx->cfgb, \
                    op_type, var_dst, var_src1, var_src2), \
                ctx->err);

        /* Group alloca and binary */
        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, logicalexpression);
        CFG_LOC_RANGE_CLONE(node_binary, logicalexpression);
        CFG_LOC_RANGE_CLONE(node_block, logicalexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca);
        xjs_cfgbuilder_block_push_back(node_block, node_binary);

        /* Chain blocks in order */
        *node_in = node_src1_in;
        xjs_cfgbuilder_link(ctx->cfgb, node_src1_out, node_src2_in);
        xjs_cfgbuilder_link(ctx->cfgb, node_src2_out, node_block);
        *node_out = node_block;

        *var_out = var_dst;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c0_expression_stringliteral_new( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_identifier_ref s)
{
    int ret = 0;
    xjs_cfg_var var_lit;
    xjs_cfg_node_ref node_lit, node_alloca, node_block;

    /* Alloca */
    var_lit = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_lit), ctx->err);

    /* String Literal */
    XJS_VNZ_ERROR_MEM( \
            node_lit = xjs_cfgbuilder_literal_string_new( \
                ctx->cfgb, ec_string_clone(s->name), var_lit), ctx->err);

    /* Block */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, s);
    CFG_LOC_RANGE_CLONE(node_lit, s);
    CFG_LOC_RANGE_CLONE(node_block, s);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_lit);

    *var_out = var_lit;
    *node_in = node_block;
    *node_out = node_block;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_assignmentexpression_memberexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_memberexpression_ref memberexpression, \
        xjs_cfg_var var_rhs)
{
    int ret = 0;
    xjs_cfg_var var_dst, var_object, var_key;
    xjs_cfg_node_ref node_object_out;
    xjs_cfg_node_ref node_key_in, node_key_out;
    xjs_cfg_node_ref node_obj_set, node_block;

    /* Evaluate the object part */
    if (xjs_c0_expression(ctx, &var_object, \
                node_in, &node_object_out, memberexpression->object) != 0)
    { goto fail; }

    /* Key */
    if (memberexpression->computed == ec_true)
    {
        if (xjs_c0_expression(ctx, &var_key, \
                    &node_key_in, &node_key_out, \
                    memberexpression->property) != 0)
        { goto fail; }
    }
    else
    {
        /* Key should be an identifier which used as a string literal */
        if (memberexpression->property->tag != xjs_ast_expression_identifier)
        { xjs_c0_ctx_error(ctx, XJS_ERRNO_INTERNAL); goto fail; }

        if (xjs_c0_expression_stringliteral_new(ctx, &var_key, \
                    &node_key_in, &node_key_out, \
                    memberexpression->property->u.xjs_ast_expression_identifier) != 0)
        { goto fail; }
    }
    xjs_cfgbuilder_link(ctx->cfgb, node_object_out, node_key_in);

    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    {
        xjs_cfg_node_ref node_alloca = NULL;
        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca); node_alloca = NULL;
    }
    /* Object Set */
    if ((node_obj_set = xjs_cfgbuilder_object_set_new( \
                    ctx->cfgb, var_dst, var_object, var_key, var_rhs)) == NULL)
    { goto fail; }
    CFG_LOC_RANGE_CLONE(node_obj_set, memberexpression);
    CFG_LOC_RANGE_CLONE(node_block, memberexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_obj_set);
    xjs_cfgbuilder_link(ctx->cfgb, node_key_out, node_block);
    *var_out = var_dst;
    *node_out = node_block;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_assignmentexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_assignmentexpression_ref assignmentexpression)
{
    int ret = 0;
    xjs_cfg_node_ref node_rhs_out;
    xjs_cfg_var var_rhs;
    xjs_cfg_node_ref node_store, node_block, node_tail;

    /* Evaluate the right hand side value */
    {
        if (xjs_c0_expression(ctx, &var_rhs, \
                    node_in, &node_rhs_out, \
                    assignmentexpression->right) != 0)
        { goto fail; }
    }
    *var_out = var_rhs;

    /* Store to left hand side thing */
    {
        /* Identifier */
        if (assignmentexpression->left->tag == xjs_ast_expression_identifier)
        {
            if (ec_string_match_c_str(assignmentexpression->op, "=") == ec_true)
            {
                ec_string *new_name = NULL;
                XJS_VNZ_ERROR_MEM(new_name = ec_string_clone( \
                            assignmentexpression->left->u.xjs_ast_expression_identifier->name), ctx->err);
                XJS_VNZ_ERROR_MEM_OR(node_store = xjs_cfgbuilder_store_new(ctx->cfgb, new_name, var_rhs), \
                        ctx->err, ec_delete(new_name));
                CFG_LOC_RANGE_CLONE(node_store, assignmentexpression);
                new_name = NULL;

                XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_block, assignmentexpression);
                xjs_cfgbuilder_block_push_back(node_block, node_store);
                node_store = NULL;

                xjs_cfgbuilder_link(ctx->cfgb, node_rhs_out, node_block);

                XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_tail, assignmentexpression);
                xjs_cfgbuilder_link(ctx->cfgb, node_block, node_tail);
                *node_out = node_tail;
            }
            else
            {
                xjs_cfg_binary_op_type type;

                /* a += b;
                 * ->
                 * t0 = a;
                 * t1 = b;
                 * t2 = t0 + t1;
                 * a = t2; */

                if (ec_string_match_c_str(assignmentexpression->op, "+=") == ec_true)
                {
                    type = XJS_CFG_BINARY_OP_TYPE_ADD;
                }
                else if (ec_string_match_c_str(assignmentexpression->op, "-=") == ec_true)
                {
                    type = XJS_CFG_BINARY_OP_TYPE_SUB;
                }
                else if (ec_string_match_c_str(assignmentexpression->op, "*=") == ec_true)
                {
                    type = XJS_CFG_BINARY_OP_TYPE_MUL;
                }
                else if (ec_string_match_c_str(assignmentexpression->op, "/=") == ec_true)
                {
                    type = XJS_CFG_BINARY_OP_TYPE_DIV;
                }
                else if (ec_string_match_c_str(assignmentexpression->op, "%=") == ec_true)
                {
                    type = XJS_CFG_BINARY_OP_TYPE_MOD;
                }
                else
                {
                    xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                    goto fail;
                }

                XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_block, assignmentexpression);
                {
                    xjs_asm_block_decl(node_block);

                    xjs_asm_var(var_id);
                    xjs_asm_var(var_sum);

                    xjs_asm_allocate_var(var_id);
                    xjs_asm_load(var_id, ec_string_clone(assignmentexpression->left->u.xjs_ast_expression_identifier->name));
                    xjs_asm_allocate_var(var_sum);
                    xjs_asm_binop(type, var_sum, var_id, var_rhs);
                    xjs_asm_store(var_sum, ec_string_clone(assignmentexpression->left->u.xjs_ast_expression_identifier->name));
                }
                xjs_cfgbuilder_link(ctx->cfgb, node_rhs_out, node_block);

                XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_tail, assignmentexpression);
                xjs_cfgbuilder_link(ctx->cfgb, node_block, node_tail);
                *node_out = node_tail;
            }
        }
        else if (assignmentexpression->left->tag == xjs_ast_expression_memberexpression)
        {
            xjs_cfg_var var1;
            xjs_cfg_node_ref node1_in, node1_out;

            if (xjs_c0_expression_assignmentexpression_memberexpression( \
                        ctx, &var1, \
                        &node1_in, &node1_out, \
                        assignmentexpression->left->u.xjs_ast_expression_memberexpression, \
                        var_rhs) != 0)
            { goto fail; }

            xjs_cfgbuilder_link(ctx->cfgb, node_rhs_out, node1_in);
            *node_out = node1_out;
        }
        else
        {
            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
            goto fail;
        }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_updateexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_updateexpression_ref updateexpression)
{
    int ret = 0;

    if (updateexpression->argument->tag == xjs_ast_expression_identifier)
    {
        ec_string *var_name = updateexpression->argument->u.xjs_ast_expression_identifier->name;
        xjs_cfg_node_ref node_block = NULL, node_tail = NULL;

        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_block, updateexpression);
        XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_tail, updateexpression);
        {
            xjs_asm_block_decl(node_block);
            xjs_asm_var(v1);
            xjs_asm_var(v2);
            xjs_asm_var(v3);

            xjs_asm_allocate_var(v1);
            xjs_asm_load(v1, ec_string_clone(var_name));
            xjs_asm_allocate_var(v2);
            xjs_asm_load_int(v2, 1);
            xjs_asm_allocate_var(v3);
            if (ec_string_match_c_str(updateexpression->op, "++"))
            { xjs_asm_add(v3, v1, v2); }
            else if (ec_string_match_c_str(updateexpression->op, "--"))
            { xjs_asm_sub(v3, v1, v2); }
            else
            { xjs_c0_ctx_error(ctx, XJS_ERRNO_INTERNAL); goto fail; }
            xjs_asm_store(v3, ec_string_clone(var_name));
            xjs_asm_jump(node_tail);

            if (updateexpression->prefix == ec_false) { *var_out = v1; }
            else { *var_out = v3; }
        }

        *node_in = node_block;
        *node_out = node_tail;
    }
    else if (updateexpression->argument->tag == xjs_ast_expression_memberexpression)
    {
        xjs_ast_memberexpression_ref memberexpr = updateexpression->argument->u.xjs_ast_expression_memberexpression;
        xjs_cfg_node_ref node_block = NULL, node_tail = NULL;

        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_block, updateexpression);
        XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_tail, updateexpression);
        {
            xjs_cfg_var v_obj, v_prop;
            xjs_cfg_node_ref node_obj_out, node_prop_out;

            /* Object */
            {
                if (xjs_c0_expression(ctx, &v_obj, \
                            node_in, &node_obj_out, \
                            memberexpr->object) != 0)
                { goto fail; }
            }

            /* Property */
            if (memberexpr->computed == ec_false)
            {
                xjs_cfg_node_ref node_prop;
                XJS_V_ERROR_INTERNAL(memberexpr->property->tag == xjs_ast_expression_identifier, ctx->err);
                XJS_VNZ_ERROR_MEM(node_prop = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_prop, updateexpression);
                {
                    xjs_asm_block_decl(node_prop);
                    xjs_asm_allocate_var(v_prop);
                    xjs_asm_load_string(v_prop, \
                            ec_string_clone(memberexpr->property->u.xjs_ast_expression_identifier->name));
                }
                xjs_cfgbuilder_link(ctx->cfgb, node_obj_out, node_prop);
                node_prop_out = node_prop;
            }
            else
            {
                xjs_cfg_node_ref node_prop_in;
                if (xjs_c0_expression(ctx, &v_prop, \
                            &node_prop_in, &node_prop_out, \
                            memberexpr->property) != 0)
                { goto fail; }
                xjs_cfgbuilder_link(ctx->cfgb, node_obj_out, node_prop_in);
            }

            xjs_cfgbuilder_link(ctx->cfgb, node_prop_out, node_block);
            {
                xjs_asm_block_decl(node_block);
                xjs_asm_var(v1);
                xjs_asm_var(v2);
                xjs_asm_var(v3);
                xjs_asm_var(v4);

                xjs_asm_allocate_var(v1);
                xjs_asm_objget(v1, v_obj, v_prop);
                xjs_asm_allocate_var(v2);
                xjs_asm_load_int(v2, 1);
                xjs_asm_allocate_var(v3);
                if (ec_string_match_c_str(updateexpression->op, "++"))
                { xjs_asm_add(v3, v1, v2); }
                else if (ec_string_match_c_str(updateexpression->op, "--"))
                { xjs_asm_sub(v3, v1, v2); }
                xjs_asm_allocate_var(v4);
                xjs_asm_objset(v4, v_obj, v_prop, v3);

                xjs_asm_jump(node_tail);
                if (updateexpression->prefix == ec_false) { *var_out = v1; }
                else { *var_out = v4; }
            }

        }
        *node_out = node_tail;
    }
    else
    {
        C0_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :"
                "error: invalid left-hand side in assignment", \
                ctx->source_filename, updateexpression->loc.start.ln, updateexpression->loc.start.col);
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_functionexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_functionexpression_ref functionexpression)
{
    int ret = 0;
    xjs_cfg_var var_func;
    xjs_cfg_node_ref sub_function_node_final;

    xjs_cfg_parameter_list_ref parameters = NULL;
    xjs_cfg_function_ref new_cfg_function = NULL;

    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_make_function = NULL;

    if ((sub_function_node_final = xjs_c0_statementlist_function_scope_statementlistitem( \
                    ctx, functionexpression->body, ec_false)) == NULL)
    { goto fail; }

    /* Alloca */
    var_func = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_func), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, functionexpression);

    /* Append the global scope statements as a function */
    XJS_VNZ_ERROR_MEM(parameters = xjs_cfg_parameter_list_new(), ctx->err);
    {
        ect_iterator(xjs_ast_parameterlist) it_param;
        ect_for(xjs_ast_parameterlist, functionexpression->params, it_param)
        {
            xjs_ast_parameter_ref param = ect_deref(xjs_ast_parameter_ref, it_param);
            xjs_cfg_parameter_ref new_param = xjs_cfg_parameter_new(ec_string_clone(param->id->name));
            XJS_VNZ_ERROR_MEM(new_param, ctx->err);
            new_param->loc.start.ln = param->loc.start.ln;
            new_param->loc.start.col = param->loc.start.col;
            new_param->loc.end.ln = param->loc.end.ln;
            new_param->loc.end.col = param->loc.end.col;
            new_param->range.start = param->range.start;
            new_param->range.end = param->range.end;
            xjs_cfg_parameter_list_push_back(parameters, new_param);
        }
    }
    XJS_VNZ_ERROR_MEM(new_cfg_function = xjs_cfg_function_new(parameters, sub_function_node_final), ctx->err);
    parameters = NULL;

    /* Make function */
    XJS_VNZ_ERROR_MEM(node_make_function = xjs_cfgbuilder_make_function_new( \
                ctx->cfgb, new_cfg_function, var_func), ctx->err);
    CFG_LOC_RANGE_CLONE(node_make_function, functionexpression);

    /* Chain nodes together */ 
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, functionexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    node_alloca = NULL;
    xjs_cfgbuilder_block_push_back(node_block, node_make_function);
    node_make_function = NULL;
    *node_in = node_block;
    *node_out = node_block;
    *var_out = var_func;

    xjs_cfg_append_function(ctx->cfgb->cfg, new_cfg_function);
    new_cfg_function = NULL;

    goto done;
fail:
    ret = -1;
done:
    ec_delete(parameters);
    ec_delete(new_cfg_function);
    return ret;
}

static int 
xjs_c0_expression_arrowfunctionexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_arrowfunctionexpression_ref arrowfunctionexpression)
{
    int ret = 0;
    xjs_cfg_var var_func;

    xjs_cfg_parameter_list_ref parameters = NULL;
    xjs_cfg_function_ref new_cfg_function = NULL;

    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_make_function = NULL;

    xjs_cfg_node_ref node_body_in, node_body_out;

    switch (arrowfunctionexpression->body.type)
    {
        case xjs_opaque_ast_arrowfunctionexpression_body_blockstmt:
            {
                if (xjs_c0_statementlistitem(ctx, &node_body_in, &node_body_out, \
                        arrowfunctionexpression->body.u.as_blockstmt) != 0)
                { goto fail; }
            }
            break;
        case xjs_opaque_ast_arrowfunctionexpression_body_expr:
            {
                xjs_cfg_var v;
                if (xjs_c0_expression(ctx, &v, &node_body_in, &node_body_out, \
                        arrowfunctionexpression->body.u.as_expr) != 0)
                { goto fail; }
                {
                    xjs_cfg_node_ref node_func_tail, node_func_return;
                    XJS_VNZ_ERROR_MEM(node_func_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                    CFG_LOC_RANGE_CLONE(node_func_tail, arrowfunctionexpression);
                    XJS_VNZ_ERROR_MEM(node_func_return = xjs_cfgbuilder_return_new(ctx->cfgb, v), ctx->err);
                    CFG_LOC_RANGE_CLONE(node_func_return, arrowfunctionexpression);
                    xjs_cfgbuilder_block_push_back(node_func_tail, node_func_return);
                    xjs_cfgbuilder_link(ctx->cfgb, node_body_out, node_func_tail);
                }
            }
            break;
    }

    /* Alloca */
    var_func = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_func), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, arrowfunctionexpression);

    /* Append the global scope statements as a function */
    XJS_VNZ_ERROR_MEM(parameters = xjs_cfg_parameter_list_new(), ctx->err);
    {
        ect_iterator(xjs_ast_parameterlist) it_param;
        ect_for(xjs_ast_parameterlist, arrowfunctionexpression->params, it_param)
        {
            xjs_ast_parameter_ref param = ect_deref(xjs_ast_parameter_ref, it_param);
            xjs_cfg_parameter_ref new_param = xjs_cfg_parameter_new(ec_string_clone(param->id->name));
            XJS_VNZ_ERROR_MEM(new_param, ctx->err);
            new_param->loc.start.ln = param->loc.start.ln;
            new_param->loc.start.col = param->loc.start.col;
            new_param->loc.end.ln = param->loc.end.ln;
            new_param->loc.end.col = param->loc.end.col;
            new_param->range.start = param->range.start;
            new_param->range.end = param->range.end;
            xjs_cfg_parameter_list_push_back(parameters, new_param);
        }
    }
    XJS_VNZ_ERROR_MEM(new_cfg_function = xjs_cfg_function_new(parameters, node_body_in), ctx->err);
    parameters = NULL;

    /* Make arrow function */
    XJS_VNZ_ERROR_MEM(node_make_function = xjs_cfgbuilder_make_arrow_function_new( \
                ctx->cfgb, new_cfg_function, var_func), ctx->err);
    CFG_LOC_RANGE_CLONE(node_make_function, arrowfunctionexpression);

    /* Chain nodes together */ 
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, arrowfunctionexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    node_alloca = NULL;
    xjs_cfgbuilder_block_push_back(node_block, node_make_function);
    node_make_function = NULL;
    *node_in = node_block;
    *node_out = node_block;
    *var_out = var_func;

    xjs_cfg_append_function(ctx->cfgb->cfg, new_cfg_function);
    new_cfg_function = NULL;

    goto done;
fail:
    ret = -1;
done:
    ec_delete(parameters);
    ec_delete(new_cfg_function);
    return ret;
}

static int 
xjs_c0_expression_objectexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_objectexpression_ref objectexpression)
{
    int ret = 0;
    xjs_cfg_var var_object;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_load_object = NULL;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_cur;

    var_object = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_object), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, objectexpression);
    XJS_VNZ_ERROR_MEM(node_load_object = xjs_cfgbuilder_literal_object_new(ctx->cfgb, var_object), ctx->err);
    CFG_LOC_RANGE_CLONE(node_load_object, objectexpression);

    /* Group alloca and literal loading */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, objectexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_load_object);

    *node_in = node_block;
    *node_out = node_block;
    node_cur = node_block;
    node_block = NULL;

    {
        ect_iterator(xjs_ast_propertylist) it_property;
        ect_for(xjs_ast_propertylist, objectexpression->properties, it_property)
        {
            xjs_ast_property_ref prop = ect_deref(xjs_ast_property_ref, it_property);

            /* value */
            {
                xjs_cfg_var var_dst, var_key, var_value;
                xjs_cfg_node_ref node_value_in, node_value_out;
                xjs_cfg_node_ref node_key_in, node_key_out;
                xjs_cfg_node_ref node_obj_set;

                if (xjs_c0_expression(ctx, &var_key, \
                            &node_key_in, &node_key_out, prop->key) != 0)
                { goto fail; }

                if (xjs_c0_expression(ctx, &var_value, \
                            &node_value_in, &node_value_out, prop->value) != 0)
                { goto fail; }

                var_dst = 0;

                if ((node_obj_set = xjs_cfgbuilder_object_set_new( \
                        ctx->cfgb, var_dst, var_object, var_key, var_value)) == NULL)
                { goto fail; }
                CFG_LOC_RANGE_CLONE(node_obj_set, objectexpression);

                /* Useless */
                (void)var_dst;

                XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_block, objectexpression);
                xjs_cfgbuilder_block_push_back(node_block, node_obj_set);

                xjs_cfgbuilder_link(ctx->cfgb, node_cur, node_key_in);
                xjs_cfgbuilder_link(ctx->cfgb, node_key_out, node_value_in);
                xjs_cfgbuilder_link(ctx->cfgb, node_value_out, node_block);
                node_cur = node_block;
                *node_out = node_block;
            }
        }
    }

    *var_out = var_object;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_arrayexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_arrayexpression_ref arrayexpression)
{
    int ret = 0;
    xjs_cfg_var var_array;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_load_array = NULL;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_cur;

    var_array = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_array), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, arrayexpression);
    XJS_VNZ_ERROR_MEM(node_load_array = xjs_cfgbuilder_literal_array_new(ctx->cfgb, var_array), ctx->err);
    CFG_LOC_RANGE_CLONE(node_load_array, arrayexpression);

    /* Group alloca and literal loading */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, arrayexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_load_array);

    *node_in = node_block;
    *node_out = node_block;
    node_cur = node_block;
    node_block = NULL;

    {
        ect_iterator(xjs_ast_expressionlist) it_elem;
        ect_for(xjs_ast_expressionlist, arrayexpression->elements, it_elem)
        {
            xjs_ast_expression_ref elem = ect_deref(xjs_ast_expression_ref, it_elem);

            /* value */
            {
                xjs_cfg_var var_element;
                xjs_cfg_node_ref node_key_in, node_key_out;
                xjs_cfg_node_ref node_array_push;

                if (xjs_c0_expression(ctx, &var_element, \
                            &node_key_in, &node_key_out, elem) != 0)
                { goto fail; }

                if ((node_array_push = xjs_cfgbuilder_array_push_new( \
                        ctx->cfgb, var_array, var_element)) == NULL)
                { goto fail; }
                CFG_LOC_RANGE_CLONE(node_array_push, arrayexpression);

                XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_block, arrayexpression);
                xjs_cfgbuilder_block_push_back(node_block, node_array_push);

                xjs_cfgbuilder_link(ctx->cfgb, node_cur, node_key_in);
                xjs_cfgbuilder_link(ctx->cfgb, node_key_out, node_block);
                node_cur = node_block;
                *node_out = node_block;
            }
        }
    }

    *var_out = var_array;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_memberexpression_expose_this( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, \
        xjs_cfg_var *var_this_out, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_memberexpression_ref memberexpression);

static int 
xjs_c0_expression_callexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_callexpression_ref callexpression)
{
    int ret = 0;
    xjs_cfg_var var_callee, var_callee_obj = 0, var_dst;
    xjs_cfg_var_list_ref var_arguments = NULL;
    xjs_cfg_node_ref node_last;
    xjs_cfg_node_ref node_call, node_block, node_tail, node_alloca;

    if (callexpression->callee->tag == xjs_ast_expression_memberexpression)
    {
        xjs_ast_memberexpression_ref memberexpression;
        memberexpression = callexpression->callee->u.xjs_ast_expression_memberexpression;

        /* In the following situation:
         * a.b();
         * 'this' bound to object of call expression of callee. */

        if (xjs_c0_expression_memberexpression_expose_this( \
                    ctx, \
                    &var_callee, &var_callee_obj, \
                    node_in, &node_last, \
                    memberexpression) != 0)
        { goto fail; }
    }
    else
    {
        /* otherwise, just evaluate callee */

        /* Callee */
        if (xjs_c0_expression(ctx, \
                    &var_callee, \
                    node_in, &node_last, \
                    callexpression->callee) != 0)
        { goto fail; }
    }
    
    /* Arguments */
    XJS_VNZ_ERROR_MEM(var_arguments = ect_list_new(xjs_cfg_var_list), ctx->err);
    {
        ect_iterator(xjs_ast_expressionlist) it_arg;
        ect_for(xjs_ast_expressionlist, callexpression->arguments, it_arg)
        {
            xjs_cfg_node_ref node_argument_in, node_argument_out;
            xjs_cfg_var var_argument;
            xjs_ast_expression_ref argument = ect_deref(xjs_ast_expression_ref, it_arg);
            if (xjs_c0_expression(ctx, 
                        &var_argument, \
                        &node_argument_in, &node_argument_out, \
                        argument) != 0)
            { goto fail; }
            ect_list_push_back(xjs_cfg_var_list, var_arguments, var_argument);
            xjs_cfgbuilder_link(ctx->cfgb, node_last, node_argument_in);
            node_last = node_argument_out;
        }
    }

    /* Call */
    {
        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_block, callexpression);
        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, callexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca); node_alloca = NULL;

        if (callexpression->callee->tag == xjs_ast_expression_memberexpression)
        {
            /* Call with 'this' bound */
            XJS_VNZ_ERROR_MEM(node_call = xjs_cfgbuilder_call_bound_this_new(ctx->cfgb, var_dst, var_callee, var_arguments, var_callee_obj), ctx->err);
            CFG_LOC_RANGE_CLONE(node_call, callexpression);
            var_arguments = NULL;
            xjs_cfgbuilder_block_push_back(node_block, node_call);
        }
        else
        {
            /* Call */
            XJS_VNZ_ERROR_MEM(node_call = xjs_cfgbuilder_call_new(ctx->cfgb, var_dst, var_callee, var_arguments), ctx->err);
            CFG_LOC_RANGE_CLONE(node_call, callexpression);
            var_arguments = NULL;
            xjs_cfgbuilder_block_push_back(node_block, node_call);
        }

        xjs_cfgbuilder_link(ctx->cfgb, node_last, node_block);

        XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_tail, callexpression);
        xjs_cfgbuilder_link(ctx->cfgb, node_block, node_tail);

        *var_out = var_dst;
        *node_out = node_tail;
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(var_arguments);
    return ret;
}

static int 
xjs_c0_expression_memberexpression_expose_this( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, \
        xjs_cfg_var *var_this_out, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_memberexpression_ref memberexpression)
{
    int ret = 0;
    xjs_cfg_var var_obj, var_key, var_dst;
    xjs_cfg_node_ref node_object_out;
    xjs_cfg_node_ref node_key_in, node_key_out;

    if (xjs_c0_expression(ctx, &var_obj, node_in, &node_object_out, memberexpression->object) != 0)
    { goto fail; }

    *var_this_out = var_obj;

    if (memberexpression->computed == ec_true)
    {
        if (xjs_c0_expression(ctx, &var_key, &node_key_in, &node_key_out, memberexpression->property) != 0)
        { goto fail; }
    }
    else
    {
        if (memberexpression->property->tag == xjs_ast_expression_identifier)
        {
            xjs_cfg_var var_lit;
            xjs_cfg_node_ref node_lit = NULL, node_block = NULL, node_alloca = NULL;
            ec_string *name = memberexpression->property->u.xjs_ast_expression_identifier->name;
            var_lit = xjs_cfgbuilder_allocate_var(ctx->cfgb);
            XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_lit), ctx->err);
            CFG_LOC_RANGE_CLONE(node_alloca, memberexpression);
            XJS_VNZ_ERROR_MEM( \
                    node_lit = xjs_cfgbuilder_literal_string_new( \
                        ctx->cfgb, ec_string_clone(name), var_lit), ctx->err);
            CFG_LOC_RANGE_CLONE(node_lit, memberexpression);
            XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
            CFG_LOC_RANGE_CLONE(node_block, memberexpression);
            xjs_cfgbuilder_block_push_back(node_block, node_alloca);
            xjs_cfgbuilder_block_push_back(node_block, node_lit);
            var_key = var_lit; node_key_in = node_block; node_key_out = node_block;
        }
        else
        { xjs_c0_ctx_error(ctx, XJS_ERRNO_INTERNAL); goto fail; }
    }

    xjs_cfgbuilder_link(ctx->cfgb, node_object_out, node_key_in);
    {
        xjs_cfg_node_ref node_block = NULL;
        xjs_cfg_node_ref node_alloca = NULL;
        xjs_cfg_node_ref node_obj_get;

        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_block, memberexpression);
        xjs_cfgbuilder_link(ctx->cfgb, node_key_out, node_block);

        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, memberexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca); node_alloca = NULL;

        if ((node_obj_get = xjs_cfgbuilder_object_get_new( \
                        ctx->cfgb, var_dst, var_obj, var_key)) == NULL)
        { goto fail; }
        CFG_LOC_RANGE_CLONE(node_obj_get, memberexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_obj_get); node_obj_get = NULL;

        *var_out = var_dst;
        *node_out = node_block;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_memberexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_memberexpression_ref memberexpression)
{
    int ret;
    xjs_cfg_var var_this;

    ret = xjs_c0_expression_memberexpression_expose_this( \
        ctx, \
        var_out, &var_this, \
        node_in, node_out, \
        memberexpression);

    return ret;
}

static int 
xjs_c0_expression_newexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_newexpression_ref newexpression)
{
    int ret = 0;
    xjs_cfg_var var_callee, var_dst;
    xjs_cfg_var_list_ref var_arguments = NULL;
    xjs_cfg_node_ref node_callee_out, node_last;
    xjs_cfg_node_ref node_new, node_block, node_tail, node_alloca;

    /* Callee */
    if (xjs_c0_expression(ctx, \
                &var_callee, node_in, &node_callee_out, \
                newexpression->callee) != 0)
    { goto fail; }
    node_last = node_callee_out;
    
    /* Arguments */
    XJS_VNZ_ERROR_MEM(var_arguments = ect_list_new(xjs_cfg_var_list), ctx->err);
    {
        ect_iterator(xjs_ast_expressionlist) it_arg;
        ect_for(xjs_ast_expressionlist, newexpression->arguments, it_arg)
        {
            xjs_cfg_node_ref node_argument_in, node_argument_out;
            xjs_cfg_var var_argument;
            xjs_ast_expression_ref argument = ect_deref(xjs_ast_expression_ref, it_arg);
            if (xjs_c0_expression(ctx, 
                        &var_argument, \
                        &node_argument_in, &node_argument_out, \
                        argument) != 0)
            { goto fail; }
            ect_list_push_back(xjs_cfg_var_list, var_arguments, var_argument);
            xjs_cfgbuilder_link(ctx->cfgb, node_last, node_argument_in);
            node_last = node_argument_out;
        }
    }

    /* Call */
    {
        XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_block, newexpression);

        /* Alloca */
        var_dst = xjs_cfgbuilder_allocate_var(ctx->cfgb);
        XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_dst), ctx->err);
        CFG_LOC_RANGE_CLONE(node_alloca, newexpression);
        xjs_cfgbuilder_block_push_back(node_block, node_alloca); node_alloca = NULL;
        /* Call */
        XJS_VNZ_ERROR_MEM(node_new = xjs_cfgbuilder_new_new(ctx->cfgb, var_dst, var_callee, var_arguments), ctx->err);
        CFG_LOC_RANGE_CLONE(node_new, newexpression);
        var_arguments = NULL;
        xjs_cfgbuilder_block_push_back(node_block, node_new);

        xjs_cfgbuilder_link(ctx->cfgb, node_last, node_block);

        XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        CFG_LOC_RANGE_CLONE(node_tail, newexpression);
        xjs_cfgbuilder_link(ctx->cfgb, node_block, node_tail);

        *var_out = var_dst;
        *node_out = node_tail;
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(var_arguments);
    return ret;
}

static int 
xjs_c0_expression_conditionalexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_conditionalexpression_ref conditionalexpression)
{
    int ret = 0;
    xjs_cfg_var var_test, var_consequent, var_alternate;
    xjs_cfg_var var_merged;
    xjs_cfg_node_ref node_test_out = NULL;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_tail = NULL;
    xjs_cfg_node_ref node_branch = NULL, node_merge = NULL;
    xjs_cfg_node_ref node_consequent_in = NULL, node_consequent_out = NULL;
    xjs_cfg_node_ref node_alternate_in = NULL, node_alternate_out = NULL;

    /* <test> -> <block:[<branch>]>   <tail:[alloca,merge]> */
    /*                   <consequent> ---------|            */
    /*                   <alternate>-----------+            */

    /* test */
    if (xjs_c0_expression(ctx, &var_test, node_in, &node_test_out, \
                conditionalexpression->test) != 0)
    { goto fail; }

    /* consequent */
    if (xjs_c0_expression(ctx, &var_consequent, &node_consequent_in, &node_consequent_out, \
                conditionalexpression->consequent) != 0)
    { goto fail; }

    /* alternate */
    if (xjs_c0_expression(ctx, &var_alternate, &node_alternate_in, &node_alternate_out, \
                conditionalexpression->alternate) != 0)
    { goto fail; }

    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, conditionalexpression);
    XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_tail, conditionalexpression);

    xjs_cfgbuilder_link(ctx->cfgb, node_consequent_out, node_tail);
    xjs_cfgbuilder_link(ctx->cfgb, node_alternate_out, node_tail);

    XJS_VNZ_ERROR_MEM(node_branch = xjs_cfgbuilder_branch_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_branch, conditionalexpression);
    xjs_cfgbuilder_branch_cond_set(node_branch, var_test);
    xjs_cfgbuilder_branch_true_branch_set(node_branch, node_consequent_in);
    xjs_cfgbuilder_branch_false_branch_set(node_branch, node_alternate_in);
    xjs_cfgbuilder_block_push_back(node_block, node_branch);

    xjs_cfgbuilder_link(ctx->cfgb, node_test_out, node_block);

    /* Alloca */
    var_merged = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_merged), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, conditionalexpression);
    /* Merge */
    XJS_VNZ_ERROR_MEM(node_merge = xjs_cfgbuilder_merge_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_merge, conditionalexpression);
    xjs_cfgbuilder_merge_test_set(node_merge, var_test);
    xjs_cfgbuilder_merge_consequent_set(node_merge, var_consequent);
    xjs_cfgbuilder_merge_alternate_set(node_merge, var_alternate);
    xjs_cfgbuilder_merge_dst_set(node_merge, var_merged);
    xjs_cfgbuilder_block_push_back(node_tail, node_alloca);
    xjs_cfgbuilder_block_push_back(node_tail, node_merge);

    *var_out = var_merged;
    *node_out = node_tail;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_expression_thisexpression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_thisexpression_ref thisexpression)
{
    int ret = 0;
    xjs_cfg_var var_this;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_alloca = NULL;
    xjs_cfg_node_ref node_this = NULL;

    var_this = xjs_cfgbuilder_allocate_var(ctx->cfgb);
    XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var_this), ctx->err);
    CFG_LOC_RANGE_CLONE(node_alloca, thisexpression);
    XJS_VNZ_ERROR_MEM(node_this = xjs_cfgbuilder_this_new(ctx->cfgb, var_this), ctx->err);
    CFG_LOC_RANGE_CLONE(node_this, thisexpression);
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, thisexpression);
    xjs_cfgbuilder_block_push_back(node_block, node_alloca);
    xjs_cfgbuilder_block_push_back(node_block, node_this);

    *var_out = var_this;
    *node_in = node_block;
    *node_out = node_block;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_expression( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_var *var_out, xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_expression_ref expr)
{
    int ret = 0;

    switch (expr->tag)
    {
        case xjs_ast_expression_literal:
            ret = xjs_c0_expression_literal( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_literal);
            break;

        case xjs_ast_expression_identifier:
            ret = xjs_c0_expression_identifier( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_identifier);
            break;

        case xjs_ast_expression_unaryexpression:
            ret = xjs_c0_expression_unaryexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_unaryexpression);
            break;

        case xjs_ast_expression_binaryexpression:
            ret = xjs_c0_expression_binaryexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_binaryexpression);
            break;

        case xjs_ast_expression_logicalexpression:
            ret = xjs_c0_expression_logicalexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_logicalexpression);
            break;

        case xjs_ast_expression_assignmentexpression:
            ret = xjs_c0_expression_assignmentexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_assignmentexpression);
            break;

        case xjs_ast_expression_updateexpression:
            ret = xjs_c0_expression_updateexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_updateexpression);
            break;

        case xjs_ast_expression_functionexpression:
            ret = xjs_c0_expression_functionexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_functionexpression);
            break;

        case xjs_ast_expression_arrowfunctionexpression:
            ret = xjs_c0_expression_arrowfunctionexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_arrowfunctionexpression);
            break;

        case xjs_ast_expression_objectexpression:
            ret = xjs_c0_expression_objectexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_objectexpression);
            break;

        case xjs_ast_expression_arrayexpression:
            ret = xjs_c0_expression_arrayexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_arrayexpression);
            break;

        case xjs_ast_expression_callexpression:
            ret = xjs_c0_expression_callexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_callexpression);
            break;

        case xjs_ast_expression_memberexpression:
            ret = xjs_c0_expression_memberexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_memberexpression);
            break;

        case xjs_ast_expression_newexpression:
            ret = xjs_c0_expression_newexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_newexpression);
            break;

        case xjs_ast_expression_conditionalexpression:
            ret = xjs_c0_expression_conditionalexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_conditionalexpression);
            break;

        case xjs_ast_expression_thisexpression:
            ret = xjs_c0_expression_thisexpression( \
                    ctx, \
                    var_out, node_in, node_out, \
                    expr->u.xjs_ast_expression_thisexpression);
            break;
    }

    return ret;
}

static int
xjs_c0_expressionstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_expressionstatement_ref expr_stmt)
{
    int ret = 0;
    xjs_cfg_var var_out;

    /* Expression */
    if (xjs_c0_expression(ctx, \
                &var_out, \
                node_in, node_out, expr_stmt->expression) != 0)
    { ret = -1; goto fail; }

fail:
    return ret;
}

static int
xjs_c0_inspectstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_inspectstatement_ref inspect_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_inspect = NULL, node_argument_out;

    /* Block */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, inspect_stmt);

    /* Test */
    {
        xjs_cfg_var var_argument;

        /* Expose condition as entry point */
        if (xjs_c0_expression(ctx, \
                    &var_argument, node_in, &node_argument_out, \
                    inspect_stmt->argument) != 0) { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_argument_out, node_block);
        /* Inspect */
        XJS_VNZ_ERROR_MEM( \
                node_inspect = xjs_cfgbuilder_inspect_new(ctx->cfgb, var_argument), \
                ctx->err);
        CFG_LOC_RANGE_CLONE(node_inspect, inspect_stmt);
        xjs_cfgbuilder_block_push_back(node_block, node_inspect);

        *node_out = node_block;
        node_inspect = NULL;
        node_block = NULL;
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(node_inspect);
    ec_delete(node_block);
    return ret;
}

static int
xjs_c0_returnstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_returnstatement_ref return_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_block = NULL;
    xjs_cfg_node_ref node_return = NULL, node_argument_out;

    /* Block */
    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, return_stmt);

    /* Test */
    {
        xjs_cfg_var var_argument;

        /* Expose condition as entry point */
        if (xjs_c0_expression(ctx, \
                    &var_argument, node_in, &node_argument_out, \
                    return_stmt->argument) != 0) { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_argument_out, node_block);
        /* Return */
        XJS_VNZ_ERROR_MEM( \
                node_return = xjs_cfgbuilder_return_new(ctx->cfgb, var_argument), \
                ctx->err);
        CFG_LOC_RANGE_CLONE(node_return, return_stmt);
        xjs_cfgbuilder_block_push_back(node_block, node_return);

        *node_out = node_block;
        node_return = NULL;
        node_block = NULL;
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(node_return);
    ec_delete(node_block);
    return ret;
}

static int
xjs_c0_breakstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_breakstatement_ref break_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_block = NULL, node_tail = NULL;
    xjs_cfg_node_ref break_point = NULL;

    if ((break_point = xjs_c0_ctx_top_break_point(ctx)) == NULL)
    {
        C0_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :"
                "error: break statement not within loop or switch", \
                ctx->source_filename, break_stmt->loc.start.ln, break_stmt->loc.start.col + 1);
    }

    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, break_stmt);
    {
        xjs_asm_block_decl(node_block);

        xjs_asm_jump(break_point);
    }
    XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_tail, break_stmt);

    *node_in = node_block;
    *node_out = node_tail;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_continuestatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_continuestatement_ref continue_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_block = NULL, node_tail = NULL;
    xjs_cfg_node_ref continue_point = NULL;

    if ((continue_point = xjs_c0_ctx_top_continue_point(ctx)) == NULL)
    {
        C0_ERROR_PRINTF("{c_str}:{size_t}:{size_t} :"
                "error: continue statement not within a loop", \
                ctx->source_filename, continue_stmt->loc.start.ln, continue_stmt->loc.start.col + 1);
    }

    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_block, continue_stmt);
    {
        xjs_asm_block_decl(node_block);

        xjs_asm_jump(continue_point);
    }
    XJS_VNZ_ERROR_MEM(node_tail = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_tail, continue_stmt);

    *node_in = node_block;
    *node_out = node_tail;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_ifstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_ifstatement_ref if_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_cond_out;
    xjs_cfg_node_ref node_entry, node_branch, node_merge;

    /* 
     * Cond
     * Entry
     * Branch Point 
     *   Consequent, Alternate
     * Merge Point */

    /* Entry Point */
    XJS_VNZ_ERROR_MEM(node_entry = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_entry, if_stmt);

    /* Branch Point */
    XJS_VNZ_ERROR_MEM(node_branch = xjs_cfgbuilder_branch_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_branch, if_stmt);

    /* Merge Point */
    XJS_VNZ_ERROR_MEM(node_merge = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_merge, if_stmt);

    /* Test */
    {
        xjs_cfg_var var_cond;

        /* Expose condition as entry point */
        if (xjs_c0_expression(ctx, \
                    &var_cond, node_in, &node_cond_out, \
                    if_stmt->test) != 0) { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_cond_out, node_entry);
        xjs_cfgbuilder_block_push_back(node_entry, node_branch);
        xjs_cfgbuilder_branch_cond_set(node_branch, var_cond);
    }

    /* Consequent */
    {
        xjs_cfg_node_ref node_consequent_in, node_consequent_out;
        if (xjs_c0_statementlistitem(ctx, \
                    &node_consequent_in, &node_consequent_out, \
                    if_stmt->consequent) != 0)
        { goto fail; }
        xjs_cfgbuilder_link(ctx->cfgb, node_consequent_out, node_merge);
        xjs_cfgbuilder_branch_true_branch_set(node_branch, node_consequent_in);
    }

    /* Alternate */
    if (if_stmt->alternate != NULL)
    {
        xjs_cfg_node_ref node_alternate_in, node_alternate_out;
        if ((xjs_c0_statementlistitem(ctx, \
                        &node_alternate_in, &node_alternate_out, \
                        if_stmt->alternate)) != 0)
        { goto fail; }
        xjs_cfgbuilder_link(ctx->cfgb, node_alternate_out, node_merge);
        xjs_cfgbuilder_branch_false_branch_set(node_branch, node_alternate_in);
    }
    else
    {
        xjs_cfgbuilder_branch_false_branch_set(node_branch, node_merge);
    }

    *node_out = node_merge;

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c0_whilestatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_whilestatement_ref while_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_prelude, node_entry, node_branch, node_again, node_merge;
    xjs_cfg_node_ref node_cond_out;
    xjs_bool bp_pushed = xjs_false;

    /* 
     * Cond
     * Entry
     * Branch Point (also Continue Point)
     *   Body
     *   Again
     * Merge Point (also Break Point) */

    XJS_VNZ_ERROR_MEM(node_prelude = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_prelude, while_stmt);
    XJS_VNZ_ERROR_MEM(node_entry = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_entry, while_stmt);
    XJS_VNZ_ERROR_MEM(node_branch = xjs_cfgbuilder_branch_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_branch, while_stmt);
    XJS_VNZ_ERROR_MEM(node_again = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_again, while_stmt);
    XJS_VNZ_ERROR_MEM(node_merge = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_merge, while_stmt);

    XJS_VEZ_ERROR_INTERNAL(xjs_c0_ctx_push_break_continue_points(ctx, node_merge, node_prelude), ctx->err);
    bp_pushed = xjs_true;

    *node_in = node_prelude;

    /* Test */
    {
        xjs_cfg_var var_cond;
        xjs_cfg_node_ref node_test_in;

        /* Expose condition as entry point */
        if (xjs_c0_expression(ctx, \
                    &var_cond, &node_test_in, &node_cond_out, \
                    while_stmt->test) != 0) { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_prelude, node_test_in);
        xjs_cfgbuilder_link(ctx->cfgb, node_cond_out, node_entry);
        xjs_cfgbuilder_block_push_back(node_entry, node_branch);
        xjs_cfgbuilder_branch_cond_set(node_branch, var_cond);
        xjs_cfgbuilder_branch_false_branch_set(node_branch, node_merge);
    }
    
    /* Body */
    {
        xjs_cfg_node_ref node_body_in, node_body_out;
        if (xjs_c0_statementlistitem(ctx, \
                    &node_body_in, &node_body_out, \
                    while_stmt->body) != 0)
        { goto fail; }
        xjs_cfgbuilder_link(ctx->cfgb, node_body_out, node_again);
        xjs_cfgbuilder_link(ctx->cfgb, node_again, node_prelude);
        xjs_cfgbuilder_branch_true_branch_set(node_branch, node_body_in);
    }

    *node_out = node_merge;

    goto done;
fail:
    ret = -1;
done:
    if (bp_pushed == xjs_true) xjs_c0_ctx_pop_break_continue_points(ctx);
    return ret;
}

static int
xjs_c0_dostatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_dostatement_ref do_stmt)
{
    int ret = 0;
    xjs_cfg_node_ref node_body_out;
    xjs_cfg_node_ref node_cond_in, node_cond_out;
    xjs_cfg_node_ref node_prelude, node_entry, node_branch, node_merge;
    xjs_bool bp_pushed = xjs_false;

    /* 
     * Prelude (also Continue Point)
     * Branch Point 
     *   Body
     *   Entry
     * Merge Point (also Break Point) */

    XJS_VNZ_ERROR_MEM(node_prelude = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_prelude, do_stmt);
    XJS_VNZ_ERROR_MEM(node_entry = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_entry, do_stmt);
    XJS_VNZ_ERROR_MEM(node_branch = xjs_cfgbuilder_branch_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_branch, do_stmt);
    XJS_VNZ_ERROR_MEM(node_merge = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_merge, do_stmt);

    XJS_VEZ_ERROR_INTERNAL(xjs_c0_ctx_push_break_continue_points(ctx, node_merge, node_merge), ctx->err);
    bp_pushed = xjs_true;

    *node_in = node_prelude;

    /* Body */
    {
        xjs_cfg_node_ref node_body_in;

        if (xjs_c0_statementlistitem(ctx, \
                    &node_body_in, &node_body_out, \
                    do_stmt->body) != 0)
        { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_prelude, node_body_in);
    }

    /* Test */
    {
        xjs_cfg_var var_cond;

        if (xjs_c0_expression(ctx, \
                    &var_cond, &node_cond_in, &node_cond_out, \
                    do_stmt->test) != 0) { goto fail; }
        xjs_cfgbuilder_link(ctx->cfgb, node_body_out, node_cond_in);
        xjs_cfgbuilder_link(ctx->cfgb, node_cond_out, node_entry);
        xjs_cfgbuilder_block_push_back(node_entry, node_branch);
        xjs_cfgbuilder_branch_cond_set(node_branch, var_cond);
        xjs_cfgbuilder_branch_true_branch_set(node_branch, *node_in);
        xjs_cfgbuilder_branch_false_branch_set(node_branch, node_merge);
    }

    *node_out = node_merge;

    goto done;
fail:
    ret = -1;
done:
    if (bp_pushed == xjs_true) xjs_c0_ctx_pop_break_continue_points(ctx);
    return ret;
}

static int
xjs_c0_forstatement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_forstatement_ref for_stmt)
{
    int ret = 0;
    /* xjs_cfg_node_ref node_body_out; */
    /* xjs_cfg_node_ref node_cond_in, node_cond_out; */
    xjs_cfg_node_ref node_init, node_test, node_update, node_merge;
    xjs_cfg_node_ref node_body_in, node_body_out;
    xjs_bool bp_pushed = xjs_false;

    XJS_VNZ_ERROR_MEM(node_init = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_init, for_stmt);
    XJS_VNZ_ERROR_MEM(node_test = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_test, for_stmt);
    XJS_VNZ_ERROR_MEM(node_update = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_update, for_stmt);
    XJS_VNZ_ERROR_MEM(node_merge = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    CFG_LOC_RANGE_CLONE(node_merge, for_stmt);

    XJS_VEZ_ERROR_INTERNAL(xjs_c0_ctx_push_break_continue_points(ctx, node_merge, node_test), ctx->err);
    bp_pushed = xjs_true;

    *node_in = node_init;

    /* Init */
    switch (for_stmt->init.type)
    {
        case xjs_ast_forstatement_init_expr:
            {
                xjs_cfg_var var_init;
                xjs_cfg_node_ref node_init_in, node_init_out;

                if (xjs_c0_expression(ctx, \
                            &var_init, &node_init_in, &node_init_out, \
                            for_stmt->init.u.as_expr) != 0) { goto fail; }

                xjs_cfgbuilder_link(ctx->cfgb, node_init_out, node_test);

                {
                    xjs_asm_block_decl(node_init);
                    {
                        xjs_asm_jump(node_init_in);
                    }
                }
            }
            break;

        case xjs_ast_forstatement_init_vardecl:
            XJS_ERROR_NOTIMP(ctx->err);
            goto fail;

        case xjs_ast_forstatement_init_null:
            {
                xjs_asm_block_decl(node_init);
                {
                    xjs_asm_jump(node_test);
                }
            }
            break;
    }

    /* Body */
    {
        if (xjs_c0_statementlistitem(ctx, \
                    &node_body_in, &node_body_out, \
                    for_stmt->body) != 0)
        { goto fail; }

        xjs_cfgbuilder_link(ctx->cfgb, node_body_out, node_update);
    }

    /* Test */
    switch (for_stmt->test.type)
    {
        case xjs_ast_forstatement_test_expr:
            {
                xjs_cfg_var var_test;
                xjs_cfg_node_ref node_test_in, node_test_out;
                xjs_cfg_node_ref node_test2;

                XJS_VNZ_ERROR_MEM(node_test2 = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                CFG_LOC_RANGE_CLONE(node_test2, for_stmt);

                if (xjs_c0_expression(ctx, \
                            &var_test, &node_test_in, &node_test_out, \
                            for_stmt->test.u.as_expr) != 0) { goto fail; }

                {
                    xjs_asm_block_decl(node_test);
                    {
                        xjs_asm_jump(node_test_in);
                    }
                }

                xjs_cfgbuilder_link(ctx->cfgb, node_test_out, node_test2);

                {
                    xjs_asm_block_decl(node_test2);
                    {
                        xjs_asm_branch(var_test, node_body_in, node_merge);
                    }
                }
            }
            break;

        case xjs_ast_forstatement_test_null:
            /* Always true */
            {
                xjs_asm_block_decl(node_test);
                {
                    xjs_asm_jump(node_body_in);
                }
            }
            break;
    }

    /* Update */
    switch (for_stmt->update.type)
    {
        case xjs_ast_forstatement_update_expr:
            {
                xjs_cfg_var var_update;
                xjs_cfg_node_ref node_update_in, node_update_out;

                if (xjs_c0_expression(ctx, \
                            &var_update, &node_update_in, &node_update_out, \
                            for_stmt->update.u.as_expr) != 0) { goto fail; }

                {
                    xjs_asm_block_decl(node_update);
                    {
                        xjs_asm_jump(node_update_in);
                    }
                }

                xjs_cfgbuilder_link(ctx->cfgb, node_update_out, node_test);
            }
            break;

        case xjs_ast_forstatement_update_null:
            {
                xjs_asm_block_decl(node_update);
                {
                    xjs_asm_jump(node_test);
                }
            }
            break;
    }
    *node_out = node_merge;

    goto done;
fail:
    ret = -1;
done:
    if (bp_pushed == xjs_true) xjs_c0_ctx_pop_break_continue_points(ctx);
    return ret;
}

static int
xjs_c0_statement( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_statement_ref stmt)
{
    int ret = 0;

    switch (stmt->tag)
    {
        case xjs_ast_statement_emptystatement:
            {
                /* Nothing to do, we place an empty block */
                xjs_cfg_node_ref new_block = NULL;
                XJS_VNZ_ERROR_MEM(new_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                *node_in = *node_out = new_block;
            }
            break;

        case xjs_ast_statement_blockstatement:
            {
                if (xjs_c0_blockstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_blockstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_expressionstatement:
            {
                if (xjs_c0_expressionstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_expressionstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_inspectstatement:
            {
                if (xjs_c0_inspectstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_inspectstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_returnstatement:
            {
                if (xjs_c0_returnstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_returnstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_breakstatement:
            {
                if (xjs_c0_breakstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_breakstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_continuestatement:
            {
                if (xjs_c0_continuestatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_continuestatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_ifstatement:
            {
                if (xjs_c0_ifstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_ifstatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_whilestatement:
            {
                if (xjs_c0_whilestatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_whilestatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_dostatement:
            {
                if (xjs_c0_dostatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_dostatement) != 0)
                { goto fail; }
            }
            break;

        case xjs_ast_statement_forstatement:
            {
                if (xjs_c0_forstatement(ctx, \
                            node_in, node_out, \
                            stmt->u.xjs_ast_statement_forstatement) != 0)
                { goto fail; }
            }
            break;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int 
xjs_c0_statementlistitem( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_statementlistitem_ref item)
{
    int ret = 0;

    switch (item->tag)
    {
        case xjs_ast_statementlistitem_statement:
            if (xjs_c0_statement(ctx, node_in, node_out, \
                    item->u.xjs_ast_statementlistitem_statement) != 0)
            { ret = -1; goto fail; }
            break;

        case xjs_ast_statementlistitem_declaration:
            if (xjs_c0_declaration( \
                    ctx, node_in, node_out, \
                    item->u.xjs_ast_statementlistitem_declaration) != 0)
            { ret = -1; goto fail; }
            break;
    }

fail:
    return ret;
}

static int
xjs_c0_statementlist( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_statementlist_ref statements)
{
    int ret = 0;
    xjs_cfg_node_ref node_base;
    ect_iterator(xjs_ast_statementlist) it;

    /* Initialize Base */
    XJS_VNZ_ERROR_MEM(node_base = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    *node_in = node_base;

    /* Block items */
    ect_for(xjs_ast_statementlist, statements, it)
    {
        xjs_ast_statementlistitem_ref item = NULL;
        item = ect_deref(xjs_ast_statementlistitem_ref, it);
        {
            xjs_cfg_node_ref node_in2, node_out2;
            if (xjs_c0_statementlistitem(ctx, \
                        &node_in2, &node_out2, item) != 0)
            { ret = -1; goto fail; }

            xjs_cfgbuilder_link(ctx->cfgb, node_base, node_in2);
            node_base = node_out2;
        }
    }

    *node_out = node_base;

    goto done;
fail:
done:
    return ret;
}

static int xjs_c0_exportdeclaration_default(
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_exportdeclaration_ref export_decl)
{
    int ret = 0;

    switch (export_decl->u.as_default->type)
    {
        case xjs_opaque_ast_exportdefaultdeclaration_type_identifier:
            {
                xjs_cfg_export_symbol_ref export_symbol;

                XJS_VNZ_ERROR_MEM(export_symbol = xjs_cfgbuilder_export_symbol_new( \
                            ec_string_new_assign_c_str("default"), \
                            ec_string_clone(export_decl->u.as_default->declaration.as_identifier->name)), ctx->err);

                xjs_cfgbuilder_append_export_symbol(ctx->cfgb, export_symbol);
            }
            break;
        case xjs_opaque_ast_exportdefaultdeclaration_type_expression:
            {
                xjs_cfg_export_symbol_ref export_symbol;
                xjs_cfg_var var_default;
                xjs_cfg_node_ref node_block = NULL, node_store = NULL, node_declvar = NULL;
                xjs_cfg_node_ref node_out2 = NULL;

                if (xjs_c0_expression(ctx, &var_default, node_in, &node_out2, \
                            export_decl->u.as_default->declaration.as_expression) != 0)
                { goto fail; }

                {
                    XJS_VNZ_ERROR_MEM(node_block = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
                    XJS_VNZ_ERROR_MEM(node_declvar = xjs_cfgbuilder_declvar_new( \
                                ctx->cfgb, ec_string_new_assign_c_str("default")), ctx->err);
                    xjs_cfgbuilder_block_push_back(node_block, node_declvar); node_declvar = NULL;
                    XJS_VNZ_ERROR_MEM(node_store = xjs_cfgbuilder_store_new( \
                                ctx->cfgb, ec_string_new_assign_c_str("default"), var_default), ctx->err);
                    xjs_cfgbuilder_block_push_back(node_block, node_store); node_store = NULL;
                    xjs_cfgbuilder_link(ctx->cfgb, node_out2, node_block);
                    *node_out = node_block;
                }

                XJS_VNZ_ERROR_MEM(export_symbol = xjs_cfgbuilder_export_symbol_new( \
                            ec_string_new_assign_c_str("default"), \
                            ec_string_new_assign_c_str("default")), ctx->err);
                xjs_cfgbuilder_append_export_symbol(ctx->cfgb, export_symbol);
            }
            break;
    }

fail:
    return ret;
}

static int xjs_c0_exportdeclaration(
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_exportdeclaration_ref export_decl)
{
    int ret = 0;

    switch (export_decl->type)
    {
        case xjs_ast_exportdeclaration_type_unknown:
        case xjs_ast_exportdeclaration_type_all:
            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
            break;
        case xjs_ast_exportdeclaration_type_default:
            ret = xjs_c0_exportdeclaration_default(ctx, \
                    node_in, node_out, \
                    export_decl);
            break;
        case xjs_ast_exportdeclaration_type_named:
            {
                ect_iterator(xjs_ast_exportspecifierlist) it_specifier;
                ect_for(xjs_ast_exportspecifierlist, export_decl->u.as_named->specifiers, it_specifier)
                {
                    xjs_ast_exportspecifier_ref specifier = ect_deref(xjs_ast_exportspecifier_ref, it_specifier);

                    switch (specifier->type)
                    {
                        case xjs_ast_exportspecifier_type_exportspecifier:
                            {
                                xjs_cfg_export_symbol_ref export_symbol;

                                XJS_VNZ_ERROR_MEM(export_symbol = xjs_cfgbuilder_export_symbol_new( \
                                            ec_string_clone(specifier->exported->name), \
                                            ec_string_clone(specifier->local->name)), ctx->err);

                                xjs_cfgbuilder_append_export_symbol(ctx->cfgb, export_symbol);
                            }
                            break;
                        case xjs_ast_exportspecifier_type_exportdefaultspecifier:
                            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                            break;
                        case xjs_ast_exportspecifier_type_exportnamespacespecifier:
                            xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                            break;
                    }

                }
            }
            break;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c0_importdeclaration(
        xjs_c0_ctx *ctx, \
        xjs_ast_importdeclaration_ref import_decl)
{
    int ret = 0;
    xjs_cfg_import_symbol_ref import_symbol = NULL;

    ect_iterator(xjs_ast_importspecifierlist) it_specifier;
    ect_for(xjs_ast_importspecifierlist, import_decl->specifiers, it_specifier)
    {
        xjs_ast_importspecifier_ref specifier = ect_deref(xjs_ast_importspecifier_ref, it_specifier);
        switch (specifier->type)
        {
            case xjs_ast_importspecifier_type_importspecifier:
                XJS_VNZ_ERROR_MEM(import_symbol = xjs_cfgbuilder_import_symbol_new( \
                            ec_string_clone(specifier->u.importspecifier.local->name), \
                            ec_string_clone(specifier->u.importspecifier.imported->name), \
                            ec_string_clone(import_decl->source->value.as_string)), ctx->err);
                xjs_cfgbuilder_append_import_symbol(ctx->cfgb, import_symbol);
                import_symbol = NULL;
                break;
            case xjs_ast_importspecifier_type_importdefaultspecifier:
                XJS_VNZ_ERROR_MEM(import_symbol = xjs_cfgbuilder_import_symbol_new( \
                            ec_string_clone(specifier->u.importdefaultspecifier.local->name), \
                            ec_string_new_assign_c_str("default"), \
                            ec_string_clone(import_decl->source->value.as_string)), ctx->err);
                xjs_cfgbuilder_append_import_symbol(ctx->cfgb, import_symbol);
                import_symbol = NULL;
                break;
            case xjs_ast_importspecifier_type_importnamespacespecifier:
                xjs_c0_ctx_error(ctx, XJS_ERRNO_NOTIMP);
                break;
        }
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(import_symbol);
    return ret;
}

static int
xjs_c0_moduleitemlist( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref *node_in, xjs_cfg_node_ref *node_out, \
        xjs_ast_moduleitemlist_ref moduleitemlist)
{
    int ret = 0;
    xjs_cfg_node_ref node_base;

    /* Initialize Base */
    XJS_VNZ_ERROR_MEM(node_base = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
    *node_in = node_base;

    ect_iterator(xjs_ast_moduleitemlist) it_moduleitem;
    ect_for(xjs_ast_moduleitemlist, moduleitemlist, it_moduleitem)
    {
        xjs_ast_moduleitem_ref moduleitem = ect_deref(xjs_ast_moduleitem_ref, it_moduleitem);
        switch (moduleitem->tag)
        {
            case xjs_ast_moduleitem_statementlistitem:
                {
                    xjs_cfg_node_ref node_in2, node_out2;
                    if (xjs_c0_statementlistitem(ctx, \
                                &node_in2, &node_out2, \
                                moduleitem->u.xjs_ast_moduleitem_statementlistitem) != 0)
                    { ret = -1; goto fail; }

                    xjs_cfgbuilder_link(ctx->cfgb, node_base, node_in2);
                    node_base = node_out2;
                }
                break;
            case xjs_ast_moduleitem_exportdeclaration:
                {
                    xjs_cfg_node_ref node_in2 = NULL, node_out2 = NULL;

                    if (xjs_c0_exportdeclaration(ctx, \
                                &node_in2, &node_out2, \
                                moduleitem->u.xjs_ast_moduleitem_exportdeclaration) != 0)
                    { ret = -1; goto fail; }

                    /* Instruction inserted */
                    if (node_in2 != NULL)
                    {
                        xjs_cfgbuilder_link(ctx->cfgb, node_base, node_in2);
                        node_base = node_out2;
                    }
                }
                break;
            case xjs_ast_moduleitem_importdeclaration:
                {
                    if (xjs_c0_importdeclaration(ctx, \
                                moduleitem->u.xjs_ast_moduleitem_importdeclaration) != 0)
                    { ret = -1; goto fail; }
                }
                break;
        }
    }

    *node_out = node_base;

fail:
    return ret;
}

static ec_bool
xjs_c0_statementlist_function_scope_is_terminated(xjs_cfg_node_ref block)
{
    ect_iterator(xjs_cfg_node_list) it;
    ect_for(xjs_cfg_node_list, block->u.as_block->items, it)
    {
        xjs_cfg_node_ref node = ect_deref(xjs_cfg_node_ref, it);
        if (node->type == XJS_CFG_NODE_TYPE_RETURN)
        {
            return ec_true;
        }
    }
    return ec_false;
}

static int
xjs_c0_statementlist_function_scope_terminate( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref block, \
        ec_bool toplevel)
{
    int ret = 0;

    if (toplevel == ec_true)
    {
        xjs_cfg_node_ref node_halt = NULL;
        XJS_VNZ_ERROR_MEM(node_halt = xjs_cfgbuilder_halt_new(ctx->cfgb), ctx->err);
        xjs_cfgbuilder_block_push_back(block, node_halt);
    }
    else
    {
        xjs_cfg_node_ref node_alloca = NULL;
        xjs_cfg_node_ref node_load_undefined = NULL;
        xjs_cfg_node_ref node_ret = NULL;
        xjs_cfg_var var;

        /* Skip if terminated by 'return' */
        if (xjs_c0_statementlist_function_scope_is_terminated(block) == ec_false)
        {
            /* Alloca */
            var = xjs_cfgbuilder_allocate_var(ctx->cfgb);
            XJS_VNZ_ERROR_MEM(node_alloca = xjs_cfgbuilder_alloca_new(ctx->cfgb, var), ctx->err);
            xjs_cfgbuilder_block_push_back(block, node_alloca); node_alloca = NULL;
            XJS_VNZ_ERROR_MEM(node_load_undefined = xjs_cfgbuilder_literal_undefined_new(ctx->cfgb, var), ctx->err);
            xjs_cfgbuilder_block_push_back(block, node_load_undefined); node_load_undefined = NULL;
            XJS_VNZ_ERROR_MEM(node_ret = xjs_cfgbuilder_return_new(ctx->cfgb, var), ctx->err);
            xjs_cfgbuilder_block_push_back(block, node_ret); node_ret = NULL;
        }
    }

fail:
    return ret;
}

static xjs_cfg_node_ref 
xjs_c0_statementlist_function_scope_statementlistitem( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlistitem_ref statements, \
        ec_bool toplevel)
{
    xjs_c0_scope_ref function_scope = NULL;
    xjs_cfg_node_ref block_vardecl = NULL;
    xjs_cfg_node_ref block_body = NULL, block_tail = NULL;
    xjs_cfg_node_ref block_final = NULL;

    /* Build a function scope */
    XJS_VNZ_ERROR_MEM( \
            function_scope = xjs_c0_ctx_append_scope(ctx), ctx->err);

    /* Collect local variables declared by 'var' */
    if ((function_scope->hoisted_vars = xjs_var_hoist_collect_localvars1( \
                ctx, statements)) == NULL)
    { goto fail; }

    if (xjs_varname_set_size(function_scope->hoisted_vars) > 0)
    {
        /* Declare 'var' variables (Produce 'block_vardecl') */
        if ((block_vardecl = xjs_var_declare_localvars(ctx, \
                        function_scope->hoisted_vars)) == NULL)
        { goto fail; }
    }

    if ((xjs_c0_statementlistitem(ctx, \
                    &block_body, &block_tail, \
                    statements)) != 0)
    { goto fail; }

    {
        if (block_vardecl == NULL)
        {
            if (block_body == NULL)
            {
                block_final = xjs_cfgbuilder_block_new(ctx->cfgb);
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_final, toplevel) != 0)
                { goto fail; }
            }
            else
            {
                block_final = block_body;
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_tail, toplevel) != 0)
                { goto fail; }
            }
        }
        else
        {
            if (block_body == NULL)
            {
                block_final = block_vardecl;
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_vardecl, toplevel) != 0)
                { goto fail; }
            }
            else
            {
                block_final = xjs_cfgbuilder_link(ctx->cfgb, block_vardecl, block_body);
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_tail, toplevel) != 0)
                { goto fail; }
            }
        }
    }

fail: 
    if (function_scope != NULL) xjs_c0_ctx_pop_scope(ctx);
    return block_final;
}

static xjs_cfg_node_ref 
xjs_c0_statementlist_function_scope_statementlist( \
        xjs_c0_ctx *ctx, \
        xjs_ast_statementlist_ref statements, \
        ec_bool toplevel)
{
    xjs_c0_scope_ref function_scope = NULL;
    xjs_cfg_node_ref block_vardecl = NULL;
    xjs_cfg_node_ref block_body = NULL, block_tail = NULL;
    xjs_cfg_node_ref block_final = NULL;

    /* Build a function scope */
    XJS_VNZ_ERROR_MEM( \
            function_scope = xjs_c0_ctx_append_scope(ctx), ctx->err);

    /* Collect local variables declared by 'var' */
    if ((function_scope->hoisted_vars = xjs_var_hoist_collect_localvars( \
                ctx, statements)) == NULL)
    { goto fail; }

    if (xjs_varname_set_size(function_scope->hoisted_vars) > 0)
    {
        /* Declare 'var' variables (Produce 'block_vardecl') */
        if ((block_vardecl = xjs_var_declare_localvars(ctx, \
                        function_scope->hoisted_vars)) == NULL)
        { goto fail; }
    }

    if (xjs_ast_statementlist_size(statements) > 0)
    {
        /* Compile statements (Produce 'block_body') */
        if ((xjs_c0_statementlist(ctx, \
                        &block_body, &block_tail, \
                        statements)) != 0)
        { goto fail; }
    }

    {
        if (block_vardecl == NULL)
        {
            if (block_body == NULL)
            {
                block_final = xjs_cfgbuilder_block_new(ctx->cfgb);
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_final, toplevel) != 0)
                { goto fail; }
            }
            else
            {
                block_final = block_body;
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_tail, toplevel) != 0)
                { goto fail; }
            }
        }
        else
        {
            if (block_body == NULL)
            {
                block_final = block_vardecl;
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_vardecl, toplevel) != 0)
                { goto fail; }
            }
            else
            {
                block_final = xjs_cfgbuilder_link(ctx->cfgb, block_vardecl, block_body);
                if (xjs_c0_statementlist_function_scope_terminate(ctx, block_tail, toplevel) != 0)
                { goto fail; }
            }
        }
    }

fail: 
    if (function_scope != NULL) xjs_c0_ctx_pop_scope(ctx);
    return block_final;}

static int
xjs_c0_statementlist_function_scope_import( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref block)
{
    int ret = 0;

    {
        xjs_asm_block_decl(block);

        /* localSymbol = imports[source][importedSymbol]; */
        ect_iterator(xjs_cfg_import_symbol_list) it_es;
        ect_for(xjs_cfg_import_symbol_list, ctx->cfgb->cfg->import_symbols, it_es)
        {
            xjs_cfg_import_symbol_ref is = ect_deref(xjs_cfg_import_symbol_ref, it_es);
            {
                xjs_asm_var(var_imports);
                xjs_asm_var(var_source);
                xjs_asm_var(var_t0);
                xjs_asm_var(var_imported);
                xjs_asm_var(var_t1);


                xjs_asm_allocate_var(var_imports);
                xjs_asm_load(var_imports, ec_string_new_assign_c_str("imports"));
                xjs_asm_allocate_var(var_source);
                xjs_asm_load_string(var_source, xjs_aux_mainname(is->source));
                xjs_asm_allocate_var(var_t0);
                xjs_asm_objget(var_t0, var_imports, var_source);

                xjs_asm_allocate_var(var_imported);
                xjs_asm_load_string(var_imported, ec_string_clone(is->imported));
                xjs_asm_allocate_var(var_t1);
                xjs_asm_objget(var_t1, var_t0, var_imported);

                xjs_asm_declvar(var_t1, ec_string_clone(is->local));
                xjs_asm_store(var_t1, ec_string_clone(is->local));
            }
        }
    }

fail:
    return ret;
}

static int
xjs_c0_statementlist_function_scope_export( \
        xjs_c0_ctx *ctx, \
        xjs_cfg_node_ref block)
{
    int ret = 0;

    {
        xjs_asm_block_decl(block);

        /* exports[exportedSymbol] = localSymbol; */
        ect_iterator(xjs_cfg_export_symbol_list) it_es;
        ect_for(xjs_cfg_export_symbol_list, ctx->cfgb->cfg->export_symbols, it_es)
        {
            xjs_cfg_export_symbol_ref es = ect_deref(xjs_cfg_export_symbol_ref, it_es);
            {
                xjs_asm_var(var_dst);
                xjs_asm_var(var_exports);
                xjs_asm_var(var_exported);
                xjs_asm_var(var_local);

                xjs_asm_allocate_var(var_dst);
                xjs_asm_allocate_var(var_exports);
                xjs_asm_load(var_exports, ec_string_new_assign_c_str("exports"));
                xjs_asm_allocate_var(var_exported);
                xjs_asm_load_string(var_exported, ec_string_clone(es->exported));
                xjs_asm_allocate_var(var_local);
                xjs_asm_load(var_local, ec_string_clone(es->local));
                xjs_asm_objset(var_dst, var_exports, var_exported, var_local);

                /* Useless */
                (void)var_dst;
            }
        }

        /* return null; */
        {
            xjs_asm_var(var);

            xjs_asm_allocate_var(var);
            xjs_asm_load_undefined(var);
            xjs_asm_ret(var);
        }
    }

fail:
    return ret;
}

static xjs_cfg_node_ref 
xjs_c0_module_function_scope( \
        xjs_c0_ctx *ctx, \
        xjs_ast_moduleitemlist_ref moduleitemlist)
{
    xjs_c0_scope_ref function_scope = NULL;
    xjs_cfg_node_ref block_vardecl = NULL;
    xjs_cfg_node_ref block_body = NULL, block_tail = NULL;
    xjs_cfg_node_ref block_final = NULL;

    /* Build a function scope */
    XJS_VNZ_ERROR_MEM( \
            function_scope = xjs_c0_ctx_append_scope(ctx), ctx->err);

    /* Collect local variables declared by 'var' */
    if ((function_scope->hoisted_vars = xjs_var_hoist_collect_localvars_from_module( \
                ctx, moduleitemlist)) == NULL)
    { goto fail; }

    if (xjs_varname_set_size(function_scope->hoisted_vars) > 0)
    {
        /* Declare 'var' variables (Produce 'block_vardecl') */
        if ((block_vardecl = xjs_var_declare_localvars(ctx, \
                        function_scope->hoisted_vars)) == NULL)
        { goto fail; }
    }

    if (xjs_ast_moduleitemlist_size(moduleitemlist) > 0)
    {
        /* Compile statements (Produce 'block_body') */
        if ((xjs_c0_moduleitemlist(ctx, \
                        &block_body, &block_tail, \
                        moduleitemlist)) != 0)
        { goto fail; }
    }

    {
        if (block_vardecl == NULL)
        {
            if (block_body == NULL)
            {
                block_final = xjs_cfgbuilder_block_new(ctx->cfgb);
                if (xjs_c0_statementlist_function_scope_export(ctx, block_final) != 0)
                { goto fail; }
            }
            else
            {
                block_final = block_body;
                if (xjs_c0_statementlist_function_scope_export(ctx, block_tail) != 0)
                { goto fail; }
            }
        }
        else
        {
            if (block_body == NULL)
            {
                block_final = block_vardecl;
                if (xjs_c0_statementlist_function_scope_export(ctx, block_vardecl) != 0)
                { goto fail; }
            }
            else
            {
                block_final = xjs_cfgbuilder_link(ctx->cfgb, block_vardecl, block_body);
                if (xjs_c0_statementlist_function_scope_export(ctx, block_tail) != 0)
                { goto fail; }
            }
        }
    }

    {
        /* import part */
        xjs_cfg_node_ref node_prefix;
        XJS_VNZ_ERROR_MEM(node_prefix = xjs_cfgbuilder_block_new(ctx->cfgb), ctx->err);
        if (xjs_c0_statementlist_function_scope_import(ctx, node_prefix) != 0)
        { goto fail; }
        block_final = xjs_cfgbuilder_link(ctx->cfgb, node_prefix, block_final);
    }

fail: 
    if (function_scope != NULL) xjs_c0_ctx_pop_scope(ctx);
    return block_final;
}

static xjs_cfg_parameter_list_ref xjs_c0_program_extract_toplevel_parameters( \
        xjs_c0_ctx *ctx)
{
    xjs_cfg_parameter_list_ref new_params = NULL;
    xjs_cfg_parameter_ref new_param = NULL;

    XJS_VNZ_ERROR_MEM(new_params = ect_list_new(xjs_cfg_parameter_list), ctx->err);

    /*
     * function (imports, exports) {
     *   Original top level code here
     * }
     */

    {
        /* imports */
        XJS_VNZ_ERROR_MEM(new_param = xjs_cfg_parameter_new(ec_string_new_assign_c_str("imports")), ctx->err);
        ect_list_push_back(xjs_cfg_parameter_list, new_params, new_param);
    }
    {
        /* exports */
        XJS_VNZ_ERROR_MEM(new_param = xjs_cfg_parameter_new(ec_string_new_assign_c_str("exports")), ctx->err);
        ect_list_push_back(xjs_cfg_parameter_list, new_params, new_param);
    }

fail:
    return new_params;
}

static int xjs_c0_program( \
        xjs_c0_ctx *ctx, \
        xjs_ast_program_ref program)
{
    int ret = 0;
    xjs_cfg_node_ref new_block = NULL;

    switch (program->tag)
    {
        case xjs_ast_program_module: 
            if ((new_block = xjs_c0_module_function_scope(ctx, \
                            program->u.xjs_ast_program_module.body)) == NULL)
            { ret = -1; goto fail; }
            /* Append the global scope statements as a function */
            {
                xjs_cfg_parameter_list_ref parameters = NULL;
                xjs_cfg_function_ref new_cfg_function;

                if ((parameters = xjs_c0_program_extract_toplevel_parameters(ctx)) == NULL)
                { goto fail; }

                XJS_VNZ_ERROR_MEM_OR(new_cfg_function = xjs_cfg_function_new(parameters, new_block), \
                        ctx->err, ec_delete(parameters));
                xjs_cfg_append_function(ctx->cfgb->cfg, new_cfg_function);

                /* Set top_level */
                xjs_cfgbuilder_set_top_level(ctx->cfgb, new_cfg_function);
            }
            break;

        case xjs_ast_program_script:
            if ((new_block = xjs_c0_statementlist_function_scope_statementlist(ctx, \
                            program->u.xjs_ast_program_script.body, \
                            ec_true)) == NULL)
            { ret = -1; goto fail; }
            /* Append the global scope statements as a function */
            {
                xjs_cfg_parameter_list_ref parameters = NULL;
                xjs_cfg_function_ref new_cfg_function;

                XJS_VNZ_ERROR_MEM(parameters = xjs_cfg_parameter_list_new(), ctx->err);
                XJS_VNZ_ERROR_MEM_OR(new_cfg_function = xjs_cfg_function_new(parameters, new_block), \
                        ctx->err, ec_delete(parameters));
                xjs_cfg_append_function(ctx->cfgb->cfg, new_cfg_function);
                /* Set top_level */
                xjs_cfgbuilder_set_top_level(ctx->cfgb, new_cfg_function);
            }
            break;
    }

fail:
    return ret;
}

/* Compiler */
xjs_cfg_ref xjs_c0_start_ex( \
        xjs_error_ref err, \
        xjs_ast_program_ref program, \
        const char *filename)
{
    xjs_c0_ctx ctx;
    xjs_cfgbuilder_ref new_cfgb = NULL;
    xjs_c0_scope_stack_ref new_scopes = NULL;
    xjs_c0_breakpoint_stack_ref new_bps = NULL;
    xjs_cfg_ref new_cfg = NULL;

    /* New IR Builder */
    XJS_VNZ_ERROR_MEM(new_cfgb = xjs_cfgbuilder_new(), err);
    /* Scope Stack */
    XJS_VNZ_ERROR_MEM(new_scopes = xjs_c0_scope_stack_new(), err);
    /* Break Point Stack */
    XJS_VNZ_ERROR_MEM(new_bps = xjs_c0_breakpoint_stack_new(), err);

    /* Initialize Context */
    xjs_c0_ctx_init(&ctx, err, new_cfgb, new_scopes, new_bps);
    xjs_c0_ctx_set_source_filename(&ctx, filename);

    if ((xjs_c0_program(&ctx, program)) != 0)
    { goto fail; }

    /* Generate IR */
    XJS_VNZ_ERROR_INTERNAL(new_cfg = xjs_cfgbuilder_cfg_generate(ctx.cfgb), err);

    goto done;
fail:
    if (new_cfg != NULL)
    { ec_delete(new_cfg); new_cfg = NULL; }
done:
    ec_delete(new_cfgb);
    ec_delete(new_scopes);
    ec_delete(new_bps);
    return new_cfg;
}

xjs_cfg_ref xjs_c0_start( \
        xjs_error_ref err, \
        xjs_ast_program_ref program)
{
    static const char *untitled = "untitled";
    return xjs_c0_start_ex( \
            err, \
            program, \
            untitled);
}

