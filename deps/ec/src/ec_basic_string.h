/* Enhanced C : Basic String
 * Copyright(c) 2017-2020 y2c2 */

#ifndef EC_BASIC_STRING_H
#define EC_BASIC_STRING_H

#include "ec_alloc.h"
#include "ec_dt.h"
#include "ec_interface.h"

#define EC_BASIC_STRING_INITIAL_LENGTH 64
#define EC_BASIC_STRING_EXTEND_NEW_COMPACITY(n) ((((n) + 1) << 1) + 1)
#define EC_BASIC_STRING_MAX(a, b) ((a) > (b) ? (a) : (b))

typedef int ec_basic_string_size_type;
#define ec_basic_string_npos ((ec_basic_string_size_type)(-1))

#define define_ec_basic_string_iterator(_typename)                             \
    typedef enum                                                               \
    {                                                                          \
        _typename##_iterator_state_body,                                       \
        _typename##_iterator_state_end                                         \
    } _typename##_iterator_state;                                              \
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

#define common_ec_basic_string(_typename, _element_type)                       \
    _typename* _typename##_new(void);                                          \
    _typename* _typename##_clone(_typename* self);                             \
    _element_type* _typename##__body_get(_typename* p);                        \
    void _typename##__body_set(_typename* p, _element_type* body);             \
    ec_basic_string_size_type _typename##__size_get(_typename* p);             \
    void _typename##__size_set(_typename* p, ec_basic_string_size_type size);  \
    ec_basic_string_size_type _typename##__capacity_get(_typename* p);         \
    void _typename##__capacity_set(_typename* p,                               \
                                   ec_basic_string_size_type capacity);        \
    void* _typename##_iterator_deref(void* iter);                              \
    void* _typename##_reverse_iterator_deref(void* iter);                      \
    void _typename##_iterator_init_begin(_typename##_iterator* iter,           \
                                         _typename* self);                     \
    void _typename##_iterator_init_end(_typename##_iterator* iter,             \
                                       _typename* self);                       \
    void _typename##_iterator_next(_typename##_iterator* iter);                \
    void _typename##_iterator_prev(_typename##_iterator* iter);                \
    ec_bool _typename##_iterator_eq(_typename##_iterator* iter1,               \
                                    _typename##_iterator* iter2);              \
    void _typename##_reverse_iterator_init_begin(                              \
        _typename##_reverse_iterator* iter, _typename* self);                  \
    void _typename##_reverse_iterator_init_end(                                \
        _typename##_reverse_iterator* iter, _typename* self);                  \
    void _typename##_reverse_iterator_next(                                    \
        _typename##_reverse_iterator* iter);                                   \
    void _typename##_reverse_iterator_prev(                                    \
        _typename##_reverse_iterator* iter);                                   \
    ec_bool _typename##_reverse_iterator_eq(                                   \
        _typename##_reverse_iterator* iter1,                                   \
        _typename##_reverse_iterator* iter2);                                  \
    void _typename##__extend(_typename* self,                                  \
                             ec_basic_string_size_type request_capacity);      \
    _typename* _typename##_assign(_typename* self, _typename* p_rhs);          \
    _typename* _typename##_assign_c_str(_typename* self, const char* p_rhs);   \
    _typename* _typename##_new_assign_c_str(const char* s);                    \
    ec_basic_string_size_type _typename##_size(const _typename* self);         \
    ec_basic_string_size_type _typename##_length(const _typename* self);       \
    ec_basic_string_size_type _typename##_capacity(const _typename* self);     \
    void _typename##_clear(_typename* self);                                   \
    ec_bool _typename##_empty(_typename* self);                                \
    _element_type* _typename##_c_str(const _typename* self);                   \
    _element_type _typename##_at(const _typename* self,                        \
                                 const ec_basic_string_size_type n);           \
    int _typename##_cmp(_typename* s1, _typename* s2);                         \
    ec_bool _typename##_eq(_typename* s1, _typename* s2);                      \
    _typename* _typename##_append(_typename* self, _typename* p_rhs);          \
    void _typename##_push_back(_typename* self, _element_type data);           \
    void _typename##_push_front(_typename* self, _element_type data);          \
    ec_basic_string_size_type _typename##_find(                                \
        _typename* self, _element_type ch, ec_basic_string_size_type pos);     \
    ec_basic_string_size_type _typename##_rfind(                               \
        _typename* self, _element_type ch, ec_basic_string_size_type pos);     \
    _typename* _typename##_substr(_typename* self,                             \
                                  ec_basic_string_size_type pos,               \
                                  ec_basic_string_size_type count)

