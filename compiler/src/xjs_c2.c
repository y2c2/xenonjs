/* XenonJS : C2
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_set.h>
#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include <ec_libc.h>
#include "xjs_types.h"
#include "xjs_cfg.h"
#include "xjs_ir.h"
#include "xjs_irbuilder.h"
#include "xjs_helper.h"
#include "xjs_c2_ctx.h"
#include "xjs_c2_blklayout.h"
#include "xjs_c2.h"

#define IR_LOC_CLONE_START(_dst, _src) \
    do { \
        xjs_ir_text_item_set_loc_start(_dst, (_src)->loc.start.ln, (_src)->loc.start.col); \
    } while (0)
#define IR_LOC_CLONE_END(_dst, _src) \
    do { \
        xjs_ir_text_item_set_loc_end(_dst, (_src)->loc.end.ln, (_src)->loc.end.col); \
    } while (0)
#define IR_LOC_CLONE(_dst, _src) \
    do { \
        IR_LOC_CLONE_START(_dst, _src); \
        IR_LOC_CLONE_END(_dst, _src); \
    } while (0)
#define IR_RANGE_CLONE(_dst, _src) \
    do { \
        xjs_ir_text_item_set_range_start(_dst, (_src)->range.start); \
        xjs_ir_text_item_set_range_end(_dst, (_src)->range.end); \
    } while (0)
#define IR_LOC_RANGE_CLONE(_dst, _src) \
    do { \
        if (ctx->filename != NULL) _dst->loc.filename = ec_string_clone(ctx->filename); \
        IR_LOC_CLONE(_dst, _src); \
        IR_RANGE_CLONE(_dst, _src); \
    } while (0)


ect_map_declare(xjs_cfg_functionid_map, xjs_cfg_function_ref, xjs_ir_functionid);

static int xjs_cfg_functionid_map_value_ctor(xjs_ir_functionid *detta_value, xjs_ir_functionid *value)
{
    *detta_value = *value;
    return 0;
}

static int xjs_cfg_functionid_map_key_ctor(xjs_cfg_function_ref *detta_key, xjs_cfg_function_ref *key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_cfg_functionid_map_key_dtor(xjs_cfg_function_ref *key)
{
    (void)key;
}

static int xjs_cfg_functionid_map_key_cmp(xjs_cfg_function_ref *a, xjs_cfg_function_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_cfg_functionid_map, \
        xjs_cfg_function_ref, xjs_cfg_functionid_map_key_ctor, xjs_cfg_functionid_map_key_dtor, \
        xjs_ir_functionid, xjs_cfg_functionid_map_value_ctor, NULL, \
        xjs_cfg_functionid_map_key_cmp);
typedef xjs_cfg_functionid_map *xjs_cfg_functionid_map_ref;

static int xjs_c2_block_layout_record_node( \
        xjs_c2_block_layout_ref block_layout, \
        xjs_cfg_node_linear_block_map *reverse_linear_block_hit_map, \
        xjs_cfg_node_ref node, \
        ec_bool anchor)
{
    int ret = 0;
    xjs_c2_linear_block_ref new_linear_block;

    /* Create the Linear Block */
    if ((new_linear_block = xjs_c2_linear_block_new()) == NULL)
    { goto fail; }
    xjs_c2_linear_block_node_set(new_linear_block, node);
    if (anchor == ec_true)
    { xjs_c2_linear_block_anchor_set(new_linear_block); }

    /* Insert into map to allow reverse search and increase counter */
    xjs_cfg_node_linear_block_map_insert(reverse_linear_block_hit_map, node, new_linear_block);

    xjs_c2_linear_block_list_push_back(block_layout->blocks, new_linear_block);

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c2_blklayout_collect_anchors(xjs_c2_ctx *ctx, \
        xjs_c2_anchor_block_set **anchors_out, \
        xjs_cfg_function_ref func)
{
    int ret = 0;
    xjs_cfg_node_ref_set *visited_nodes = NULL;
    xjs_c2_anchor_block_set *anchors = NULL;
    xjs_cfg_node_stack *walk_stack = NULL;

    XJS_VNZ_ERROR_MEM(anchors = ect_set_new(xjs_c2_anchor_block_set), ctx->err);

    /* Record visited nodes */
    XJS_VNZ_ERROR_MEM(visited_nodes = ect_set_new(xjs_cfg_node_ref_set), ctx->err);

    /* Build the walk stack */
    XJS_VNZ_ERROR_MEM(walk_stack = ect_stack_new(xjs_cfg_node_stack), ctx->err);

    /* Push the entry node */
    xjs_cfg_node_stack_push(walk_stack, func->body);

    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_cfg_node_ref node_cur = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        if (ect_set_count(xjs_cfg_node_ref_set, visited_nodes, node_cur) != 0)
        { continue; }
        ect_set_insert(xjs_cfg_node_ref_set, visited_nodes, node_cur);

        if (node_cur->type == XJS_CFG_NODE_TYPE_BLOCK)
        {
            ect_iterator(xjs_cfg_node_list) it;
            ect_for(xjs_cfg_node_list, node_cur->u.as_block->items, it)
            {
                xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                xjs_cfg_node_stack_push(walk_stack, item);
            }
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_JUMP)
        {
            xjs_c2_anchor_block_set_insert(anchors, node_cur->u.as_jump->dest);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_jump->dest);
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_BRANCH)
        {
            xjs_c2_anchor_block_set_insert(anchors, node_cur->u.as_branch->true_branch);
            xjs_c2_anchor_block_set_insert(anchors, node_cur->u.as_branch->false_branch);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->true_branch);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->false_branch);
        }
        else
        {
            /* Nothing to do */
        }
    }

    *anchors_out = anchors;

    goto done;
fail:
    ec_delete(anchors);
    ret = -1;
done:
    ec_delete(walk_stack);
    ec_delete(visited_nodes);
    return ret;
}

