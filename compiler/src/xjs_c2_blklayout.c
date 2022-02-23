/* XenonJS : C2 : Block Layout
 * Copyright(c) 2017 y2c2 */

#include <ec_list.h>
#include <ec_stack.h>
#include <ec_set.h>
#include <ec_algorithm.h>
#include "xjs_alloc.h"
#include "xjs_helper.h"
#include "xjs_irbuilder.h"
#include "xjs_c2_blklayout.h"

/* ect_list_declare(xjs_cfg_node_list, int); */
ect_stack_define(xjs_cfg_node_stack, xjs_cfg_node_ref, xjs_cfg_node_list);

/* Liner Block */

static void xjs_c2_linear_block_ctor(void *data)
{
    xjs_c2_linear_block_ref r = data;
    r->node = NULL;
    r->anchor.enabled = ec_false;
    r->anchor.counter = 0;
    r->cancel_jump = ec_false;
}

static void xjs_c2_linear_block_dtor(void *data)
{
    xjs_c2_linear_block_ref r = data;
    (void)r;
}

xjs_c2_linear_block_ref xjs_c2_linear_block_new(void)
{
    xjs_c2_linear_block_ref r = ec_newcd(xjs_c2_linear_block, \
            xjs_c2_linear_block_ctor, xjs_c2_linear_block_dtor);
    return r;
}

void xjs_c2_linear_block_node_set(xjs_c2_linear_block_ref block, xjs_cfg_node_ref node)
{
    block->node = node;
}

void xjs_c2_linear_block_anchor_set(xjs_c2_linear_block_ref block)
{
    block->anchor.enabled = ec_true;
}

void xjs_c2_linear_block_anchor_inc_counter(xjs_c2_linear_block_ref block)
{
    block->anchor.counter++;
}

/* CFG node -> Linear Block Mapping */
static int xjs_cfg_node_linear_block_map_key_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_cfg_node_linear_block_map_key_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_cfg_node_linear_block_map_value_ctor(xjs_c2_linear_block_ref *detta_key, xjs_c2_linear_block_ref *key)
{
    *detta_key = *key;
    return 0;
}

ect_map_define_declared(xjs_cfg_node_linear_block_map, \
        xjs_cfg_node_ref, xjs_cfg_node_linear_block_map_key_ctor, NULL, \
        xjs_c2_linear_block_ref, xjs_cfg_node_linear_block_map_value_ctor, NULL, \
        xjs_cfg_node_linear_block_map_key_cmp);

/* Liner Block List */

static void xjs_c2_linear_block_list_node_dtor(xjs_c2_linear_block_ref node)
{
    ec_delete(node);
}

ect_list_define_declared(xjs_c2_linear_block_list, \
        xjs_c2_linear_block_ref, xjs_c2_linear_block_list_node_dtor);

/* Anchor Set */

static int xjs_c2_anchor_block_set_key_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_c2_anchor_block_set_key_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_set_define_declared(xjs_c2_anchor_block_set, xjs_cfg_node_ref, \
        xjs_c2_anchor_block_set_key_ctor, NULL, 
        xjs_c2_anchor_block_set_key_cmp);

/* Visited Node Set */

static int xjs_c2_visited_node_set_key_ctor(xjs_cfg_node_ref *detta_key, xjs_cfg_node_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_c2_visited_node_set_key_cmp(xjs_cfg_node_ref *a, xjs_cfg_node_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_set_define_declared(xjs_c2_visited_node_set, xjs_cfg_node_ref, \
        xjs_c2_visited_node_set_key_ctor, NULL, xjs_c2_visited_node_set_key_cmp);

/* Block Layout */

static void xjs_c2_block_layout_ctor(void *data)
{
    xjs_c2_block_layout_ref r = data;
    r->blocks = xjs_c2_linear_block_list_new();
}

static void xjs_c2_block_layout_dtor(void *data)
{
    xjs_c2_block_layout_ref r = data;
    ec_delete(r->blocks);
}

xjs_c2_block_layout_ref xjs_c2_block_layout_new(void)
{
    xjs_c2_block_layout_ref r = ec_newcd(xjs_c2_block_layout, \
            xjs_c2_block_layout_ctor, xjs_c2_block_layout_dtor);
    return r;
}

