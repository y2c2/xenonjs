/* XenonJS : Runtime Time System : Builtin Functions : Infrastructure
 * Copyright(c) 2017 y2c2 */

#include "xjr_bltin_infra.h"

static void xjr_bltin_object(xjr_native_fn_args *args)
{
    /* return {} */
    args->ret = xjr_val_make_object(args->mp);
    xjr_val_properties_install(args->vm, args->ret);
}

static void xjr_bltin_function_prototype_call(xjr_native_fn_args *args)
{
    xjr_val this_arg;

    /* f.call(this?, args?); */

    if (args->argc == 0)
    {
        this_arg = args->_this;
    }
    else if (args->argc >= 1)
    {
        this_arg = args->argv[0];
    }

    (void)this_arg;
}

static void xjr_bltin_function_nothing(xjr_native_fn_args *args)
{
    /* return undefined; */
    args->ret = XJR_VAL_MAKE_UNDEFINED();
}

static void xjr_bltin_number(xjr_native_fn_args *args)
{
    /* return 0; */
    args->ret = XJR_VAL_MAKE_INTEGER(0);
    xjr_val_properties_install(args->vm, args->ret);
}


#define XJR_BLTIN_NUMBER_PARSEINT_RADIX_AUTO 0

/* Number.parseInt(string,[ radix ]) */
static void xjr_bltin_number_parseint(xjr_native_fn_args *args)
{
    xjr_val str = 0;
    int radix = 10;

    if (args->argc == 0)
    {
        args->ret = XJR_VAL_MAKE_UNDEFINED();
        return;
    }
    if (args->argc >= 1)
    {
        str = args->argv[0];
        if (XJR_VAL_IS_INTEGER(str))
        {
            /* Convert to string type */
            str = xjr_val_make_string_from_int(args->mp, XJR_VAL_AS_INTEGER_UNTAG(str));
        }
        else if (XJR_VAL_IS_FLOAT(str))
        {
            /* Convert to string type */
            str = xjr_val_make_string_from_int(args->mp, (int)(xjr_val_extract_f64(args->mp, str)));
        }
        else if (XJR_VAL_IS_STRING(str))
        {
            /* Do nothing */
        }
        else
        {
            args->ret = XJR_VAL_MAKE_UNDEFINED();
            return;
        }
    }
    if (args->argc >= 2)
    {
        if (XJR_VAL_IS_UNDEFINED(args->argv[1]))
        {
            radix = 0;
        }
        else if (XJR_VAL_IS_INTEGER(args->argv[1]))
        {
            radix = XJR_VAL_AS_INTEGER_UNTAG(args->argv[1]);
            if ((radix < 2) || (radix > 36))
            {
                args->ret = XJR_VAL_MAKE_UNDEFINED();
                return;
            }
        }
        else
        {
            args->ret = XJR_VAL_MAKE_UNDEFINED();
            return;
        }
    }

    {
        char *p = xjr_val_as_string_body(args->mp, str);
        xjr_size_t len = xjr_val_as_string_length(args->mp, str);
        int result = 0;
        xjr_bool negative = xjr_false;
        xjr_bool passed_once = xjr_false;

#define _is_ws(ch) \
        (((ch) == (char)('\t')) || \
         ((ch) == (char)('\v')) || \
         ((ch) == (char)(' ')) || \
         ((ch) == (char)('\f')) || \
         ((ch) == (char)('\r')) || \
         ((ch) == (char)('\n')) || \
         ((ch) == (char)(0xA0)))

        /* Skip whitespaces */
        while ((len != 0) && (_is_ws(*p))) { p++; len--; }

        /* No content remain */
        if (len == 0) { args->ret = XJR_VAL_MAKE_UNDEFINED(); return; }

        /* Negative */
        if (*p == '-') { negative = xjr_true; p++; len--; }

        if (radix == 16)
        {
            if (len >= 2 && (*p == '0') && ((*(p + 1) == 'x') || (*(p + 1) == 'X')))
            {
                p += 2;
                len -= 2;
            }
        }

        /* No content remain */
        if (len == 0) { args->ret = XJR_VAL_MAKE_UNDEFINED(); return; }

        while (len != 0)
        {
            char ch = *p;
            int v;

            if (('0' <= ch) && (ch <= '9')) { v = ch - '0'; }
            else if (('a' <= ch) && (ch <= 'z')) { v = ch - 'a' + 10; }
            else if (('A' <= ch) && (ch <= 'Z')) { v = ch - 'A' + 10; }
            else { break; }

            if (v >= radix) { break; }

            result = result * radix + v;

            passed_once = xjr_true;
            p++;
            len--;
        }

        if (passed_once == xjr_false) { args->ret = XJR_VAL_MAKE_UNDEFINED(); return; }
        if (negative == xjr_true) { result = 0 - result; }

        args->ret = XJR_VAL_MAKE_INTEGER(result);
    }
}

