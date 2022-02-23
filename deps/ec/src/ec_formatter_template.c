/* Enhanced C : Formatter : Template
 * Copyright(c) 2017-2020 y2c2 */

#include "ec_formatter_template.h"
#include "ec_algorithm.h"
#include "ec_dt.h"
#include "ec_encoding.h"
#include "ec_formatter_dt.h"
#include "ec_libc.h"
#include "ec_string.h"

typedef enum
{
    ec_formatter_build_template_parse_state_init,
    ec_formatter_build_template_parse_state_rcurlybracket, /* } */
    ec_formatter_build_template_parse_state_text,
    ec_formatter_build_template_parse_state_anchor_name,   /* { */
    ec_formatter_build_template_parse_state_anchor_name1,  /* {name */
    ec_formatter_build_template_parse_state_anchor_key,    /* '{name( */
    ec_formatter_build_template_parse_state_anchor_key1,   /* '{name(key */
    ec_formatter_build_template_parse_state_anchor_value,  /* '{name(key:' */
    ec_formatter_build_template_parse_state_anchor_value1, /* '{name(key:value'
                                                            */
    ec_formatter_build_template_parse_state_anchor_close,  /* '{name(key:value)'
                                                            */
} ec_formatter_build_template_parse_state_t;

#define ec_char_t_deref(_it) (*((ec_char_t*)ec_deref(_it)))

#define FAIL()                                                                 \
    do                                                                         \
    {                                                                          \
        ret = -1;                                                              \
        goto fail;                                                             \
    } while (0)

#define FIN()                                                                  \
    do                                                                         \
    {                                                                          \
        goto finish;                                                           \
    } while (0)

#define JMP(_x)                                                                \
    do                                                                         \
    {                                                                          \
        state = _x;                                                            \
    } while (0)

#define V_NE_NULL(_x)                                                          \
    if ((_x) == NULL)                                                          \
    FAIL()

#define V_EQ_ZERO(_x)                                                          \
    if ((_x) != 0)                                                             \
    FAIL()

static int insert_anchor(ec_formatter_template_t* tplt,
                         ec_formatter_t* formatter, ec_string* name,
                         ec_formatter_plugin_make_anchor_args* anchor_args)
{
    int ret = 0;
    ec_formatter_template_item_t* new_item;
    ec_formatter_plugin_t* plugin;
    /* Search the plugin by anchor name */
    if ((plugin = ec_formatter_find_plugin_by_name(formatter, name)) == NULL)
    {
        FAIL();
    }
    /* Create a new anchor item */
    V_NE_NULL(new_item =
                  ec_formatter_template_item_new_anchor(plugin, anchor_args));
    /* Append */
    ec_formatter_template_push_back(tplt, new_item);
fail:
    return ret;
}