static int xjs_c2_blklayout_merge_inplace_jumps( \
        xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_c2_block_layout_ref block_layout)
{
    int ret = 0;
    ect_iterator(xjs_c2_linear_block_list) it_cur, it_next, it_end;

    (void)ctx;
    (void)irbf;

    /* FIXME: When successfully eliminated a block, should stay at the place
     * and retry instead of move on */

    /* Initialize 'cur' and 'end' */
    xjs_c2_linear_block_list_iterator_init_end(&it_end, block_layout->blocks);
    xjs_c2_linear_block_list_iterator_init_begin(&it_cur, block_layout->blocks);

    /* Exit when no block */
    if (xjs_c2_linear_block_list_iterator_eq(&it_cur, &it_end)) { return 0; }

    /* Initialize 'next' */
    xjs_c2_linear_block_list_iterator_init_begin(&it_next, block_layout->blocks);
    xjs_c2_linear_block_list_iterator_next(&it_next);

    for (; !xjs_c2_linear_block_list_iterator_eq(&it_next, &it_end); )\
    {
        xjs_c2_linear_block_ref block_cur, block_next;
        block_cur = ect_deref(xjs_c2_linear_block_ref, it_cur);
        block_next = ect_deref(xjs_c2_linear_block_ref, it_next);

        {
            ect_reverse_iterator(xjs_cfg_node_list) it_last_inst;
            xjs_cfg_node_ref last_inst;
            xjs_cfg_node_list_reverse_iterator_init_begin(&it_last_inst, block_cur->node->u.as_block->items);
            last_inst = ect_deref(xjs_cfg_node_ref, it_last_inst);
            /* Jump to the next block */
            if ((last_inst->type == XJS_CFG_NODE_TYPE_JUMP) && \
                    (last_inst->u.as_jump->dest == block_next->node))
            {
                /* Cancel the jump */
                block_cur->cancel_jump = ec_true;

                /* Cancel the anchor label of next block */
                if (block_next->anchor.counter == 0)
                {
                    XJS_ERROR_INTERNAL(ctx->err);
                }
                else if (block_next->anchor.counter == 1)
                {
                    block_next->anchor.enabled = ec_false;
                    block_next->anchor.counter = 0;
                }
            }
            /* Another case to jump to the next block */
            else if ((last_inst->type == XJS_CFG_NODE_TYPE_BRANCH) && \
                    (last_inst->u.as_branch->false_branch == block_next->node))
            {
                /* Cancel the jump */
                block_cur->cancel_jump = ec_true;

                /* Cancel the anchor label of next block */
                if (block_next->anchor.counter == 0)
                {
                    XJS_ERROR_INTERNAL(ctx->err);
                }
                else if (block_next->anchor.counter == 1)
                {
                    block_next->anchor.enabled = ec_false;
                    block_next->anchor.counter = 0;
                }
            }
        }

        xjs_c2_linear_block_list_iterator_next(&it_cur);
        xjs_c2_linear_block_list_iterator_next(&it_next);
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c2_blklayout_fill(xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_c2_block_layout_ref block_layout, \
        xjs_cfg_function_ref func)
{
    int ret = 0;
    xjs_c2_anchor_block_set *anchors = NULL;
    xjs_cfg_node_stack *walk_stack = NULL;
    xjs_c2_visited_node_set *visited_nodes = NULL;
    xjs_cfg_node_linear_block_map *reverse_linear_block_hit_map = NULL;
    ec_bool anchor;

    /* Collect anchors */
    if (xjs_c2_blklayout_collect_anchors(ctx, &anchors, func) != 0)
    { goto fail; }

    /* Visited nodes */
    XJS_VNZ_ERROR_MEM(visited_nodes = ect_set_new(xjs_c2_visited_node_set), ctx->err);

    /* Map to locate to linear block and increase hit counter */
    XJS_VNZ_ERROR_MEM(reverse_linear_block_hit_map = xjs_cfg_node_linear_block_map_new(), ctx->err);

    /* Build the walk stack */
    XJS_VNZ_ERROR_MEM(walk_stack = ect_stack_new(xjs_cfg_node_stack), ctx->err);

#define MAP_VAR(cfg_var) \
    do { \
        xjs_ir_var ir_var; \
        if (xjs_irbuilder_function_mapped(irbf, cfg_var) == ec_false) { \
            ir_var = xjs_irbuilder_function_allocate_var(irbf); \
            xjs_irbuilder_function_map_var(irbf, cfg_var, ir_var); } \
    } while (0)

    /* Push the entry node */
    xjs_cfg_node_stack_push(walk_stack, func->body);
    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_cfg_node_ref node_cur = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        /* Prevent duplicate visit */
        if (xjs_c2_visited_node_set_count(visited_nodes, node_cur) != 0) { continue; }
        xjs_c2_visited_node_set_insert(visited_nodes, node_cur);

        switch (node_cur->type)
        {
            case XJS_CFG_NODE_TYPE_UNKNOWN:
            case XJS_CFG_NODE_TYPE_DROP:
                goto fail;

            case XJS_CFG_NODE_TYPE_ALLOCA:
                MAP_VAR(node_cur->u.as_alloca->var);
                break;

            case XJS_CFG_NODE_TYPE_LITERAL:
                MAP_VAR(node_cur->u.as_literal->var);
                break;

            case XJS_CFG_NODE_TYPE_LOAD:
                MAP_VAR(node_cur->u.as_load->var);
                break;

            case XJS_CFG_NODE_TYPE_STORE:
                MAP_VAR(node_cur->u.as_store->var);
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_SET:
                MAP_VAR(node_cur->u.as_object_set->dst);
                MAP_VAR(node_cur->u.as_object_set->obj);
                MAP_VAR(node_cur->u.as_object_set->key);
                MAP_VAR(node_cur->u.as_object_set->value);
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_GET:
                MAP_VAR(node_cur->u.as_object_get->dst);
                MAP_VAR(node_cur->u.as_object_get->obj);
                MAP_VAR(node_cur->u.as_object_get->key);
                break;

            case XJS_CFG_NODE_TYPE_ARRAY_PUSH:
                MAP_VAR(node_cur->u.as_array_push->arr);
                MAP_VAR(node_cur->u.as_array_push->elem);
                break;

            case XJS_CFG_NODE_TYPE_UNARY_OP:
                MAP_VAR(node_cur->u.as_unary_op->var_dst);
                MAP_VAR(node_cur->u.as_unary_op->var_src);
                break;

            case XJS_CFG_NODE_TYPE_BINARY_OP:
                MAP_VAR(node_cur->u.as_binary_op->var_dst);
                MAP_VAR(node_cur->u.as_binary_op->var_src1);
                MAP_VAR(node_cur->u.as_binary_op->var_src2);
                break;

            case XJS_CFG_NODE_TYPE_HALT:
            case XJS_CFG_NODE_TYPE_DECLVAR:
                break;

            case XJS_CFG_NODE_TYPE_BLOCK:
                anchor = (xjs_c2_anchor_block_set_count(anchors, node_cur) == 0) ? ec_false : ec_true;
                if (anchor == ec_true)
                {
                    /* Assign the block with an label for anchor */
                    xjs_ir_label ir_label = xjs_irbuilder_function_allocate_label(irbf);
                    xjs_irbuilder_function_map_label(irbf, node_cur, ir_label);
                }
                XJS_VEZ_ERROR_INTERNAL(
                        xjs_c2_block_layout_record_node( \
                            block_layout, reverse_linear_block_hit_map, \
                            node_cur, anchor), ctx->err);
                {
                    ect_iterator(xjs_cfg_node_list) it;
                    ect_for(xjs_cfg_node_list, node_cur->u.as_block->items, it)
                    {
                        xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                        xjs_cfg_node_stack_push(walk_stack, item);
                    }
                }
                break;

            case XJS_CFG_NODE_TYPE_JUMP:
                {
                    xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_jump->dest);
                }
                break;

            case XJS_CFG_NODE_TYPE_BRANCH:
                {
                    /* To give out a reasonable layout result, true branch is push
                     * later than false branch to make sure been pop earlier and
                     * processed earlier */
                    xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->false_branch);
                    xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->true_branch);
                }
                break;

            case XJS_CFG_NODE_TYPE_MERGE:
                break;

            case XJS_CFG_NODE_TYPE_MAKE_FUNCTION:
                break;

            case XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION:
                break;

            case XJS_CFG_NODE_TYPE_INSPECT:
                MAP_VAR(node_cur->u.as_inspect->var);
                break;

            case XJS_CFG_NODE_TYPE_RETURN:
                MAP_VAR(node_cur->u.as_return->var);
                break;

            case XJS_CFG_NODE_TYPE_CALL:
                MAP_VAR(node_cur->u.as_call->dst);
                MAP_VAR(node_cur->u.as_call->callee);
                {
                    ect_iterator(xjs_cfg_var_list) it;
                    ect_for(xjs_cfg_var_list, node_cur->u.as_call->arguments, it)
                    {
                        xjs_cfg_var argument = ect_deref(xjs_cfg_var, it);
                        MAP_VAR(argument);
                    }
                }
                if (node_cur->u.as_call->bound_this.enabled == xjs_true)
                {
                    MAP_VAR(node_cur->u.as_call->bound_this._this);
                }
                break;

            case XJS_CFG_NODE_TYPE_NEW:
                MAP_VAR(node_cur->u.as_new->dst);
                MAP_VAR(node_cur->u.as_new->callee);
                {
                    ect_iterator(xjs_cfg_var_list) it;
                    ect_for(xjs_cfg_var_list, node_cur->u.as_new->arguments, it)
                    {
                        xjs_cfg_var argument = ect_deref(xjs_cfg_var, it);
                        MAP_VAR(argument);
                    }
                }
                break;

            case XJS_CFG_NODE_TYPE_THIS:
                MAP_VAR(node_cur->u.as_this->dst);
                break;
        }
    }