static void xjr_bltin_function(xjr_native_fn_args *args)
{
    /* return function() {}; */
    args->ret = xjr_val_make_native_function(args->mp, args->env, xjr_bltin_function_nothing);
    xjr_val_properties_install(args->vm, args->ret);
}

static void xjr_bltin_array(xjr_native_fn_args *args)
{
    /* return [] */
    args->ret = xjr_val_make_array(args->mp);
    xjr_val_properties_install(args->vm, args->ret);
}

static void xjr_bltin_string(xjr_native_fn_args *args)
{
    /* return "" */
    args->ret = xjr_val_make_string_from_heap(args->mp, "", 0);
    xjr_val_properties_install(args->vm, args->ret);
}

static void xjr_bltin_string_fromcharcode(xjr_native_fn_args *args)
{
    xjr_size_t i;
    xjr_mbuf_t mbuf;

    /* Create a concated string */
    if (xjr_mbuf_init(&mbuf, args->mp, \
                xjr_mbuf_malloc_by_mp, \
                xjr_mbuf_free_by_mp, \
                xjr_memcpy) != 0)
    { args->ret = XJR_VAL_MAKE_UNDEFINED(); return ; }

    for (i = 0; i != args->argc; i++)
    {
        xjr_val arg = args->argv[i];
        xjr_u8 charcode;

        if (!XJR_VAL_IS_INTEGER(arg)) break;
        charcode = (xjr_u8)XJR_VAL_AS_INTEGER_UNTAG(arg);

        xjr_mbuf_append(&mbuf, (const char *)&charcode, 1);
    }

    args->ret = xjr_val_make_string_from_heap(args->mp, xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));
    xjr_val_properties_install(args->vm, args->ret);

    xjr_mbuf_uninit(&mbuf);
}

/* str.length */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/length */
static void xjr_bltin_string_prototype_length_get(xjr_native_fn_args *args)
{
    xjr_val s = args->_this;
    if (!XJR_VAL_IS_STRING(s)) return;
    args->ret = XJR_VAL_MAKE_INTEGER((int)xjr_val_as_string_length(args->mp, s));
}

/* character = str.charAt(index) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/charAt */
static void xjr_bltin_string_prototype_charat(xjr_native_fn_args *args)
{
    xjr_val s;
    int idx = 0;
    int len;

    if (args->argc == 0) { idx = 0; }
    else if (args->argc >= 1) { idx = XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]); }
    s = args->_this;
    if (!XJR_VAL_IS_STRING(s)) return;
    len = (int)xjr_val_as_string_length(args->mp, s);
    if (len <= idx)
    {
        args->ret = xjr_val_make_string_from_heap(args->mp, "", 0);
    }
    else
    {
        args->ret = xjr_val_make_string_from_heap(args->mp, \
                xjr_val_as_string_body(args->mp, s) + idx, 1);
    }
    xjr_val_properties_install(args->vm, args->ret);
}

