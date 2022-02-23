/* Enhanced C : Container : map
 * Copyright(c) 2017-2020 y2c2 */

/* Container that stores unique keys and mapped values
 *
 *
 * HEADER (<typename>.h file):
 *
 * Prototype:
 * ----------------------
 * ect_map_declare(_typename, _key_type, _value_type)
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new vector
 * _key_type: Data type of mapped value in the container
 * _value_type: Data type of mapped value in the container
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_map.h>
 * ect_map_declare(map_int_t, int, int);
 * ----------------------
 *
 *
 * SOURCE (<typename>.c file):
 *
 * Prototype:
 * ----------------------
 * ect_map_define_declared(_typename, _key_type, _key_ctor, _key_dtor,
 * _value_type, _value_ctor, _value_dtor, _key_cmp);
 * ect_map_define_undeclared(_typename, _key_type, _key_ctor, _key_dtor,
 * _value_type, _value_ctor, _value_dtor, _key_cmp);
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_map.h>
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
 * ect_map_define_undeclared(map_int_t, int, int_ctor, int_dtor, int, int_ctor,
 *                           int_dtor, int_cmp);
 * ----------------------
 */

#ifndef EC_MAP_H
#define EC_MAP_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"
#include "ec_rbt.h"

#define declare_ec_map_iterator(_typename)                                     \
    struct _typename##_iterator_tag;                                           \
    typedef struct _typename##_iterator_tag _typename##_iterator;              \
    struct _typename##_reverse_iterator_tag;                                   \
    typedef struct _typename##_reverse_iterator_tag _typename##_reverse_iterator

#define define_ec_map_iterator(_typename)                                      \
    struct _typename##_element;                                                \
    typedef struct _typename##_element _typename##_element_t;                  \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    enum _typename##_iterator_state_tag{_typename##_iterator_state_body,       \
                                        _typename##_iterator_state_end};       \
    typedef enum _typename##_iterator_state_tag _typename##_iterator_state;    \
    struct _typename##_iterator_tag                                            \
    {                                                                          \
        ec_interface_iterator_map_placeholder;                                 \
        _typename##_iterator_state state;                                      \
        _typename* _container;                                                 \
        ec_rbt_iterator_t rbt_iterator;                                        \
    };                                                                         \
    typedef struct _typename##_iterator_tag _typename##_iterator

#define common_ec_map(_typename, _key_type, _value_type)                       \
    void* _typename##_new(void);                                               \
    void* _typename##_iterator_deref_key(void* iter);                          \
    void* _typename##_iterator_deref_value(void* iter);                        \
    ec_size_t _typename##_size(_typename* self);                               \
    ec_bool _typename##_empty(_typename* self);                                \
    void _typename##_insert(_typename* self, _key_type key,                    \
                            _value_type value);                                \
    void _typename##_erase(_typename* self, _key_type key);                    \
    void _typename##_clear(_typename* self);                                   \
    _value_type _typename##_get(_typename* self, _key_type key);               \
    ec_size_t _typename##_count(_typename* self, _key_type key)

#define ect_map_declare(_typename, _key_type, _value_type)                     \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    define_ec_map_iterator(_typename);                                         \
    common_ec_map(_typename, _key_type, _value_type);                          \
    ec_declare_iterator_members_map(_typename);                                \
    void* _typename##_new(void)

