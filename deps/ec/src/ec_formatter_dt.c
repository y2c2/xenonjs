/* Enhanced C : Formatter : Data Types
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_dt.h"
#include "ec_algorithm.h"
#include "ec_alloc.h"
#include "ec_encoding.h"
#include "ec_libc.h"
#include <stdarg.h>

/* Make Anchor */

void ec_formatter_plugin_args_make_anchor_set_data(
    ec_formatter_plugin_args_make_anchor_t* args,
    ec_formatter_anchor_data_t data)
{
    args->data = data;
}

void ec_formatter_plugin_args_make_anchor_set_data_dtor(
    ec_formatter_plugin_args_make_anchor_t* args,
    ec_formatter_anchor_data_dtor_t data_dtor)
{
    args->data_dtor = data_dtor;
}

/* Apply Anchor */

void ec_formatter_plugin_args_apply_anchor_append_string(
    ec_formatter_plugin_args_apply_anchor_t* args, ec_string* s)
{
    ec_string_append(args->result, s);
}

void ec_formatter_plugin_args_apply_anchor_append_char(
    ec_formatter_plugin_args_apply_anchor_t* args, ec_char_t c)
{
    ec_string_push_back(args->result, c);
}

void ec_formatter_plugin_args_apply_anchor_append_c_str(
    ec_formatter_plugin_args_apply_anchor_t* args, const char* s)
{
    ec_encoding_t enc;
    ec_string* decoded_s;
    ec_encoding_utf8_init(&enc);
    ec_encoding_decode(&enc, &decoded_s, (const ec_byte_t*)s, ec_strlen(s));
    ec_formatter_plugin_args_apply_anchor_append_string(args, decoded_s);
    ec_delete(decoded_s);
}

static void ec_formatter_plugin_node_dtor(ec_formatter_plugin_t* ptr);
static void
ec_formatter_template_item_node_dtor(ec_formatter_template_item_t* ptr);

static int
ec_formatter_plugin_make_anchor_args_string_ctor(ec_string** detta_key,
                                                 ec_string** key)
{
    *detta_key = *key;
    return 0;
}

static void
ec_formatter_plugin_make_anchor_args_string_dtor(ec_string** detta_key)
{
    (void)detta_key;
}

static int ec_formatter_plugin_make_anchor_args_string_cmp(ec_string** a,
                                                           ec_string** b)
{
    return ec_string_cmp(*a, *b);
}

ect_map_define_declared(ec_formatter_plugin_make_anchor_args, ec_string*,
                        ec_formatter_plugin_make_anchor_args_string_ctor,
                        ec_formatter_plugin_make_anchor_args_string_dtor,
                        ec_string*,
                        ec_formatter_plugin_make_anchor_args_string_ctor,
                        ec_formatter_plugin_make_anchor_args_string_dtor,
                        ec_formatter_plugin_make_anchor_args_string_cmp);

ect_list_define_declared(ec_formatter_plugin_list, ec_formatter_plugin_t*,
                         ec_formatter_plugin_node_dtor);

ect_list_define_declared(ec_formatter_template_item_list,
                         ec_formatter_template_item_t*,
                         ec_formatter_template_item_node_dtor);

static void ec_formatter_plugin_node_dtor(ec_formatter_plugin_t* ptr)
{
    ec_formatter_plugin_t* plugin = ptr;
    if (plugin->name != NULL)
        ec_delete(plugin->name);
    ec_free(plugin);
}

ec_formatter_plugin_t*
ec_formatter_plugin_new(const char* name, ec_formatter_plugin_cb_init_t cb_init,
                        ec_formatter_plugin_cb_uninit_t cb_uninit,
                        ec_formatter_plugin_cb_make_anchor_t cb_make_anchor,
                        ec_formatter_plugin_cb_apply_anchor_t cb_apply_anchor)
{
    ec_encoding_t enc;
    ec_formatter_plugin_t* new_plugin;
    ec_encoding_utf8_init(&enc);
    if ((new_plugin = ec_malloc(sizeof(ec_formatter_plugin_t))) == NULL)
    {
        return NULL;
    }
    if (ec_encoding_decode(&enc, &new_plugin->name, (const ec_byte_t*)name,
                           ec_strlen(name)) != 0)
    {
        ec_free(new_plugin);
        return NULL;
    }
    new_plugin->cb_init = cb_init;
    new_plugin->cb_uninit = cb_uninit;
    new_plugin->cb_make_anchor = cb_make_anchor;
    new_plugin->cb_apply_anchor = cb_apply_anchor;
    return new_plugin;
}