/* str.charCodeAt(index) */
/* https://developer.mozilla.org/ja/docs/Web/JavaScript/Reference/Global_Objects/String/charCodeAt */
static void xjr_bltin_string_prototype_charcodeat(xjr_native_fn_args *args)
{
    xjr_val s;
    int idx = 0;
    int len;

    if (args->argc == 0) { idx = 0; }
    else if (args->argc >= 1) { idx = XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]); }
    s = args->_this;
    if (!XJR_VAL_IS_STRING(s)) return;
    len = (int)xjr_val_as_string_length(args->mp, s);
    if ((len <= idx) || (idx < 0))
    {
        args->ret = XJR_VAL_MAKE_UNDEFINED();
    }
    else
    {
        args->ret = XJR_VAL_MAKE_INTEGER((int)((xjr_val_as_string_body(args->mp, args->_this))[idx]));
    }
}

/* character = str.indexOf(searchValue[, fromIndex]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/indexOf */
static void xjr_bltin_string_prototype_indexof(xjr_native_fn_args *args)
{
    xjr_val v_search_value = XJR_VAL_MAKE_UNDEFINED();
    xjr_val v_from_index = XJR_VAL_MAKE_UNDEFINED();
    int idx;

    if (args->argc >= 1) { v_search_value = args->argv[0]; }
    if (args->argc >= 2) { v_from_index = args->argv[1]; }

    idx = xjr_val_as_string_indexof(args->mp, args->_this, v_search_value, v_from_index);

    args->ret = XJR_VAL_MAKE_INTEGER(idx);
}

/* str.split([separator[, limit]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/split */
static void xjr_bltin_string_prototype_split(xjr_native_fn_args *args)
{
    xjr_val separator;
    xjr_val ret;

    ret = xjr_val_make_array(args->mp);
    xjr_val_properties_install(args->vm, ret);

    if (args->argc == 0)
    {
        xjr_val elem;
        elem = xjr_val_make_string_from_heap(args->mp, \
                xjr_val_as_string_body(args->mp, args->_this), \
                xjr_val_as_string_length(args->mp, args->_this));
        xjr_val_properties_install(args->vm, elem);
        xjr_val_as_array_push(args->mp, ret, elem);
        args->ret = ret;
        return;
    }
    else
    {
        separator = args->argv[0];
        if (!XJR_VAL_IS_STRING(separator)) return;
    }

    {
        char *body = xjr_val_as_string_body(args->mp, args->_this);
        xjr_size_t len = xjr_val_as_string_length(args->mp, args->_this);
        char *sep_body = xjr_val_as_string_body(args->mp, separator);
        xjr_size_t sep_len = xjr_val_as_string_length(args->mp, separator);
        char *p = body;
        xjr_mbuf_t mbuf;

        if (xjr_mbuf_init(&mbuf, args->mp, \
                    xjr_mbuf_malloc_by_mp, \
                    xjr_mbuf_free_by_mp, \
                    xjr_memcpy) != 0)
        { args->ret = XJR_VAL_MAKE_UNDEFINED(); return ; }

        while (len != 0)
        {
            if (sep_len == 0)
            {
                xjr_val elem;
                elem = xjr_val_make_string_from_heap(args->mp, p, 1);
                xjr_val_properties_install(args->vm, elem);
                xjr_val_as_array_push(args->mp, ret, elem);
                p++;
                len--;
            }
            else
            {
                /* Is it a cutting point? */
                if ((len >= sep_len) && (xjr_strncmp(p, sep_body, sep_len) == 0))
                {
                    if (xjr_mbuf_size(&mbuf) != 0)
                    {
                        xjr_val elem;
                        elem = xjr_val_make_string_from_heap(args->mp, xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));
                        xjr_val_properties_install(args->vm, elem);
                        xjr_val_as_array_push(args->mp, ret, elem);
                        xjr_mbuf_clear(&mbuf);
                    }
                    p += sep_len;
                    len -= sep_len;
                }
                else
                {
                    xjr_mbuf_append(&mbuf, p, 1);
                    p++;
                    len--;
                }
            }
        }
        if (xjr_mbuf_size(&mbuf) != 0)
        {
            xjr_val elem;
            elem = xjr_val_make_string_from_heap(args->mp, xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));
            xjr_val_properties_install(args->vm, elem);
            xjr_val_as_array_push(args->mp, ret, elem);
            xjr_mbuf_clear(&mbuf);
        }

        args->ret = ret;

        xjr_mbuf_uninit(&mbuf);
    }
}

