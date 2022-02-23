/* XenonJS : Dependency Walk
 * Copyright(c) 2017 y2c2 */

#include <ec_set.h>
#include <ec_list.h>
#include <ec_string.h>
#include <ec_algorithm.h>
#include <ec_encoding.h>
#include <ec_encoding_utf8.h>
#include <ec_libc.h>
#include "xjs_l1_stdlib.h"
#include "xjs_helper.h"
#include "xjs_depwalk.h"

static xjs_bool _is_system_lib(ec_string *path)
{
    /* If the path contains seperator '/', it is considered as
     * a user lib, otherwise system lib */
    return ((ec_string_find(path, '/', 0) == ec_string_npos) && \
            (ec_string_find(path, '\\', 0) == ec_string_npos)) ? xjs_true : xjs_false;
}

static ec_string *_user_lib_module_name(ec_string *import_path)
{
    ec_string_size_type pos_slash = ec_string_rfind(import_path, '/', ec_string_length(import_path));
    ec_string_size_type pos_dot = ec_string_rfind(import_path, '.', ec_string_length(import_path));

    if (pos_slash == ec_string_npos)
    {
        pos_slash = ec_string_rfind(import_path, '\\', ec_string_length(import_path));
    }

    /* Error when no '/' which means it is not a relative or absolute path
     * of a user library */
    if (pos_slash == ec_string_npos) return NULL;

    if (pos_dot == ec_string_npos)
    {
        /* No '.' found, the whole file name used as the module name */
        return ec_string_substr(import_path, pos_slash + 1, ec_string_length(import_path) - pos_slash - 1);
    }

    return ec_string_substr(import_path, pos_slash + 1, pos_dot - pos_slash - 1);
}

static ec_string *_user_lib_fullpath(ec_string *cur_path, ec_string *import_path)
{
    ec_string *abs_path = NULL;

    /* TODO: Handle the import path generally */
    if (!((ec_string_length(import_path) > 2) && \
            (ec_string_at(import_path, 0) == '.') && \
            (ec_string_at(import_path, 1) == '/')))
    { return NULL; }

    if ((abs_path = ec_string_clone(cur_path)) == NULL)
    { return NULL; }

    ec_string_push_back(abs_path, '/');
    {
        ec_string *s1 = ec_string_substr(import_path, 2, ec_string_length(import_path) - 2);
        if (s1 == NULL)
        {
            ec_delete(abs_path);
            return NULL;
        }
        ec_string_append(abs_path, s1);
        ec_delete(s1);
    }

    return abs_path;
}