#undef MAP_VAR

    xjs_c2_visited_node_set_clear(visited_nodes);

    /* Increase Counter of how many times that the layouted blocks
     * has been jumped from other place */
    xjs_cfg_node_stack_push(walk_stack, func->body);
    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_c2_linear_block_ref linear_block;
        xjs_cfg_node_ref node_cur = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        /* Prevent duplicate visit */
        if (xjs_c2_visited_node_set_count(visited_nodes, node_cur) != 0) { continue; }
        xjs_c2_visited_node_set_insert(visited_nodes, node_cur);

        if (node_cur->type == XJS_CFG_NODE_TYPE_BLOCK)
        {
            ect_iterator(xjs_cfg_node_list) it;
            ect_for(xjs_cfg_node_list, node_cur->u.as_block->items, it)
            {
                xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                xjs_cfg_node_stack_push(walk_stack, item);
            }
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_JUMP)
        {
            if (xjs_cfg_node_linear_block_map_count(reverse_linear_block_hit_map, \
                    node_cur->u.as_jump->dest) == 0)
            {
                XJS_ERROR_INTERNAL(ctx->err);
            }
            linear_block = xjs_cfg_node_linear_block_map_get( \
                    reverse_linear_block_hit_map, node_cur->u.as_jump->dest);
            xjs_c2_linear_block_anchor_inc_counter(linear_block);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_jump->dest);
        }
        else if (node_cur->type == XJS_CFG_NODE_TYPE_BRANCH)
        {
            linear_block = xjs_cfg_node_linear_block_map_get( \
                    reverse_linear_block_hit_map, node_cur->u.as_branch->true_branch);
            xjs_c2_linear_block_anchor_inc_counter(linear_block);
            linear_block = xjs_cfg_node_linear_block_map_get( \
                    reverse_linear_block_hit_map, node_cur->u.as_branch->false_branch);
            xjs_c2_linear_block_anchor_inc_counter(linear_block);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->true_branch);
            xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->false_branch);
        }
    }

    /* Merge in-place jumps */
    if (xjs_c2_blklayout_merge_inplace_jumps(ctx, irbf, block_layout) != 0)
    { goto fail; }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(walk_stack);
    ec_delete(anchors);
    ec_delete(visited_nodes);
    ec_delete(reverse_linear_block_hit_map);
    return ret;
}

static int xjs_c2_blklayout_relayout(xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_c2_block_layout_ref block_layout, \
        xjs_cfg_function_ref func)
{
    (void)ctx;
    (void)irbf;
    (void)block_layout;
    (void)func;
    return 0;
}

static int xjs_c2_blklayout_extract(xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_c2_block_layout_ref *block_layout_out, \
        xjs_cfg_function_ref func)
{
    int ret = 0;
    xjs_c2_block_layout_ref new_block_layout = NULL;

    /* Create an empty layout */
    XJS_VNZ_ERROR_MEM(new_block_layout = xjs_c2_block_layout_new(), ctx->err);

    /* Fill the layout by walking though all nodes */
    if ((ret = xjs_c2_blklayout_fill(ctx, irbf, new_block_layout, func)) != 0)
    { goto fail; }

    /* Compute all permutations of blocks to optimize control flow */
    if ((ret = xjs_c2_blklayout_relayout(ctx, irbf, new_block_layout, func)) != 0)
    { goto fail; }

    *block_layout_out = new_block_layout;
    goto done;
fail:
    ec_delete(new_block_layout);
    ret = -1;
done:
    return ret;
}

