/* Enhanced C : Interface
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_INTERFACE_H
#define EC_INTERFACE_H

typedef void* (*ec_interface_callback_t)(void*);

struct ec_interface_iterator
{
    ec_interface_callback_t cb_deref;
};
typedef struct ec_interface_iterator ec_interface_iterator_t;

#define ec_interface_iterator_placeholder ec_interface_iterator_t _interface

struct ec_interface_iterator_map
{
    ec_interface_callback_t cb_deref_key;
    ec_interface_callback_t cb_deref_value;
};
typedef struct ec_interface_iterator_map ec_interface_iterator_map_t;

#define ec_interface_iterator_map_placeholder                                  \
    ec_interface_iterator_map_t _interface

#define ec_declare_iterator_members(_typename)                                 \
    void* _typename##_iterator_deref(void* iter);                              \
    void* _typename##_reverse_iterator_deref(void* iter);                      \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* ptr);                      \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* ptr);                        \
    void _typename##_iterator_next(_typename##_iterator* iter);                \
    void _typename##_iterator_prev(_typename##_iterator* iter);                \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2);              \
    void _typename##_reverse_iterator_init_begin(                              \
        _typename##_reverse_iterator* iter, _typename* ptr);                   \
    void _typename##_reverse_iterator_init_end(                                \
        _typename##_reverse_iterator* iter, _typename* ptr);                   \
    void _typename##_reverse_iterator_next(                                    \
        _typename##_reverse_iterator* iter);                                   \
    void _typename##_reverse_iterator_prev(                                    \
        _typename##_reverse_iterator* iter);                                   \
    ec_bool _typename##_reverse_iterator_eq(                                   \
        _typename##_reverse_iterator* iter1,                                   \
        _typename##_reverse_iterator* iter2)

#define ec_declare_iterator_members_set(_typename, _element_type)              \
    void* _typename##_reverse_iterator_deref(void* iter);                      \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* ptr);                      \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* ptr);                        \
    void _typename##_iterator_next(_typename##_iterator* iter);                \
    void _typename##_iterator_prev(_typename##_iterator* iter);                \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2)

#define ec_declare_iterator_members_map(_typename)                             \
    void* _typename##_iterator_deref_key(void* iter);                          \
    void* _typename##_iterator_deref_value(void* iter);                        \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* ptr);                      \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* ptr);                        \
    void _typename##_iterator_next(_typename##_iterator* iter);                \
    void _typename##_iterator_prev(_typename##_iterator* iter);                \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2)

#endif
