/* Enhanced C : Algorithm
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_ALGORITHM_H
#define EC_ALGORITHM_H

#include "ec_dt.h"
#include "ec_gensym.h"
#include "ec_interface.h"

/* Typed */

#define ect_iterator(_typename) _typename##_iterator

#define ect_reverse_iterator(_typename) _typename##_reverse_iterator

#define ect_for_with_end(_typename, _container, _iter, _iter_end)              \
    _typename##_iterator _iter_end;                                            \
    _typename##_iterator_init_begin(&_iter, _container);                       \
    _typename##_iterator_init_end(&_iter_end, _container);                     \
    for (; !(_typename##_iterator_eq(&_iter, &_iter_end));                     \
         _typename##_iterator_next(&_iter))

#define ect_for(_typename, _container, _iter)                                  \
    ect_for_with_end(_typename, _container, _iter, ec_gensym(iter_end_))

#define ect_for_reverse_with_end(_typename, _container, _iter, _iter_end)      \
    _typename##_reverse_iterator _iter_end;                                    \
    _typename##_reverse_iterator_init_begin(&_iter, _container);               \
    _typename##_reverse_iterator_init_end(&_iter_end, _container);             \
    for (; !(_typename##_reverse_iterator_eq(&_iter, &_iter_end));             \
         _typename##_reverse_iterator_next(&_iter))

#define ect_for_reverse(_typename, _container, _iter)                          \
    ect_for_reverse_with_end(_typename, _container, _iter, ec_gensym(iter_end_))

#define ect_iterator_deref(_typename, _iter)                                   \
    (_typename##_iterator_deref((void*)(&(_iter))))

#define ect_reverse_iterator_deref(_typename, _iter)                           \
    (_typename##_reverse_iterator_deref((void*)(&(_iter))))

/*
typedef ec_bool (*ec_unarypredicate_t)(void *value);

#define ect_find_if(_typename, _iter_first, _iter_last, _pred)  \
    (ect_find_if_raw(_iter_first, _iter_last, _pred))
    */

/* Untyped */

#define ec_deref(_iter) (_iter._interface.cb_deref((void*)(&(_iter))))

#define ec_deref_key(_iter) (_iter._interface.cb_deref_key((void*)(&(_iter))))

#define ec_deref_value(_iter)                                                  \
    (_iter._interface.cb_deref_value((void*)(&(_iter))))

#define ect_deref(_typename, _iter) (*((_typename*)(ec_deref(_iter))))

#define ect_deref_key(_typename, _iter) (*((_typename*)(ec_deref_key(_iter))))

#define ect_deref_value(_typename, _iter)                                      \
    (*((_typename*)(ec_deref_value(_iter))))

#endif