static int xjs_c2_gen_ir_single( \
        xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node, \
        ec_bool cancel_jump, \
        xjs_cfg_functionid_map_ref functionid_map)
{
    int ret = 0;
    xjs_ir_text_item_ref inst = NULL;

    switch (node->type)
    {
        case XJS_CFG_NODE_TYPE_UNKNOWN:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_CFG_NODE_TYPE_HALT:
            XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_halt_new(), ctx->err);
            IR_LOC_RANGE_CLONE(inst, node);
            xjs_irbuilder_function_push_back_text_item(irbf, inst);
            inst = NULL;
            break;

        case XJS_CFG_NODE_TYPE_ALLOCA:
            {
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_alloca->var);
                XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_alloca_new(ir_var), ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_JUMP:
            if (cancel_jump == ec_false)
            {
                xjs_ir_label ir_label = xjs_irbuilder_function_find_label(irbf, node->u.as_jump->dest);
                XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_br_new(ir_label), ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_BRANCH:
            {
                xjs_ir_var ir_var_cond = xjs_irbuilder_function_find_var(irbf, node->u.as_branch->cond);
                xjs_ir_label ir_label_true = xjs_irbuilder_function_find_label(irbf, node->u.as_branch->true_branch);
                XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_br_cond_new(ir_var_cond, ir_label_true), ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst); inst = NULL;
                if (cancel_jump == ec_false)
                {
                    xjs_ir_label ir_label_false = xjs_irbuilder_function_find_label(irbf, node->u.as_branch->false_branch);
                    XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_br_new(ir_label_false), ctx->err);
                    IR_LOC_RANGE_CLONE(inst, node);
                    xjs_irbuilder_function_push_back_text_item(irbf, inst); inst = NULL;
                }
            }
            break;

        case XJS_CFG_NODE_TYPE_MERGE:
            {
                xjs_ir_var ir_var_test = xjs_irbuilder_function_find_var(irbf, node->u.as_merge->test);
                xjs_ir_var ir_var_consequent = xjs_irbuilder_function_find_var(irbf, node->u.as_merge->consequent);
                xjs_ir_var ir_var_alternate = xjs_irbuilder_function_find_var(irbf, node->u.as_merge->alternate);
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_merge->dst);
                XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_merge_new( \
                            ir_var_test, ir_var_consequent, ir_var_alternate, ir_var_dst), \
                        ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst); inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_LITERAL:
            {
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_literal->var);
                switch (node->u.as_literal->type)
                {
                    case XJS_CFG_LITERAL_TYPE_UNKNOWN:
                        XJS_ERROR_INTERNAL(ctx->err);
                        break;

                    case XJS_CFG_LITERAL_TYPE_NULL:
                        inst = xjs_ir_text_item_load_null_new(ir_var);
                        break;

                    case XJS_CFG_LITERAL_TYPE_UNDEFINED:
                        inst = xjs_ir_text_item_load_undefined_new(ir_var);
                        break;

                    case XJS_CFG_LITERAL_TYPE_BOOL:
                        inst = xjs_ir_text_item_load_bool_new(ir_var, \
                                node->u.as_literal->u.as_bool == ec_false ? xjs_false : xjs_true);
                        break;

                    case XJS_CFG_LITERAL_TYPE_STRING:
                        {
                            xjs_ir_dataid dataid = xjs_ir_string_ir_dataid_map_get( \
                                    ctx->irb->string_map, node->u.as_literal->u.as_string);
                            inst = xjs_ir_text_item_load_string_new(ir_var, dataid);
                        }
                        break;

                    case XJS_CFG_LITERAL_TYPE_NUMBER:
                        inst = xjs_ir_text_item_load_number_new(ir_var, \
                                node->u.as_literal->u.as_number);
                        break;

                    case XJS_CFG_LITERAL_TYPE_OBJECT:
                        inst = xjs_ir_text_item_load_object_new(ir_var);
                        break;

                    case XJS_CFG_LITERAL_TYPE_ARRAY:
                        inst = xjs_ir_text_item_load_array_new(ir_var);
                        break;
                }
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_UNARY_OP:
            {
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_unary_op->var_dst);
                xjs_ir_var ir_var_src = xjs_irbuilder_function_find_var(irbf, node->u.as_unary_op->var_src);
                switch (node->u.as_unary_op->type)
                {
                    case XJS_CFG_UNARY_OP_TYPE_UNKNOWN:
                        XJS_ERROR_INTERNAL(ctx->err);
                        break;

                    case XJS_CFG_UNARY_OP_TYPE_NOT:
                        inst = xjs_ir_text_item_unary_not_new(ir_var_dst, ir_var_src);
                        break;

                    case XJS_CFG_UNARY_OP_TYPE_BNOT:
                        inst = xjs_ir_text_item_unary_bnot_new(ir_var_dst, ir_var_src);
                        break;

                    case XJS_CFG_UNARY_OP_TYPE_ADD:
                        inst = xjs_ir_text_item_unary_add_new(ir_var_dst, ir_var_src);
                        break;

                    case XJS_CFG_UNARY_OP_TYPE_SUB:
                        inst = xjs_ir_text_item_unary_sub_new(ir_var_dst, ir_var_src);
                        break;
                }
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_BINARY_OP:
            {
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_binary_op->var_dst);
                xjs_ir_var ir_var_src1 = xjs_irbuilder_function_find_var(irbf, node->u.as_binary_op->var_src1);
                xjs_ir_var ir_var_src2 = xjs_irbuilder_function_find_var(irbf, node->u.as_binary_op->var_src2);
                switch (node->u.as_binary_op->type)
                {
                    case XJS_CFG_BINARY_OP_TYPE_UNKNOWN:
                        XJS_ERROR_INTERNAL(ctx->err);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_ADD:
                        inst = xjs_ir_text_item_binary_add_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_SUB:
                        inst = xjs_ir_text_item_binary_sub_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_MUL:
                        inst = xjs_ir_text_item_binary_mul_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_DIV:
                        inst = xjs_ir_text_item_binary_div_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_MOD:
                        inst = xjs_ir_text_item_binary_mod_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_E2:
                        inst = xjs_ir_text_item_binary_e2_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_NE2:
                        inst = xjs_ir_text_item_binary_ne2_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_E3:
                        inst = xjs_ir_text_item_binary_e3_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_NE3:
                        inst = xjs_ir_text_item_binary_ne3_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_L:
                        inst = xjs_ir_text_item_binary_l_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_LE:
                        inst = xjs_ir_text_item_binary_le_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_G:
                        inst = xjs_ir_text_item_binary_g_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_GE:
                        inst = xjs_ir_text_item_binary_ge_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_AND:
                        inst = xjs_ir_text_item_binary_and_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;

                    case XJS_CFG_BINARY_OP_TYPE_OR:
                        inst = xjs_ir_text_item_binary_or_new(ir_var_dst, ir_var_src1, ir_var_src2);
                        break;
                }
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_DECLVAR:
            {
                xjs_ir_dataid dataid;
                dataid = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, node->u.as_declvar->name);
                inst = xjs_ir_text_item_declvar_new(dataid);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_LOAD:
            {
                xjs_ir_dataid dataid;
                dataid = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, node->u.as_load->name);
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_load->var);
                inst = xjs_ir_text_item_load_new(ir_var, dataid);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_STORE:
            {
                xjs_ir_dataid dataid;
                dataid = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, node->u.as_store->name);
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_store->var);
                inst = xjs_ir_text_item_store_new(dataid, ir_var);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_OBJECT_SET:
            {
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_object_set->dst);
                xjs_ir_var ir_var_object = xjs_irbuilder_function_find_var(irbf, node->u.as_object_set->obj);
                xjs_ir_var ir_var_key = xjs_irbuilder_function_find_var(irbf, node->u.as_object_set->key);
                xjs_ir_var ir_var_value = xjs_irbuilder_function_find_var(irbf, node->u.as_object_set->value);
                inst = xjs_ir_text_item_object_set_new(ir_var_dst, ir_var_object, ir_var_key, ir_var_value);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_OBJECT_GET:
            {
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_object_get->dst);
                xjs_ir_var ir_var_object = xjs_irbuilder_function_find_var(irbf, node->u.as_object_get->obj);
                xjs_ir_var ir_var_key = xjs_irbuilder_function_find_var(irbf, node->u.as_object_get->key);
                inst = xjs_ir_text_item_object_get_new(ir_var_dst, ir_var_object, ir_var_key);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_ARRAY_PUSH:
            {
                xjs_ir_var ir_var_arr = xjs_irbuilder_function_find_var(irbf, node->u.as_array_push->arr);
                xjs_ir_var ir_var_elem = xjs_irbuilder_function_find_var(irbf, node->u.as_array_push->elem);
                inst = xjs_ir_text_item_array_push_new(ir_var_arr, ir_var_elem);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_DROP:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_CFG_NODE_TYPE_BLOCK:
            XJS_ERROR_INTERNAL(ctx->err);
            break;

        case XJS_CFG_NODE_TYPE_MAKE_FUNCTION:
            {
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_make_function->var_dst);
                inst = xjs_ir_text_item_make_function_new(ir_var, \
                        ect_map_get(xjs_cfg_functionid_map, functionid_map, node->u.as_make_function->func));
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION:
           {
               xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_make_function->var_dst);
                inst = xjs_ir_text_item_make_arrow_function_new(ir_var, \
                        ect_map_get(xjs_cfg_functionid_map, functionid_map, node->u.as_make_function->func));
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_INSPECT:
            {
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_inspect->var);
                inst = xjs_ir_text_item_inspect_new(ir_var);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_RETURN:
            {
                xjs_ir_var ir_var = xjs_irbuilder_function_find_var(irbf, node->u.as_return->var);
                inst = xjs_ir_text_item_ret_new(ir_var);
                XJS_VNZ_ERROR_MEM(inst, ctx->err);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_CALL:
            {
                xjs_ir_var_list_ref ir_var_arguments = NULL;

                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_call->dst);
                xjs_ir_var ir_var_callee = xjs_irbuilder_function_find_var(irbf, node->u.as_call->callee);

                ir_var_arguments = ect_list_new(xjs_ir_var_list);
                XJS_VNZ_ERROR_MEM(ir_var_arguments, ctx->err);
                {
                    ect_iterator(xjs_cfg_var_list) it_argument;
                    ect_for(xjs_cfg_var_list, node->u.as_call->arguments, it_argument)
                    {
                        xjs_cfg_var arg = ect_deref(xjs_cfg_var, it_argument);
                        xjs_ir_var ir_arg = xjs_irbuilder_function_find_var(irbf, arg);
                        ect_list_push_back(xjs_ir_var_list, \
                                ir_var_arguments, ir_arg);
                    }
                }

                if (node->u.as_call->bound_this.enabled == xjs_false)
                {
                    inst = xjs_ir_text_item_call_new(ir_var_dst, ir_var_callee, ir_var_arguments);
                }
                else
                {
                    xjs_ir_var ir_var_this = xjs_irbuilder_function_find_var(irbf, node->u.as_call->bound_this._this);
                    inst = xjs_ir_text_item_call_bound_this_new(ir_var_dst, ir_var_callee, ir_var_arguments, ir_var_this);
                }
                XJS_VNZ_ERROR_MEM_OR(inst, ctx->err, ec_delete(ir_var_arguments));
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_NEW:
            {
                xjs_ir_var_list_ref ir_var_arguments = NULL;

                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_new->dst);
                xjs_ir_var ir_var_callee = xjs_irbuilder_function_find_var(irbf, node->u.as_new->callee);

                ir_var_arguments = ect_list_new(xjs_ir_var_list);
                XJS_VNZ_ERROR_MEM(ir_var_arguments, ctx->err);
                {
                    ect_iterator(xjs_cfg_var_list) it_argument;
                    ect_for(xjs_cfg_var_list, node->u.as_new->arguments, it_argument)
                    {
                        xjs_cfg_var arg = ect_deref(xjs_cfg_var, it_argument);
                        xjs_ir_var ir_arg = xjs_irbuilder_function_find_var(irbf, arg);
                        ect_list_push_back(xjs_ir_var_list, \
                                ir_var_arguments, ir_arg);
                    }
                }

                inst = xjs_ir_text_item_new_new(ir_var_dst, ir_var_callee, ir_var_arguments);
                XJS_VNZ_ERROR_MEM_OR(inst, ctx->err, ec_delete(ir_var_arguments));
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;

        case XJS_CFG_NODE_TYPE_THIS:
            {
                xjs_ir_var ir_var_dst = xjs_irbuilder_function_find_var(irbf, node->u.as_new->dst);
                inst = xjs_ir_text_item_this_new(ir_var_dst);
                IR_LOC_RANGE_CLONE(inst, node);
                xjs_irbuilder_function_push_back_text_item(irbf, inst);
                inst = NULL;
            }
            break;
    }

    goto done;
fail:
    ret = -1;
    ec_delete(inst);
done:
    return ret;
}

