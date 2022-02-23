/* XenonJS : Dependency Table
 * Copyright(c) 2017 y2c2 */

#include <ec_alloc.h>
#include <ec_string.h>
#include <ec_set.h>
#include <ec_algorithm.h>
#include <ec_libc.h>
#include "xjs_deptbl.h"

static void xjs_dep_import_ctor(void *data)
{
    xjs_dep_import_ref r = data;
    r->name = NULL;
    r->fullpath = NULL;
    r->static_linked = xjs_false;
}

static void xjs_dep_import_dtor(void *data)
{
    xjs_dep_import_ref r = data;
    ec_delete(r->name);
    ec_delete(r->fullpath);
}

static xjs_dep_import_ref xjs_dep_import_new( \
        xjs_dep_import_type type, \
        ec_string *name, \
        ec_string *fullpath)
{
    xjs_dep_import_ref r = ec_newcd(xjs_dep_import, \
            xjs_dep_import_ctor, \
            xjs_dep_import_dtor);
    r->name = name;
    r->fullpath = fullpath;
    r->type = type;
    return r;
}

xjs_dep_import_ref xjs_dep_import_user_new( \
        ec_string *name, \
        ec_string *fullpath)
{
    return xjs_dep_import_new(XJS_DEP_IMPORT_TYPE_USER, name, fullpath);
}

xjs_dep_import_ref xjs_dep_import_system_new( \
        ec_string *name)
{
    return xjs_dep_import_new(XJS_DEP_IMPORT_TYPE_SYS, name, NULL);
}

void xjs_dep_import_set_static_linked(xjs_dep_import_ref import, xjs_bool value)
{
    import->static_linked = value;
}

