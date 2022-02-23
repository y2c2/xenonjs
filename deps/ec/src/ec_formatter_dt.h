/* Enhanced C : Formatter : Data Types
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_FORMATTER_DT_H
#define EC_FORMATTER_DT_H

#include "ec_dt.h"
#include "ec_list.h"
#include "ec_map.h"
#include "ec_string.h"
#include <stdarg.h>

typedef void* ec_formatter_anchor_data_t;
typedef void (*ec_formatter_anchor_data_dtor_t)(
    ec_formatter_anchor_data_t instance_data);

/* Formatter : Plugin : Arguments */

ect_map_declare(ec_formatter_plugin_make_anchor_args, ec_string*, ec_string*);

/* Init */

struct ec_formatter_plugin_args_init
{
    int dummy;
};
typedef struct ec_formatter_plugin_args_init ec_formatter_plugin_args_init_t;

/* Uninit */

struct ec_formatter_plugin_args_uninit
{
    int dummy;
};
typedef struct ec_formatter_plugin_args_uninit
    ec_formatter_plugin_args_uninit_t;

/* Make Anchor */

struct ec_formatter_plugin_args_make_anchor
{
    /* Parameters and arguments for making an anchor */
    ec_formatter_plugin_make_anchor_args* anchor_args;

    ec_formatter_anchor_data_t data;
    ec_formatter_anchor_data_dtor_t data_dtor;
};
typedef struct ec_formatter_plugin_args_make_anchor
    ec_formatter_plugin_args_make_anchor_t;

void ec_formatter_plugin_args_make_anchor_set_data(
    ec_formatter_plugin_args_make_anchor_t* args,
    ec_formatter_anchor_data_t data);
void ec_formatter_plugin_args_make_anchor_set_data_dtor(
    ec_formatter_plugin_args_make_anchor_t* args,
    ec_formatter_anchor_data_dtor_t data_dtor);

/* Apply Anchor */

struct ec_formatter_plugin_args_apply_anchor
{
    ec_formatter_anchor_data_t data;
    ec_string* result;
    va_list ap;
};
typedef struct ec_formatter_plugin_args_apply_anchor
    ec_formatter_plugin_args_apply_anchor_t;

void ec_formatter_plugin_args_apply_anchor_append_string(
    ec_formatter_plugin_args_apply_anchor_t* args, ec_string* s);

void ec_formatter_plugin_args_apply_anchor_append_char(
    ec_formatter_plugin_args_apply_anchor_t* args, ec_char_t c);

void ec_formatter_plugin_args_apply_anchor_append_c_str(
    ec_formatter_plugin_args_apply_anchor_t* args, const char* s);

#define ec_formatter_plugin_args_apply_anchor_va_arg(_args, _type)             \
    (va_arg(args->ap, _type))

/* Formatter : Plugin */

/* Initialize Callback */
typedef int (*ec_formatter_plugin_cb_init_t)(
    ec_formatter_plugin_args_init_t* args);
/* Uninitialize Callback */
typedef int (*ec_formatter_plugin_cb_uninit_t)(
    ec_formatter_plugin_args_uninit_t* args);
/* Make Anchor */
typedef int (*ec_formatter_plugin_cb_make_anchor_t)(
    ec_formatter_plugin_args_make_anchor_t* args);
/* Apply Anchor */
typedef int (*ec_formatter_plugin_cb_apply_anchor_t)(
    ec_formatter_plugin_args_apply_anchor_t* args);

struct ec_formatter_plugin
{
    ec_string* name;
    ec_formatter_plugin_cb_init_t cb_init;
    ec_formatter_plugin_cb_uninit_t cb_uninit;
    ec_formatter_plugin_cb_make_anchor_t cb_make_anchor;
    ec_formatter_plugin_cb_apply_anchor_t cb_apply_anchor;
};
typedef struct ec_formatter_plugin ec_formatter_plugin_t;

ec_formatter_plugin_t*
ec_formatter_plugin_new(const char* name, ec_formatter_plugin_cb_init_t cb_init,
                        ec_formatter_plugin_cb_uninit_t cb_uninit,
                        ec_formatter_plugin_cb_make_anchor_t cb_make_anchor,
                        ec_formatter_plugin_cb_apply_anchor_t cb_apply_anchor);

/* Formatter : Plugin List */

ect_list_declare(ec_formatter_plugin_list, ec_formatter_plugin_t*);

/* Formatter : Template Item Type */

enum ec_formatter_template_item_type
{
    EC_FORMATTER_TEMPLATE_ITEM_TYPE_TEXT,
    EC_FORMATTER_TEMPLATE_ITEM_TYPE_ANCHOR,
};
typedef enum ec_formatter_template_item_type ec_formatter_template_item_type_t;

struct ec_formatter_template_item_anchor
{
    ec_formatter_plugin_t* plugin;
    ec_formatter_anchor_data_t* anchor_data;
    ec_formatter_anchor_data_dtor_t anchor_data_dtor;
};
typedef struct ec_formatter_template_item_anchor
    ec_formatter_template_item_anchor_t;

/* Formatter : Template Item */

struct ec_formatter_template_item
{
    ec_formatter_template_item_type_t type;
    union
    {
        ec_string* text;
        ec_formatter_template_item_anchor_t anchor;
    } u;
};
typedef struct ec_formatter_template_item ec_formatter_template_item_t;

ec_formatter_template_item_t*
ec_formatter_template_item_new_text(ec_string* text);
ec_formatter_template_item_t* ec_formatter_template_item_new_anchor(
    ec_formatter_plugin_t* plugin,
    ec_formatter_plugin_make_anchor_args* anchor_args);

/* Formatter : Template Item List */

ect_list_declare(ec_formatter_template_item_list,
                 ec_formatter_template_item_t*);

/* Formatter : Template */

struct ec_formatter_template
{
    ec_formatter_template_item_list* items;
};
typedef struct ec_formatter_template ec_formatter_template_t;

ec_formatter_template_t* ec_formatter_template_new(void);
void ec_formatter_template_push_back(ec_formatter_template_t* tplt,
                                     ec_formatter_template_item_t* item);
ec_size_t ec_formatter_template_items_count(ec_formatter_template_t* tplt);

/* Formatter */

struct ec_formatter
{
    ec_formatter_plugin_list* plugins;
};
typedef struct ec_formatter ec_formatter_t;

ec_formatter_t* ec_formatter_new(void);
ec_formatter_plugin_t*
ec_formatter_find_plugin_by_name(ec_formatter_t* formatter, ec_string* name);

#endif
