/* Enhanced C : Formatter
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter.h"
#include "ec_algorithm.h"
#include "ec_dt.h"
#include "ec_formatter_dt.h"
#include "ec_formatter_std.h"
#include "ec_formatter_template.h"
#include "ec_libc.h"
#include "ec_string.h"
#include <stdarg.h>

ec_formatter_t* ec_formatter_std_new(void)
{
    ec_formatter_t* new_formatter = ec_formatter_new();
    if (new_formatter == NULL)
        return NULL;

    if (ec_formatter_install_std_plugins(new_formatter) != 0)
    {
        goto fail;
    }
    goto done;
fail:
    ec_delete(new_formatter);
    new_formatter = NULL;
done:
    return new_formatter;
}

int ec_formatter_install_plugin(
    ec_formatter_t* formatter, const char* name,
    ec_formatter_plugin_cb_init_t cb_init,
    ec_formatter_plugin_cb_uninit_t cb_uninit,
    ec_formatter_plugin_cb_make_anchor_t cb_make_anchor,
    ec_formatter_plugin_cb_apply_anchor_t cb_apply_anchor)
{
    ec_formatter_plugin_t* new_plugin;

    if ((new_plugin =
             ec_formatter_plugin_new(name, cb_init, cb_uninit, cb_make_anchor,
                                     cb_apply_anchor)) == NULL)
    {
        return -1;
    }
    ect_list_push_back(ec_formatter_plugin_list, formatter->plugins,
                       new_plugin);
    return 0;
}

int ec_formatter_formatv(ec_formatter_t* formatter, ec_string** out,
                         const char* fmt, va_list ap)
{
    int ret = 0;
    ec_formatter_template_t* tplt = NULL;
    ect_iterator(ec_formatter_template_item_list) iter_tplt_item;
    ec_formatter_template_item_t* tplt_item;
    ec_string* result = NULL;
    /* Build template */
    if ((ret = ec_formatter_template_build_from_fmt_cstr(&tplt, formatter,
                                                         fmt)) != 0)
    {
        goto fail;
    }
    /* Create string to contain the result */
    if ((result = ec_string_new()) == NULL)
    {
        ret = -1;
        goto fail;
    }
    /* Iterate template items */
    ect_for(ec_formatter_template_item_list, tplt->items, iter_tplt_item)
    {
        tplt_item = ect_deref(ec_formatter_template_item_t*, iter_tplt_item);
        switch (tplt_item->type)
        {
        case EC_FORMATTER_TEMPLATE_ITEM_TYPE_TEXT:
            ec_string_append(result, tplt_item->u.text);
            break;
        case EC_FORMATTER_TEMPLATE_ITEM_TYPE_ANCHOR:
        {
            ec_formatter_plugin_args_apply_anchor_t args;
            ec_string* applied_result;
            /* Create a blank string to contain applied anchor */
            if ((applied_result = ec_string_new()) == NULL)
            {
                ret = -1;
                goto fail;
            }
            /* Apply */
            args.data = tplt_item->u.anchor.anchor_data;
            args.result = applied_result;
            va_copy(args.ap, ap);
            if (tplt_item->u.anchor.plugin->cb_apply_anchor(&args) == 0)
            {
                ec_string_append(result, applied_result);
            }
            va_copy(ap, args.ap);

            ec_delete(applied_result);
        }
        break;
        }
    }
    *out = result;
    goto done;
fail:
    ec_delete(result);
done:
    ec_delete(tplt);
    return ret;
}

int ec_formatter_format(ec_formatter_t* formatter, ec_string** out,
                        const char* fmt, ...)
{
    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = ec_formatter_formatv(formatter, out, fmt, ap);
    va_end(ap);
    return ret;
}