/* str.padStart(targetLength [, padString]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/padStart */
static void xjr_bltin_string_prototype_padStart(xjr_native_fn_args *args)
{
    xjr_val pad_string;
    int to_fill = 0;

    if (args->argc == 0)
    {
        args->ret = args->_this;
        return;
    }

    if (args->argc >= 1)
    {
        if (XJR_VAL_IS_INTEGER(args->argv[0]))
        {
            to_fill = (int)XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]) - \
                      (int)xjr_val_as_string_length(args->mp, args->_this);
            if (to_fill <= 0)
            {
                args->ret = args->_this;
                return;
            }
        }
        else { return; }
    }

    if (args->argc >= 2)
    {
        if (XJR_VAL_IS_STRING(args->argv[1])) { pad_string = args->argv[1]; }
        else { return; }
    }
    else
    {
        pad_string = xjr_val_make_string_from_heap(args->mp, " ", 1);
    }

    {
        char *body = xjr_val_as_string_body(args->mp, pad_string);
        int len = (int)xjr_val_as_string_length(args->mp, pad_string);
        int idx = 0;
        xjr_mbuf_t mbuf;

        if (xjr_mbuf_init(&mbuf, args->mp, \
                    xjr_mbuf_malloc_by_mp, \
                    xjr_mbuf_free_by_mp, \
                    xjr_memcpy) != 0)
        { args->ret = XJR_VAL_MAKE_UNDEFINED(); return ; }

        while (to_fill-- != 0)
        {
            xjr_mbuf_append(&mbuf, body + idx, 1);

            idx++;
            if (idx >= len) idx = 0;
        }

        xjr_mbuf_append(&mbuf, \
                xjr_val_as_string_body(args->mp, args->_this), \
                xjr_val_as_string_length(args->mp, args->_this));

        args->ret = xjr_val_make_string_from_heap(args->mp, \
                xjr_mbuf_body(&mbuf), xjr_mbuf_size(&mbuf));
        xjr_val_properties_install(args->vm, args->ret);

        xjr_mbuf_uninit(&mbuf);
    }
}

/* str.substr(start[, length]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/substr */
static void xjr_bltin_string_prototype_substr(xjr_native_fn_args *args)
{
    int start = 0, _orig_length, length;

    _orig_length = length = (int)xjr_val_as_string_length(args->mp, args->_this);

    if (args->argc >= 1)
    {
        if (XJR_VAL_IS_INTEGER(args->argv[0]) == 0) { return; }
        start = XJR_VAL_AS_INTEGER_UNTAG(args->argv[0]);
    }
    if (args->argc >= 2)
    {
        if (XJR_VAL_IS_INTEGER(args->argv[1]) == 0) { return; }
        length = XJR_VAL_AS_INTEGER_UNTAG(args->argv[1]);
    }

    if (start < 0)
    {
        start = _orig_length + start;
        if (start < 0) start = 0;
    }
    if (start > _orig_length) start = _orig_length;
    if (length < 0) length = 0;
    if (length > (_orig_length - start))
    { length = _orig_length - start; }

    {
        char *p = xjr_val_as_string_body(args->mp, args->_this);
        args->ret = xjr_val_make_string_from_heap(args->mp, \
                p + start, (xjr_size_t)length);
        xjr_val_properties_install(args->vm, args->ret);
    }
}

