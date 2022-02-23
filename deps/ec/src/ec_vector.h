/* Enhanced C : Container : Vector
 * Copyright(c) 2017-2020 y2c2 */

/* Container for contiguously stored elements
 *
 *
 * HEADER (<typename>.h file):
 *
 * Prototype:
 * ----------------------
 * ect_vector_declare(_typename, _element_type);
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
 * #include <ec_vector.h>
 * ect_vector_declare(vector_int_t, int);
 * ----------------------
 *
 *
 * SOURCE (<typename>.c file):
 *
 * Prototype:
 * ----------------------
 * ect_vector_define_declared(_typename, _element_type, _node_dtor);
 * ect_vector_define_undeclared(_typename, _element_type, _node_dtor);
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
 * #include <ec_vector.h>
 * ect_vector_define_undeclared(vector_int_t, int, NULL);
 * ----------------------
 */

#ifndef EC_VECTOR_H
#define EC_VECTOR_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"

#define EC_VECTOR_DEFAULT_CAPACITY 8
#define EC_VECTOR_EXTEND_NEW_COMPACITY(n) ((((n) + 1) << 1) + 1)
#define EC_VECTOR_MAX(a, b) ((a) > (b) ? (a) : (b))

#define declare_ec_vector_iterator(_typename)                                  \
    struct _typename##_iterator_tag;                                           \
    typedef struct _typename##_iterator_tag _typename##_iterator;              \
    struct _typename##_reverse_iterator_tag;                                   \
    typedef struct _typename##_reverse_iterator_tag _typename##_reverse_iterator

#define define_ec_vector_iterator(_typename)                                   \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    enum _typename##_iterator_state_tag{_typename##_iterator_state_body,       \
                                        _typename##_iterator_state_end};       \
    typedef enum _typename##_iterator_state_tag _typename##_iterator_state;    \
    struct _typename##_iterator_tag                                            \
    {                                                                          \
        ec_interface_iterator_placeholder;                                     \
        _typename##_iterator_state state;                                      \
        int idx;                                                               \
        _typename* _container;                                                 \
    };                                                                         \
    typedef struct _typename##_iterator_tag _typename##_iterator;              \
    struct _typename##_reverse_iterator_tag                                    \
    {                                                                          \
        ec_interface_iterator_placeholder;                                     \
        _typename##_iterator_state state;                                      \
        int idx;                                                               \
        _typename* _container;                                                 \
    };                                                                         \
    typedef struct _typename##_reverse_iterator_tag _typename##_reverse_iterator

#define common_ec_vector(_typename, _element_type)                             \
    void* _typename##_new(void);                                               \
    void _typename##__extend(_typename* self, ec_size_t request_capacity);     \
    _typename* _typename##_assign(_typename* self, _typename* p_rhs);          \
    _typename* _typename##_clone(_typename* self);                             \
    _element_type _typename##_at(_typename* self, ec_size_t n);                \
    void _typename##_set(_typename* self, ec_size_t n, _element_type data);    \
    _element_type _typename##_front(_typename* self);                          \
    _element_type _typename##_back(_typename* self);                           \
    ec_bool _typename##_empty(_typename* self);                                \
    ec_size_t _typename##_size(_typename* self);                               \
    void _typename##_clear(_typename* self);                                   \
    void _typename##_push_back(_typename* self, _element_type data);           \
    void _typename##_pop_back(_typename* self)

#define ect_vector_declare(_typename, _element_type)                           \
    define_ec_vector_iterator(_typename);                                      \
    common_ec_vector(_typename, _element_type);                                \
    ec_declare_iterator_members(_typename);                                    \
    void* _typename##_new(void)

