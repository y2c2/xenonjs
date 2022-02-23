/* Enhanced C : Container : List
 * Copyright(c) 2017-2020 y2c2 */

/* Container which supports constant time insertion and removel from anywhere.

 *
 * HEADER (<typename>.h file):
 *
 * Prototype:
 * ----------------------
 * ect_list_declare(_typename, _element_type);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new list
 * _element_type: Data type of each element in the container
 * ----------------------
 *
 * Sample:
 * ----------------------
 * ect_list_declare(list_int_t, int);
 * ----------------------
 *
 *
 * SOURCE (<typename>.c file):
 *
 * Prototype:
 * ----------------------
 * ect_list_define_declared(_typename, _element_type, _node_dtor);
 * ect_list_define_undeclared(_typename, _element_type, _node_dtor);
 * ----------------------
 *
 * Parameters:
 * ----------------------
 * _typename: The name of the new list
 * _element_type: Data type of each element in the container
 * _node_dtor: Destructor of each element
 * ----------------------
 *
 * Sample:
 * ----------------------
 * ect_list_define_undeclared(list_int_t, int, NULL);
 * ----------------------
 */

#ifndef EC_LIST_H
#define EC_LIST_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"

#define declare_ec_list_iterator(_typename)                                    \
    struct _typename##_iterator_tag;                                           \
    typedef struct _typename##_iterator_tag _typename##_iterator;              \
    struct _typename##_reverse_iterator_tag;                                   \
    typedef struct _typename##_reverse_iterator_tag _typename##_reverse_iterator

#define define_ec_list_iterator(_typename)                                     \
    struct _typename##_element;                                                \
    typedef struct _typename##_element _typename##_element_t;                  \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    enum _typename##_iterator_state_tag{_typename##_iterator_state_body,       \
                                        _typename##_iterator_state_end};       \
    typedef enum _typename##_iterator_state_tag _typename##_iterator_state;    \
    struct _typename##_iterator_tag                                            \
    {                                                                          \
        ec_interface_iterator_placeholder;                                     \
        _typename##_iterator_state state;                                      \
        _typename##_element_t* element;                                        \
        _typename* _container;                                                 \
    };                                                                         \
    typedef struct _typename##_iterator_tag _typename##_iterator;              \
    struct _typename##_reverse_iterator_tag                                    \
    {                                                                          \
        ec_interface_iterator_placeholder;                                     \
        _typename##_iterator_state state;                                      \
        _typename##_element_t* element;                                        \
        _typename* _container;                                                 \
    };                                                                         \
    typedef struct _typename##_reverse_iterator_tag _typename##_reverse_iterator

#define common_ec_list(_typename, _element_type)                               \
    void* _typename##_new(void);                                               \
    _element_type _typename##_front(_typename* self);                          \
    _element_type _typename##_back(_typename* self);                           \
    ec_size_t _typename##_size(_typename* self);                               \
    ec_bool _typename##_empty(_typename* self);                                \
    void _typename##_clear(_typename* self);                                   \
    void _typename##_push_back(_typename* self, _element_type data);           \
    void _typename##_push_front(_typename* self, _element_type data);          \
    void _typename##_pop_back(_typename* self);                                \
    void _typename##_pop_front(_typename* self)

#define ect_list_declare(_typename, _element_type)                             \
    define_ec_list_iterator(_typename);                                        \
    common_ec_list(_typename, _element_type);                                  \
    ec_declare_iterator_members(_typename);                                    \
    void* _typename##_new(void)

