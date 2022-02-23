/* Enhanced C : Container : Set
 * Copyright(c) 2017-2020 y2c2 */

/* Container that stores unique elements
 *
 *
 * HEADER (<typename>.h file):
 *
 * Prototype:
 * ----------------------
 * ect_set_declare(_typename, _element_type);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new vector
 * _element_type: Data type of each element in the container
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_set.h>
 * ect_set_declare(set_int_t, int);
 * ----------------------
 *
 *
 * SOURCE (<typename>.c file):
 *
 * Prototype:
 * ----------------------
 * ect_set_define_declared(_typename, _element_type, _element_ctor,
 * _element_dtor, _element_cmp); ect_set_define_undeclared(_typename,
 * _element_type, _element_ctor, _element_dtor, _element_cmp);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new vector
 * _element_type: Data type of each element in the container
 * _node_dtor: Destructor of each element
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_set.h>
 *
 * static int int_ctor(int* detta_key, int* key)
 * {
 *     *detta_key = *key;
 *     return 0;
 * }
 *
 * static void int_dtor(int* detta_key) { (void)detta_key; }
 *
 * static int int_cmp(int* a, int* b)
 * {
 *     if (*a > *b)
 *         return 1;
 *     else if (*a < *b)
 *         return -1;
 *     return 0;
 * }
 *
 * ect_set_define_undeclared(set_int_t, int, int_ctor, int_dtor, int_cmp);
 * ----------------------
 */

#ifndef EC_SET_H
#define EC_SET_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"
#include "ec_rbt.h"

#define declare_ec_set_iterator(_typename)                                     \
    struct _typename##_iterator_tag;                                           \
    typedef struct _typename##_iterator_tag _typename##_iterator

#define define_ec_set_iterator(_typename)                                      \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    enum _typename##_iterator_state_tag{_typename##_iterator_state_body,       \
                                        _typename##_iterator_state_end};       \
    typedef enum _typename##_iterator_state_tag _typename##_iterator_state;    \
    struct _typename##_iterator_tag                                            \
    {                                                                          \
        ec_interface_iterator_placeholder;                                     \
        _typename##_iterator_state state;                                      \
        _typename* _container;                                                 \
        ec_rbt_iterator_t rbt_iterator;                                        \
    };                                                                         \
    typedef struct _typename##_iterator_tag _typename##_iterator

#define common_ec_set(_typename, _element_type)                                \
    void* _typename##_new(void);                                               \
    void* _typename##_iterator_deref(void* iter);                              \
    ec_size_t _typename##_size(_typename* self);                               \
    ec_bool _typename##_empty(_typename* self);                                \
    void _typename##_insert(_typename* self, _element_type data);              \
    void _typename##_erase(_typename* self, _element_type data);               \
    void _typename##_clear(_typename* self);                                   \
    ec_size_t _typename##_count(_typename* self, _element_type data)

#define ect_set_declare(_typename, _element_type)                              \
    define_ec_set_iterator(_typename);                                         \
    common_ec_set(_typename, _element_type);                                   \
    ec_declare_iterator_members_set(_typename, _element_type);                 \
    void* _typename##_new(void)