/* a.length */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/length */
static void xjr_bltin_array_prototype_length_get(xjr_native_fn_args *args)
{
    xjr_val a = args->_this;
    if (!XJR_VAL_IS_ARRAY(a)) return;
    args->ret = XJR_VAL_MAKE_INTEGER((int)xjr_val_as_array_size(args->mp, a));
}

/* a.reverse() */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/reverse */
static void xjr_bltin_array_prototype_reverse(xjr_native_fn_args *args)
{
    args->ret = xjr_val_as_array_reverse(args->mp, args->_this);
}

/* arr.join([separator]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/join */
static void xjr_bltin_array_prototype_join(xjr_native_fn_args *args)
{
    xjr_val separator;

    /* Separator (',' for default) */
    if (args->argc >= 1) { separator = args->argv[0]; }
    else { separator = xjr_val_make_string_from_heap(args->mp, ",", 1); }
    if (!XJR_VAL_IS_STRING(separator))
    { args->ret = XJR_VAL_MAKE_UNDEFINED(); return; }

    args->ret = xjr_val_as_array_join(args->mp, args->_this, separator);
    xjr_val_properties_install(args->vm, args->ret);
}

/* arr.pop() */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/pop */
static void xjr_bltin_array_prototype_pop(xjr_native_fn_args *args)
{
    args->ret = xjr_val_as_array_pop(args->mp, args->_this);
}

/* arr.shift() */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/shift */
static void xjr_bltin_array_prototype_shift(xjr_native_fn_args *args)
{
    args->ret = xjr_val_as_array_shift(args->mp, args->_this);
}

/* arr.push(element1[, ...[, elementN]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/push */
static void xjr_bltin_array_prototype_push(xjr_native_fn_args *args)
{
    xjr_val a = args->_this;
    xjr_size_t i;

    for (i = 0; i != args->argc; i++)
    {
        xjr_val cur = args->argv[i];
        xjr_val_as_array_push(args->mp, a, cur);
    }

    args->ret = XJR_VAL_MAKE_INTEGER((int)xjr_val_as_array_size(args->mp, a));
}

/* arr.unshift(element1[, ...[, elementN]]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/unshift */
static void xjr_bltin_array_prototype_unshift(xjr_native_fn_args *args)
{
    xjr_val a = args->_this;
    xjr_size_t i;

    for (i = 0; i != args->argc; i++)
    {
        xjr_val cur = args->argv[i];
        xjr_val_as_array_unshift(args->mp, a, cur);
    }

    args->ret = XJR_VAL_MAKE_INTEGER((int)xjr_val_as_array_size(args->mp, a));
}

/* arr.sort([compareFunction]) */
/* https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/sort */
/* static void xjr_bltin_array_prototype_sort(xjr_native_fn_args *args) */
/* { */
/*     xjr_val_as_array_sort(args->mp, args->_this); */
/*     args->ret = args->_this; */
/* } */