static int xjs_dependency_walk_item( \
        xjs_error_ref err, \
        xjs_dep_table_ref deptbl, \
        xjs_ir_ref ir, \
        xjs_dep_loaditem_list_ref pending_list, \
        xjs_dep_loaditem_list_ref loaded_list, \
        xjs_dep_module_ref *entry_dep_module, \
        const char *entry, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb, \
        int generate_debug_info, \
        xjs_ir_dtor_pool_ref ir_dtor_pool)
{
    int ret = 0;
    xjs_dep_module_ref new_dep_module = NULL;
    ec_string *cur_path = NULL, *cur_filename = NULL;
    xjs_dep_import_ref new_dep_import = NULL;

    XJS_VNZ_ERROR_MEM(new_dep_module = xjs_dep_module_new(), err);
    xjs_dep_module_ir_set(new_dep_module, ir);

    /* Module Name */
    {
        ec_encoding_t enc;
        ec_string *encoded_name;
        ec_encoding_utf8_init(&enc);
        XJS_VEZ_ERROR_INTERNAL( \
                ec_encoding_decode( \
                    &enc, \
                    &encoded_name, \
                    (ec_byte_t *)ir->module.name, ec_strlen(ir->module.name)), err);
        xjs_dep_module_name_set(new_dep_module, encoded_name);

        /* Entry Module */
        if ((ec_strlen(entry) == ec_strlen(ir->module.name)) && \
                (ec_strncmp(entry, ir->module.name, ec_strlen(entry)) == 0))
        {
            *entry_dep_module = new_dep_module;
        }
    }
    /* File */
    {
        ec_encoding_t enc;
        ec_string *decoded_name;
        ec_encoding_utf8_init(&enc);
        XJS_VEZ_ERROR_INTERNAL( \
                ec_encoding_decode( \
                    &enc, \
                    &decoded_name, \
                    (ec_byte_t *)ir->module.fullpath, ec_strlen(ir->module.fullpath)), err);
        xjs_dep_module_fullpath_set(new_dep_module, decoded_name);
        /* Current path */
        {
            ec_string_size_type pos_slash = ec_string_rfind(decoded_name, '/', ec_string_length(decoded_name));
            if (pos_slash == ec_string_npos)
            { pos_slash = ec_string_rfind(decoded_name, '\\', ec_string_length(decoded_name)); }
            if (pos_slash == ec_string_npos)
            {
                XJS_ERROR(err, XJS_ERRNO_LINK);
                XJS_ERROR_PRINTF(err, \
                        "error: invalid path of module '{string}'", \
                        decoded_name);
                goto fail;
            }
            cur_path = ec_string_substr(decoded_name, 0, pos_slash);
            cur_filename = ec_string_substr(decoded_name, pos_slash + 1, ec_string_length(decoded_name) - pos_slash - 1);
        }
    }
    {
        ect_iterator(xjs_ir_import_list) it_import_symbol;
        ect_for(xjs_ir_import_list, ir->imported, it_import_symbol)
        {
            xjs_ir_import_item_ref import_item = ect_deref(xjs_ir_import_item_ref, it_import_symbol);
            xjs_ir_data_item_ref data_item = xjs_ir_get_data_item_by_dataid(ir, import_item->source);
            if (data_item->type != xjs_ir_data_item_type_string)
            { goto fail; }
            {
                ec_string *import_path = data_item->u.as_string.value;

                /* 'import_path' may contain:
                 * 1. full path (e.g. '/usr/lib/js/example.js')
                 * 2. relative path to the location of the module file itself (e.g. './lib.js')
                 * 3. system lib name (e.g. 'fs' or 'http')
                 * Process it */
                if (_is_system_lib(import_path) == xjs_true)
                {
                    XJS_VNZ_ERROR_MEM(new_dep_import = xjs_dep_import_system_new(ec_string_clone(import_path)), err);

                    if ((xjs_dep_loaditem_list_find(loaded_list, import_path) == NULL) && \
                            (xjs_dep_loaditem_list_find(pending_list, import_path) == NULL))
                    {
                        xjs_ir_ref ir_syslib = NULL;
                        char *name = NULL;
                        xjs_size_t name_len;
                        char *source = NULL;
                        xjs_size_t source_len = 0;
                        char *lib_fullpath = NULL;
                        xjs_size_t lib_fullpath_len = 0;

                        /* Encode name */
                        ec_encoding_t enc;
                        ec_encoding_utf8_init(&enc);
                        XJS_VEZ_ERROR_INTERNAL( \
                                ec_encoding_encode( \
                                    &enc, \
                                    (ec_byte_t **)&name, &name_len, import_path), err);

                        if ((load_sys_lib_cb == NULL) || \
                                (load_sys_lib_cb(&source, &source_len, \
                                                 &lib_fullpath, &lib_fullpath_len, \
                                                 name, name_len, 0) != 0))
                        {
                            /* Could probably be native library */
                        }
                        else
                        {
                            if (xjs_l1_ir_generate(err, \
                                        &ir_syslib, \
                                        source, source_len, \
                                        name, name_len, \
                                        lib_fullpath, lib_fullpath_len, \
                                        generate_debug_info) != 0)
                            {
                                ec_free(source);
                                ec_free(name);
                                if (lib_fullpath != NULL) ec_free(lib_fullpath);
                                goto fail;
                            }

                            /* Static linked, * prevent from
                             * generating 'dynlib' instruction later */
                            xjs_dep_import_set_static_linked(new_dep_import, xjs_true);
                            {
                                /* ec_string *import_path_full = ec_string_new_assign_c_str("/"); */
                                /* ec_string_append(import_path_full, import_path); */
                                ect_list_push_front(xjs_dep_loaditem_list, pending_list, xjs_dep_loaditem_new( \
                                            ir_syslib, ec_string_clone(import_path), ec_string_new_assign_c_str(lib_fullpath)));
                            }
                            /* Add a weak reference to this mid-way generated IR */
                            ect_set_insert(xjs_ir_dtor_pool, ir_dtor_pool, ir_syslib);
                        }
                        ec_free(source);
                        ec_free(name);
                        if (lib_fullpath != NULL) ec_free(lib_fullpath);
                    }
                }
                else
                {
                    ec_string *user_lib_module_name = _user_lib_module_name(import_path);
                    ec_string *user_lib_fullpath = _user_lib_fullpath(cur_path, import_path);

                    if (xjs_dep_loaditem_list_find(loaded_list, user_lib_module_name) != NULL)
                    {
                        /* import something has been loaded, nothing to do with it */
                    }
                    else
                    {
                        if (xjs_dep_loaditem_list_find(pending_list, user_lib_module_name) != NULL)
                        {
                            /* To be loaded later, already in pending, nothing to do currently */
                        }
                        else
                        {
                            
                            XJS_ERROR(err, XJS_ERRNO_LINK);
                            XJS_ERROR_PRINTF(err, \
                                    "{string}: error: can not load user module '{string}'", \
                                    cur_filename, \
                                    user_lib_module_name);
                            goto fail;
                            /*
                               ect_list_push_back(xjs_dep_loaditem_list, pending_list, \
                               xjs_dep_loaditem_new( \
                               NULL, ec_string_clone(user_lib_module_name), ec_string_clone(user_lib_fullpath)));
                               */
                        }
                    }

                    XJS_VNZ_ERROR_MEM(new_dep_import = xjs_dep_import_user_new(user_lib_module_name, user_lib_fullpath), err);
                }
                if (xjs_dep_module_has_import(new_dep_module, new_dep_import) == xjs_true)
                {
                    ec_delete(new_dep_import);
                }
                else
                {
                    xjs_dep_module_add_import(new_dep_module, new_dep_import);
                }
                new_dep_import = NULL;
            }
        }
    }
    xjs_dep_table_insert(deptbl, new_dep_module);
    new_dep_module = NULL;
    ec_delete(cur_path); cur_path = NULL;
    ec_delete(cur_filename); cur_filename = NULL;

    goto done;
fail:
    ret = -1;
done:
    ec_delete(cur_path);
    ec_delete(cur_filename);
    ec_delete(new_dep_module);
    ec_delete(new_dep_import);
    return ret;
}

