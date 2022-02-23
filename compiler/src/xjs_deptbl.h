/* XenonJS : Dependency Table
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_DEPTBL_H
#define XJS_DEPTBL_H

#include <ec_set.h>
#include <ec_list.h>
#include <ec_stack.h>
#include <ec_string.h>
#include "xjs_ir.h"

/* Import */
typedef enum
{
    XJS_DEP_IMPORT_TYPE_SYS,
    XJS_DEP_IMPORT_TYPE_USER,
} xjs_dep_import_type;

struct xjs_opaque_dep_import
{
    xjs_dep_import_type type;
    ec_string *name;
    ec_string *fullpath;
    xjs_bool static_linked;
};
typedef struct xjs_opaque_dep_import xjs_dep_import;
typedef struct xjs_opaque_dep_import *xjs_dep_import_ref;

ect_set_declare(xjs_dep_import_set, xjs_dep_import_ref);
typedef xjs_dep_import_set *xjs_dep_import_set_ref;

xjs_dep_import_ref xjs_dep_import_user_new( \
        ec_string *name, \
        ec_string *fullpath);

xjs_dep_import_ref xjs_dep_import_system_new( \
        ec_string *name);
void xjs_dep_import_set_static_linked(xjs_dep_import_ref import, xjs_bool value);

/* Module */
struct xjs_opaque_dep_module
{
    xjs_ir_ref ir;
    ec_string *name;
    ec_string *fullpath;

    ec_bool mark;

    xjs_dep_import_set_ref imports;

    xjs_ir_functionid original_toplevel_funcid;
};
typedef struct xjs_opaque_dep_module xjs_dep_module;
typedef struct xjs_opaque_dep_module *xjs_dep_module_ref;

xjs_dep_module_ref xjs_dep_module_new(void);
void xjs_dep_module_ir_set(xjs_dep_module_ref dep_module, xjs_ir_ref ir);
void xjs_dep_module_name_set(xjs_dep_module_ref dep_module, ec_string *name);
void xjs_dep_module_fullpath_set(xjs_dep_module_ref dep_module, ec_string *fullpath);
xjs_bool xjs_dep_module_has_import(xjs_dep_module_ref dep_module, xjs_dep_import_ref new_import);
void xjs_dep_module_add_import(xjs_dep_module_ref dep_module, xjs_dep_import_ref new_import);

void xjs_dep_module_mark_clear(xjs_dep_module_ref dep_module);
void xjs_dep_module_mark_set(xjs_dep_module_ref dep_module);

ect_set_declare(xjs_dep_module_set, xjs_dep_module_ref);
typedef xjs_dep_module_set *xjs_dep_module_set_ref;

ect_list_declare(xjs_dep_module_list, xjs_dep_module_ref);
typedef xjs_dep_module_list *xjs_dep_module_list_ref;
ect_stack_declare(xjs_dep_module_stack, xjs_dep_module_ref, xjs_dep_module_list);
typedef xjs_dep_module_stack *xjs_dep_module_stack_ref;

/* Table */
struct xjs_opaque_dep_table
{
    xjs_dep_module_set_ref modules;

    xjs_dep_module_ref entry;
};
typedef struct xjs_opaque_dep_table xjs_dep_table;
typedef struct xjs_opaque_dep_table *xjs_dep_table_ref;

xjs_dep_table_ref xjs_dep_table_new(void);
void xjs_dep_table_insert(xjs_dep_table_ref tbl, xjs_dep_module_ref new_module);
void xjs_dep_table_entry_set(xjs_dep_table_ref tbl, xjs_dep_module_ref entry);
void xjs_dep_table_mark_clear(xjs_dep_table_ref tbl);
xjs_dep_module_ref xjs_dep_table_find_by_module_name(xjs_dep_table_ref tbl, ec_string *name);
void xjs_dep_table_wipeout_clean_modules(xjs_dep_table_ref tbl);

/* Loading queue */
struct xjs_opaque_dep_loaditem
{
    xjs_ir_ref ir;
    ec_string *name;
    ec_string *fullpath;
};
typedef struct xjs_opaque_dep_loaditem xjs_dep_loaditem;
typedef struct xjs_opaque_dep_loaditem *xjs_dep_loaditem_ref;

xjs_dep_loaditem_ref xjs_dep_loaditem_new( \
        xjs_ir_ref ir, \
        ec_string *name, \
        ec_string *fullpath);

ect_list_declare(xjs_dep_loaditem_list, xjs_dep_loaditem_ref);
typedef xjs_dep_loaditem_list *xjs_dep_loaditem_list_ref;

xjs_dep_loaditem_ref xjs_dep_loaditem_list_find(xjs_dep_loaditem_list_ref lst, \
        ec_string *name);

/* IR destruction pool */
ect_set_declare(xjs_ir_dtor_pool, xjs_ir_ref);
typedef xjs_ir_dtor_pool *xjs_ir_dtor_pool_ref;

#endif