#define ect_vector_define(_typename, _element_type, _node_dtor)                \
    struct _typename##_element                                                 \
    {                                                                          \
        _element_type internal_data;                                           \
    };                                                                         \
    typedef struct _typename##_element _typename##_element_t;                  \
    struct _typename##_container                                               \
    {                                                                          \
        struct _typename##_element* _body;                                     \
        ec_size_t size;                                                        \
        ec_size_t capacity;                                                    \
        void (*__node_dtor)(_element_type);                                    \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_vector(_typename, _element_type);                                \
    ec_declare_iterator_members(_typename);                                    \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->_body = NULL;                                                    \
        self->__node_dtor = _node_dtor;                                        \
        self->size = 0;                                                        \
        self->capacity = 0;                                                    \
    }                                                                          \
    static void _typename##_dtor(_typename* self)                              \
    {                                                                          \
        struct _typename##_element* _node_cur;                                 \
        if (self->_body == NULL)                                               \
            return;                                                            \
        _node_cur = &self->_body[0];                                           \
        ec_size_t cnt = self->size;                                            \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            while (cnt-- != 0)                                                 \
            {                                                                  \
                self->__node_dtor(_node_cur->internal_data);                   \
                _node_cur++;                                                   \
            }                                                                  \
        }                                                                      \
        ec_free(self->_body);                                                  \
    }                                                                          \
    void* _typename##_new(void)                                                \
    {                                                                          \
        return ec_newcd(_typename, _typename##_ctor,                           \
                        (ec_dtor_t)(_typename##_dtor));                        \
    }                                                                          \
    void _typename##__extend(_typename* self, ec_size_t request_capacity)      \
    {                                                                          \
        (void)self;                                                            \
        (void)request_capacity;                                                \
    }                                                                          \
    _typename* _typename##_assign(_typename* self, _typename* p_rhs)           \
    {                                                                          \
        _typename* n = _typename##_new();                                      \
        (void)self;                                                            \
        (void)p_rhs;                                                           \
        return n;                                                              \
    }                                                                          \
    _typename* _typename##_clone(_typename* self)                              \
    {                                                                          \
        _typename* n = _typename##_new();                                      \
        return _typename##_assign(n, self);                                    \
    }                                                                          \
    void* _typename##_iterator_deref(void* iter)                               \
    {                                                                          \
        _typename##_iterator* typed_iter = (_typename##_iterator*)iter;        \
        return (void*)(&typed_iter->_container->_body[typed_iter->idx]);       \
    }                                                                          \
    void* _typename##_reverse_iterator_deref(void* iter)                       \
    {                                                                          \
        _typename##_reverse_iterator* typed_iter =                             \
            (_typename##_reverse_iterator*)iter;                               \
        return (void*)(&typed_iter->_container->_body[typed_iter->idx]);       \
    }                                                                          \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* self)                      \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        if (self->size == 0)                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_body;                     \
        }                                                                      \
        iter->idx = 0;                                                         \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* self)                        \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        iter->state = _typename##_iterator_state_end;                          \
        iter->idx = (int)self->size;                                           \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_iterator_next(_typename##_iterator* iter)                 \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
            return;                                                            \
        iter->idx++;                                                           \
        if (iter->idx == (int)iter->_container->size)                          \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
    }                                                                          \
    void _typename##_iterator_prev(_typename##_iterator* iter)                 \
    {                                                                          \
        if (iter->idx == 0)                                                    \
            return;                                                            \
        iter->idx--;                                                           \
        iter->state = _typename##_iterator_state_body;                         \
    }                                                                          \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2)               \
    {                                                                          \
        return ((iter1->state == iter2->state) && (iter1->idx == iter2->idx))  \
                   ? ec_true                                                   \
                   : ec_false;                                                 \
    }                                                                          \
    void _typename##_reverse_iterator_init_begin(                              \
        _typename##_reverse_iterator* iter, _typename* self)                   \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_reverse_iterator_deref;        \
        if (self->size == 0)                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_body;                     \
        }                                                                      \
        iter->idx = (int)self->size - 1;                                       \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_reverse_iterator_init_end(                                \
        _typename##_reverse_iterator* iter, _typename* self)                   \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_reverse_iterator_deref;        \
        iter->state = _typename##_iterator_state_end;                          \
        iter->idx = -1;                                                        \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_reverse_iterator_next(_typename##_reverse_iterator* iter) \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
            return;                                                            \
        iter->idx--;                                                           \
        if (iter->idx == -1)                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
    }                                                                          \
    void _typename##_reverse_iterator_prev(_typename##_reverse_iterator* iter) \
    {                                                                          \
        if (iter->idx == (int)iter->_container->size)                          \
            return;                                                            \
        iter->idx++;                                                           \
        iter->state = _typename##_iterator_state_body;                         \
    }                                                                          \
    ec_bool _typename##_reverse_iterator_eq(                                   \
        _typename##_reverse_iterator* iter1,                                   \
        _typename##_reverse_iterator* iter2)                                   \
    {                                                                          \
        return ((iter1->state == iter2->state) && (iter1->idx == iter2->idx))  \
                   ? ec_true                                                   \
                   : ec_false;                                                 \
    }                                                                          \
    _element_type _typename##_at(_typename* self, ec_size_t n)                 \
    {                                                                          \
        return self->_body[n].internal_data;                                   \
    }                                                                          \
    void _typename##_set(_typename* self, ec_size_t n, _element_type data)     \
    {                                                                          \
        self->_body[n].internal_data = data;                                   \
    }                                                                          \
    _element_type _typename##_front(_typename* self)                           \
    {                                                                          \
        return self->_body[0].internal_data;                                   \
    }                                                                          \
    _element_type _typename##_back(_typename* self)                            \
    {                                                                          \
        return self->_body[self->size - 1].internal_data;                      \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return self->size == 0 ? ec_true : ec_false;                           \
    }                                                                          \
    ec_size_t _typename##_size(_typename* self) { return self->size; }         \
    void _typename##_clear(_typename* self)                                    \
    {                                                                          \
        struct _typename##_element* _node_cur = &self->_body[0];               \
        ec_size_t cnt = self->size;                                            \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            while (cnt-- != 0)                                                 \
            {                                                                  \
                self->__node_dtor(_node_cur->internal_data);                   \
                _node_cur++;                                                   \
            }                                                                  \
        }                                                                      \
        ec_free(self->_body);                                                  \
        self->_body = NULL;                                                    \
        self->size = 0;                                                        \
        self->capacity = 0;                                                    \
    }                                                                          \
    void _typename##_push_back(_typename* self, _element_type data)            \
    {                                                                          \
        ec_size_t cnt;                                                         \
        _typename##_element_t* new_node;                                       \
        if (self->_body == NULL)                                               \
        {                                                                      \
            self->_body = ec_malloc(sizeof(struct _typename##_element) *       \
                                    EC_VECTOR_DEFAULT_CAPACITY);               \
            if (self->_body == NULL)                                           \
            {                                                                  \
                return;                                                        \
            }                                                                  \
            self->size = 0;                                                    \
            self->capacity = EC_VECTOR_DEFAULT_CAPACITY;                       \
        }                                                                      \
        else if (self->size == self->capacity)                                 \
        {                                                                      \
            struct _typename##_element *new_body = NULL, *node_dst, *node_src; \
            ec_size_t new_capacity = self->capacity * 2;                       \
            new_body =                                                         \
                ec_malloc(sizeof(struct _typename##_element) * new_capacity);  \
            if (new_body == NULL)                                              \
            {                                                                  \
                return;                                                        \
            }                                                                  \
            {                                                                  \
                cnt = self->size;                                              \
                node_dst = new_body;                                           \
                node_src = self->_body;                                        \
                while (cnt-- != 0)                                             \
                {                                                              \
                    node_dst->internal_data = node_src->internal_data;         \
                    node_dst++;                                                \
                    node_src++;                                                \
                }                                                              \
                ec_free(self->_body);                                          \
                self->_body = new_body;                                        \
            }                                                                  \
            self->capacity = new_capacity;                                     \
        }                                                                      \
        new_node = &self->_body[self->size];                                   \
        new_node->internal_data = data;                                        \
        self->size++;                                                          \
    }                                                                          \
    void _typename##_pop_back(_typename* self)                                 \
    {                                                                          \
        self->size--;                                                          \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            self->__node_dtor(self->_body[self->size].internal_data);          \
        }                                                                      \
    }                                                                          \
    void* _typename##_new(void)

#define ect_vector_define_declared(_typename, _element_type, _node_dtor)       \
    declare_ec_vector_iterator(_typename);                                     \
    ect_vector_define(_typename, _element_type, _node_dtor)

#define ect_vector_define_undeclared(_typename, _element_type, _node_dtor)     \
    define_ec_vector_iterator(_typename);                                      \
    ect_vector_define(_typename, _element_type, _node_dtor)

#ifndef ECT_VECTOR_GENERIC_INTERFACE
#define ECT_VECTOR_GENERIC_INTERFACE
#define ect_vector_new(_typename) _typename##_new()
#define ect_vector_reverse_deref(_typename, _iter)                             \
    _typename##_reverse_deref(_iter)
#define ect_vector_iterator_init_begin(_typename, _iter, _self)                \
    _typename##_iterator_init_begin(_iter, _self)
#define ect_vector_iterator_init_end(_typename, _iter, _self)                  \
    _typename##_iterator_init_end(_iter, _self)
#define ect_vector_iterator_next(_typename, _iter)                             \
    _typename##_iterator_next(_iter)
#define ect_vector_iterator_prev(_typename, _iter)                             \
    _typename##_iterator_prev(_iter)
#define ect_vector_iterator_eq(_typename, _iter1, _iter2)                      \
    _typename##_iterator_eq(_iter1, _iter2)
#define ect_vector_reverse_iterator_init_begin(_typename, _iter, _self)        \
    _typename##_reverse_iterator_init_begin(_iter, _self)
#define ect_vector_reverse_iterator_init_end(_typename, _iter, _self)          \
    _typename##_reverse_iterator_init_end(_iter, _self)
#define ect_vector_reverse_iterator_next(_typename, _iter)                     \
    _typename##_reverse_iterator_next(_iter)
#define ect_vector_reverse_iterator_prev(_typename, _iter)                     \
    _typename##_reverse_iterator_prev(_iter)
#define ect_vector_reverse_iterator_eq(_typename, _iter1, _iter2)              \
    _typename##_reverse_iterator_eq(_iter1, _iter2)
#define ect_vector_set(_typename, _self, _n, _data)                            \
    _typename##_set(_self, _n, _data)
#define ect_vector_at(_typename, _self, _n) _typename##_at(_self, _n)
#define ect_vector_front(_typename, _self) _typename##_front(_self)
#define ect_vector_back(_typename, _self) _typename##_back(_self)
#define ect_vector_empty(_typename, _self) _typename##_empty(_self)
#define ect_vector_size(_typename, _self) _typename##_size(_self)
#define ect_vector_clear(_typename, _self) _typename##_clear(_self)
#define ect_vector_push_back(_typename, _self, _data)                          \
    _typename##_push_back(_self, _data)
#define ect_vector_pop_back(_typename, _self) _typename##_pop_back(_self)
#define ect_vector_deref(_typename, _iter) _typename##_deref(_iter)
#endif

#endif