static int xjs_l1_insert_stdlib( \
        xjs_error_ref err, \
        xjs_dep_table_ref deptbl)
{
    int ret = 0;
    xjs_dep_import_ref new_dep_import = NULL;

    {
        ect_iterator(xjs_dep_module_set) it_module;
        ect_for(xjs_dep_module_set, deptbl->modules, it_module)
        {
            xjs_dep_module_ref module_cur = ect_deref(xjs_dep_module_ref, it_module);

            if (ec_string_match_c_str(module_cur->name, "stdlib") == ec_true) continue;

            XJS_VNZ_ERROR_MEM(new_dep_import = xjs_dep_import_user_new( \
                        ec_string_new_assign_c_str("stdlib"), \
                        ec_string_new_assign_c_str("/stdlib.js")), err);
            xjs_dep_module_add_import(module_cur, new_dep_import);
            new_dep_import = NULL;
        }
    }

    goto done;
fail:
    ret = -1;
done:
    ec_delete(new_dep_import);
    return ret;
}

int xjs_dependency_walk( \
        xjs_error_ref err, \
        xjs_dep_table_ref *deptbl_out, \
        xjs_dep_loaditem_list_ref pending_list, \
        const char *entry, \
        xjs_ir_ref ir_stdlib, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb, \
        int generate_debug_info, \
        xjs_ir_dtor_pool_ref ir_dtor_pool)
{
    int ret = 0;
    xjs_dep_table_ref new_deptbl = NULL;
    xjs_dep_module_ref entry_dep_module = NULL;
    xjs_dep_loaditem_list_ref loaded_list = NULL;

    XJS_VNZ_ERROR_MEM(new_deptbl = xjs_dep_table_new(), err);

    XJS_VNZ_ERROR_MEM(loaded_list = ect_list_new(xjs_dep_loaditem_list), err);

    /* Build graph */
    while (ect_list_size(xjs_dep_loaditem_list, pending_list) != 0)
    {
        xjs_dep_loaditem_ref item_to_load = ect_list_back(xjs_dep_loaditem_list, pending_list);
        xjs_ir_ref ir_to_load = item_to_load->ir;

        if (xjs_dependency_walk_item(err, \
                    new_deptbl, ir_to_load, \
                    pending_list, loaded_list, \
                    &entry_dep_module, entry, \
                    load_sys_lib_cb, \
                    generate_debug_info, \
                    ir_dtor_pool) != 0)
        { goto fail; }

        ect_list_push_back(xjs_dep_loaditem_list, loaded_list, xjs_dep_loaditem_new( \
                    item_to_load->ir, \
                    ec_string_clone(item_to_load->name), ec_string_clone(item_to_load->fullpath)));

        ect_list_pop_back(xjs_dep_loaditem_list, pending_list);
    }

    /* Entry */
    if (entry_dep_module == NULL)
    {
        /* what is the entry even if not specified */
        if (ect_set_size(xjs_dep_module_set, new_deptbl->modules) == 1)
        {
            /* The only one module should be the entry */
            ect_iterator(xjs_dep_module_set) it;
            ect_list_iterator_init_begin(xjs_dep_module_set, &it, new_deptbl->modules);
            entry_dep_module = ect_deref(xjs_dep_module_ref, it);
        }
        else
        {
            /* TODO: Should do some guess */
            XJS_ERROR(err, XJS_ERRNO_LINK);
            XJS_ERROR_PUTS(err, "error: invalid entry specified");
            goto fail;
        }
    }

    if (ir_stdlib != NULL)
    {
        if (xjs_l1_insert_stdlib(err, new_deptbl) != 0)
        { goto fail; }
    }

    xjs_dep_table_entry_set(new_deptbl, entry_dep_module);

    *deptbl_out = new_deptbl;

    goto done;
fail:
    ec_delete(new_deptbl);
    ret = -1;
done:
    ec_delete(loaded_list);
    return ret;
}