int ec_formatter_template_build_from_fmt(ec_formatter_template_t** tplt_out,
                                         ec_formatter_t* formatter,
                                         ec_string* fmt)
{
    int ret = 0;
    ec_formatter_template_t* new_tplt = NULL;
    ec_formatter_plugin_make_anchor_args* new_anchor_args = NULL;
    ec_formatter_build_template_parse_state_t state;
    ect_iterator(ec_string) fmt_it, fmt_it_end;
    ec_char_t ch;
    /* Text */
    ec_string* text = NULL;
    /* Anchor */
    ec_string *name = NULL, *key = NULL, *value = NULL;
    /* New template */
    V_NE_NULL(new_tplt = ec_formatter_template_new());
    /* Parse */
    state = ec_formatter_build_template_parse_state_init;
    ec_string_iterator_init_begin(&fmt_it, fmt);
    ec_string_iterator_init_end(&fmt_it_end, fmt);
    for (;;)
    {
        switch (state)
        {
        case ec_formatter_build_template_parse_state_init:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FIN();
            }
            else if (ec_char_t_deref(fmt_it) == '}')
            {
                JMP(ec_formatter_build_template_parse_state_rcurlybracket);
                ec_string_iterator_next(&fmt_it);
            }
            else if (ec_char_t_deref(fmt_it) == '{')
            {
                JMP(ec_formatter_build_template_parse_state_anchor_name);
                ec_string_iterator_next(&fmt_it);
            }
            else
            {
                V_NE_NULL(text = ec_string_new());
                ec_string_push_back(text, ec_char_t_deref(fmt_it));
                ec_string_iterator_next(&fmt_it);
                JMP(ec_formatter_build_template_parse_state_text);
            }
            break;

        case ec_formatter_build_template_parse_state_rcurlybracket:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else if (ec_char_t_deref(fmt_it) == '}')
            {
                ec_formatter_template_item_t* new_text;
                V_NE_NULL(text = ec_string_new());
                ec_string_push_back(text, (ec_char_t)('}'));
                V_NE_NULL(new_text = ec_formatter_template_item_new_text(text));
                ec_delete(text);
                text = NULL;
                ec_formatter_template_push_back(new_tplt, new_text);
                ec_string_iterator_next(&fmt_it);
                JMP(ec_formatter_build_template_parse_state_init);
            }
            else
            {
                FAIL();
            }
            break;

        case ec_formatter_build_template_parse_state_text:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                ec_formatter_template_item_t* new_text;
                V_NE_NULL(new_text = ec_formatter_template_item_new_text(text));
                ec_delete(text);
                text = NULL;
                ec_formatter_template_push_back(new_tplt, new_text);
                FIN();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    if (ec_string_length(text) != 0)
                    {
                        ec_formatter_template_item_t* new_text;
                        V_NE_NULL(
                            new_text =
                                ec_formatter_template_item_new_text(text));
                        ec_delete(text);
                        text = NULL;
                        ec_formatter_template_push_back(new_tplt, new_text);
                    }
                    JMP(ec_formatter_build_template_parse_state_anchor_name);
                    ec_string_iterator_next(&fmt_it);
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else
                {
                    ec_string_push_back(text, ec_char_t_deref(fmt_it));
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_name:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    ec_formatter_template_item_t* new_text;
                    V_NE_NULL(text = ec_string_new());
                    ec_string_push_back(text, ch);
                    V_NE_NULL(new_text =
                                  ec_formatter_template_item_new_text(text));
                    ec_delete(text);
                    text = NULL;
                    ec_formatter_template_push_back(new_tplt, new_text);
                    ec_string_iterator_next(&fmt_it);
                    JMP(ec_formatter_build_template_parse_state_init);
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else if (ch == '(')
                {
                    FAIL();
                }
                else if (ch == ')')
                {
                    FAIL();
                }
                else
                {
                    V_NE_NULL(name = ec_string_new());
                    V_NE_NULL(new_anchor_args = ect_map_new(
                                  ec_formatter_plugin_make_anchor_args));
                    ec_string_push_back(name, ch);
                    ec_string_iterator_next(&fmt_it);
                    JMP(ec_formatter_build_template_parse_state_anchor_name1);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_name1:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    FAIL();
                }
                else if (ch == '}')
                {
                    V_EQ_ZERO(insert_anchor(new_tplt, formatter, name,
                                            new_anchor_args));
                    ec_delete(name);
                    name = NULL;
                    ec_delete(new_anchor_args);
                    new_anchor_args = NULL;
                    JMP(ec_formatter_build_template_parse_state_init);
                    ec_string_iterator_next(&fmt_it);
                }
                else if (ch == '(')
                {
                    JMP(ec_formatter_build_template_parse_state_anchor_key);
                    ec_string_iterator_next(&fmt_it);
                }
                else
                {
                    ec_string_push_back(name, ch);
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_key:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    FAIL();
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else if (ch == '(')
                {
                    FAIL();
                }
                else if (ch == ')')
                {
                    FAIL();
                }
                else if (ch == ',')
                {
                    FAIL();
                }
                else if (ch == ':')
                {
                    FAIL();
                }
                else
                {
                    V_NE_NULL(key = ec_string_new());
                    JMP(ec_formatter_build_template_parse_state_anchor_key1);
                    ec_string_push_back(key, ch);
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_key1:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    FAIL();
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else if (ch == '(')
                {
                    FAIL();
                }
                else if (ch == ')')
                {
                    FAIL();
                }
                else if (ch == ',')
                {
                    FAIL();
                }
                else if (ch == ':')
                {
                    JMP(ec_formatter_build_template_parse_state_anchor_value);
                    ec_string_iterator_next(&fmt_it);
                }
                else
                {
                    ec_string_push_back(key, ch);
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_value:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    FAIL();
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else if (ch == '(')
                {
                    FAIL();
                }
                else if (ch == ')')
                {
                    FAIL();
                }
                else if (ch == ':')
                {
                    FAIL();
                }
                else if (ch == ',')
                {
                    FAIL();
                }
                else
                {
                    JMP(ec_formatter_build_template_parse_state_anchor_value1);
                    V_NE_NULL(value = ec_string_new());
                    ec_string_push_back(value, ch);
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_value1:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '{')
                {
                    FAIL();
                }
                else if (ch == '}')
                {
                    FAIL();
                }
                else if (ch == '(')
                {
                    FAIL();
                }
                else if (ch == ':')
                {
                    FAIL();
                }
                else if (ch == ',')
                {
                    ect_map_insert(ec_formatter_plugin_make_anchor_args,
                                   new_anchor_args, key, value);
                    ec_delete(key);
                    ec_delete(value);
                    key = NULL;
                    value = NULL;
                    JMP(ec_formatter_build_template_parse_state_anchor_name);
                    ec_string_iterator_next(&fmt_it);
                }
                else if (ch == ')')
                {
                    ect_map_insert(ec_formatter_plugin_make_anchor_args,
                                   new_anchor_args, key, value);
                    ec_delete(key);
                    ec_delete(value);
                    key = NULL;
                    value = NULL;
                    JMP(ec_formatter_build_template_parse_state_anchor_close);
                    ec_string_iterator_next(&fmt_it);
                }
                else
                {
                    ec_string_push_back(key, ch);
                    ec_string_iterator_next(&fmt_it);
                }
            }
            break;

        case ec_formatter_build_template_parse_state_anchor_close:
            if (ec_string_iterator_eq(&fmt_it, &fmt_it_end))
            {
                FAIL();
            }
            else
            {
                ch = ec_char_t_deref(fmt_it);
                if (ch == '}')
                {
                    V_EQ_ZERO(insert_anchor(new_tplt, formatter, name,
                                            new_anchor_args));
                    ec_delete(name);
                    name = NULL;
                    ec_delete(new_anchor_args);
                    new_anchor_args = NULL;
                    JMP(ec_formatter_build_template_parse_state_init);
                    ec_string_iterator_next(&fmt_it);
                }
                else
                {
                    FAIL();
                }
            }
            break;
        }
    }
finish:
    *tplt_out = new_tplt;
    goto done;
fail:
    ec_delete(new_anchor_args);
    ec_delete(new_tplt);
    ec_delete(key);
    ec_delete(value);
    ec_delete(text);
    ec_delete(name);
done:
    return ret;
}

int ec_formatter_template_build_from_fmt_cstr(
    ec_formatter_template_t** tplt_out, ec_formatter_t* formatter,
    const char* fmt)
{
    int ret = 0;
    ec_encoding_t enc;
    ec_string* decoded_fmt = NULL;
    /* Initialize UTF-8 Encoding */
    ec_encoding_utf8_init(&enc);
    /* Decode */
    if (ec_encoding_decode(&enc, &decoded_fmt, (const ec_byte_t*)fmt,
                           ec_strlen(fmt)) != 0)
    {
        return -1;
    }
    /* Parse */
    ret =
        ec_formatter_template_build_from_fmt(tplt_out, formatter, decoded_fmt);
    ec_delete(decoded_fmt);
    return ret;
}

ec_size_t ec_formatter_template_items_count(ec_formatter_template_t* tplt)
{
    return ect_list_size(ec_formatter_template_item_list, tplt->items);
}
