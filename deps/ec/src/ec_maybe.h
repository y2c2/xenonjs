/* Enhanced C : Container : Maybe
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_MAYBE_H
#define EC_MAYBE_H

#include "ec_alloc.h"
#include "ec_dt.h"

#define ect_maybe_declare_common(_typename, _element_type)                     \
    void* _typename##_new_just(_element_type just_value);                      \
    void* _typename##_new_nothing(void);                                       \
    _element_type _typename##_unwrap(_typename* self);                         \
    ec_bool _typename##_is_nothing(_typename* self);                           \
    ec_bool _typename##_is_just(_typename* self, _element_type just_value)

#define ect_maybe_declare(_typename, _element_type)                            \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    ect_maybe_declare_common(_typename, _element_type)

#define ect_maybe_define(_typename, _element_type, _node_dtor, _node_cmp)      \
    enum _typename##_container_tag{_typename##_container_tag_just,             \
                                   _typename##_container_tag_nothing};         \
    struct _typename##_container                                               \
    {                                                                          \
        enum _typename##_container_tag tag;                                    \
        _element_type just_value;                                              \
        void (*__just_value_dtor)(_element_type*);                             \
        int (*__just_value_cmp)(_element_type*, _element_type*);               \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    ect_maybe_declare_common(_typename, _element_type);                        \
    static void _typename##_ctor_just(_typename* self)                         \
    {                                                                          \
        self->tag = _typename##_container_tag_just;                            \
    }                                                                          \
    static void _typename##_dtor_just(_typename* self)                         \
    {                                                                          \
        if (self->__just_value_dtor != NULL)                                   \
        {                                                                      \
            self->__just_value_dtor(&self->just_value);                        \
        }                                                                      \
    }                                                                          \
    static void _typename##_ctor_nothing(_typename* self)                      \
    {                                                                          \
        self->tag = _typename##_container_tag_nothing;                         \
    }                                                                          \
    void* _typename##_new_just(_element_type just_value)                       \
    {                                                                          \
        _typename* p =                                                         \
            ec_newcd(_typename, _typename##_ctor_just, _typename##_dtor_just); \
        p->just_value = just_value;                                            \
        p->__just_value_dtor = _node_dtor;                                     \
        p->__just_value_cmp = _node_cmp;                                       \
        return p;                                                              \
    }                                                                          \
    void* _typename##_new_nothing(void)                                        \
    {                                                                          \
        return ec_newcd(_typename, _typename##_ctor_nothing, NULL);            \
    }                                                                          \
    _element_type _typename##_unwrap(_typename* self)                          \
    {                                                                          \
        return self->just_value;                                               \
    }                                                                          \
    ec_bool _typename##_is_nothing(_typename* self)                            \
    {                                                                          \
        return (self->tag == _typename##_container_tag_nothing) ? ec_true      \
                                                                : ec_false;    \
    }                                                                          \
    ec_bool _typename##_is_just(_typename* self, _element_type just_value)     \
    {                                                                          \
        if (self->tag == _typename##_container_tag_nothing)                    \
            return ec_false;                                                   \
        if (self->__just_value_cmp == NULL)                                    \
            return ec_false;                                                   \
        return (self->__just_value_cmp(&self->just_value, &just_value) == 0)   \
                   ? ec_true                                                   \
                   : ec_false;                                                 \
    }                                                                          \
    void* _typename##_new_nothing(void)

#ifndef ECT_MAYBE_GENERIC_INTERFACE
#define ECT_MAYBE_GENERIC_INTERFACE
#define ect_maybe_new_just(_typename, _just_value)                             \
    _typename##_new_just(_just_value)
#define ect_maybe_new_nothing(_typename) _typename##_new_nothing()
#define ect_maybe_is_nothing(_typename, _self) _typename##_is_nothing(_self)
#define ect_maybe_is_just(_typename, _self, _just_value)                       \
    _typename##_is_just(_self, _just_value)
#define ect_maybe_unwrap(_typename, _self) _typename##_unwrap(_self)

#define ect_maybe_match_begin(_typename, _self)                                \
    {                                                                          \
        maybe_int* _##_typename##_match_this = _self;                          \
        if (0)                                                                 \
        {                                                                      \
        }

#define ect_maybe_match_just(_typename, _just_value)                           \
    else if (ect_maybe_is_just(maybe_int, _##_typename##_match_this,           \
                               _just_value))

#define ect_maybe_match_nothing(_typename)                                     \
    else if (ect_maybe_is_nothing(maybe_int, _##_typename##_match_this))

#define ect_maybe_match_otherwise(_typename) else

#define ect_maybe_match_end() }

#endif

#endif