static void
ec_formatter_template_item_node_dtor(ec_formatter_template_item_t* ptr)
{
    ec_formatter_template_item_t* template_item = ptr;
    switch (template_item->type)
    {
    case EC_FORMATTER_TEMPLATE_ITEM_TYPE_TEXT:
        ec_delete(template_item->u.text);
        break;

    case EC_FORMATTER_TEMPLATE_ITEM_TYPE_ANCHOR:
        break;
    }
    ec_free(template_item);
}

ec_formatter_template_item_t*
ec_formatter_template_item_new_text(ec_string* text)
{
    ec_formatter_template_item_t* new_template_item;
    if ((new_template_item = (ec_formatter_template_item_t*)ec_malloc(
             sizeof(ec_formatter_template_item_t))) == NULL)
    {
        return NULL;
    }
    new_template_item->type = EC_FORMATTER_TEMPLATE_ITEM_TYPE_TEXT;
    if ((new_template_item->u.text = ec_string_new()) == NULL)
    {
        return NULL;
    }
    ec_string_assign(new_template_item->u.text, text);
    return new_template_item;
}

ec_formatter_template_item_t* ec_formatter_template_item_new_anchor(
    ec_formatter_plugin_t* plugin,
    ec_formatter_plugin_make_anchor_args* anchor_args)
{
    ec_formatter_template_item_t* new_template_item;
    if ((new_template_item = (ec_formatter_template_item_t*)ec_malloc(
             sizeof(ec_formatter_template_item_t))) == NULL)
    {
        return NULL;
    }
    new_template_item->type = EC_FORMATTER_TEMPLATE_ITEM_TYPE_ANCHOR;
    new_template_item->u.anchor.plugin = plugin;
    new_template_item->u.anchor.anchor_data = NULL;
    new_template_item->u.anchor.anchor_data_dtor = NULL;
    if (plugin->cb_make_anchor != NULL)
    {
        ec_formatter_plugin_args_make_anchor_t args_make_anchor;
        args_make_anchor.anchor_args = anchor_args;
        args_make_anchor.data = NULL;
        args_make_anchor.data_dtor = NULL;
        if (plugin->cb_make_anchor(&args_make_anchor) != 0)
        {
            ec_free(new_template_item);
            return NULL;
        }
        new_template_item->u.anchor.anchor_data = args_make_anchor.data;
        new_template_item->u.anchor.anchor_data_dtor =
            args_make_anchor.data_dtor;
    }
    return new_template_item;
}

static void ec_formatter_template_ctor(void* ptr)
{
    ec_formatter_template_t* tplt = ptr;
    tplt->items = ect_list_new(ec_formatter_template_item_list);
}

static void ec_formatter_template_dtor(void* ptr)
{
    ec_formatter_template_t* tplt = ptr;
    ec_delete(tplt->items);
}

ec_formatter_template_t* ec_formatter_template_new(void)
{
    return ec_newcd(ec_formatter_template_t, ec_formatter_template_ctor,
                    ec_formatter_template_dtor);
}

void ec_formatter_template_push_back(ec_formatter_template_t* tplt,
                                     ec_formatter_template_item_t* item)
{
    ec_formatter_template_item_list_push_back(tplt->items, item);
}

static void ec_formatter_ctor(void* ptr)
{
    ec_formatter_t* formatter = ptr;

    formatter->plugins = ect_list_new(ec_formatter_plugin_list);
}

static void ec_formatter_dtor(void* ptr)
{
    ec_formatter_t* formatter = ptr;
    if (ptr != NULL)
    {
        ec_delete(formatter->plugins);
    }
}

ec_formatter_t* ec_formatter_new(void)
{
    ec_formatter_t* new_formatter = NULL;
    if ((new_formatter = ec_newcd(ec_formatter_t, ec_formatter_ctor,
                                  ec_formatter_dtor)) == NULL)
    {
        return NULL;
    }
    return new_formatter;
}

ec_formatter_plugin_t*
ec_formatter_find_plugin_by_name(ec_formatter_t* formatter, ec_string* name)
{
    ec_string* plugin_name;
    ec_formatter_plugin_t* plugin;
    ect_iterator(ec_formatter_plugin_list) iter_plugin;
    ect_for(ec_formatter_plugin_list, formatter->plugins, iter_plugin)
    {
        plugin = ect_deref(ec_formatter_plugin_t*, iter_plugin);
        plugin_name = plugin->name;
        if (ec_string_eq(name, plugin_name))
        {
            return plugin;
        }
    }
    return NULL;
}
