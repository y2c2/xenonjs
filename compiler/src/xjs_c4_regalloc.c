/* XenonJS : C4 : Register Allocation
 * Copyright(c) 2017 y2c2 */

#include <ec_algorithm.h>
#include <ec_map.h>
#include <ec_bitset.h>
#include <ec_bitdet.h>
#include <ec_vector.h>
#include <ec_matrix.h>
#include "xjs_dt.h"
#include "xjs_types.h"
#include "xjs_alloc.h"
#include "xjs_helper.h"
#include "xjs_c4_ctx.h"
#include "xjs_c4_regalloc.h"

#ifndef MAX2
#define MAX2(a, b) ((a)<(b)?(b):(a))
#endif

/* #define REGSCHED_R1_VERBOSE */

/* Life Interval */
typedef struct xjs_opaque_var_life_interval
{
    ec_size_t start, end;
} xjs_var_life_interval;
typedef struct xjs_opaque_var_life_interval *xjs_var_life_interval_ref;

xjs_var_life_interval_ref xjs_var_life_interval_new(ec_size_t point);

/* Map (var -> life interval) */
ect_map_declare(xjs_life_point_map, xjs_ir_var, xjs_var_life_interval_ref);
typedef xjs_life_point_map *xjs_life_point_map_ref;

static void xjs_var_life_interval_ctor(void *data)
{
    xjs_var_life_interval_ref r = data;
    r->start = r->end = 0;
}

static void xjs_var_life_interval_dtor(void *data)
{
    xjs_var_life_interval_ref r = data;
    (void)r;
}

xjs_var_life_interval_ref xjs_var_life_interval_new(ec_size_t point)
{
    xjs_var_life_interval_ref r = ec_newcd(xjs_var_life_interval, \
            xjs_var_life_interval_ctor, xjs_var_life_interval_dtor);
    r->start = r->end = point;
    return r;
}