static int xjs_c2_gen_ir_block( \
        xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_node_ref node_block, \
        ec_bool anchor, \
        ec_bool cancel_jump, \
        xjs_cfg_functionid_map_ref functionid_map)
{
    int ret = 0;
    ect_iterator(xjs_cfg_node_list) it;
    xjs_cfg_block_ref block = node_block->u.as_block;

    if (anchor == ec_true)
    {
        xjs_ir_text_item_ref inst = NULL;
        xjs_ir_label lbl = xjs_irbuilder_function_find_label(irbf, node_block);
        XJS_VNZ_ERROR_MEM(inst = xjs_ir_text_item_label_new(lbl), ctx->err);
        IR_LOC_RANGE_CLONE(inst, node_block);
        xjs_irbuilder_function_push_back_text_item(irbf, inst);
    }

    ect_for(xjs_cfg_node_list, block->items, it)
    {
        xjs_cfg_node_ref node = ect_deref(xjs_cfg_node_ref, it);

        if (xjs_c2_gen_ir_single(ctx, irbf, node, cancel_jump, functionid_map) != 0)
        { goto fail; }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c2_rename_remap( \
        xjs_irbuilder_function_ref irbf, \
        xjs_ir_var_map *var_map, \
        xjs_ir_label_map *label_map)
{
    ect_iterator(xjs_ir_text_item_list) it;
    xjs_ir_var new_var_pool = 0;
    xjs_ir_label new_label_pool = 0;

#define REMAP_LABEL(lbl) \
    do { \
        if (xjs_ir_label_map_count(label_map, lbl) == 0) \
        { xjs_ir_label_map_insert(label_map, lbl, new_label_pool++); } \
    } while (0)

#define REMAP_VAR(var) \
    do { \
        if (xjs_ir_var_map_count(var_map, var) == 0) \
        { xjs_ir_var_map_insert(var_map, var, new_var_pool++); } \
    } while (0)

    /* First pass, collect all mapping of var and label */
    ect_for(xjs_ir_text_item_list, irbf->f->text_items, it)
    {
        xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it);
        switch (ti->type)
        {
            case xjs_ir_text_item_type_nop:
            case xjs_ir_text_item_type_halt:
            case xjs_ir_text_item_type_push_scope:
            case xjs_ir_text_item_type_pop_scope:
                break;

            case xjs_ir_text_item_type_dynlib:
                REMAP_LABEL(ti->u.as_dynlib.exports);
                REMAP_LABEL(ti->u.as_dynlib.module_name);
                break;

            case xjs_ir_text_item_type_label:
                REMAP_LABEL(ti->u.as_label.lbl);
                break;
            case xjs_ir_text_item_type_br:
                REMAP_LABEL(ti->u.as_br.dest);
                break;
            case xjs_ir_text_item_type_br_cond:
                REMAP_VAR(ti->u.as_br_cond.cond);
                REMAP_LABEL(ti->u.as_br_cond.dest);
                break;
            case xjs_ir_text_item_type_merge:
                REMAP_VAR(ti->u.as_merge.test);
                REMAP_VAR(ti->u.as_merge.consequent);
                REMAP_VAR(ti->u.as_merge.alternate);
                REMAP_VAR(ti->u.as_merge.dst);
                break;
            case xjs_ir_text_item_type_alloca:
                REMAP_VAR(ti->u.as_alloca.var);
                break;
            case xjs_ir_text_item_type_load_undefined:
                REMAP_VAR(ti->u.as_load_undefined.var);
                break;
            case xjs_ir_text_item_type_load_null:
                REMAP_VAR(ti->u.as_load_null.var);
                break;
            case xjs_ir_text_item_type_load_bool:
                REMAP_VAR(ti->u.as_load_bool.var);
                break;
            case xjs_ir_text_item_type_load_string:
                REMAP_VAR(ti->u.as_load_string.var);
                break;
            case xjs_ir_text_item_type_load_number:
                REMAP_VAR(ti->u.as_load_number.var);
                break;
            case xjs_ir_text_item_type_load_object:
                REMAP_VAR(ti->u.as_load_object.var);
                break;
            case xjs_ir_text_item_type_load_array:
                REMAP_VAR(ti->u.as_load_array.var);
                break;
            case xjs_ir_text_item_type_declvar:
                break;
            case xjs_ir_text_item_type_load:
                REMAP_VAR(ti->u.as_load.var);
                break;
            case xjs_ir_text_item_type_store:
                REMAP_VAR(ti->u.as_store.var);
                break;
            case xjs_ir_text_item_type_object_set:
                REMAP_VAR(ti->u.as_object_set.dst);
                REMAP_VAR(ti->u.as_object_set.obj);
                REMAP_VAR(ti->u.as_object_set.member);
                REMAP_VAR(ti->u.as_object_set.src);
                break;
            case xjs_ir_text_item_type_object_get:
                REMAP_VAR(ti->u.as_object_get.dst);
                REMAP_VAR(ti->u.as_object_get.obj);
                REMAP_VAR(ti->u.as_object_get.member);
                break;
            case xjs_ir_text_item_type_array_push:
                REMAP_VAR(ti->u.as_array_push.arr);
                REMAP_VAR(ti->u.as_array_push.elem);
                break;
            case xjs_ir_text_item_type_make_function:
                REMAP_VAR(ti->u.as_make_function.var);
                break;
            case xjs_ir_text_item_type_make_arrow_function:
                REMAP_VAR(ti->u.as_make_arrow_function.var);
                break;
            case xjs_ir_text_item_type_inspect:
                REMAP_VAR(ti->u.as_inspect.var);
                break;
            case xjs_ir_text_item_type_ret:
                REMAP_VAR(ti->u.as_ret.var);
                break;
            case xjs_ir_text_item_type_call:
                REMAP_VAR(ti->u.as_call.dst);
                REMAP_VAR(ti->u.as_call.callee);
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, ti->u.as_call.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        REMAP_VAR(var_arg);
                    }
                }
                break;
            case xjs_ir_text_item_type_new:
                REMAP_VAR(ti->u.as_new.dst);
                REMAP_VAR(ti->u.as_new.callee);
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, ti->u.as_new.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        REMAP_VAR(var_arg);
                    }
                }
                break;
            case xjs_ir_text_item_type_this:
                REMAP_VAR(ti->u.as_this.dst);
                break;
            case xjs_ir_text_item_type_binary_add:
            case xjs_ir_text_item_type_binary_sub:
            case xjs_ir_text_item_type_binary_mul:
            case xjs_ir_text_item_type_binary_div:
            case xjs_ir_text_item_type_binary_mod:
            case xjs_ir_text_item_type_binary_e2:
            case xjs_ir_text_item_type_binary_ne2:
            case xjs_ir_text_item_type_binary_e3:
            case xjs_ir_text_item_type_binary_ne3:
            case xjs_ir_text_item_type_binary_l:
            case xjs_ir_text_item_type_binary_le:
            case xjs_ir_text_item_type_binary_g:
            case xjs_ir_text_item_type_binary_ge:
            case xjs_ir_text_item_type_binary_and:
            case xjs_ir_text_item_type_binary_or:
                REMAP_VAR(ti->u.as_binary_op.lhs);
                REMAP_VAR(ti->u.as_binary_op.rhs);
                REMAP_VAR(ti->u.as_binary_op.dst);
                break;
            case xjs_ir_text_item_type_unary_not:
            case xjs_ir_text_item_type_unary_bnot:
            case xjs_ir_text_item_type_unary_add:
            case xjs_ir_text_item_type_unary_sub:
                REMAP_VAR(ti->u.as_unary_op.src);
                REMAP_VAR(ti->u.as_unary_op.dst);
                break;
        }
    }

    return 0;
}