#define ect_set_define(_typename, _element_type, _element_ctor, _element_dtor, \
                       _element_cmp)                                           \
    struct _typename##_container                                               \
    {                                                                          \
        ec_rbt_t* _body;                                                       \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_set(_typename, _element_type);                                   \
    ec_declare_iterator_members_set(_typename, _element_type);                 \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->_body = ec_rbt_new(ec_malloc, ec_free, sizeof(_element_type),    \
                                 (ec_rbt_node_ctor_cb_t)_element_ctor,         \
                                 (ec_rbt_node_dtor_cb_t)_element_dtor, 0,      \
                                 NULL, NULL, (ec_rbt_cmp_cb_t)_element_cmp);   \
    }                                                                          \
    static void _typename##_dtor(_typename* self)                              \
    {                                                                          \
        ec_rbt_destroy(self->_body);                                           \
    }                                                                          \
    void* _typename##_new(void)                                                \
    {                                                                          \
        return ec_newcd(_typename, _typename##_ctor,                           \
                        (ec_dtor_t)(_typename##_dtor));                        \
    }                                                                          \
    void* _typename##_iterator_deref(void* iter)                               \
    {                                                                          \
        _typename##_iterator* typed_iter = (_typename##_iterator*)iter;        \
        return ec_rbt_iterator_deref_key(&typed_iter->rbt_iterator);           \
    }                                                                          \
    ec_size_t _typename##_size(_typename* self)                                \
    {                                                                          \
        return (ec_size_t)ec_rbt_size(self->_body);                            \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return ec_rbt_size(self->_body) == 0 ? ec_true : ec_false;             \
    }                                                                          \
    void _typename##_insert(_typename* self, _element_type data)               \
    {                                                                          \
        ec_rbt_insert(self->_body, &data, NULL);                               \
    }                                                                          \
    void _typename##_erase(_typename* self, _element_type data)                \
    {                                                                          \
        void* value;                                                           \
        if (ec_rbt_search_key(self->_body, &value, &data) != 0)                \
            return;                                                            \
        ec_rbt_remove(self->_body, &data);                                     \
    }                                                                          \
    void _typename##_clear(_typename* self) { ec_rbt_clear(self->_body); }     \
    ec_size_t _typename##_count(_typename* self, _element_type data)           \
    {                                                                          \
        void* value;                                                           \
        return (ec_rbt_search(self->_body, &value, &data) == 0) ? 1 : 0;       \
    }                                                                          \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* self)                      \
    {                                                                          \
        ec_rbt_iterator_init(self->_body, &iter->rbt_iterator);                \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        iter->_container = self;                                               \
        if (ec_rbt_size(self->_body) == 0)                                     \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_body;                     \
        }                                                                      \
    }                                                                          \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* self)                        \
    {                                                                          \
        ec_rbt_iterator_init(self->_body, &iter->rbt_iterator);                \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        iter->_container = self;                                               \
        iter->state = _typename##_iterator_state_end;                          \
    }                                                                          \
    void _typename##_iterator_next(_typename##_iterator* iter)                 \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
            return;                                                            \
        ec_rbt_iterator_next(&iter->rbt_iterator);                             \
        if (ec_rbt_iterator_isend(&iter->rbt_iterator))                        \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
    }                                                                          \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2)               \
    {                                                                          \
        if (iter1->state != iter2->state)                                      \
            return ec_false;                                                   \
        else if (iter1->state == _typename##_iterator_state_end)               \
            return ec_true;                                                    \
        else                                                                   \
            return (ec_rbt_iterator_eq(&iter1->rbt_iterator,                   \
                                       &iter2->rbt_iterator))                  \
                       ? ec_true                                               \
                       : ec_false;                                             \
    }                                                                          \
    void* _typename##_new(void)

#define ect_set_define_declared(_typename, _element_type, _element_ctor,       \
                                _element_dtor, _element_cmp)                   \
    declare_ec_set_iterator(_typename);                                        \
    ect_set_define(_typename, _element_type, _element_ctor, _element_dtor,     \
                   _element_cmp)

#define ect_set_define_undeclared(_typename, _element_type, _element_ctor,     \
                                  _element_dtor, _element_cmp)                 \
    define_ec_set_iterator(_typename);                                         \
    ect_set_define(_typename, _element_type, _element_ctor, _element_dtor,     \
                   _element_cmp)

#ifndef ECT_SET_GENERIC_INTERFACE
#define ECT_SET_GENERIC_INTERFACE
#define ect_set_new(_typename) _typename##_new()
#define ect_set_iterator_deref(_typename, _element)                            \
    _typename##_iterator_deref(_element)
#define ect_set_size(_typename, _self) _typename##_size(_self)
#define ect_set_empty(_typename, _self) _typename##_empty(_self)
#define ect_set_insert(_typename, _self, _data) _typename##_insert(_self, _data)
#define ect_set_erase(_typename, _self, _data) _typename##_erase(_self, _data)
#define ect_set_clear(_typename, _self) _typename##_clear(_self)
#define ect_set_count(_typename, _self, _data) _typename##_count(_self, _data)
#define ect_set_find(_typename, _self, _data) _typename##_find(_self, _data)
#endif

#endif
