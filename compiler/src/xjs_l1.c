/* XenonJS : L1
 * Copyright(c) 2017 y2c2 */

#include <ec_set.h>
#include <ec_list.h>
#include <ec_stack.h>
#include <ec_map.h>
#include <ec_algorithm.h>
#include "xjs_irbuilder.h"
#include "xjs_error.h"
#include "xjs_helper.h"
#include "xjs_deptbl.h"
#include "xjs_depwalk.h"
#include "xjs_l1_stdlib.h"
#include "xjs_l1.h"

/* Function ID Map */

ect_map_declare(xjs_ir_functionid_map, xjs_ir_functionid, xjs_ir_functionid);

static int xjs_ir_functionid_map_value_ctor(xjs_ir_functionid *detta_value, xjs_ir_functionid *value)
{ *detta_value = *value; return 0; }

static int xjs_ir_functionid_map_key_ctor(xjs_ir_functionid *detta_key, xjs_ir_functionid *key)
{ *detta_key = *key; return 0; }

static void xjs_ir_functionid_map_key_dtor(xjs_ir_functionid *key)
{ (void)key; }

static int xjs_ir_functionid_map_key_cmp(xjs_ir_functionid *a, xjs_ir_functionid *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_map_define_declared(xjs_ir_functionid_map, \
        xjs_ir_functionid, xjs_ir_functionid_map_key_ctor, xjs_ir_functionid_map_key_dtor, \
        xjs_ir_functionid, xjs_ir_functionid_map_value_ctor, NULL, \
        xjs_ir_functionid_map_key_cmp);
typedef xjs_ir_functionid_map *xjs_ir_functionid_map_ref;


/* Function ID List */

ect_list_declare(xjs_ir_functionid_list, xjs_ir_functionid);

ect_list_define_declared(xjs_ir_functionid_list, xjs_ir_functionid, NULL);
typedef xjs_ir_functionid_list *xjs_ir_functionid_list_ref;

/* Function ID Set */

static int xjs_ir_functionid_set_key_ctor(xjs_ir_functionid *detta_key, xjs_ir_functionid *key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_ir_functionid_set_key_dtor(xjs_ir_functionid *detta_key)
{
    (void)detta_key;
}

static int xjs_ir_functionid_set_key_cmp(xjs_ir_functionid *a, xjs_ir_functionid *b)
{
    if (*a > *b) return 1;
    else if (*a < *b) return -1;
    return 0;
}

ect_set_define_undeclared(xjs_ir_functionid_set, xjs_ir_functionid, \
        xjs_ir_functionid_set_key_ctor, xjs_ir_functionid_set_key_dtor, xjs_ir_functionid_set_key_cmp);
typedef xjs_ir_functionid_set *xjs_ir_functionid_set_ref;

/* String Set */

static int xjs_ir_string_set_key_ctor(ec_string **detta_key, ec_string **key)
{
    *detta_key = *key;
    return 0;
}

static void xjs_ir_string_set_key_dtor(ec_string **detta_key)
{
    ec_delete(*detta_key);
}

static int xjs_ir_string_set_key_cmp(ec_string **a, ec_string **b)
{
    return ec_string_cmp(*a, *b);
}

ect_set_define_undeclared(xjs_ir_string_set, ec_string *, \
        xjs_ir_string_set_key_ctor, xjs_ir_string_set_key_dtor, xjs_ir_string_set_key_cmp);
typedef xjs_ir_string_set *xjs_ir_string_set_ref;

/* Cycle reference detection */

static int xjs_l1_cycleref_detect( \
        xjs_error_ref err, \
        xjs_dep_table_ref deptbl)
{
    int ret = 0;
    xjs_dep_module_stack_ref dep_module_stack = NULL;
    ect_iterator(xjs_dep_module_set) it_module;
    ect_iterator(xjs_dep_import_set) it_import;

    /* Clear marks of all modules */
    xjs_dep_table_mark_clear(deptbl);

    /* Cycle reference detection */
    ect_for(xjs_dep_module_set, deptbl->modules, it_module)
    {
        xjs_dep_module_ref module_start = ect_deref(xjs_dep_module_ref, it_module);

        XJS_VNZ_ERROR_MEM(dep_module_stack = ect_stack_new(xjs_dep_module_stack), err);
        ect_stack_push(xjs_dep_module_stack, dep_module_stack, module_start);
        while (!ect_stack_empty(xjs_dep_module_stack, dep_module_stack))
        {
            xjs_dep_module_ref module_cur = ect_stack_top(xjs_dep_module_stack, dep_module_stack);
            ect_stack_pop(xjs_dep_module_stack, dep_module_stack);

            if (module_cur->mark == ec_false)
            {
                xjs_dep_module_mark_set(module_cur);

                ect_for(xjs_dep_import_set, module_cur->imports, it_import)
                {
                    xjs_dep_import_ref import = ect_deref(xjs_dep_import_ref, it_import);
                    switch (import->type)
                    {
                        case XJS_DEP_IMPORT_TYPE_SYS:
                            {
                                /* TODO: Add a unique item in import section of merged IR */
                            }
                            break;
                        case XJS_DEP_IMPORT_TYPE_USER:
                            {
                                xjs_dep_module_ref imported_module = xjs_dep_table_find_by_module_name(deptbl, import->name);
                                if (imported_module == NULL)
                                {
                                    XJS_ERROR(err, XJS_ERRNO_LINK);
                                    XJS_ERROR_PRINTF_SOURCE(err, \
                                            module_cur->fullpath, \
                                            "error: module '{string}' imported but not been linked", \
                                            import->name);
                                    ret = -1;
                                    goto fail;
                                }
                                ect_stack_push(xjs_dep_module_stack, dep_module_stack, imported_module);
                            }
                            break;
                    }
                }
            }
            else
            {
                if (module_cur == module_start)
                {
                    /* Visited module */
                    XJS_ERROR(err, XJS_ERRNO_LINK);
                    XJS_ERROR_PRINTF_SOURCE(err, \
                            module_cur->fullpath, \
                            "error: cycle reference on module '{string}' detected", module_cur->name);
                    ret = -1;
                    goto fail;
                }
            }

        }
        ec_delete(dep_module_stack); dep_module_stack = NULL;

        /* Clear all */
        xjs_dep_table_mark_clear(deptbl);
    }

fail:
    ec_delete(dep_module_stack);
    return ret;
}

static int xjs_l1_wipeout_unused_module( \
        xjs_error_ref err, \
        xjs_dep_table_ref deptbl)
{
    int ret = 0;
    xjs_dep_module_stack_ref dep_module_stack = NULL;
    xjs_dep_module_ref entry = deptbl->entry;
    ect_iterator(xjs_dep_import_set) it_import;

    /* Clear marks of all modules */
    xjs_dep_table_mark_clear(deptbl);

    /* Mark modules viable from entry */
    XJS_VNZ_ERROR_MEM(dep_module_stack = ect_stack_new(xjs_dep_module_stack), err);
    ect_stack_push(xjs_dep_module_stack, dep_module_stack, entry);
    while (!ect_stack_empty(xjs_dep_module_stack, dep_module_stack))
    {
        xjs_dep_module_ref module_cur = ect_stack_top(xjs_dep_module_stack, dep_module_stack);
        ect_stack_pop(xjs_dep_module_stack, dep_module_stack);

        xjs_dep_module_mark_set(module_cur);

        ect_for(xjs_dep_import_set, module_cur->imports, it_import)
        {
            xjs_dep_import_ref import;
            xjs_dep_module_ref imported_module;
            import = ect_deref(xjs_dep_import_ref, it_import);
            switch (import->type)
            {
                case XJS_DEP_IMPORT_TYPE_USER:
                    imported_module = xjs_dep_table_find_by_module_name(deptbl, import->name);
                    ect_stack_push(xjs_dep_module_stack, dep_module_stack, imported_module);
                    break;
                case XJS_DEP_IMPORT_TYPE_SYS:
                    imported_module = xjs_dep_table_find_by_module_name(deptbl, import->name);
                    if (imported_module == NULL) continue;
                    ect_stack_push(xjs_dep_module_stack, dep_module_stack, imported_module);
                    break;
            }
        }
    }

    /* Wipeout the clean modules */
    xjs_dep_table_wipeout_clean_modules(deptbl);

fail:
    ec_delete(dep_module_stack);
    return ret;
}

static int xjs_l1_extract_loading_chain( \
        xjs_error_ref err, \
        xjs_dep_module_list_ref *loading_chain_out, \
        xjs_dep_table_ref deptbl)
{
    int ret = 0;
    xjs_dep_module_stack_ref dep_module_stack = NULL;
    xjs_dep_module_ref entry = deptbl->entry;
    ect_iterator(xjs_dep_import_set) it_import;
    xjs_dep_module_list_ref loading_chain = NULL;

    /* Post-order walking to build the ordered loading chain
     * loading of each module should be put in seperate closure */

    /* Clear marks of all modules */
    xjs_dep_table_mark_clear(deptbl);

    /* Create an empty loading chain */
    XJS_VNZ_ERROR_MEM(loading_chain = ect_list_new(xjs_dep_module_list), err);

    XJS_VNZ_ERROR_MEM(dep_module_stack = ect_stack_new(xjs_dep_module_stack), err);
    ect_stack_push(xjs_dep_module_stack, dep_module_stack, entry);
    while (!ect_stack_empty(xjs_dep_module_stack, dep_module_stack))
    {
        xjs_dep_module_ref module_cur = ect_stack_top(xjs_dep_module_stack, dep_module_stack);
        ect_stack_pop(xjs_dep_module_stack, dep_module_stack);

        if (module_cur->mark == ec_false)
        {
            /* Fresh, mark it as visited and put it back */
            xjs_dep_module_mark_set(module_cur);
            ect_stack_push(xjs_dep_module_stack, dep_module_stack, module_cur);

            /* Push its children inside */
            ect_for(xjs_dep_import_set, module_cur->imports, it_import)
            {
                xjs_dep_import_ref import;
                import = ect_deref(xjs_dep_import_ref, it_import);
                if (import->type == XJS_DEP_IMPORT_TYPE_USER)
                {
                    xjs_dep_module_ref imported_module = xjs_dep_table_find_by_module_name(deptbl, import->name);
                    ect_stack_push(xjs_dep_module_stack, dep_module_stack, imported_module);
                }
                else if (import->type == XJS_DEP_IMPORT_TYPE_SYS)
                {
                    xjs_dep_module_ref imported_module = xjs_dep_table_find_by_module_name(deptbl, import->name);
                    if (imported_module == NULL) continue;
                    ect_stack_push(xjs_dep_module_stack, dep_module_stack, imported_module);
                }
            }
        }
        else
        {
            /* Load in chain */
            ect_list_push_back(xjs_dep_module_list, loading_chain, module_cur);
        }

        xjs_dep_module_mark_set(module_cur);
    }

    *loading_chain_out = loading_chain;
    loading_chain = NULL;

fail:
    ec_delete(dep_module_stack);
    ec_delete(loading_chain);
    return ret;
}

static int xjs_l1_merge_ir( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        xjs_dep_module_list_ref loading_chain)
{
    int ret = 0;
    ect_iterator(xjs_dep_module_list) it_module;
    ect_iterator(xjs_ir_data_item_list) it_di;
    ect_iterator(xjs_ir_text_item_list) it_ti;
    ect_iterator(xjs_ir_function_list) it_fn;
    ect_iterator(xjs_ir_parameter_list) it_param;
    ect_iterator(xjs_dep_import_set) it_import;
    xjs_irbuilder_ref irb = NULL;
    xjs_ir_function_ref new_fn = NULL;
    xjs_ir_dataid dataid_baseline = 0;
    xjs_ir_dataid_map *dataid_map = NULL;
    xjs_ir_functionid functionid_baseline = 0;
    xjs_ir_functionid functionid_idx = 0;
    xjs_irbuilder_function_ref irbf = NULL;
    xjs_ir_var_list_ref args = NULL;
    xjs_ir_string_set_ref loaded_sys_libs = NULL;

    XJS_VNZ_ERROR_MEM(irb = xjs_irbuilder_new(), err);

    XJS_VNZ_ERROR_MEM(loaded_sys_libs = ect_set_new(xjs_ir_string_set), err);

    ect_for(xjs_dep_module_list, loading_chain, it_module)
    {
        xjs_dep_module_ref module_cur = ect_deref(xjs_dep_module_ref, it_module);

        /* Clear all maps */
        XJS_VNZ_ERROR_MEM(dataid_map = ect_map_new(xjs_ir_dataid_map), err);

        /* data section */
        {
            /* string */
            ect_for(xjs_ir_data_item_list, module_cur->ir->data->items, it_di)
            {
                xjs_ir_data_item_ref di = ect_deref(xjs_ir_data_item_ref, it_di);
                xjs_ir_data_item_ref cloned_di;
                XJS_VNZ_ERROR_MEM(cloned_di = xjs_ir_data_item_clone(di), err);
                XJS_VEZ_ERROR_INTERNAL(ect_map_count(xjs_ir_dataid_map, dataid_map, cloned_di->dataid), err);
                ect_map_insert(xjs_ir_dataid_map, dataid_map, cloned_di->dataid, dataid_baseline);
                cloned_di->dataid = dataid_baseline;
                xjs_irbuilder_push_back_data_item(irb, cloned_di);
                dataid_baseline++;
            }
        }

        /* text section */
        ect_for(xjs_ir_function_list, module_cur->ir->text->functions, it_fn)
        {
            xjs_ir_function_ref fn = ect_deref(xjs_ir_function_ref, it_fn);

            if (module_cur->ir->toplevel == fn)
            {
                /* Top Level */
                module_cur->original_toplevel_funcid = functionid_idx;
                /* ect_set_insert(xjs_ir_functionid_set, toplevel_functions, functionid_idx); */
            }

            XJS_VNZ_ERROR_MEM(new_fn = xjs_ir_function_clone(fn), err);

#define APPLY_DATAID(_dataid) \
            do { _dataid = xjs_ir_dataid_map_get(dataid_map, _dataid); } while (0)

            /* Paramaters */
            ect_for(xjs_ir_parameter_list, new_fn->parameters, it_param)
            {
                xjs_ir_parameter_ref param = ect_deref(xjs_ir_parameter_ref, it_param);
                APPLY_DATAID(param->varname);
            }

            /* Instructions */
            ect_for(xjs_ir_text_item_list, new_fn->text_items, it_ti)
            {
                xjs_ir_text_item_ref ti = ect_deref(xjs_ir_text_item_ref, it_ti);

                if (ti->type == xjs_ir_text_item_type_load_string)
                { APPLY_DATAID(ti->u.as_load_string.dataid); }
                else if (ti->type == xjs_ir_text_item_type_declvar)
                { APPLY_DATAID(ti->u.as_declvar.variable); }
                else if (ti->type == xjs_ir_text_item_type_load)
                { APPLY_DATAID(ti->u.as_load.variable); }
                else if (ti->type == xjs_ir_text_item_type_store)
                { APPLY_DATAID(ti->u.as_store.variable); }
                else if (ti->type == xjs_ir_text_item_type_make_function)
                { ti->u.as_make_function.func += functionid_baseline; }
            }

#undef APPLY_DATAID

            xjs_irbuilder_append_function(irb, new_fn);
            new_fn = NULL;
            functionid_idx++;
        }
        functionid_baseline += ect_list_size( \
                xjs_ir_function_list, \
                module_cur->ir->text->functions);

        ec_delete(dataid_map); dataid_map = NULL;
    }

    /* Merged entry */
    {

        XJS_VNZ_ERROR_MEM(irbf = xjs_irbuilder_function_new(), err);
        xjs_ir_function_ref merged_entry;
        xjs_ir_var var_dst = xjs_irbuilder_function_allocate_var(irbf);
        xjs_ir_var var_exports;
        xjs_ir_var var_imports = xjs_irbuilder_function_allocate_var(irbf);
        xjs_ir_var var_void;

        /* imports = {}; */
        {
            xjs_ir_text_item_ref ti;
            ti = xjs_ir_text_item_alloca_new(var_dst);
            xjs_irbuilder_function_push_back_text_item(irbf, ti);
            ti = xjs_ir_text_item_alloca_new(var_imports);
            xjs_irbuilder_function_push_back_text_item(irbf, ti);
            ti = xjs_ir_text_item_load_object_new(var_imports);
            xjs_irbuilder_function_push_back_text_item(irbf, ti);
        }
        /* void */
        {
        }

        ect_for(xjs_dep_module_list, loading_chain, it_module)
        {
            xjs_dep_module_ref module_cur = ect_deref(xjs_dep_module_ref, it_module);
            xjs_ir_functionid funcid = module_cur->original_toplevel_funcid;
            xjs_ir_var var_func = xjs_irbuilder_function_allocate_var(irbf);

            /* dynlib in imports */
            {
                ect_for(xjs_dep_import_set, module_cur->imports, it_import)
                {
                    ec_string *import_name;
                    xjs_dep_import_ref import = ect_deref(xjs_dep_import_ref, it_import);

                    /* Skip non-system library */
                    if (import->type != XJS_DEP_IMPORT_TYPE_SYS) continue;
                    /* TODO: Skip staticly linked library */
                    if (import->static_linked == xjs_true) continue;
                    import_name = import->name;

                    /* Skip imported lib */
                    if (ect_set_count(xjs_ir_string_set, loaded_sys_libs, import_name) != 0) continue;

                    /* exports = {}; */
                    {
                        xjs_ir_text_item_ref ti;
                        var_exports = xjs_irbuilder_function_allocate_var(irbf);
                        ti = xjs_ir_text_item_alloca_new(var_exports);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);
                        ti = xjs_ir_text_item_load_object_new(var_exports);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);
                    }

                    {
                        xjs_ir_var var_module_name;
                        xjs_ir_text_item_ref ti;

                        /* dynlib(exports, moduleName) */
                        xjs_ir_data_item_ref data_item = xjs_ir_get_data_item_by_string(irb->ir, import_name);
                        XJS_VNZ_ERROR_INTERNAL(data_item, err);
                        var_module_name = xjs_irbuilder_function_allocate_var(irbf);
                        ti = xjs_ir_text_item_alloca_new(var_module_name);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);
                        ti = xjs_ir_text_item_load_string_new(var_module_name, data_item->dataid);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);
                        ti = xjs_ir_text_item_dynlib_new(var_exports, var_module_name);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);

                        /* imports[moduleName] = exports; */
                        ti = xjs_ir_text_item_object_set_new(var_dst, var_imports, var_module_name, var_exports);
                        xjs_irbuilder_function_push_back_text_item(irbf, ti);
                    }

                    /* Prevent duplicate loading */
                    ect_set_insert(xjs_ir_string_set, loaded_sys_libs, ec_string_clone(import_name));
                }
            }

            /* exports = {}; */
            {
                xjs_ir_text_item_ref ti;
                var_exports = xjs_irbuilder_function_allocate_var(irbf);
                ti = xjs_ir_text_item_alloca_new(var_exports);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
                ti = xjs_ir_text_item_load_object_new(var_exports);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
            }

            /* varVoid = originalTopLevel0(imports, exports); */
            {
                xjs_ir_text_item_ref ti;
                ti = xjs_ir_text_item_alloca_new(var_func);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
                ti = xjs_ir_text_item_make_function_new(var_func, funcid);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);

                var_void = xjs_irbuilder_function_allocate_var(irbf);
                ti = xjs_ir_text_item_alloca_new(var_void);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
                {
                    XJS_VNZ_ERROR_MEM(args = ect_list_new(xjs_ir_var_list), err);
                    ect_list_push_back(xjs_ir_var_list, args, var_imports);
                    ect_list_push_back(xjs_ir_var_list, args, var_exports);
                    ti = xjs_ir_text_item_call_new(var_void, var_func, args);
                    args = NULL;
                }
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
            }

            /* imports[moduleName] = exports; */
            if (ect_list_size(xjs_ir_export_list, module_cur->ir->exported) != 0)
            {
                xjs_ir_text_item_ref ti;
                xjs_ir_var var_module_name = xjs_irbuilder_function_allocate_var(irbf);
                xjs_ir_data_item_ref data_item = xjs_ir_get_data_item_by_string(irb->ir, module_cur->name);
                ti = xjs_ir_text_item_alloca_new(var_module_name);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
                ti = xjs_ir_text_item_load_string_new(var_module_name, data_item->dataid);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
                ti = xjs_ir_text_item_object_set_new(var_dst, var_imports, var_module_name, var_exports);
                xjs_irbuilder_function_push_back_text_item(irbf, ti);
            }
        }
        /* halt */
        {
            xjs_ir_text_item_ref ti = xjs_ir_text_item_halt_new();
            xjs_irbuilder_function_push_back_text_item(irbf, ti);
        }
        merged_entry = xjs_irbuilder_function_generate(irbf);
        xjs_irbuilder_append_function(irb, merged_entry);
        xjs_irbuilder_set_toplevel_function(irb, merged_entry);
    }

    XJS_VNZ_ERROR_INTERNAL(*ir_out = xjs_irbuilder_generate_ir(irb), err);

    goto done;