#define ect_list_define(_typename, _element_type, _node_dtor)                  \
    struct _typename##_element                                                 \
    {                                                                          \
        _element_type internal_data;                                           \
        struct _typename##_element *_prev, *_next;                             \
    };                                                                         \
    typedef struct _typename##_element _typename##_element_t;                  \
    struct _typename##_container                                               \
    {                                                                          \
        struct _typename##_element *_front, *_back;                            \
        ec_size_t size;                                                        \
        void (*__node_dtor)(_element_type);                                    \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_list(_typename, _element_type);                                  \
    ec_declare_iterator_members(_typename);                                    \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->_front = self->_back = NULL;                                     \
        self->__node_dtor = _node_dtor;                                        \
        self->size = 0;                                                        \
    }                                                                          \
    static void _typename##_dtor(_typename* self)                              \
    {                                                                          \
        struct _typename##_element *_node_cur = self->_front, *_node_next;     \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            while (_node_cur != NULL)                                          \
            {                                                                  \
                _node_next = _node_cur->_next;                                 \
                self->__node_dtor(_node_cur->internal_data);                   \
                ec_free(_node_cur);                                            \
                _node_cur = _node_next;                                        \
            };                                                                 \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            while (_node_cur != NULL)                                          \
            {                                                                  \
                _node_next = _node_cur->_next;                                 \
                ec_free(_node_cur);                                            \
                _node_cur = _node_next;                                        \
            };                                                                 \
        }                                                                      \
    }                                                                          \
    void* _typename##_new(void)                                                \
    {                                                                          \
        return ec_newcd(_typename, _typename##_ctor,                           \
                        (ec_dtor_t)(_typename##_dtor));                        \
    }                                                                          \
    void* _typename##_iterator_deref(void* iter)                               \
    {                                                                          \
        return (void*)&(                                                       \
            (((_typename##_iterator*)iter)->element->internal_data));          \
    }                                                                          \
    void* _typename##_reverse_iterator_deref(void* iter)                       \
    {                                                                          \
        return (void*)(&(                                                      \
            ((_typename##_reverse_iterator*)iter)->element->internal_data));   \
    }                                                                          \
    void _typename##_push_back(_typename* self, _element_type data)            \
    {                                                                          \
        struct _typename##_element* new_node =                                 \
            ec_malloc(sizeof(struct _typename##_element));                     \
        new_node->internal_data = data;                                        \
        new_node->_prev = new_node->_next = NULL;                              \
        if (self->_back == NULL)                                               \
        {                                                                      \
            self->_front = self->_back = new_node;                             \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            self->_back->_next = new_node;                                     \
            new_node->_prev = self->_back;                                     \
            self->_back = new_node;                                            \
        }                                                                      \
        self->size++;                                                          \
    }                                                                          \
    void _typename##_push_front(_typename* self, _element_type data)           \
    {                                                                          \
        struct _typename##_element* new_node =                                 \
            ec_malloc(sizeof(struct _typename##_element));                     \
        new_node->internal_data = data;                                        \
        new_node->_prev = new_node->_next = NULL;                              \
        if (self->_back == NULL)                                               \
        {                                                                      \
            self->_front = self->_back = new_node;                             \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            self->_front->_prev = new_node;                                    \
            new_node->_next = self->_front;                                    \
            self->_front = new_node;                                           \
        }                                                                      \
        self->size++;                                                          \
    }                                                                          \
    void _typename##_pop_back(_typename* self)                                 \
    {                                                                          \
        _typename##_element_t* element_prev = self->_back->_prev;              \
        self->size--;                                                          \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            self->__node_dtor(self->_back->internal_data);                     \
        }                                                                      \
        ec_free(self->_back);                                                  \
        if (element_prev != NULL)                                              \
            element_prev->_next = NULL;                                        \
        self->_back = element_prev;                                            \
        if (self->_back == NULL)                                               \
            self->_front = NULL;                                               \
    }                                                                          \
    void _typename##_pop_front(_typename* self)                                \
    {                                                                          \
        _typename##_element_t* element_next = self->_front->_next;             \
        self->size--;                                                          \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            self->__node_dtor(self->_front->internal_data);                    \
        }                                                                      \
        ec_free(self->_front);                                                 \
        if (element_next != NULL)                                              \
            element_next->_prev = NULL;                                        \
        self->_front = element_next;                                           \
        if (self->_front == NULL)                                              \
            self->_back = NULL;                                                \
    }                                                                          \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* self)                      \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        if (self->size == 0)                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
            iter->element = NULL;                                              \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_body;                     \
            iter->element = self->_front;                                      \
        }                                                                      \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* self)                        \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_iterator_deref;                \
        iter->state = _typename##_iterator_state_end;                          \
        iter->element = NULL;                                                  \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_iterator_next(_typename##_iterator* iter)                 \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
            return;                                                            \
        if ((iter->element = iter->element->_next) == NULL)                    \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
    }                                                                          \
    void _typename##_iterator_prev(_typename##_iterator* iter)                 \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
        {                                                                      \
            if ((iter->element = iter->_container->_back) != NULL)             \
            {                                                                  \
                iter->state = _typename##_iterator_state_body;                 \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->element = iter->element->_prev;                              \
        }                                                                      \
    }                                                                          \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2)               \
    {                                                                          \
        return ((iter1->state == iter2->state) &&                              \
                (iter1->element == iter2->element))                            \
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
            iter->element = NULL;                                              \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->state = _typename##_iterator_state_body;                     \
            iter->element = self->_back;                                       \
        }                                                                      \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_reverse_iterator_init_end(                                \
        _typename##_reverse_iterator* iter, _typename* self)                   \
    {                                                                          \
        iter->_interface.cb_deref = _typename##_reverse_iterator_deref;        \
        iter->state = _typename##_iterator_state_end;                          \
        iter->element = NULL;                                                  \
        iter->_container = self;                                               \
    }                                                                          \
    void _typename##_reverse_iterator_next(_typename##_reverse_iterator* iter) \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
            return;                                                            \
        if ((iter->element = iter->element->_prev) == NULL)                    \
        {                                                                      \
            iter->state = _typename##_iterator_state_end;                      \
        }                                                                      \
    }                                                                          \
    void _typename##_reverse_iterator_prev(_typename##_reverse_iterator* iter) \
    {                                                                          \
        if (iter->state == _typename##_iterator_state_end)                     \
        {                                                                      \
            if ((iter->element = iter->_container->_back) != NULL)             \
            {                                                                  \
                iter->state = _typename##_iterator_state_body;                 \
            }                                                                  \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            iter->element = iter->element->_next;                              \
        }                                                                      \
    }                                                                          \
    ec_bool _typename##_reverse_iterator_eq(                                   \
        _typename##_reverse_iterator* iter1,                                   \
        _typename##_reverse_iterator* iter2)                                   \
    {                                                                          \
        return ((iter1->state == iter2->state) &&                              \
                (iter1->element == iter2->element))                            \
                   ? ec_true                                                   \
                   : ec_false;                                                 \
    }                                                                          \
    _element_type _typename##_front(_typename* self)                           \
    {                                                                          \
        return self->_front->internal_data;                                    \
    }                                                                          \
    _element_type _typename##_back(_typename* self)                            \
    {                                                                          \
        return self->_back->internal_data;                                     \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return self->size == 0 ? ec_true : ec_false;                           \
    }                                                                          \
    ec_size_t _typename##_size(_typename* self) { return self->size; }         \
    void _typename##_clear(_typename* self)                                    \
    {                                                                          \
        struct _typename##_element *_node_cur = self->_front, *_node_next;     \
        if (self->__node_dtor != NULL)                                         \
        {                                                                      \
            while (_node_cur != NULL)                                          \
            {                                                                  \
                _node_next = _node_cur->_next;                                 \
                self->__node_dtor(_node_cur->internal_data);                   \
                ec_free(_node_cur);                                            \
                _node_cur = _node_next;                                        \
            };                                                                 \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            while (_node_cur != NULL)                                          \
            {                                                                  \
                _node_next = _node_cur->_next;                                 \
                ec_free(_node_cur);                                            \
                _node_cur = _node_next;                                        \
            };                                                                 \
        }                                                                      \
        self->_front = self->_back = NULL;                                     \
        self->size = 0;                                                        \
    }                                                                          \
    void* _typename##_new(void)

#define ect_list_define_declared(_typename, _element_type, _node_dtor)         \
    declare_ec_list_iterator(_typename);                                       \
    ect_list_define(_typename, _element_type, _node_dtor)

#define ect_list_define_undeclared(_typename, _element_type, _node_dtor)       \
    define_ec_list_iterator(_typename);                                        \
    ect_list_define(_typename, _element_type, _node_dtor)

#ifndef ECT_LIST_GENERIC_INTERFACE
#define ECT_LIST_GENERIC_INTERFACE
#define ect_list_new(_typename) _typename##_new()
#define ect_list_iterator_deref(_typename, _element)                           \
    _typename##_iterator_deref(_element)
#define ect_list_reverse_iterator_deref(_typename, _element)                   \
    _typename##_reverse_iterator_deref(_element)
#define ect_list_iterator_init_begin(_typename, _iter, _self)                  \
    _typename##_iterator_init_begin(_iter, _self)
#define ect_list_iterator_init_end(_typename, _iter, _self)                    \
    _typename##_iterator_init_end(_iter, _self)
#define ect_list_iterator_next(_typename, _iter)                               \
    _typename##_iterator_next(_iter)
#define ect_list_iterator_prev(_typename, _iter)                               \
    _typename##_iterator_prev(_iter)
#define ect_list_iterator_eq(_typename, _iter1, _iter2)                        \
    _typename##_iterator_eq(_iter1, _iter2)
#define ect_list_reverse_iterator_init_begin(_typename, _iter, _self)          \
    _typename##_reverse_iterator_init_begin(_iter, _self)
#define ect_list_reverse_iterator_init_end(_typename, _iter, _self)            \
    _typename##_reverse_iterator_init_end(_iter, _self)
#define ect_list_reverse_iterator_next(_typename, _iter)                       \
    _typename##_reverse_iterator_next(_iter)
#define ect_list_reverse_iterator_prev(_typename, _iter)                       \
    _typename##_reverse_iterator_prev(_iter)
#define ect_list_reverse_iterator_eq(_typename, _iter1, _iter2)                \
    _typename##_reverse_iterator_eq(_iter1, _iter2)
#define ect_list_front(_typename, _self) _typename##_front(_self)
#define ect_list_back(_typename, _self) _typename##_back(_self)
#define ect_list_size(_typename, _self) _typename##_size(_self)
#define ect_list_empty(_typename, _self) _typename##_empty(_self)
#define ect_list_clear(_typename, _self) _typename##_clear(_self)
#define ect_list_push_back(_typename, _self, _data)                            \
    _typename##_push_back(_self, _data)
#define ect_list_push_front(_typename, _self, _data)                           \
    _typename##_push_front(_self, _data)
#define ect_list_pop_back(_typename, _self) _typename##_pop_back(_self)
#define ect_list_pop_front(_typename, _self) _typename##_pop_front(_self)
#endif

#endif
