/* XenonJS : Runtime Time System : Builtin Functions : JSON
 * Copyright(c) 2017 y2c2 */

#include "xjr_mbuf.h"
#include "xjr_libc.h"
#include "xjr_mp.h"
#include "xjr_bltin_json.h"

/* JSON.stringify(value[, replacer[, space]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON/stringify */
static void xjr_bltin_json_stringify(xjr_native_fn_args *args)
{
    xjr_mbuf_t mbuf;
    xjr_val value = XJR_VAL_MAKE_UNDEFINED();
    xjr_val replacer = XJR_VAL_MAKE_UNDEFINED();
    xjr_val space = XJR_VAL_MAKE_UNDEFINED();

    if (args->argc >= 1) { value = args->argv[0]; }
    if (args->argc >= 2) { replacer = args->argv[1]; }
    if (args->argc >= 3) { space = args->argv[2]; }

    (void)replacer;
    (void)space;

    if (xjr_mbuf_init(&mbuf, args->mp, \
                xjr_mbuf_malloc_by_mp, \
                xjr_mbuf_free_by_mp, \
                xjr_memcpy) != 0)
    { args->ret = XJR_VAL_MAKE_UNDEFINED(); return; }

    xjr_vm_inspect(args->mp, \
           &mbuf, xjr_mbuf_write_cb, \
           value, \
           XJR_VM_INSPECT_MODE_JSON_STRINGIFY);

    args->ret = xjr_val_make_string_from_heap(args->mp, \
            xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));

    xjr_mbuf_uninit(&mbuf);
}

#ifndef ISDIGIT
#define ISDIGIT(ch) (('0' <= (ch)) && ((ch) <= '9'))
#endif

#ifndef ISHEXDIGIT
#define  ISHEXDIGIT(ch) \
    (ISDIGIT(ch) || (('a' <= (ch)) && ((ch) <= 'f')) || (('A' <= (ch)) && ((ch) <= 'F')))
#endif

#ifndef ISHEXDIGIT_S4
#define ISHEXDIGIT_S4(p) \
    ((ISHEXDIGIT(*(p + 0))) && (ISHEXDIGIT(*(p + 1))) && (ISHEXDIGIT(*(p + 2))) && (ISHEXDIGIT(*(p + 3))))
#endif

#ifndef ISALPHA
#define ISALPHA(ch) ((('a' <= (ch)) && ((ch) <= 'z')) || (('A' <= (ch)) && ((ch) <= 'Z')))
#endif

#ifndef ISID
#define ISID(ch) (ISALPHA(ch) || ISDIGIT(ch))
#endif

#ifndef ISWS
#define ISWS(ch) (((ch) == '\t') || ((ch) == '\r') || ((ch) == '\n') || ((ch) == ' '))
#endif

#define IS_HYPER_ID(ch) (((ch)&128)!=0?1:0)

static xjr_size_t id_hyper_length(char ch)
{
    xjr_size_t bytes_number;
    unsigned char uch = (unsigned char)ch;
    /* 0xxxxxxx */
    if ((uch & 0x80) == 0) bytes_number = 1;
    /* 110xxxxx, 10xxxxxx */
    else if ((uch & 0xe0) == 0xc0) bytes_number = 2;
    /* 1110xxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xf0) == 0xe0) bytes_number = 3;
    /* 11110xxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xf8) == 0xf0) bytes_number = 4;
    /* 111110xx, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xfc) == 0xf8) bytes_number = 5;
    /* 1111110x, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx, 10xxxxxx */
    else if ((uch & 0xfe) == 0xfc) bytes_number = 6;
    else bytes_number = 0;
    return bytes_number;
}

static xjr_val json_parse_in(xjr_native_fn_args *args, \
        char **p_io, xjr_size_t *len_io);

static void json_skip_whitespace(char **p_io, xjr_size_t *len_io)
{
    while ((*len_io != 0) && (ISWS(**p_io)))
    {
        (*p_io)++;
        (*len_io)--;
    }
}