static int xjs_c2_rename_apply( \
        xjs_irbuilder_function_ref irbf, \
        xjs_ir_var_map *var_map, \
        xjs_ir_label_map *label_map)
{
    ect_iterator(xjs_ir_text_item_list) it;

#define APPLY_LABEL(lbl) \
    do { lbl = xjs_ir_label_map_get(label_map, lbl); } while (0)

#define APPLY_VAR(var) \
    do { var = xjs_ir_var_map_get(var_map, var); } while (0)

    ect_for(xjs_ir_text_item_list, irbf->f->text_items, it)
    {
        xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it);
        switch (ti->type)
        {
            case xjs_ir_text_item_type_nop:
            case xjs_ir_text_item_type_halt:
            case xjs_ir_text_item_type_push_scope:
            case xjs_ir_text_item_type_pop_scope:
                break;

            case xjs_ir_text_item_type_dynlib:
                APPLY_VAR(ti->u.as_dynlib.exports);
                APPLY_VAR(ti->u.as_dynlib.module_name);
                break;

            case xjs_ir_text_item_type_label:
                APPLY_LABEL(ti->u.as_label.lbl);
                break;
            case xjs_ir_text_item_type_br:
                APPLY_LABEL(ti->u.as_br.dest);
                break;
            case xjs_ir_text_item_type_br_cond:
                APPLY_VAR(ti->u.as_br_cond.cond);
                APPLY_LABEL(ti->u.as_br_cond.dest);
                break;
            case xjs_ir_text_item_type_merge:
                APPLY_VAR(ti->u.as_merge.test);
                APPLY_VAR(ti->u.as_merge.consequent);
                APPLY_VAR(ti->u.as_merge.alternate);
                APPLY_VAR(ti->u.as_merge.dst);
                break;
            case xjs_ir_text_item_type_alloca:
                APPLY_VAR(ti->u.as_alloca.var);
                break;
            case xjs_ir_text_item_type_load_undefined:
                APPLY_VAR(ti->u.as_load_undefined.var);
                break;
            case xjs_ir_text_item_type_load_null:
                APPLY_VAR(ti->u.as_load_null.var);
                break;
            case xjs_ir_text_item_type_load_bool:
                APPLY_VAR(ti->u.as_load_bool.var);
                break;
            case xjs_ir_text_item_type_load_string:
                APPLY_VAR(ti->u.as_load_string.var);
                break;
            case xjs_ir_text_item_type_load_number:
                APPLY_VAR(ti->u.as_load_number.var);
                break;
            case xjs_ir_text_item_type_load_object:
                APPLY_VAR(ti->u.as_load_object.var);
                break;
            case xjs_ir_text_item_type_load_array:
                APPLY_VAR(ti->u.as_load_array.var);
                break;
            case xjs_ir_text_item_type_declvar:
                break;
            case xjs_ir_text_item_type_load:
                APPLY_VAR(ti->u.as_load.var);
                break;
            case xjs_ir_text_item_type_store:
                APPLY_VAR(ti->u.as_store.var);
                break;
            case xjs_ir_text_item_type_object_set:
                APPLY_VAR(ti->u.as_object_set.dst);
                APPLY_VAR(ti->u.as_object_set.obj);
                APPLY_VAR(ti->u.as_object_set.member);
                APPLY_VAR(ti->u.as_object_set.src);
                break;
            case xjs_ir_text_item_type_object_get:
                APPLY_VAR(ti->u.as_object_get.dst);
                APPLY_VAR(ti->u.as_object_get.obj);
                APPLY_VAR(ti->u.as_object_get.member);
                break;
            case xjs_ir_text_item_type_array_push:
                APPLY_VAR(ti->u.as_array_push.arr);
                APPLY_VAR(ti->u.as_array_push.elem);
                break;
            case xjs_ir_text_item_type_make_function:
                APPLY_VAR(ti->u.as_make_function.var);
                break;
            case xjs_ir_text_item_type_make_arrow_function:
                APPLY_VAR(ti->u.as_make_arrow_function.var);
                break;
            case xjs_ir_text_item_type_inspect:
                APPLY_VAR(ti->u.as_inspect.var);
                break;
            case xjs_ir_text_item_type_ret:
                APPLY_VAR(ti->u.as_ret.var);
                break;
            case xjs_ir_text_item_type_call:
                APPLY_VAR(ti->u.as_call.dst);
                APPLY_VAR(ti->u.as_call.callee);
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, ti->u.as_call.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        ect_deref(xjs_ir_var, it_arg) = xjs_ir_var_map_get(var_map, var_arg);
                    }
                }
                if (ti->u.as_call.bound_this.enabled == xjs_true)
                {
                    APPLY_VAR(ti->u.as_call.bound_this._this);
                }
                break;
            case xjs_ir_text_item_type_new:
                APPLY_VAR(ti->u.as_new.dst);
                APPLY_VAR(ti->u.as_new.callee);
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, ti->u.as_new.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        ect_deref(xjs_ir_var, it_arg) = xjs_ir_var_map_get(var_map, var_arg);
                    }
                }
                break;
            case xjs_ir_text_item_type_this:
                APPLY_VAR(ti->u.as_this.dst);
                break;
            case xjs_ir_text_item_type_binary_add:
            case xjs_ir_text_item_type_binary_sub:
            case xjs_ir_text_item_type_binary_mul:
            case xjs_ir_text_item_type_binary_div:
            case xjs_ir_text_item_type_binary_mod:
            case xjs_ir_text_item_type_binary_e2:
            case xjs_ir_text_item_type_binary_ne2:
            case xjs_ir_text_item_type_binary_e3:
            case xjs_ir_text_item_type_binary_ne3:
            case xjs_ir_text_item_type_binary_l:
            case xjs_ir_text_item_type_binary_le:
            case xjs_ir_text_item_type_binary_g:
            case xjs_ir_text_item_type_binary_ge:
            case xjs_ir_text_item_type_binary_and:
            case xjs_ir_text_item_type_binary_or:
                APPLY_VAR(ti->u.as_binary_op.lhs);
                APPLY_VAR(ti->u.as_binary_op.rhs);
                APPLY_VAR(ti->u.as_binary_op.dst);
                break;
            case xjs_ir_text_item_type_unary_not:
            case xjs_ir_text_item_type_unary_bnot:
            case xjs_ir_text_item_type_unary_add:
            case xjs_ir_text_item_type_unary_sub:
                APPLY_VAR(ti->u.as_unary_op.src);
                APPLY_VAR(ti->u.as_unary_op.dst);
                break;
        }
    }
    return 0;
}

static int xjs_c2_rename( \
        xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf)
{
    int ret = 0;
    xjs_ir_var_map *var_map = NULL;
    xjs_ir_label_map *label_map = NULL;

    XJS_VNZ_ERROR_MEM(var_map = xjs_ir_var_map_new(), ctx->err);
    XJS_VNZ_ERROR_MEM(label_map = xjs_ir_label_map_new(), ctx->err);

    /* Remap */
    xjs_c2_rename_remap(irbf, var_map, label_map);

    /* Apply with remapped vars and labels */
    xjs_c2_rename_apply(irbf, var_map, label_map);

    goto done;
fail:
    ret = -1;
done:
    ec_delete(var_map);
    ec_delete(label_map);
    return ret;
}