#define declare_ec_basic_string(_typename, _element_type)                      \
    struct _typename##_element;                                                \
    typedef struct _typename##_element _typename##_element_t;                  \
    struct _typename##_container;                                              \
    typedef struct _typename##_container _typename;                            \
    define_ec_basic_string_iterator(_typename);                                \
    common_ec_basic_string(_typename, _element_type);                          \
    ec_declare_iterator_members(_typename);                                    \
    _typename* _typename##_new(void)

#define define_ec_basic_string(_typename, _element_type)                       \
    struct _typename##_container                                               \
    {                                                                          \
        _element_type* _body;                                                  \
        ec_basic_string_size_type size;                                        \
        ec_basic_string_size_type capacity;                                    \
    };                                                                         \
    typedef struct _typename##_container _typename;                            \
    define_ec_basic_string_iterator(_typename);                                \
    common_ec_basic_string(_typename, _element_type);                          \
    _element_type* _typename##__body_get(_typename* p) { return p->_body; }    \
    void _typename##__body_set(_typename* p, _element_type* body)              \
    {                                                                          \
        p->_body = body;                                                       \
    }                                                                          \
    ec_basic_string_size_type _typename##__size_get(_typename* p)              \
    {                                                                          \
        return p->size;                                                        \
    }                                                                          \
    void _typename##__size_set(_typename* p, ec_basic_string_size_type size)   \
    {                                                                          \
        p->size = size;                                                        \
    }                                                                          \
    ec_basic_string_size_type _typename##__capacity_get(_typename* p)          \
    {                                                                          \
        return p->capacity;                                                    \
    }                                                                          \
    void _typename##__capacity_set(_typename* p,                               \
                                   ec_basic_string_size_type capacity)         \
    {                                                                          \
        p->capacity = capacity;                                                \
    }                                                                          \
    static void _typename##_ctor(_typename* self)                              \
    {                                                                          \
        if ((self->_body = (_element_type*)ec_malloc(                          \
                 sizeof(_element_type) *                                       \
                 (EC_BASIC_STRING_INITIAL_LENGTH + 1))) == NULL)               \
        {                                                                      \
            return;                                                            \
        }                                                                      \
        self->_body[0] = (_element_type)0;                                     \
        self->size = 0;                                                        \
        self->capacity = EC_BASIC_STRING_INITIAL_LENGTH;                       \
    }                                                                          \
    static void _typename##_dtor(_typename* self) { ec_free(self->_body); }    \
    _typename* _typename##_new(void)                                           \
    {                                                                          \
        return (_typename*)ec_newcd(_typename, _typename##_ctor,               \
                                    (ec_dtor_t)(_typename##_dtor));            \
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
    void _typename##__extend(_typename* self,                                  \
                             ec_basic_string_size_type request_capacity)       \
    {                                                                          \
        ec_basic_string_size_type new_capacity =                               \
            EC_BASIC_STRING_EXTEND_NEW_COMPACITY(                              \
                EC_BASIC_STRING_MAX(self->capacity, request_capacity));        \
        _element_type *new_body, *src_p, *dst_p;                               \
        ec_basic_string_size_type cnt;                                         \
        if (request_capacity < self->capacity)                                 \
            return;                                                            \
        new_body = (_element_type*)ec_malloc(sizeof(_element_type) *           \
                                             (ec_size_t)new_capacity);         \
        if (new_body == NULL)                                                  \
            return;                                                            \
        cnt = self->size;                                                      \
        dst_p = new_body;                                                      \
        src_p = self->_body;                                                   \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            *dst_p++ = *src_p++;                                               \
        }                                                                      \
        *dst_p++ = (_element_type)0;                                           \
        self->capacity = new_capacity;                                         \
        ec_free(self->_body);                                                  \
        self->_body = new_body;                                                \
    }                                                                          \
    _typename* _typename##_assign(_typename* self, _typename* p_rhs)           \
    {                                                                          \
        _element_type *src_p, *dst_p;                                          \
        ec_basic_string_size_type cnt;                                         \
        if (p_rhs->size >= self->capacity)                                     \
        {                                                                      \
            _typename##__extend(self, p_rhs->size);                            \
        }                                                                      \
        cnt = p_rhs->size;                                                     \
        dst_p = self->_body;                                                   \
        src_p = p_rhs->_body;                                                  \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            *dst_p++ = *src_p++;                                               \
        }                                                                      \
        *dst_p++ = (_element_type)0;                                           \
        self->size = p_rhs->size;                                              \
        return self;                                                           \
    }                                                                          \
    _typename* _typename##_assign_c_str(_typename* self, const char* p_rhs)    \
    {                                                                          \
        _element_type* dst_p;                                                  \
        const char* src_p;                                                     \
        ec_basic_string_size_type rhs_len =                                    \
                                      (ec_basic_string_size_type)ec_strlen(    \
                                          p_rhs),                              \
                                  cnt;                                         \
        if (rhs_len >= self->capacity)                                         \
        {                                                                      \
            _typename##__extend(self, rhs_len);                                \
        }                                                                      \
        dst_p = self->_body;                                                   \
        src_p = p_rhs;                                                         \
        cnt = rhs_len;                                                         \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            *dst_p++ = (_element_type)(*src_p);                                \
            src_p++;                                                           \
        }                                                                      \
        *dst_p++ = (_element_type)0;                                           \
        self->size = rhs_len;                                                  \
        return self;                                                           \
    }                                                                          \
    _typename* _typename##_new_assign_c_str(const char* s)                     \
    {                                                                          \
        _typename* p = _typename##_new();                                      \
        return _typename##_assign_c_str(p, s);                                 \
    }                                                                          \
    ec_basic_string_size_type _typename##_size(const _typename* self)          \
    {                                                                          \
        return self->size;                                                     \
    }                                                                          \
    ec_basic_string_size_type _typename##_length(const _typename* self)        \
    {                                                                          \
        return self->size;                                                     \
    }                                                                          \
    ec_basic_string_size_type _typename##_capacity(const _typename* self)      \
    {                                                                          \
        return self->capacity;                                                 \
    }                                                                          \
    void _typename##_clear(_typename* self)                                    \
    {                                                                          \
        ec_free(self->_body);                                                  \
        if ((self->_body = (_element_type*)ec_malloc(                          \
                 sizeof(_element_type) *                                       \
                 (EC_BASIC_STRING_INITIAL_LENGTH + 1))) == NULL)               \
        {                                                                      \
            return;                                                            \
        }                                                                      \
        self->size = 0;                                                        \
        self->capacity = EC_BASIC_STRING_INITIAL_LENGTH;                       \
    }                                                                          \
    ec_bool _typename##_empty(_typename* self)                                 \
    {                                                                          \
        return self->size == 0 ? ec_true : ec_false;                           \
    }                                                                          \
    _element_type* _typename##_c_str(const _typename* self)                    \
    {                                                                          \
        return self->_body;                                                    \
    }                                                                          \
    _element_type _typename##_at(const _typename* self,                        \
                                 const ec_basic_string_size_type n)            \
    {                                                                          \
        return self->_body[n];                                                 \
    }                                                                          \
    int _typename##_cmp(_typename* s1, _typename* s2)                          \
    {                                                                          \
        _element_type *p1 = s1->_body, *p2 = s2->_body;                        \
        ec_basic_string_size_type len1 = s1->size, len2 = s2->size;            \
        while (len1 != 0 && len2 != 0 && *p1 == *p2)                           \
        {                                                                      \
            len1--;                                                            \
            len2--;                                                            \
            p1++;                                                              \
            p2++;                                                              \
        }                                                                      \
        if (*p1 < *p2)                                                         \
            return -1;                                                         \
        else if (*p1 > *p2)                                                    \
            return 1;                                                          \
        return 0;                                                              \
    }                                                                          \
    ec_bool _typename##_eq(_typename* s1, _typename* s2)                       \
    {                                                                          \
        _element_type *p1 = s1->_body, *p2 = s2->_body;                        \
        ec_basic_string_size_type len = s1->size;                              \
        if (s1->size != s2->size)                                              \
            return ec_false;                                                   \
        while (len-- != 0)                                                     \
        {                                                                      \
            if (*p1 != *p2)                                                    \
                return ec_false;                                               \
            p1++;                                                              \
            p2++;                                                              \
        }                                                                      \
        return ec_true;                                                        \
    }                                                                          \
    _typename* _typename##_append(_typename* self, _typename* p_rhs)           \
    {                                                                          \
        ec_basic_string_size_type new_size =                                   \
            _typename##_length(self) + _typename##_length(p_rhs);              \
        int cnt;                                                               \
        _element_type *dst_p, *src_p;                                          \
        if (new_size >= self->capacity)                                        \
        {                                                                      \
            _typename##__extend(self, new_size);                               \
        }                                                                      \
        cnt = (int)p_rhs->size;                                                \
        dst_p = self->_body + self->size;                                      \
        src_p = p_rhs->_body;                                                  \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            *dst_p++ = *src_p++;                                               \
        }                                                                      \
        *dst_p++ = (_element_type)0;                                           \
        self->size = new_size;                                                 \
        return self;                                                           \
    }                                                                          \
    void _typename##_push_back(_typename* self, _element_type data)            \
    {                                                                          \
        _element_type* dst_p;                                                  \
        if (self->size + 1 >= self->capacity)                                  \
        {                                                                      \
            _typename##__extend(self, self->size + 1);                         \
        }                                                                      \
        dst_p = self->_body + self->size;                                      \
        *dst_p++ = data;                                                       \
        *dst_p++ = (_element_type)0;                                           \
        self->size++;                                                          \
    }                                                                          \
    void _typename##_push_front(_typename* self, _element_type data)           \
    {                                                                          \
        _element_type* dst_p;                                                  \
        ec_basic_string_size_type cnt;                                         \
        if (self->size + 1 >= self->capacity)                                  \
        {                                                                      \
            _typename##__extend(self, self->size + 1);                         \
        }                                                                      \
        cnt = self->size;                                                      \
        dst_p = self->_body + self->size;                                      \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            *dst_p = *(dst_p - 1);                                             \
            dst_p--;                                                           \
        }                                                                      \
        self->_body[0] = data;                                                 \
        self->size++;                                                          \
        self->_body[self->size] = (_element_type)0;                            \
    }                                                                          \
    ec_basic_string_size_type _typename##_find(                                \
        _typename* self, _element_type ch, ec_basic_string_size_type pos)      \
    {                                                                          \
        _element_type* p = self->_body + pos;                                  \
        ec_basic_string_size_type cnt = self->size - pos;                      \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            if (*p == ch)                                                      \
            {                                                                  \
                return pos;                                                    \
            }                                                                  \
            pos++;                                                             \
            p++;                                                               \
        }                                                                      \
        return ec_basic_string_npos;                                           \
    }                                                                          \
    ec_basic_string_size_type _typename##_rfind(                               \
        _typename* self, _element_type ch, ec_basic_string_size_type pos)      \
    {                                                                          \
        _element_type* p = self->_body + pos;                                  \
        ec_basic_string_size_type cnt = pos + 1;                               \
        while (cnt-- != 0)                                                     \
        {                                                                      \
            if (*p == ch)                                                      \
            {                                                                  \
                return pos;                                                    \
            }                                                                  \
            pos--;                                                             \
            p--;                                                               \
        }                                                                      \
        return ec_basic_string_npos;                                           \
    }                                                                          \
    _typename* _typename##_substr(_typename* self,                             \
                                  ec_basic_string_size_type pos,               \
                                  ec_basic_string_size_type count)             \
    {                                                                          \
        _typename* new_str = _typename##_new();                                \
        while ((count-- > 0) && (pos < self->size))                            \
            _typename##_push_back(new_str, _typename##_at(self, pos++));       \
        return new_str;                                                        \
    }                                                                          \
    _typename* _typename##_new(void)

#ifndef ECT_BASIC_STRING_GENERIC_INTERFACE
#define ECT_BASIC_STRING_GENERIC_INTERFACE
#define ect_basic_string_new(_typename) _typename##_new()
#endif

#endif