static xjr_val json_parse_in_number(char **p_io, xjr_size_t *len_io)
{
    char *p = *p_io;
    xjr_size_t len = *len_io;
    int value = 0;
    double value_double = 0.0;
    xjr_bool negative = xjr_false;
    xjr_val result = XJR_VAL_MAKE_UNDEFINED();
    double base;

    /* Negative */
    if (*p == '-') { negative = xjr_true; p++; len--;}

    /* Integer Part */
    if (len == 0) { return XJR_VAL_MAKE_UNDEFINED(); }
    if (*p == '0')
    {
        value = 0;
        p++; len--;
    }
    else if (('1' <= *p) && (*p <= '9'))
    {
        while ((len > 0) && (ISDIGIT(*p)))
        {
            value = value * 10 + ((*p) - '0');
            p++; len--;
        }
    }
    else
    {
        return XJR_VAL_MAKE_UNDEFINED();
    }
    /* Fill double part */
    value_double = (double)value;

    /* Fractal Part */
    if ((len > 0) && (*p == '.'))
    {
        /* Skip '.' */
        p++; len--;

        base = 0.1;
        while ((len > 0) && (ISDIGIT(*p)))
        {
            value_double += base * ((*p) - '0');
            base /= 10;
            p++; len--;
        }
    }

    /* Negative */
    if (negative == xjr_true)
    {
        value = -value;
        value_double = -value_double;
    }

    (void)value_double;

    result = XJR_VAL_MAKE_INTEGER((int)value);

    *p_io = p;
    *len_io = len;

    return result;
}

static int json_parse_in_string_hexchar_to_num(char ch)
{
    int result;

    if (('0' <= ch) && (ch <= '9')) { result = (int)ch - (int)'0'; }
    else if (('a' <= ch) && (ch <= 'f')) { result = (int)ch - (int)'a'; }
    else if (('A' <= ch) && (ch <= 'F')) { result = (int)ch - (int)'A'; }
    else result = -1;

    return result;
}

static int json_parse_in_string_hexchar_to_num_s4(char *p)
{
    unsigned int result = 0;
    int t;

    if ((t = json_parse_in_string_hexchar_to_num(*p)) == -1) { return -1; }
    result = (unsigned int)t;
    if ((t = json_parse_in_string_hexchar_to_num(*(p + 1))) == -1) { return -1; }
    result = (result << 4) | (unsigned int)t;
    if ((t = json_parse_in_string_hexchar_to_num(*(p + 2))) == -1) { return -1; }
    result = (result << 4) | (unsigned int)t;
    if ((t = json_parse_in_string_hexchar_to_num(*(p + 3))) == -1) { return -1; }
    result = (result << 4) | (unsigned int)t;

    return (int)result;
}

typedef enum
{
    UJSON_PARSE_IN_STRING_STATE_INIT = 0,
    UJSON_PARSE_IN_STRING_STATE_ESCAPE,
} json_parse_in_string_state_t;