fail:
    ret = -1;
done:
    ec_delete(irb);
    ec_delete(new_fn);
    ec_delete(dataid_map);
    ec_delete(loaded_sys_libs);
    ec_delete(irbf);
    ec_delete(args);
    return ret;
}

int xjs_l1_start( \
        xjs_error_ref err, \
        xjs_ir_ref *ir_out, \
        xjs_ir_ref *ir_in, xjs_size_t ir_in_count, \
        const char *entry, \
        const xjs_bool no_stdlib, \
        const char *stdlib_source_data, \
        const xjs_size_t stdlib_source_len, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb, \
        int generate_debug_info)
{
    int ret = 0;
    xjs_ir_ref ir = NULL;
    xjs_dep_table_ref deptbl = NULL;
    xjs_dep_module_list_ref loading_chain = NULL;
    xjs_ir_ref ir_stdlib = NULL;
    xjs_dep_loaditem_list_ref load_list = NULL;
    xjs_ir_dtor_pool_ref ir_dtor_pool = NULL;

    /* Dynamicly maintained 'todo list' of modules to be loaded */
    XJS_VNZ_ERROR_MEM(load_list = ect_list_new(xjs_dep_loaditem_list), err);

    /* Build Standard Library */
    if (no_stdlib == xjs_false)
    {
        if (xjs_l1_stdlib_generate(err, &ir_stdlib, \
                    stdlib_source_data, stdlib_source_len, 0) != 0)
        { goto fail; }
        {
            xjs_dep_loaditem_ref new_item;
            XJS_VNZ_ERROR_MEM(new_item = xjs_dep_loaditem_new(ir_stdlib, \
                        ec_string_new_assign_c_str(XJS_STDLIB_MODULE_NAME), \
                        ec_string_new_assign_c_str(XJS_STDLIB_FILE_NAME)), err);
            ect_list_push_back(xjs_dep_loaditem_list, load_list, new_item);
        }
    }

    /* Append IRs into load list */
    {
        xjs_size_t i;
        for (i = 0; i < ir_in_count; i++)
        {
            xjs_ir_ref ir_cur = ir_in[i];
            {
                xjs_dep_loaditem_ref new_item;
                XJS_VNZ_ERROR_MEM(new_item = xjs_dep_loaditem_new(ir_cur, \
                            ec_string_new_assign_c_str(ir_cur->module.name), \
                            ec_string_new_assign_c_str(ir_cur->module.fullpath)), err);
                ect_list_push_back(xjs_dep_loaditem_list, load_list, new_item);
            }
        }
    }

    XJS_VNZ_ERROR_MEM(ir_dtor_pool = ect_set_new(xjs_ir_dtor_pool), err);

    /* Dependency Analysis */
    if (xjs_dependency_walk(err, &deptbl, \
                load_list, \
                entry, \
                ir_stdlib, \
                load_sys_lib_cb, \
                generate_debug_info, \
                ir_dtor_pool) != 0)
    { goto fail; }

    /* Cycle reference checking */
    if (xjs_l1_cycleref_detect(err, deptbl) != 0)
    { goto fail; }

    /* Wipeout Unused module */
    if (xjs_l1_wipeout_unused_module(err, deptbl) != 0)
    { goto fail; }

    /* Extract loading chain */
    if (xjs_l1_extract_loading_chain(err, &loading_chain, deptbl) != 0)
    { goto fail; }

    /* Collect imported system libraries */
    /*
    if (xjs_l1_collect_imported_system_libs(err, loading_chain) != 0)
    { goto fail; }
    */

    /* Merge IR */
    if (xjs_l1_merge_ir(err, &ir, loading_chain) != 0)
    { goto fail; }

    *ir_out = ir; ir = NULL; 
    goto done;
fail:
    ret = -1;
done:
    ec_delete(ir);
    ec_delete(ir_stdlib);
    ec_delete(deptbl);
    ec_delete(loading_chain);
    ec_delete(load_list);
    ec_delete(ir_dtor_pool);
    return ret;
}