static int xjs_dep_import_set_key_ctor(xjs_dep_import_ref *detta_key, xjs_dep_import_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_dep_import_set_key_dtor(xjs_dep_import_ref *key)
{
    ec_delete(*key);
    return 0;
}

static int xjs_dep_import_set_key_cmp(xjs_dep_import_ref *a, xjs_dep_import_ref *b)
{
    int v;

    v = ec_string_cmp((*a)->name, (*b)->name);

    if (v < 0) return -1;
    else if (v > 0) return 1;
    return 0;
}

ect_set_define_declared(xjs_dep_import_set, xjs_dep_import_ref, \
        xjs_dep_import_set_key_ctor, xjs_dep_import_set_key_dtor,
        xjs_dep_import_set_key_cmp);

static void xjs_dep_module_ctor(void *data)
{
    xjs_dep_module_ref r = data;
    r->imports = ect_set_new(xjs_dep_import_set);
    r->name = NULL;
    r->fullpath = NULL;
    r->ir = NULL;
    r->mark = ec_false;

    r->original_toplevel_funcid = 0;
}

static void xjs_dep_module_dtor(void *data)
{
    xjs_dep_module_ref r = data;
    ec_delete(r->imports);
    ec_delete(r->name);
    ec_delete(r->fullpath);
}

xjs_dep_module_ref xjs_dep_module_new(void)
{
    xjs_dep_module_ref r = ec_newcd(xjs_dep_module, \
            xjs_dep_module_ctor, \
            xjs_dep_module_dtor);
    return r;
}

void xjs_dep_module_ir_set(xjs_dep_module_ref dep_module, xjs_ir_ref ir)
{
    dep_module->ir = ir;
}

void xjs_dep_module_name_set(xjs_dep_module_ref dep_module, ec_string *name)
{
    dep_module->name = name;
}

void xjs_dep_module_fullpath_set(xjs_dep_module_ref dep_module, ec_string *fullpath)
{
    dep_module->fullpath = fullpath;
}

xjs_bool xjs_dep_module_has_import(xjs_dep_module_ref dep_module, xjs_dep_import_ref new_import)
{
    return (ect_set_count(xjs_dep_import_set, dep_module->imports, new_import) != 0) ? xjs_true : xjs_false;
}

void xjs_dep_module_add_import(xjs_dep_module_ref dep_module, xjs_dep_import_ref new_import)
{
    ect_set_insert(xjs_dep_import_set, dep_module->imports, new_import);
}

void xjs_dep_module_mark_clear(xjs_dep_module_ref dep_module)
{
    dep_module->mark = ec_false;
}

void xjs_dep_module_mark_set(xjs_dep_module_ref dep_module)
{
    dep_module->mark = ec_true;
}

static int xjs_dep_module_set_key_ctor(xjs_dep_module_ref *detta_key, xjs_dep_module_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_dep_module_set_key_dtor(xjs_dep_module_ref *key)
{
    ec_delete(*key);
    return 0;
}

static int xjs_dep_module_set_key_cmp(xjs_dep_module_ref *a, xjs_dep_module_ref *b)
{
    int v;

    v = ec_string_cmp((*a)->name, (*b)->name);
    if (v < 0) return -1;
    else if (v > 0) return 1;
    return 0;
}

ect_set_define_declared(xjs_dep_module_set, xjs_dep_module_ref, \
        xjs_dep_module_set_key_ctor, xjs_dep_module_set_key_dtor, 
        xjs_dep_module_set_key_cmp);

ect_list_define_declared(xjs_dep_module_list, xjs_dep_module_ref, NULL);
ect_stack_define(xjs_dep_module_stack, xjs_dep_module_ref, xjs_dep_module_list);

static void xjs_dep_table_ctor(void *data)
{
    xjs_dep_table_ref r = data;
    r->modules = ect_set_new(xjs_dep_module_set);
    r->entry = NULL;
}

static void xjs_dep_table_dtor(void *data)
{
    xjs_dep_table_ref r = data;
    ec_delete(r->modules);
}

xjs_dep_table_ref xjs_dep_table_new(void)
{
    xjs_dep_table_ref r = ec_newcd(xjs_dep_table, \
            xjs_dep_table_ctor,
            xjs_dep_table_dtor);
    return r;
}

void xjs_dep_table_insert(xjs_dep_table_ref tbl, xjs_dep_module_ref new_module)
{
    ect_set_insert(xjs_dep_module_set, tbl->modules, new_module);
}

void xjs_dep_table_entry_set(xjs_dep_table_ref tbl, xjs_dep_module_ref entry_module)
{
    tbl->entry = entry_module;
}

xjs_dep_module_ref xjs_dep_table_find_by_module_name(xjs_dep_table_ref tbl, ec_string *name)
{
    ect_iterator(xjs_dep_module_set) it;
    ect_for(xjs_dep_module_set, tbl->modules, it)
    {
        xjs_dep_module_ref module = ect_deref(xjs_dep_module_ref, it);
        if (ec_string_cmp(module->name, name) == 0)
        {
            return module;
        }
    }
    return NULL;
}

void xjs_dep_table_mark_clear(xjs_dep_table_ref tbl)
{
    ect_iterator(xjs_dep_module_set) it;
    ect_for(xjs_dep_module_set, tbl->modules, it)
    {
        xjs_dep_module_ref module = ect_deref(xjs_dep_module_ref, it);
        module->mark = ec_false;
    }
}

void xjs_dep_table_wipeout_clean_modules(xjs_dep_table_ref tbl)
{
    ect_iterator(xjs_dep_module_set) it;
    ect_for(xjs_dep_module_set, tbl->modules, it)
    {
        xjs_dep_module_ref module = ect_deref(xjs_dep_module_ref, it);
        if (module->mark == ec_false)
        {
            /* Clear */
            ect_set_erase(xjs_dep_module_set, tbl->modules, module);
        }
    }
}

static void xjs_dep_loaditem_ctor(void *data)
{
    xjs_dep_loaditem_ref r = data;
    r->ir = NULL;
    r->name = NULL;
    r->fullpath = NULL;
}

static void xjs_dep_loaditem_dtor(void *data)
{
    xjs_dep_loaditem_ref r = data;
    ec_delete(r->name);
    ec_delete(r->fullpath);
}

xjs_dep_loaditem_ref xjs_dep_loaditem_new( \
        xjs_ir_ref ir, \
        ec_string *name, \
        ec_string *fullpath)
{
    xjs_dep_loaditem_ref r = ec_newcd( \
            xjs_dep_loaditem, \
            xjs_dep_loaditem_ctor, \
            xjs_dep_loaditem_dtor);
    r->ir = ir;
    r->name = name;
    r->fullpath = fullpath;
    return r;
}

static void xjs_dep_loaditem_list_node_dtor(xjs_dep_loaditem_ref r)
{
    ec_delete(r);
}

ect_list_define_declared(xjs_dep_loaditem_list, xjs_dep_loaditem_ref, xjs_dep_loaditem_list_node_dtor);

xjs_dep_loaditem_ref xjs_dep_loaditem_list_find(xjs_dep_loaditem_list_ref lst, \
        ec_string *name)
{
    ect_iterator(xjs_dep_loaditem_list) it;
    ect_for(xjs_dep_loaditem_list, lst, it)
    {
        xjs_dep_loaditem_ref item = ect_deref(xjs_dep_loaditem_ref, it);
        if (ec_string_cmp(item->name, name) == 0)
        {
            return item;
        }
    }

    return NULL;
}

static int xjs_ir_dtor_pool_key_ctor(xjs_ir_ref *detta_key, xjs_ir_ref *key)
{
    *detta_key = *key;
    return 0;
}

static int xjs_ir_dtor_pool_key_dtor(xjs_ir_ref *key)
{
    ec_delete(*key);
    return 0;
}

static int xjs_ir_dtor_pool_key_cmp(xjs_ir_ref *a, xjs_ir_ref *b)
{
    if (*a < *b) return -1;
    else if (*a > *b) return 1;
    return 0;
}

ect_set_define_declared(xjs_ir_dtor_pool, xjs_ir_ref, \
        xjs_ir_dtor_pool_key_ctor, xjs_ir_dtor_pool_key_dtor, \
        xjs_ir_dtor_pool_key_cmp);