#define define_ec_map(_typename, _key_type, _key_ctor, _key_dtor, _value_type, \
                      _value_ctor, _value_dtor, _key_cmp)                      \
    struct _typename##_container                                               \
    {                                                                          \
        ec_rbt_t* _body;                                                       \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_map(_typename, _key_type, _value_type);                          \
    ec_declare_iterator_members_map(_typename);                                \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->_body = ec_rbt_new(                                              \
            ec_malloc, ec_free, sizeof(_key_type),                             \
            (ec_rbt_node_ctor_cb_t)_key_ctor,                                  \
            (ec_rbt_node_dtor_cb_t)_key_dtor, sizeof(_value_type),             \
            (ec_rbt_node_ctor_cb_t)_value_ctor,                                \
            (ec_rbt_node_dtor_cb_t)_value_dtor, (ec_rbt_cmp_cb_t)_key_cmp);    \
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
    void* _typename##_iterator_deref_key(void* iter)                           \
    {                                                                          \
        _typename##_iterator* typed_iter = (_typename##_iterator*)iter;        \
        return ec_rbt_iterator_deref_key(&typed_iter->rbt_iterator);           \
    }                                                                          \
    void* _typename##_iterator_deref_value(void* iter)                         \
    {                                                                          \
        _typename##_iterator* typed_iter = (_typename##_iterator*)iter;        \
        return ec_rbt_iterator_deref_value(&typed_iter->rbt_iterator);         \
    }                                                                          \
    ec_size_t _typename##_size(_typename* self)                                \
    {                                                                          \
        return (ec_size_t)ec_rbt_size(self->_body);                            \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return ec_rbt_size(self->_body) == 0 ? ec_true : ec_false;             \
    }                                                                          \
    void _typename##_insert(_typename* self, _key_type key, _value_type value) \
    {                                                                          \
        ec_rbt_insert(self->_body, &key, &value);                              \
    }                                                                          \
    void _typename##_erase(_typename* self, _key_type key)                     \
    {                                                                          \
        ec_rbt_remove(self->_body, &key);                                      \
    }                                                                          \
    void _typename##_clear(_typename* self) { ec_rbt_clear(self->_body); }     \
    _value_type _typename##_get(_typename* self, _key_type key)                \
    {                                                                          \
        void* value;                                                           \
        if (ec_rbt_search(self->_body, &value, &key) == 0)                     \
        {                                                                      \
            return **((_value_type**)value);                                   \
        }                                                                      \
        return **((_value_type**)self->_body->nil->value);                     \
    }                                                                          \
    ec_size_t _typename##_count(_typename* self, _key_type key)                \
    {                                                                          \
        void* value;                                                           \
        if (ec_rbt_search(self->_body, &value, &key) == 0)                     \
        {                                                                      \
            return 1;                                                          \
        }                                                                      \
        return 0;                                                              \
    }                                                                          \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* self)                      \
    {                                                                          \
        ec_rbt_iterator_init(self->_body, &iter->rbt_iterator);                \
        iter->_interface.cb_deref_key = _typename##_iterator_deref_key;        \
        iter->_interface.cb_deref_value = _typename##_iterator_deref_value;    \
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
        iter->_interface.cb_deref_key = _typename##_iterator_deref_key;        \
        iter->_interface.cb_deref_value = _typename##_iterator_deref_value;    \
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

#define ect_map_define_declared(_typename, _key_type, _key_ctor, _key_dtor,    \
                                _value_type, _value_ctor, _value_dtor,         \
                                _key_cmp)                                      \
    declare_ec_map_iterator(_typename);                                        \
    define_ec_map(_typename, _key_type, _key_ctor, _key_dtor, _value_type,     \
                  _value_ctor, _value_dtor, _key_cmp)

#define ect_map_define_undeclared(_typename, _key_type, _key_ctor, _key_dtor,  \
                                  _value_type, _value_ctor, _value_dtor,       \
                                  _key_cmp)                                    \
    define_ec_map_iterator(_typename);                                         \
    define_ec_map(_typename, _key_type, _key_ctor, _key_dtor, _value_type,     \
                  _value_ctor, _value_dtor, _key_cmp)

#ifndef ECT_MAP_GENERIC_INTERFACE
#define ECT_MAP_GENERIC_INTERFACE
#define ect_map_new(_typename) _typename##_new()
#define ect_map_iterator_deref(_typename, _element)                            \
    _typename##_iterator_deref(_element)
#define ect_map_size(_typename, _self) _typename##_size(_self)
#define ect_map_empty(_typename, _self) _typename##_empty(_self)
#define ect_map_insert(_typename, _self, _key, _value)                         \
    _typename##_insert(_self, _key, _value)
#define ect_map_erase(_typename, _self, _key) _typename##_erase(_self, _key)
#define ect_map_clear(_typename, _self) _typename##_clear(_self)
#define ect_map_count(_typename, _self, _data) _typename##_count(_self, _data)
#define ect_map_get(_typename, _self, _key) _typename##_get(_self, _key)
#endif

#endif