static int xjs_life_point_map_key_ctor(xjs_ir_var *detta_key, xjs_ir_var *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_life_point_map_key_cmp(xjs_ir_var *a, xjs_ir_var *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_life_point_map_value_ctor(xjs_var_life_interval_ref *detta_value, xjs_var_life_interval_ref *value)
{
    *detta_value = *value;
    return 0;
}

static void xjs_life_point_map_value_dtor(xjs_var_life_interval_ref *value)
{
    ec_delete(*value);
}

ect_map_define_declared(xjs_life_point_map, \
        xjs_ir_var, xjs_life_point_map_key_ctor, NULL, \
        xjs_var_life_interval_ref, xjs_life_point_map_value_ctor, xjs_life_point_map_value_dtor, \
        xjs_life_point_map_key_cmp);


static int xjs_ir_var_reg_map_key_ctor(xjs_ir_var *detta_key, xjs_ir_var *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_var_reg_map_key_cmp(xjs_ir_var *a, xjs_ir_var *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

static int xjs_ir_var_reg_map_value_ctor(xjs_ir_reg *detta_value, xjs_ir_reg *value)
{
    *detta_value = *value;
    return 0;
}

ect_map_define_undeclared(xjs_ir_var_reg_map, \
        xjs_ir_var, xjs_ir_var_reg_map_key_ctor, NULL, \
        xjs_ir_reg, xjs_ir_var_reg_map_value_ctor, NULL, \
        xjs_ir_var_reg_map_key_cmp);
typedef xjs_ir_var_reg_map *xjs_ir_var_reg_map_ref;



/* vreg allocation matrix */
ect_vector_declare(xjs_ir_reg_vector, int);
ect_vector_define_declared(xjs_ir_reg_vector, int, NULL);
ect_matrix_define_undeclared(xjs_ir_reg_matrix, int, NULL);

typedef enum
{
    XJS_REGSCHED_TYPE_UNKNOWN = 0,
    XJS_REGSCHED_TYPE_SIMPLE = 1,
    XJS_REGSCHED_TYPE_R1 = 2,
} xjs_regsched_type;

struct xjs_opaque_regsched
{
    xjs_regsched_type type;
    union
    {
        /* Simple allocator requires the same number
         * of registers as the number of IR var, and yields
         * a new register to a new IR var, this is absolute buggy
         * which causes some registers *ALWAYS* holds the reference
         * to the resource if not exists before exiting from 
         * the stack frame */
        struct
        {
            ec_size_t regs_count;
            xjs_ir_reg next_reg;
            xjs_ir_var_reg_map_ref allocated_regs;
        } as_simple;

        struct
        {
            xjs_ir_reg_matrix *reg_alloc_tbl;
            ec_size_t regs_count;
        } as_r1;
    } u;

    xjs_error_ref err;
};

static void xjs_regsched_ctor(void *data)
{
    xjs_regsched_ref r = data;
    r->type = XJS_REGSCHED_TYPE_UNKNOWN;
}

static void xjs_regsched_dtor(void *data)
{
    xjs_regsched_ref r = data;
    switch (r->type)
    {
        case XJS_REGSCHED_TYPE_UNKNOWN:
            break;
        case XJS_REGSCHED_TYPE_SIMPLE:
            ec_delete(r->u.as_simple.allocated_regs);
            break;
        case XJS_REGSCHED_TYPE_R1:
            ec_delete(r->u.as_r1.reg_alloc_tbl);
            break;
    }
}

xjs_regsched_ref xjs_regsched_new(xjs_error_ref err)
{
    xjs_regsched_ref r = xjs_newcd(xjs_regsched, \
            xjs_regsched_ctor, xjs_regsched_dtor);
    r->err = err;
    return r;
}

int xjs_regsched_set_simple(xjs_regsched_ref sched, \
        ec_size_t regs_count)
{
    if (sched->type != XJS_REGSCHED_TYPE_UNKNOWN) { return -1; }

    sched->type = XJS_REGSCHED_TYPE_SIMPLE;
    sched->u.as_simple.next_reg = 0;
    sched->u.as_simple.regs_count = regs_count;
    if ((sched->u.as_simple.allocated_regs = ect_map_new(xjs_ir_var_reg_map)) == NULL)
    { return -1; }

    return 0;
}

/* Recording new IR variables used in function scope */
static int update_point( \
        xjs_error_ref err, \
        xjs_life_point_map_ref lpm, \
        xjs_size_t cur_point, \
        xjs_ir_var ir_var)
{
    int ret = 0;

    if (ect_map_count(xjs_life_point_map, lpm, ir_var) == 0)
    {
        /* Not found, create a new record */
        xjs_var_life_interval_ref new_interval;

        XJS_VNZ_ERROR_MEM(new_interval = xjs_var_life_interval_new(cur_point), err);
        ect_map_insert(xjs_life_point_map, lpm, ir_var, new_interval);
    }
    else
    {
        xjs_var_life_interval_ref interval = ect_map_get(xjs_life_point_map, lpm, ir_var);

        interval->end = cur_point;
    }

fail:
    return ret;
}

static int extend_point( \
        xjs_error_ref err, \
        xjs_life_point_map_ref lpm, \
        xjs_size_t cur_point, \
        xjs_ir_label_point_map_ref map_ir_label_to_point, \
        xjs_ir_label dest)
{
    int ret = 0;
    ect_iterator(xjs_life_point_map) it_lp;
    ec_size_t dest_point;

    XJS_V_ERROR_INTERNAL(ect_map_count(xjs_ir_label_point_map, map_ir_label_to_point, dest) != 0, err);
    dest_point = ect_map_get(xjs_ir_label_point_map, map_ir_label_to_point, dest);

    ect_for(xjs_life_point_map, lpm, it_lp)
    {
        xjs_var_life_interval_ref lp = ect_deref_value(xjs_var_life_interval_ref, it_lp);
        if (lp->end < dest_point) { continue; }
        lp->end = cur_point;
    }

    goto done;
fail:
    ret = -1;
done:
    return ret;
}

/*
static int extend_point( \
        xjs_error_ref err, \
        xjs_life_point_map_ref lpm, \
        xjs_size_t cur_point, \
        xjs_ir_label_point_map_ref map_ir_label_to_point, \
        xjs_ir_var dest)
{
    int ret = 0;
    ect_iterator(xjs_life_point_map) it_life_interval;
    ec_size_t dest_point;

    return 0;

    XJS_V_ERROR_INTERNAL(ect_map_count(xjs_ir_label_point_map, map_ir_label_to_point, dest) != 0, err);
    dest_point = ect_map_get(xjs_ir_label_point_map, map_ir_label_to_point, dest);

    ect_for(xjs_life_point_map, lpm, it_life_interval)
    {
        xjs_var_life_interval_ref life_interval = ect_deref_value(xjs_var_life_interval_ref, it_life_interval);
        if (life_interval->end < dest_point)
        {
        continue;
        }
        life_interval->end = cur_point;
    }

fail:
    return ret;
}
*/

#define FOR_EACH_IR_ITEM_BEGIN(_func, _item) \
        ect_iterator(xjs_ir_text_item_list) it; \
        ect_for(xjs_ir_text_item_list, _func->text_items, it) { \
            xjs_ir_text_item_ref _item = ect_deref(xjs_ir_text_item_ref, it); \

#define FOR_EACH_IR_ITEM_END() }

static xjs_life_point_map_ref
xjs_c4_function_var_life_interval_extract( \
        xjs_error_ref err, \
        xjs_ir_function_ref func, \
        xjs_ir_label_point_map_ref map_ir_label_to_point)
{
    xjs_life_point_map_ref lpm = NULL;
    ec_size_t cur_point = 0;

    XJS_VNZ_ERROR_MEM(lpm = ect_map_new(xjs_life_point_map), err);

#define UPDATE_POINT(_var) \
    do { \
        if (update_point(err, lpm, cur_point, _var) != 0) { goto fail; } \
    } while (0);

#define EXTEND_POINT(_dest_lbl) \
    do { \
        if (extend_point(err, lpm, cur_point, map_ir_label_to_point, _dest_lbl) != 0) { goto fail; } \
    } while (0)

    /*
    {
        if (extend_point(err, lpm, cur_point, map_ir_label_to_point, _dest_lbl) != 0) { goto fail; } \
    } while (0);
    */

    FOR_EACH_IR_ITEM_BEGIN(func, item)
    {
        switch (item->type)
        {
            case xjs_ir_text_item_type_nop:
            case xjs_ir_text_item_type_halt:
            case xjs_ir_text_item_type_push_scope:
            case xjs_ir_text_item_type_pop_scope:
            case xjs_ir_text_item_type_label:
                break;

            case xjs_ir_text_item_type_dynlib:
                UPDATE_POINT(item->u.as_dynlib.exports);
                UPDATE_POINT(item->u.as_dynlib.module_name);
                break;

            case xjs_ir_text_item_type_br:
                EXTEND_POINT(item->u.as_br.dest);
                break;

            case xjs_ir_text_item_type_br_cond:
                UPDATE_POINT(item->u.as_br_cond.cond);
                EXTEND_POINT(item->u.as_br_cond.dest);
                break;

            case xjs_ir_text_item_type_merge:
                UPDATE_POINT(item->u.as_merge.test);
                UPDATE_POINT(item->u.as_merge.consequent);
                UPDATE_POINT(item->u.as_merge.alternate);
                UPDATE_POINT(item->u.as_merge.dst);
                break;

            case xjs_ir_text_item_type_alloca:
                UPDATE_POINT(item->u.as_alloca.var);
                break;

            case xjs_ir_text_item_type_load_undefined:
                UPDATE_POINT(item->u.as_load_undefined.var);
                break;

            case xjs_ir_text_item_type_load_null:
                UPDATE_POINT(item->u.as_load_null.var);
                break;

            case xjs_ir_text_item_type_load_bool:
                UPDATE_POINT(item->u.as_load_bool.var);
                break;

            case xjs_ir_text_item_type_load_string:
                UPDATE_POINT(item->u.as_load_string.var);
                break;

            case xjs_ir_text_item_type_load_number:
                UPDATE_POINT(item->u.as_load_number.var);
                break;

            case xjs_ir_text_item_type_load_object:
                UPDATE_POINT(item->u.as_load_object.var);
                break;

            case xjs_ir_text_item_type_load_array:
                UPDATE_POINT(item->u.as_load_array.var);
                break;

            case xjs_ir_text_item_type_declvar:
                break;

            case xjs_ir_text_item_type_load:
                UPDATE_POINT(item->u.as_load.var);
                break;

            case xjs_ir_text_item_type_store:
                UPDATE_POINT(item->u.as_store.var);
                break;

            case xjs_ir_text_item_type_object_set:
                UPDATE_POINT(item->u.as_object_set.dst);
                UPDATE_POINT(item->u.as_object_set.obj);
                UPDATE_POINT(item->u.as_object_set.member);
                UPDATE_POINT(item->u.as_object_set.src);
                break;

            case xjs_ir_text_item_type_object_get:
                UPDATE_POINT(item->u.as_object_get.dst);
                UPDATE_POINT(item->u.as_object_get.member);
                UPDATE_POINT(item->u.as_object_get.obj);
                break;

            case xjs_ir_text_item_type_array_push:
                UPDATE_POINT(item->u.as_array_push.arr);
                UPDATE_POINT(item->u.as_array_push.elem);
                break;

            case xjs_ir_text_item_type_inspect:
                UPDATE_POINT(item->u.as_inspect.var);
                break;

            case xjs_ir_text_item_type_ret:
                UPDATE_POINT(item->u.as_ret.var);
                break;

            case xjs_ir_text_item_type_make_function:
                UPDATE_POINT(item->u.as_make_function.var);
                break;

            case xjs_ir_text_item_type_make_arrow_function:
                UPDATE_POINT(item->u.as_make_arrow_function.var);
                break;

            case xjs_ir_text_item_type_call:
                UPDATE_POINT(item->u.as_call.dst);
                UPDATE_POINT(item->u.as_call.callee);
                if (item->u.as_call.bound_this.enabled == xjs_true)
                {
                    UPDATE_POINT(item->u.as_call.bound_this._this);
                }
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, item->u.as_call.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        UPDATE_POINT(var_arg);
                    }
                }
                break;

            case xjs_ir_text_item_type_new:
                UPDATE_POINT(item->u.as_new.dst);
                UPDATE_POINT(item->u.as_new.callee);
                {
                    ect_iterator(xjs_ir_var_list) it_arg;
                    ect_for(xjs_ir_var_list, item->u.as_new.arguments, it_arg)
                    {
                        xjs_ir_var var_arg = ect_deref(xjs_ir_var, it_arg);
                        UPDATE_POINT(var_arg);
                    }
                }
                break;

            case xjs_ir_text_item_type_this:
                UPDATE_POINT(item->u.as_this.dst);
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
                UPDATE_POINT(item->u.as_binary_op.dst);
                UPDATE_POINT(item->u.as_binary_op.lhs);
                UPDATE_POINT(item->u.as_binary_op.rhs);
                break;

            case xjs_ir_text_item_type_unary_not:
            case xjs_ir_text_item_type_unary_bnot:
            case xjs_ir_text_item_type_unary_add:
            case xjs_ir_text_item_type_unary_sub:
                UPDATE_POINT(item->u.as_unary_op.dst);
                UPDATE_POINT(item->u.as_unary_op.src);
                break;
        }

        cur_point++;
    }
    FOR_EACH_IR_ITEM_END();

fail:
    return lpm;
}

static int allocate_vreg(ec_bitset *vreg_using)
{
    ec_size_t idx;

    for (idx = 0; idx != ec_bitset_size(vreg_using); idx++)
    {
        if (ec_bitset_test(vreg_using, idx) == ec_false)
        {
            ec_bitset_set(vreg_using, idx);
            return (int)idx;
        }
    }

    /* No available vreg */
    return -1;
}

static void deallocate_vreg(ec_bitset *vreg_using, xjs_ir_reg vreg)
{
    ec_bitset_reset(vreg_using, vreg);
}

static xjs_ir_label_point_map_ref
xjs_c4_label_point_map_extract( \
        xjs_error_ref err, \
        xjs_ir_function_ref func)
{
    xjs_ir_label_point_map_ref map_ir_label_to_point = NULL;
    ec_size_t point;

    XJS_VNZ_ERROR_MEM(map_ir_label_to_point = ect_map_new(xjs_ir_label_point_map), err);
    point = 0;
    FOR_EACH_IR_ITEM_BEGIN(func, item)
    {
        if (item->type == xjs_ir_text_item_type_label)
        {
            ect_map_insert(xjs_ir_label_point_map, \
                    map_ir_label_to_point, item->u.as_label.lbl, point);
        }
        point++;
    }
    FOR_EACH_IR_ITEM_END();
fail:
    return map_ir_label_to_point;
}

int xjs_regsched_set_r1( \
        xjs_error_ref err, \
        xjs_regsched_ref sched, \
        xjs_ir_function_ref func)
{
    int ret = 0;
    xjs_life_point_map_ref lpm = NULL;
    ect_iterator(xjs_life_point_map) it_lp;
    ec_bitdet *ir_var_life_det = NULL;
    ec_size_t ir_var_num;
    ec_bitset *vreg_using = NULL;
    ec_size_t period_end = 0;
    xjs_ir_reg_vector *vec_ir_var_to_reg = NULL;
    xjs_ir_label_point_map_ref map_ir_label_to_point = NULL;

    if (sched->type != XJS_REGSCHED_TYPE_UNKNOWN) { return -1; }

    /* Using R1 Scheduler */
    sched->type = XJS_REGSCHED_TYPE_R1;
    sched->u.as_r1.regs_count = 0;
    sched->u.as_r1.reg_alloc_tbl = NULL;

    if ((map_ir_label_to_point = xjs_c4_label_point_map_extract( \
                    err, func)) == NULL)
    { goto fail; }

    /* Extract life LPM */
    if ((lpm = xjs_c4_function_var_life_interval_extract(sched->err, func, map_ir_label_to_point)) == NULL)
    { goto fail; }

    /* DEBUG */
#ifdef REGSCHED_R1_VERBOSE
    printf("Life Intervals:\n");
    {
        ect_for(xjs_life_point_map, lpm, it_lp)
        {
            xjs_ir_var key = ect_deref_key(xjs_ir_var, it_lp);
            xjs_var_life_interval_ref value = ect_deref_value(xjs_var_life_interval_ref, it_lp);
            printf("  t%d:[%d, %d]\n", (int)key, (int)value->start, (int)value->end);
        }
    }
#endif

    {
        /* Number of all virtual regs */
        ir_var_num = ect_map_size(xjs_life_point_map, lpm);

        /* Period */
        ect_for(xjs_life_point_map, lpm, it_lp)
        {
            xjs_var_life_interval_ref value = ect_deref_value(xjs_var_life_interval_ref, it_lp);
            if (value->end > period_end) period_end = value->end;
        }
        period_end += 1;

        /* [ir_var_num, period] */
        XJS_VNZ_ERROR_MEM(ir_var_life_det = ec_bitdet_new(MAX2(ir_var_num, 1), MAX2(period_end, 1)), err);

        /* Fill with usage */
        ect_for(xjs_life_point_map, lpm, it_lp)
        {
            xjs_ir_var key = ect_deref_key(xjs_ir_var, it_lp);
            xjs_var_life_interval_ref value = ect_deref_value(xjs_var_life_interval_ref, it_lp);
            ec_size_t life_point;

            for (life_point = value->start; life_point <= value->end; life_point++)
            {
                /* Set as in using at that life point */ 
                ec_bitdet_set(ir_var_life_det, key, life_point);
            }
        }

        /* Find out the maximum requirement of vreg */
        {
            xjs_size_t vreg_count = 0;
            xjs_ir_var ir_var;
            ec_size_t lp;

            for (lp = 0; lp != period_end; lp++)
            {
                xjs_size_t vreg_living = 0;
                for (ir_var = 0; ir_var != ir_var_num; ir_var++)
                {
                    vreg_living += ec_bitdet_test(ir_var_life_det, ir_var, lp) == ec_true ? 1 : 0;
                }
                if (vreg_living > vreg_count) vreg_count = vreg_living;
            }
            sched->u.as_r1.regs_count = vreg_count;
        }
    }

    /* Debug */
    /*
    {
        xjs_ir_var ir_var;
        ec_size_t lp;

        for (lp = 0; lp != period_end; lp++)
        {
            printf("%4d: ", (int)lp);
            for (ir_var = 0; ir_var != ir_var_num; ir_var++)
            {
                printf("%c", \
                        ec_bitdet_test(ir_var_life_det, ir_var, lp) == ec_true ? '1' : '0');
            }
            printf("\n");
        }
    }
    */

    /* Build allocate table which maps
     * *ir_var* to *vreg*
     * in each life point */
    XJS_VNZ_ERROR_MEM(vreg_using = ec_bitset_new(MAX2(sched->u.as_r1.regs_count, 1)), err);

    /* Create allocate table [lp, ir_var] */
    XJS_VNZ_ERROR_MEM(vec_ir_var_to_reg = ect_vector_new(xjs_ir_reg_vector), err);
    {
        ec_size_t i;
        for (i = 0; i != ir_var_num; i++)
        { ect_vector_push_back(xjs_ir_reg_vector, vec_ir_var_to_reg, -1); }
    }
    XJS_VNZ_ERROR_MEM(sched->u.as_r1.reg_alloc_tbl = ect_matrix_new(xjs_ir_reg_matrix, ir_var_num, period_end), err);
    {
        xjs_size_t cur_lp = 0;

        /* Iterate in 'timeline' mode */
        for (cur_lp = 0; cur_lp != period_end; cur_lp++)
        {
            ect_for(xjs_life_point_map, lpm, it_lp)
            {
                xjs_ir_var ir_var = ect_deref_key(xjs_ir_var, it_lp);
                xjs_var_life_interval_ref period = ect_deref_value(xjs_var_life_interval_ref, it_lp);
                if (cur_lp == period->start)
                {
                    /* Allocate a vreg to this ir var */
                    int vreg = allocate_vreg(vreg_using);
                    XJS_V_ERROR_INTERNAL(vreg >= 0, err);
                    /* printf("lp %d: map IR var (%d) to vreg (%d)\n", (int)cur_lp, ir_var, vreg); */
                    ect_vector_set(xjs_ir_reg_vector, vec_ir_var_to_reg, ir_var, vreg);
                }
            }

            {
                ec_size_t i;
                /* printf("lp %d: copy vector ", (int)cur_lp); */
                for (i = 0; i != ir_var_num; i++)
                {
                    int vreg = ect_vector_at(xjs_ir_reg_vector, vec_ir_var_to_reg, i);
                    /* printf("%5d ", vreg); */
                    ect_matrix_set(xjs_ir_reg_matrix, sched->u.as_r1.reg_alloc_tbl, \
                            i, cur_lp, vreg);
                }
                /* printf("\n"); */
            }

            ect_for(xjs_life_point_map, lpm, it_lp)
            {
                xjs_ir_var ir_var = ect_deref_key(xjs_ir_var, it_lp);
                xjs_var_life_interval_ref period = ect_deref_value(xjs_var_life_interval_ref, it_lp);
                if (cur_lp == period->end)
                {
                    int vreg = -1;
                    vreg = ect_vector_at(xjs_ir_reg_vector, vec_ir_var_to_reg, ir_var);
                    /* printf("lp %d: unmap IR var (%d) to reg(%d)\n", (int)cur_lp, ir_var, vreg); */
                    ect_vector_set(xjs_ir_reg_vector, vec_ir_var_to_reg, ir_var, -1);
                    XJS_V_ERROR_INTERNAL(vreg != -1, err);
                    deallocate_vreg(vreg_using, (xjs_ir_reg)vreg);
                }
            }
        }
    }
#ifdef REGSCHED_R1_VERBOSE
    printf("Allocation matrix:\n");
    {
        ec_size_t x, y;
        for (y = 0; y != ect_matrix_height(xjs_ir_reg_matrix, sched->u.as_r1.reg_alloc_tbl); y++)
        {
            printf("%4d: ", (int)y);
            for (x = 0; x != ect_matrix_width(xjs_ir_reg_matrix, sched->u.as_r1.reg_alloc_tbl); x++)
            {
                int r = ect_matrix_get(xjs_ir_reg_matrix, sched->u.as_r1.reg_alloc_tbl, x, y);
                if (r == -1) printf("  N");
                else printf("%3d", r);
            }
            printf("\n");
        }
    }
    printf("\n");
#endif

    ec_delete(vreg_using); vreg_using = NULL;

    goto done;
fail:
    ret = -1;
done:
    ec_delete(ir_var_life_det);
    ec_delete(vec_ir_var_to_reg);
    ec_delete(vreg_using);
    ec_delete(lpm);
    ec_delete(map_ir_label_to_point);
    return ret;
}

int xjs_regsched_alloc(xjs_regsched_ref sched, \
        xjs_size_t lp_cur, \
        xjs_ir_var var)
{
    if (sched->type == XJS_REGSCHED_TYPE_UNKNOWN)
    {
        return -1;
    }
    else if (sched->type == XJS_REGSCHED_TYPE_SIMPLE)
    {
        if (ect_map_count(xjs_ir_var_reg_map, \
                    sched->u.as_simple.allocated_regs, var) != 0)
        {
            return ect_map_get(xjs_ir_var_reg_map, \
                    sched->u.as_simple.allocated_regs, var);
        }
        else
        {
            xjs_ir_reg reg = sched->u.as_simple.next_reg;
            ect_map_insert(xjs_ir_var_reg_map, sched->u.as_simple.allocated_regs, var, sched->u.as_simple.next_reg);
            sched->u.as_simple.next_reg++;
            return reg;
        }
    }
    else if (sched->type == XJS_REGSCHED_TYPE_R1)
    {
        int r = ect_matrix_get(xjs_ir_reg_matrix, sched->u.as_r1.reg_alloc_tbl, var, lp_cur);
        return r;
    }

    return -1;
}

/* Get the number requirement of registers */
ec_size_t xjs_regsched_requirement(xjs_regsched_ref sched)
{
    if (sched->type == XJS_REGSCHED_TYPE_SIMPLE)
    {
        return sched->u.as_simple.regs_count;
    }
    else if (sched->type == XJS_REGSCHED_TYPE_R1)
    {
        return sched->u.as_r1.regs_count;
    }
    else
    {
        return 0;
    }
}