static int xjs_c2_gen_ir( \
        xjs_c2_ctx *ctx, \
        xjs_irbuilder_function_ref irbf, \
        xjs_cfg_function_ref func, \
        xjs_c2_block_layout_ref block_layout, \
        xjs_cfg_functionid_map_ref functionid_map)
{
    int ret = 0;
    ect_iterator(xjs_c2_linear_block_list) it;

    (void)func;

    ect_for(xjs_c2_linear_block_list, block_layout->blocks, it)
    {
        xjs_c2_linear_block_ref block = ect_deref(xjs_c2_linear_block_ref, it);
        xjs_cfg_node_ref node = block->node;

        if (node->type == XJS_CFG_NODE_TYPE_BLOCK)
        {
            if (xjs_c2_gen_ir_block(ctx, irbf, node, \
                        block->anchor.enabled, \
                        block->cancel_jump, \
                        functionid_map) != 0)
            { goto fail; }
        }
        else
        {
            XJS_ERROR_NOTIMP(ctx->err);
        }
    }

    /* Rename vars and labels */
    if (xjs_c2_rename(ctx, irbf) != 0) { goto fail; }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int
xjs_c2_write_text_section_func( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_function_ref func, \
        xjs_cfg_functionid_map_ref functionid_map, \
        xjs_bool top_level)
{
    int ret = 0;
    xjs_irbuilder_function_ref irbf = NULL;
    xjs_c2_block_layout_ref block_layout = NULL;

    /* Create a new IR function builder for top level code */
    XJS_VNZ_ERROR_MEM(irbf = xjs_irbuilder_function_new(), ctx->err);

    /* Parameters */
    {
        ect_iterator(xjs_cfg_parameter_list) it_param;
        ect_for(xjs_cfg_parameter_list, func->parameters, it_param)
        {
            xjs_cfg_parameter_ref param = ect_deref(xjs_cfg_parameter_ref, it_param);
            xjs_ir_dataid parameter_dataid = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, param->name);
            XJS_VEZ_ERROR_INTERNAL(xjs_irbuilder_function_push_back_parameter_with_loc_range( \
                        irbf, parameter_dataid, \
                        param->loc.start.ln, param->loc.start.col, \
                        param->loc.end.ln, param->loc.end.col, \
                        param->range.start, param->range.end), ctx->err);
        }
    }

    /* Layout blocks */
    if (xjs_c2_blklayout_extract( \
                ctx, irbf, &block_layout, func) != 0)
    { goto fail; }

    /* Generate IR */
    if (xjs_c2_gen_ir(ctx, irbf, func, block_layout, functionid_map) != 0)
    { goto fail; }

    /* Walk though the top level which is also the
     * entrance of the entire code */
    /* if (xjs_c2_node(ctx, irbf, &v, cfg->top_level) != 0) { ret = -1; goto fail; } */

    /* TODO: How do I use 'v' */

    {
        xjs_ir_function_ref f = xjs_irbuilder_function_generate(irbf);
        if (top_level == xjs_true) { ctx->irb->ir->toplevel = f; }
        xjs_irbuilder_append_function(ctx->irb, f);
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(block_layout);
    ec_delete(irbf);
    return ret;
}

static int
xjs_c2_write_text_section( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_ref cfg, \
        xjs_cfg_functionid_map_ref functionid_map)
{
    int ret = 0;

    ect_iterator(xjs_cfg_function_list) it_func;
    ect_for(xjs_cfg_function_list, cfg->functions, it_func)
    {
        xjs_cfg_function_ref func = ect_deref(xjs_cfg_function_ref, it_func);

        if (xjs_c2_write_text_section_func(ctx, func, functionid_map, \
                    (func == cfg->top_level) ? xjs_true : xjs_false) != 0)
        { goto fail; }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static int xjs_c2_collect_data_section_func_parameters( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_function_ref func)
{
    ect_iterator(xjs_cfg_parameter_list) it_param;
    ect_for(xjs_cfg_parameter_list, func->parameters, it_param)
    {
        xjs_cfg_parameter_ref param = ect_deref(xjs_cfg_parameter_ref, it_param);
        if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                    param->name) == 0)
        {
            xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                    ec_string_clone(param->name), ctx->irb->dataid_pool++);
        }
    }

    return 0;
}

static int xjs_c2_collect_data_section_func_body( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_function_ref func)
{
    int ret = 0;
    xjs_cfg_node_ref_set *visited_nodes = NULL;
    xjs_cfg_node_stack *walk_stack = NULL;

    /* Record visited nodes */
    XJS_VNZ_ERROR_MEM(visited_nodes = ect_set_new(xjs_cfg_node_ref_set), ctx->err);

    /* Build the walk stack */
    XJS_VNZ_ERROR_MEM(walk_stack = ect_stack_new(xjs_cfg_node_stack), ctx->err);

    /* Push the entry node */
    xjs_cfg_node_stack_push(walk_stack, func->body);

    /* 1st pass: collect symbols and strings */
    while (!xjs_cfg_node_stack_empty(walk_stack))
    {
        xjs_cfg_node_ref node_cur = xjs_cfg_node_stack_top(walk_stack);
        xjs_cfg_node_stack_pop(walk_stack);

        if (ect_set_count(xjs_cfg_node_ref_set, visited_nodes, node_cur) != 0) { continue; }
        ect_set_insert(xjs_cfg_node_ref_set, visited_nodes, node_cur);

        switch (node_cur->type)
        {
            case XJS_CFG_NODE_TYPE_BLOCK:
                {
                    ect_iterator(xjs_cfg_node_list) it;
                    ect_for(xjs_cfg_node_list, node_cur->u.as_block->items, it)
                    {
                        xjs_cfg_node_ref item = ect_deref(xjs_cfg_node_ref, it);
                        xjs_cfg_node_stack_push(walk_stack, item);
                    }
                }
                break;

            case XJS_CFG_NODE_TYPE_JUMP:
                xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_jump->dest);
                break;

            case XJS_CFG_NODE_TYPE_BRANCH:
                xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->true_branch);
                xjs_cfg_node_stack_push(walk_stack, node_cur->u.as_branch->false_branch);
                break;

            case XJS_CFG_NODE_TYPE_MERGE:
                break;

            case XJS_CFG_NODE_TYPE_UNKNOWN:
            case XJS_CFG_NODE_TYPE_DROP:
                XJS_ERROR_INTERNAL(ctx->err);
                goto fail;

            case XJS_CFG_NODE_TYPE_HALT:
            case XJS_CFG_NODE_TYPE_ALLOCA:
            case XJS_CFG_NODE_TYPE_UNARY_OP:
            case XJS_CFG_NODE_TYPE_BINARY_OP:
                /* Nothing to record */
                break;

            case XJS_CFG_NODE_TYPE_LOAD:
                if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                            node_cur->u.as_load->name) == 0)
                {
                    xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                            ec_string_clone(node_cur->u.as_load->name), ctx->irb->dataid_pool++);
                }
                break;

            case XJS_CFG_NODE_TYPE_STORE:
                if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                            node_cur->u.as_store->name) == 0)
                {
                    xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                            ec_string_clone(node_cur->u.as_store->name), ctx->irb->dataid_pool++);
                }
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_SET:
                break;

            case XJS_CFG_NODE_TYPE_OBJECT_GET:
                break;

            case XJS_CFG_NODE_TYPE_ARRAY_PUSH:
                break;

            case XJS_CFG_NODE_TYPE_DECLVAR:
                if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                            node_cur->u.as_declvar->name) == 0)
                {
                    xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                            ec_string_clone(node_cur->u.as_declvar->name), ctx->irb->dataid_pool++);
                }
                break;

            case XJS_CFG_NODE_TYPE_LITERAL:
                switch (node_cur->u.as_literal->type)
                {
                    case XJS_CFG_LITERAL_TYPE_UNKNOWN:
                        XJS_ERROR_INTERNAL(ctx->err);
                        goto fail;

                    case XJS_CFG_LITERAL_TYPE_OBJECT:
                    case XJS_CFG_LITERAL_TYPE_ARRAY:
                    case XJS_CFG_LITERAL_TYPE_NULL:
                    case XJS_CFG_LITERAL_TYPE_BOOL:
                    case XJS_CFG_LITERAL_TYPE_NUMBER:
                    case XJS_CFG_LITERAL_TYPE_UNDEFINED:
                        /* Nothing to record */
                        break;

                    case XJS_CFG_LITERAL_TYPE_STRING:
                        /* TODO: Transcode to raw stirng */
                        if (xjs_ir_string_ir_dataid_map_count(ctx->irb->string_map, \
                                    node_cur->u.as_literal->u.as_string) == 0)
                        {
                            xjs_ir_string_ir_dataid_map_insert(ctx->irb->string_map, \
                                    ec_string_clone(node_cur->u.as_literal->u.as_string), \
                                    ctx->irb->dataid_pool++);
                        }
                        break;
                }
                break;

            case XJS_CFG_NODE_TYPE_MAKE_FUNCTION:
                /* Nothing to record */
                break;

            case XJS_CFG_NODE_TYPE_MAKE_ARROW_FUNCTION:
                /* Nothing to record */
                break;

            case XJS_CFG_NODE_TYPE_INSPECT:
                /* Nothing to record */
                break;

            case XJS_CFG_NODE_TYPE_RETURN:
                /* Nothing to record */
                break;

            case XJS_CFG_NODE_TYPE_CALL:
            case XJS_CFG_NODE_TYPE_NEW:
            case XJS_CFG_NODE_TYPE_THIS:
                /* Nothing to record */
                break;
        }
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(walk_stack);
    ec_delete(visited_nodes);
    return ret;
}

