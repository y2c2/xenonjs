/* Enhanced C : Container : Stack
 * Copyright(c) 2017-2020 y2c2 */

/* A container adapter to provide LIFO operation to a queue
 *
 *
 * HEADER (<typename>.h file):
 *
 * Prototype:
 * ----------------------
 * ect_stack_declare(_typename, _element_type, _base_container);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new vector
 * _element_type: Data type of each element in the container
 * _base_container: The base container
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_list.h>
 * #include <ec_stack.h>
 * ect_list_declare(base_list_int_t, int);
 * ect_stack_declare(stack_int_t, int, base_list_int_t);
 * ----------------------
 *
 *
 * SOURCE (<typename>.c file):
 *
 * Prototype:
 * ----------------------
 * ect_stack_define(_typename, _element_type, _base_container);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new vector
 * _element_type: Data type of each element in the container
 * _base_container: The base container
 * ----------------------
 *
 * Sample:
 * ----------------------
 * #include <ec_list.h>
 * #include <ec_stack.h>
 * ect_list_define_undeclared(base_list_int_t, int, NULL);
 * ect_stack_define(stack_int_t, int, base_list_int_t);
 * ----------------------
 */

#ifndef EC_STACK_H
#define EC_STACK_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"

#define common_ec_stack(_typename, _element_type, _base_container)             \
    void* _typename##_new(void);                                               \
    ec_bool _typename##_empty(_typename* self);                                \
    ec_size_t _typename##_size(_typename* self);                               \
    void _typename##_clear(_typename* self);                                   \
    void _typename##_push(_typename* self, _element_type data);                \
    void _typename##_pop(_typename* self);                                     \
    _element_type _typename##_top(_typename* self)

#define ect_stack_declare(_typename, _element_type, _base_container)           \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    common_ec_stack(_typename, _element_type, _base_container)

#define ect_stack_define(_typename, _element_type, _base_container)            \
    struct _typename##_container                                               \
    {                                                                          \
        _base_container* _base;                                                \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_stack(_typename, _element_type, _base_container);                \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->_base = _base_container##_new();                                 \
    }                                                                          \
    static void _typename##_dtor(_typename* self) { ec_delete(self->_base); }  \
    void* _typename##_new(void)                                                \
    {                                                                          \
        return ec_newcd(_typename, _typename##_ctor,                           \
                        (ec_dtor_t)(_typename##_dtor));                        \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return _base_container##_empty(self->_base);                           \
    }                                                                          \
    ec_size_t _typename##_size(_typename* self)                                \
    {                                                                          \
        return _base_container##_size(self->_base);                            \
    }                                                                          \
    void _typename##_clear(_typename* self)                                    \
    {                                                                          \
        _base_container##_clear(self->_base);                                  \
    }                                                                          \
    void _typename##_push(_typename* self, _element_type data)                 \
    {                                                                          \
        _base_container##_push_back(self->_base, data);                        \
    }                                                                          \
    void _typename##_pop(_typename* self)                                      \
    {                                                                          \
        _base_container##_pop_back(self->_base);                               \
    }                                                                          \
    _element_type _typename##_top(_typename* self)                             \
    {                                                                          \
        return _base_container##_back(self->_base);                            \
    }                                                                          \
    void* _typename##_new(void)

#ifndef ECT_STACK_GENERIC_INTERFACE
#define ECT_STACK_GENERIC_INTERFACE
#define ect_stack_new(_typename) _typename##_new()
#define ect_stack_empty(_typename, _self) _typename##_empty(_self)
#define ect_stack_size(_typename, _self) _typename##_size(_self)
#define ect_stack_clear(_typename, _self) _typename##_clear(_self)
#define ect_stack_push(_typename, _self, _data) _typename##_push(_self, _data)
#define ect_stack_pop(_typename, _self) _typename##_pop(_self)
#define ect_stack_top(_typename, _self) _typename##_top(_self)
#endif

#endif