static xjr_val json_parse_in_string(xjr_native_fn_args *args, \
        char **p_io, xjr_size_t *len_io)
{
    char *p = *p_io;
    xjr_size_t len = *len_io;
    xjr_val result = XJR_VAL_MAKE_UNDEFINED();
    xjr_mbuf_t buffer;
    json_parse_in_string_state_t state = UJSON_PARSE_IN_STRING_STATE_INIT;
    xjr_size_t bytes_number;
    xjr_size_t ch_len = 0;
    int value_u;
    char writebuf[7];

    /* Initialize buffer */
    if (xjr_mbuf_init(&buffer, args->mp, \
                xjr_mbuf_malloc_by_mp, \
                xjr_mbuf_free_by_mp, \
                xjr_memcpy) != 0)
    { return XJR_VAL_MAKE_UNDEFINED(); }

    /* Skip '"' or '''*/
    p++; len--;

    while (len > 0)
    {
        switch (state)
        {
            case UJSON_PARSE_IN_STRING_STATE_INIT:
                if (*p == '\"')
                {
                    goto finish;
                }
                else if (*p == '\\')
                {
                    if ((len < 2)) { goto fail; }
                    state = UJSON_PARSE_IN_STRING_STATE_ESCAPE;
                    p++; len--;
                }
                else if (IS_HYPER_ID(*p))
                {
                    bytes_number = id_hyper_length(*p);
                    if (len < bytes_number) { goto fail; }
                    if (xjr_mbuf_append(&buffer, p, bytes_number) != 0)
                    { goto fail; }
                    ch_len++;
                    p += bytes_number; len -= bytes_number;
                }
                else
                {
                    if (xjr_mbuf_append(&buffer, p, 1) != 0)
                    { goto fail; }
                    ch_len++;
                    p++; len--;
                }
                break;

            case UJSON_PARSE_IN_STRING_STATE_ESCAPE:
                if (*p == '"')
                { if (xjr_mbuf_append(&buffer, "\"", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == '\\')
                { if (xjr_mbuf_append(&buffer, "\\", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == '/')
                { if (xjr_mbuf_append(&buffer, "/", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == 'b')
                { if (xjr_mbuf_append(&buffer, "\b", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == 'f')
                { if (xjr_mbuf_append(&buffer, "\f", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == 'n')
                { if (xjr_mbuf_append(&buffer, "\n", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == 'r')
                { if (xjr_mbuf_append(&buffer, "\r", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if (*p == 't')
                { if (xjr_mbuf_append(&buffer, "\t", 1) != 0) { goto fail; } ch_len++; p++; len--; }
                else if ((*p == 'u') && (len >= 5) && ISHEXDIGIT_S4(p + 1))
                {
                    value_u = json_parse_in_string_hexchar_to_num_s4(p + 1);
                    if (value_u == -1) goto fail;
                    if ((0 <= value_u) && (value_u <= 0x7F))
                    {
                        writebuf[0] = ((char)value_u);
                        if (xjr_mbuf_append(&buffer, writebuf, 1) != 0) { goto fail; }
                        ch_len++;
                        p += 5; len -= 5; 
                    }
                    else if ((0x80 <= value_u) && (value_u <= 0x7FF))
                    {
                        writebuf[0] = (char)(0xc0 | (((unsigned int)value_u) >> 6));
                        writebuf[1] = (char)(0x80 | (((unsigned int)value_u) & 0x3f));
                        if (xjr_mbuf_append(&buffer, writebuf, 2) != 0) { goto fail; }
                        ch_len++;
                        p += 5; len -= 5; 
                    }
                    else if ((0x800 <= value_u) && (value_u <= 0x7FFF))
                    {
                        writebuf[0] = (char)(0xe0 | (((unsigned int)value_u) >> 12));
                        writebuf[1] = (char)(0x80 | (((unsigned int)value_u >> 6) & 0x3f));
                        writebuf[2] = (char)(0x80 | (((unsigned int)value_u) & 0x3f));
                        if (xjr_mbuf_append(&buffer, writebuf, 3) != 0) { goto fail; }
                        ch_len++;
                        p += 5; len -= 5; 
                    }
                    else { goto fail; }
                }
                else
                { goto fail; }

                /* Reset state */
                state = UJSON_PARSE_IN_STRING_STATE_INIT;
                break;
        }
    }

finish:

    if (len == 0) { goto fail; }

    /* Skip '"' */
    p++; len--;
    
    result = xjr_val_make_string_from_heap(args->mp, \
            xjr_mbuf_body(&buffer), xjr_mbuf_size(&buffer));

    *p_io = p;
    *len_io = len;

fail:
    /* Uninitialize buffer */
    xjr_mbuf_uninit(&buffer);

    return result;
}

typedef enum
{
    UJSON_PARSE_IN_ARRAY_STATE_INIT = 0,
    UJSON_PARSE_IN_ARRAY_STATE_VALUE,
    UJSON_PARSE_IN_ARRAY_STATE_COMMA,
    UJSON_PARSE_IN_ARRAY_STATE_FINISH,
} json_parse_in_array_state_t;

static xjr_val json_parse_in_array(xjr_native_fn_args *args, \
        char **p_io, xjr_size_t *len_io)
{
    char *p = *p_io;
    xjr_size_t len = *len_io;
    xjr_val new_array = XJR_VAL_MAKE_UNDEFINED();
    xjr_val new_element = XJR_VAL_MAKE_UNDEFINED();
    /* json_array_item_t *new_element_item = NULL; */
    json_parse_in_array_state_t state = UJSON_PARSE_IN_ARRAY_STATE_INIT;

    new_array = xjr_val_make_array(args->mp);
    if (!XJR_VAL_IS_ARRAY(new_array)) { return XJR_VAL_MAKE_UNDEFINED(); }

    /* Skip '[' */
    p++; len--;

    while ((len != 0) && (state != UJSON_PARSE_IN_ARRAY_STATE_FINISH))
    {
        switch (state)
        {
            case UJSON_PARSE_IN_ARRAY_STATE_INIT:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                if (*p == ']')
                {
                    state = UJSON_PARSE_IN_ARRAY_STATE_FINISH;
                }
                else
                {
                    new_element = json_parse_in(args, &p, &len);
                    if (xjr_val_as_array_push(args->mp, new_array, new_element) != 0) { goto fail; }
                    state = UJSON_PARSE_IN_ARRAY_STATE_VALUE;
                }
                break;

            case UJSON_PARSE_IN_ARRAY_STATE_VALUE:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                if (*p == ']')
                {
                    state = UJSON_PARSE_IN_ARRAY_STATE_FINISH;
                }
                else if (*p == ',')
                {
                    p++; len--;
                    state = UJSON_PARSE_IN_ARRAY_STATE_COMMA;
                }
                else
                {
                    goto fail;
                }
                break;

            case UJSON_PARSE_IN_ARRAY_STATE_COMMA:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                new_element = json_parse_in(args, &p, &len);
                if (xjr_val_as_array_push(args->mp, new_array, new_element) != 0) { goto fail; }

                state = UJSON_PARSE_IN_ARRAY_STATE_VALUE;
                break;

            case UJSON_PARSE_IN_ARRAY_STATE_FINISH:
                break;
        }
    }

    /* Skip ']' */
    p++; len--;

    *p_io = p;
    *len_io = len;
    goto done;
fail:
done:
    return new_array;
}

typedef enum
{
    UJSON_PARSE_IN_OBJECT_STATE_INIT = 0,
    UJSON_PARSE_IN_OBJECT_STATE_KEY,
    UJSON_PARSE_IN_OBJECT_STATE_COLON,
    UJSON_PARSE_IN_OBJECT_STATE_VALUE,
    UJSON_PARSE_IN_OBJECT_STATE_COMMA,
    UJSON_PARSE_IN_OBJECT_STATE_FINISH,
} json_parse_in_object_state_t;

static xjr_val json_parse_in_object(xjr_native_fn_args *args, \
        char **p_io, xjr_size_t *len_io)
{
    char *p = *p_io;
    xjr_size_t len = *len_io;
    xjr_val new_array = XJR_VAL_MAKE_UNDEFINED();
    xjr_val new_key = XJR_VAL_MAKE_UNDEFINED();
    xjr_val new_value = XJR_VAL_MAKE_UNDEFINED();
    json_parse_in_object_state_t state = UJSON_PARSE_IN_OBJECT_STATE_INIT;

    new_array = xjr_val_make_object(args->mp);
    if (!XJR_VAL_IS_OBJECT(new_array)) { return XJR_VAL_MAKE_UNDEFINED(); }

    /* Skip '{' */
    p++; len--;

    while ((len != 0) && (state != UJSON_PARSE_IN_OBJECT_STATE_FINISH))
    {
        switch (state)
        {
            case UJSON_PARSE_IN_OBJECT_STATE_INIT:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                if (*p == '}')
                {
                    state = UJSON_PARSE_IN_OBJECT_STATE_FINISH;
                }
                else
                {
                    new_key = json_parse_in(args, &p, &len);
                    if (!XJR_VAL_IS_STRING(new_key)) { goto fail; }

                    state = UJSON_PARSE_IN_OBJECT_STATE_KEY;
                }
                break;

            case UJSON_PARSE_IN_OBJECT_STATE_KEY:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                if (*p != ':') { goto fail; }
                p++; len--;

                state = UJSON_PARSE_IN_OBJECT_STATE_COLON;

                break;

            case UJSON_PARSE_IN_OBJECT_STATE_COLON:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                new_value = json_parse_in(args, &p, &len);

                {
                    xjr_val_properties *props = xjr_val_as_object_property_get(args->mp, new_array);
                    if (xjr_val_properties_set_by_name(args->mp, props, \
                                XJR_VAL_PROPERTY_TYPE_NORMAL, \
                                xjr_val_as_string_body(args->mp, new_key), \
                                xjr_val_as_string_length(args->mp, new_key), \
                                new_value) != 0)
                    { goto fail; }
                }

                state = UJSON_PARSE_IN_OBJECT_STATE_VALUE;

                break;

            case UJSON_PARSE_IN_OBJECT_STATE_VALUE:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                if (*p == '}')
                {
                    state = UJSON_PARSE_IN_OBJECT_STATE_FINISH;
                }
                else if (*p == ',')
                {
                    state = UJSON_PARSE_IN_OBJECT_STATE_COMMA;
                    p++; len--;
                }
                else
                {
                    goto fail;
                }

                break;

            case UJSON_PARSE_IN_OBJECT_STATE_COMMA:
                json_skip_whitespace(&p, &len);
                if (len == 0) { goto fail; }

                new_key = json_parse_in(args, &p, &len);

                state = UJSON_PARSE_IN_OBJECT_STATE_KEY;

                break;

            case UJSON_PARSE_IN_OBJECT_STATE_FINISH:
                break;
        }
    }

    /* Skip '}' */
    p++; len--;

    *p_io = p;
    *len_io = len;
    goto done;
fail:
done:
    return new_array;
}

#define MATCH_IDENTIFIER(p, len, expected_s, expected_len) \
    (((len == expected_len) || ((len > expected_len) && (!ISID(*(p + expected_len))))) && \
     (xjr_strncmp(p, expected_s, expected_len) == 0))

static xjr_val json_parse_in(xjr_native_fn_args *args, \
        char **p_io, xjr_size_t *len_io)
{
    char *p = *p_io;
    xjr_size_t len = *len_io;
    xjr_val result = XJR_VAL_MAKE_UNDEFINED();

    /* Skip whitespace */
    json_skip_whitespace(&p, &len);

    if (ISDIGIT(*p))
    {
        result = json_parse_in_number(&p, &len);
    }
    else if (*p == '-')
    {
        result = json_parse_in_number(&p, &len);
    }
    else if (*p == '\"')
    {
        result = json_parse_in_string(args, &p, &len);
    }
    else if (MATCH_IDENTIFIER(p, len, "null", 4))
    {
        result = XJR_VAL_MAKE_NULL();
        p += 4; len -= 4;
    }
    else if (MATCH_IDENTIFIER(p, len, "true", 4))
    {
        result = XJR_VAL_MAKE_BOOLEAN_TRUE();
        p += 4; len -= 4;
    }
    else if (MATCH_IDENTIFIER(p, len, "false", 5))
    {
        result = XJR_VAL_MAKE_BOOLEAN_FALSE();
        p += 5; len -= 5;
    }
    else if (*p == '[')
    {
        result = json_parse_in_array(args, &p, &len);
    }
    else if (*p == '{')
    {
        result = json_parse_in_object(args, &p, &len);
    }
    else
    {
        result = XJR_VAL_MAKE_UNDEFINED();
    }

    *p_io = p;
    *len_io = len;
    return result;
}

static xjr_val json_parse(xjr_native_fn_args *args, \
        char *s, xjr_size_t len)
{
    return json_parse_in(args, &s, &len);
}

/* JSON.parse(text[, reviver]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON/parse */
static void xjr_bltin_json_parse(xjr_native_fn_args *args)
{
    xjr_val text = XJR_VAL_MAKE_UNDEFINED();
    xjr_val reviver = XJR_VAL_MAKE_UNDEFINED();

    if (args->argc >= 1) { text = args->argv[0]; }
    if (args->argc >= 2) { reviver = args->argv[1]; }

    (void)reviver;

    if (!XJR_VAL_IS_STRING(text))
    {
        args->ret = XJR_VAL_MAKE_UNDEFINED();
        return;
    }

    args->ret = json_parse(args, \
            xjr_val_as_string_body(args->mp, text), \
            xjr_val_as_string_length(args->mp, text));
}

int xjr_bltin_json_init(xjr_vm *vm, xjr_urid_t env)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;
    int ret = 0;

    /* JSON */
    {
        xjr_val obj_json;

        /* 'JSON' */
        XJR_BLTIN_CREATE_OBJ(obj_json, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());

        XJR_BLTIN_METHOD(obj_json, "stringify", 9, xjr_bltin_json_stringify);
        XJR_BLTIN_METHOD(obj_json, "parse", 5, xjr_bltin_json_parse);

        /* global.JSON */
        V_OOM(xjr_vm_env_set_var(mp, env, "JSON", 4, obj_json) == 0);
    }

fail:
    return ret;
}