int xjr_bltin_infra_init(xjr_vm *vm, xjr_urid_t env)
{
    xjr_mp_t *mp = vm->rts.rheap.mp;

    /* undefined */
    V_OOM(xjr_vm_env_set_var(mp, env, "undefined", 9, XJR_VAL_MAKE_UNDEFINED()) == 0);

    /* Object */
    {
        xjr_val object;

        /* 'Object.prototype' */
        XJR_BLTIN_CREATE_OBJ(vm->fundamental.global_object_prototype, xjr_val_make_object(mp), \
                XJR_VAL_MAKE_NULL(), XJR_VAL_MAKE_UNDEFINED());

        /* 'Object' function */
        XJR_BLTIN_CREATE_OBJ(object, xjr_val_make_native_function(mp, env, xjr_bltin_object), \
                XJR_VAL_MAKE_UNDEFINED(), vm->fundamental.global_object_prototype);

        /* global.Object */
        V_OOM(xjr_vm_env_set_var(mp, env, "Object", 6, object) == 0);
    }

    /* Function */
    {
        xjr_val func;

        /* 'Function.prototype' */
        XJR_BLTIN_CREATE_OBJ(vm->fundamental.global_function_prototype, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());
        XJR_BLTIN_GETTER(vm->fundamental.global_function_prototype, "call", 4, xjr_bltin_function_prototype_call);

        /* 'Function' function */
        XJR_BLTIN_CREATE_OBJ(func, xjr_val_make_native_function(mp, env, xjr_bltin_function), \
                XJR_VAL_MAKE_UNDEFINED(), vm->fundamental.global_function_prototype);

        /* global.Function */
        V_OOM(xjr_vm_env_set_var(mp, env, "Function", 8, func) == 0);
    }

    /* Number */
    {
        xjr_val num;

        /* 'Number.prototype' */
        XJR_BLTIN_CREATE_OBJ(vm->fundamental.global_number_prototype, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());

        /* 'Number' function */
        XJR_BLTIN_CREATE_OBJ(num, xjr_val_make_native_function(mp, env, xjr_bltin_number), \
                XJR_VAL_MAKE_UNDEFINED(), vm->fundamental.global_function_prototype);
        XJR_BLTIN_METHOD(num, "parseInt", 8, xjr_bltin_number_parseint);

        /* global.Number */
        V_OOM(xjr_vm_env_set_var(mp, env, "Number", 6, num) == 0);
    }

    /* String */
    {
        xjr_val str;

        /* 'String.prototype' */
        XJR_BLTIN_CREATE_OBJ(vm->fundamental.global_string_prototype, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());
        XJR_BLTIN_GETTER(vm->fundamental.global_string_prototype, "length", 6, xjr_bltin_string_prototype_length_get);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "charCodeAt", 10, xjr_bltin_string_prototype_charcodeat);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "charAt", 6, xjr_bltin_string_prototype_charat);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "indexOf", 7, xjr_bltin_string_prototype_indexof);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "padStart", 8, xjr_bltin_string_prototype_padStart);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "split", 5, xjr_bltin_string_prototype_split);
        XJR_BLTIN_METHOD(vm->fundamental.global_string_prototype, "substr", 6, xjr_bltin_string_prototype_substr);

        /* 'String' function */
        XJR_BLTIN_CREATE_OBJ(str, xjr_val_make_native_function(mp, env, xjr_bltin_string), \
                XJR_VAL_MAKE_UNDEFINED(), vm->fundamental.global_string_prototype);
        XJR_BLTIN_METHOD(str, "fromCharCode", 12, xjr_bltin_string_fromcharcode);

        /* global.String */
        V_OOM(xjr_vm_env_set_var(mp, env, "String", 6, str) == 0);
    }

    /* Array */
    {
        xjr_val arr;

        /* 'Array.prototype' */
        XJR_BLTIN_CREATE_OBJ(vm->fundamental.global_array_prototype, xjr_val_make_object(mp), \
                vm->fundamental.global_object_prototype, XJR_VAL_MAKE_UNDEFINED());
        XJR_BLTIN_GETTER(vm->fundamental.global_array_prototype, "length", 6, xjr_bltin_array_prototype_length_get);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "reverse", 7, xjr_bltin_array_prototype_reverse);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "join", 4, xjr_bltin_array_prototype_join);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "pop", 3, xjr_bltin_array_prototype_pop);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "push", 4, xjr_bltin_array_prototype_push);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "shift", 5, xjr_bltin_array_prototype_shift);
        XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "unshift", 7, xjr_bltin_array_prototype_unshift);
        /* XJR_BLTIN_METHOD(vm->fundamental.global_array_prototype, "sort", 4, xjr_bltin_array_prototype_sort); */

        /* 'Array' function */
        XJR_BLTIN_CREATE_OBJ(arr, xjr_val_make_native_function(mp, env, xjr_bltin_array), \
                XJR_VAL_MAKE_UNDEFINED(), vm->fundamental.global_array_prototype);

        /* global.Array */
        V_OOM(xjr_vm_env_set_var(mp, env, "Array", 5, arr) == 0);
    }


    return 0;
fail:
    return -1;
}