static int xjs_c2_collect_data_section( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_ref cfg)
{
    int ret = 0;

    /* Functions */
    {
        ect_iterator(xjs_cfg_function_list) it_func;
        ect_for(xjs_cfg_function_list, cfg->functions, it_func)
        {
            xjs_cfg_function_ref func = ect_deref(xjs_cfg_function_ref, it_func);
            /* Parameters */
            if (xjs_c2_collect_data_section_func_parameters(ctx, func) != 0)
            { goto fail; }
            /* Body */
            if (xjs_c2_collect_data_section_func_body(ctx, func) != 0)
            { goto fail; }
        }
    }

    /* Exports */
    {
        ect_iterator(xjs_cfg_export_symbol_list) it_export;
        ect_for(xjs_cfg_export_symbol_list, cfg->export_symbols, it_export)
        {
            xjs_cfg_export_symbol_ref export_symbol = ect_deref(xjs_cfg_export_symbol_ref, it_export);
            if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                        export_symbol->exported) == 0)
            {
                xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                        ec_string_clone(export_symbol->exported), ctx->irb->dataid_pool++);
            }
            if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                        export_symbol->local) == 0)
            {
                xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                        ec_string_clone(export_symbol->local), ctx->irb->dataid_pool++);
            }
        }
    }

    /* Imports */
    {
        ect_iterator(xjs_cfg_import_symbol_list) it_import;
        ect_for(xjs_cfg_import_symbol_list, cfg->import_symbols, it_import)
        {
            xjs_cfg_import_symbol_ref import_symbol = ect_deref(xjs_cfg_import_symbol_ref, it_import);
            if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                        import_symbol->imported) == 0)
            {
                xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                        ec_string_clone(import_symbol->imported), ctx->irb->dataid_pool++);
            }
            if (xjs_ir_symbol_ir_dataid_map_count(ctx->irb->symbol_map, \
                        import_symbol->local) == 0)
            {
                xjs_ir_symbol_ir_dataid_map_insert(ctx->irb->symbol_map, \
                        ec_string_clone(import_symbol->local), ctx->irb->dataid_pool++);
            }
            if (xjs_ir_string_ir_dataid_map_count(ctx->irb->string_map, \
                        import_symbol->source) == 0)
            {
                xjs_ir_string_ir_dataid_map_insert(ctx->irb->string_map, \
                        ec_string_clone(import_symbol->source), ctx->irb->dataid_pool++);
            }
        }
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

static xjs_cfg_functionid_map_ref
xjs_c2_collect_text_section( \
        xjs_c2_ctx *ctx, \
        xjs_cfg_ref cfg)
{
    xjs_cfg_functionid_map_ref new_functionid_map = NULL;
    xjs_ir_functionid functionid = 0;

    (void)ctx;

    if ((new_functionid_map = ect_map_new(xjs_cfg_functionid_map)) == NULL)
    { return NULL; }

    ect_iterator(xjs_cfg_function_list) it_func;
    ect_for(xjs_cfg_function_list, cfg->functions, it_func)
    {
        xjs_cfg_function_ref func = ect_deref(xjs_cfg_function_ref, it_func);
        ect_map_insert(xjs_cfg_functionid_map, new_functionid_map, func, functionid++);
    }

    return new_functionid_map;
}

static int xjs_c2_write_data_section( \
        xjs_c2_ctx *ctx)
{
    /* Symbols */
    {
        ect_iterator(xjs_ir_symbol_ir_dataid_map) it;
        ect_for(xjs_ir_symbol_ir_dataid_map, ctx->irb->symbol_map, it)
        {
            ec_string *symbol = ect_deref_key(ec_string *, it);
            xjs_ir_dataid dataid = ect_deref_value(xjs_ir_dataid, it);
            xjs_ir_data_item_ref item = xjs_ir_data_item_symbol_new(dataid, ec_string_clone(symbol));
            xjs_irbuilder_push_back_data_item(ctx->irb, item);
        }
    }
    /* Strings */
    {
        ect_iterator(xjs_ir_string_ir_dataid_map) it;
        ect_for(xjs_ir_string_ir_dataid_map, ctx->irb->string_map, it)
        {
            ec_string *str = ect_deref_key(ec_string *, it);
            xjs_ir_dataid dataid = ect_deref_value(xjs_ir_dataid, it);
            xjs_ir_data_item_ref item = xjs_ir_data_item_string_new(dataid, ec_string_clone(str));
            xjs_irbuilder_push_back_data_item(ctx->irb, item);
        }
    }

    return 0;
}

static int xjs_c2_write_export_section( \
        xjs_c2_ctx *ctx, xjs_cfg_ref cfg)
{
    ect_iterator(xjs_cfg_export_symbol_list) it_export;
    ect_for(xjs_cfg_export_symbol_list, cfg->export_symbols, it_export)
    {
        xjs_cfg_export_symbol_ref export_symbol = ect_deref(xjs_cfg_export_symbol_ref, it_export);
        xjs_ir_dataid dataid_exported, dataid_local;
        dataid_exported = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, export_symbol->exported);
        dataid_local = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, export_symbol->local);
        xjs_irbuilder_append_exported_symbol(ctx->irb, dataid_exported, dataid_local);
    }

    return 0;
}

static int xjs_c2_write_import_section( \
        xjs_c2_ctx *ctx, xjs_cfg_ref cfg)
{
    ect_iterator(xjs_cfg_import_symbol_list) it_import;
    ect_for(xjs_cfg_import_symbol_list, cfg->import_symbols, it_import)
    {
        xjs_cfg_import_symbol_ref import_symbol = ect_deref(xjs_cfg_import_symbol_ref, it_import);
        xjs_ir_dataid dataid_imported, dataid_local, dataid_source;
        dataid_local = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, import_symbol->local);
        dataid_imported = xjs_ir_symbol_ir_dataid_map_get(ctx->irb->symbol_map, import_symbol->imported);
        dataid_source = xjs_ir_string_ir_dataid_map_get(ctx->irb->string_map, import_symbol->source);
        xjs_irbuilder_append_imported_symbol(ctx->irb, dataid_local, dataid_imported, dataid_source);
    }

    return 0;
}

/* C2 (CFG -> IR) */
xjs_ir_ref xjs_c2_start_ex( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg, \
        const char *filename)
{
    xjs_c2_ctx ctx;
    xjs_ir_ref new_ir = NULL;
    xjs_cfg_functionid_map_ref functionid_map = NULL;
    xjs_irbuilder_ref new_irb = NULL;
    ec_string *u_filename = NULL;

    XJS_VNZ_ERROR_MEM(new_irb = xjs_irbuilder_new(), err);
   
    xjs_c2_ctx_init(&ctx, err, new_irb);

    if ((filename != NULL) && (ec_strlen(filename) != 0))
    {
        ec_encoding_t enc;

        ec_encoding_utf8_init(&enc);
        if (ec_encoding_decode(&enc, &u_filename, (const ec_byte_t *)filename, ec_strlen(filename)) != 0)
        {
            XJS_VNZ_ERROR_INTERNAL(NULL, err);
            goto fail;
        }
        ctx.filename = u_filename;
    }

    /* Collect data items */
    if (xjs_c2_collect_data_section(&ctx, cfg) != 0) { goto fail; }

    /* Collect functions */
    if ((functionid_map = xjs_c2_collect_text_section(&ctx, cfg)) == NULL) { goto fail; }

    /* Write text items */
    if (xjs_c2_write_text_section(&ctx, cfg, functionid_map) != 0) { goto fail; }

    /* Write data items */
    if (xjs_c2_write_data_section(&ctx) != 0) { goto fail; }

    /* Write exported items */
    if (xjs_c2_write_export_section(&ctx, cfg) != 0) { goto fail; }

    /* Write imported items */
    if (xjs_c2_write_import_section(&ctx, cfg) != 0) { goto fail; }

    XJS_VNZ_ERROR_INTERNAL(new_ir = xjs_irbuilder_generate_ir(new_irb), err);

    goto done;
fail:
    if (new_ir != NULL)
    { ec_delete(new_ir); new_ir = NULL; }
done:
    ec_delete(new_irb);
    ec_delete(functionid_map);
    ec_delete(u_filename);
    return new_ir;
}

xjs_ir_ref xjs_c2_start( \
        xjs_error_ref err, \
        xjs_cfg_ref cfg)
{
    return xjs_c2_start_ex( \
            err, \
            cfg, \
            NULL);
}

