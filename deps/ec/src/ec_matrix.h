/* Enhanced C : Container : Matrix
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_MATRIX_H
#define EC_MATRIX_H

#include "ec_alloc.h"
#include "ec_dt.h"

#define common_ec_matrix(_typename, _element_type)                             \
    void* _typename##_new(ec_size_t w, ec_size_t h);                           \
    ec_size_t _typename##_width(_typename* self);                              \
    ec_size_t _typename##_height(_typename* self);                             \
    _element_type _typename##_get(_typename* self, const ec_size_t x,          \
                                  const ec_size_t y);                          \
    void _typename##_set(_typename* self, const ec_size_t x,                   \
                         const ec_size_t y, _element_type data);               \
    void _typename##_fill(_typename* self, _element_type data)

#define ect_matrix_declare(_typename, _element_type)                           \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    common_ec_matrix(_typename, _element_type);                                \
    void* _typename##_new(ec_size_t w, ec_size_t h)

#define ect_matrix_define(_typename, _element_type, _node_dtor)                \
    struct _typename##_container                                               \
    {                                                                          \
        _element_type* elements;                                               \
        ec_size_t w, h;                                                        \
        void (*__node_dtor)(_element_type);                                    \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    common_ec_matrix(_typename, _element_type);                                \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        self->elements = NULL;                                                 \
        self->__node_dtor = _node_dtor;                                        \
        self->w = self->h = 0;                                                 \
    }                                                                          \
    static void _typename##_dtor(_typename* self)                              \
    {                                                                          \
        ec_size_t x, y;                                                        \
        for (y = 0; y != self->h; y++)                                         \
        {                                                                      \
            for (x = 0; x != self->w; x++)                                     \
            {                                                                  \
                if (self->__node_dtor != NULL)                                 \
                {                                                              \
                    self->__node_dtor(self->elements[y * self->w + x]);        \
                }                                                              \
            }                                                                  \
        }                                                                      \
        ec_free(self->elements);                                               \
    }                                                                          \
    void* _typename##_new(ec_size_t w, ec_size_t h)                            \
    {                                                                          \
        _typename* r;                                                          \
        r = ec_newcd(_typename, _typename##_ctor,                              \
                     (ec_dtor_t)(_typename##_dtor));                           \
        r->w = w;                                                              \
        r->h = h;                                                              \
        if ((r->elements = ec_malloc(sizeof(_element_type) * w * h)) == NULL)  \
        {                                                                      \
            ec_delete(r);                                                      \
            return NULL;                                                       \
        }                                                                      \
        return r;                                                              \
    }                                                                          \
    ec_size_t _typename##_width(_typename* self) { return self->w; }           \
    ec_size_t _typename##_height(_typename* self) { return self->h; }          \
    _element_type _typename##_get(_typename* self, const ec_size_t x,          \
                                  const ec_size_t y)                           \
    {                                                                          \
        return self->elements[y * self->w + x];                                \
    }                                                                          \
    void _typename##_set(_typename* self, const ec_size_t x,                   \
                         const ec_size_t y, _element_type data)                \
    {                                                                          \
        self->elements[y * self->w + x] = data;                                \
    }                                                                          \
    void _typename##_fill(_typename* self, _element_type data)                 \
    {                                                                          \
        ec_size_t x, y;                                                        \
        for (y = 0; y != self->h; y++)                                         \
        {                                                                      \
            for (x = 0; x != self->w; x++)                                     \
            {                                                                  \
                self->elements[y * self->w + x] = data;                        \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    void* _typename##_new(ec_size_t w, ec_size_t h)

#define ect_matrix_define_declared(_typename, _element_type, _node_dtor)       \
    ect_matrix_define(_typename, _element_type, _node_dtor)

#define ect_matrix_define_undeclared(_typename, _element_type, _node_dtor)     \
    ect_matrix_define(_typename, _element_type, _node_dtor)

#ifndef ECT_MATRIX_GENERIC_INTERFACE
#define ECT_MATRIX_GENERIC_INTERFACE
#define ect_matrix_new(_typename, _w, _h) _typename##_new(_w, _h)
#define ect_matrix_width(_typename, _self) _typename##_width(_self)
#define ect_matrix_height(_typename, _self) _typename##_height(_self)
#define ect_matrix_get(_typename, _self, _x, _y) _typename##_get(_self, _x, _y)
#define ect_matrix_set(_typename, _self, _x, _y, _data)                        \
    _typename##_set(_self, _x, _y, _data)
#define ect_matrix_fill(_typename, _self, _data) _typename##_fill(_self, _data)
#endif

#endif
