/* XenonJS : C2 : Block Layout
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_C2_BLKLAYOUT_H
#define XJS_C2_BLKLAYOUT_H

#include <ec_list.h>
#include <ec_set.h>
#include <ec_map.h>
#include "xjs_types.h"
#include "xjs_cfg.h"
#include "xjs_irbuilder.h"
#include "xjs_c2_ctx.h"

struct xjs_opaque_c2_linear_block;
typedef struct xjs_opaque_c2_linear_block xjs_c2_linear_block;
typedef struct xjs_opaque_c2_linear_block *xjs_c2_linear_block_ref;

struct xjs_opaque_c2_block_layout;
typedef struct xjs_opaque_c2_block_layout xjs_c2_block_layout;
typedef struct xjs_opaque_c2_block_layout *xjs_c2_block_layout_ref;

ect_stack_declare(xjs_cfg_node_stack, xjs_cfg_node_ref, xjs_cfg_node_list);

/* Liner Block */

struct xjs_opaque_c2_linear_block
{
    /* Node */
    xjs_cfg_node_ref node;

    /* Is destination of jumping, should add
     * a label at the beginning of the block */
    struct
    {
        ec_bool enabled;
        ec_size_t counter;
    } anchor;

    /* We don't have to jump if the jump
     * is an in-place jump */
    ec_bool cancel_jump;
};

xjs_c2_linear_block_ref xjs_c2_linear_block_new(void);
void xjs_c2_linear_block_node_set(xjs_c2_linear_block_ref block, xjs_cfg_node_ref node);
void xjs_c2_linear_block_anchor_set(xjs_c2_linear_block_ref block);
void xjs_c2_linear_block_anchor_inc_counter(xjs_c2_linear_block_ref block);

/* CFG node -> Linear Block Mapping */
ect_map_declare(xjs_cfg_node_linear_block_map, xjs_cfg_node_ref, xjs_c2_linear_block_ref);

/* Linear Block List */

ect_list_declare(xjs_c2_linear_block_list, xjs_c2_linear_block_ref);
typedef xjs_c2_linear_block_list *xjs_c2_linear_block_list_ref;

/* Anchor Set */
ect_set_declare(xjs_c2_anchor_block_set, xjs_cfg_node_ref);

/* Visited Node Set */
ect_set_declare(xjs_c2_visited_node_set, xjs_cfg_node_ref);

/* Block Layout */

struct xjs_opaque_c2_block_layout
{
    /* Sequence of all blocks which
     * includes nodes in type Block, Branch and Jump */
    xjs_c2_linear_block_list_ref blocks;
};

xjs_c2_block_layout_ref xjs_c2_block_layout_new(void);

#endif

