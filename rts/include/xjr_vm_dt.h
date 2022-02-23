/* XenonJS : Runtime Time System : VM : Data Types
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_VM_DT_H
#define XJR_VM_DT_H

#include "xjr_dt.h"
#include "xjr_err.h"
#include "xjr_mp.h"
#include "xjr_x.h"
#include "xjr_inspect.h"
#include "xjr_extn.h"

#define XJR_VM_LIB_CAPACITY 32
#define XJR_VM_LIB_POOL_SIZE 255

#define XJR_VM_EXTERNAL_VAL_CAPACITY 32
#define XJR_VM_EXTERNAL_CALL_STACK_DEPTH_MAX 5

#define XJR_VM_OPTIONS_NOGC (1 << 0)
#define XJR_VM_OPTIONS_NOSTDLIB (1 << 1)

struct xjr_opaque_vm;
typedef struct xjr_opaque_vm xjr_vm;

struct xjr_opaque_lib_ctx;
typedef struct xjr_opaque_lib_ctx xjr_lib_ctx;

struct xjr_opaque_debugger_ctx;
typedef struct xjr_opaque_debugger_ctx xjr_debugger_ctx;

struct xjr_opaque_crash_ctx;
typedef struct xjr_opaque_crash_ctx xjr_crash_ctx;

typedef int (*xjr_lib_install_cb)(xjr_lib_ctx *ctx);
typedef int (*xjr_debugger_cb)(xjr_debugger_ctx *ctx);
typedef int (*xjr_crash_cb)(xjr_crash_ctx *ctx);

typedef xjr_bool (*xjr_gc_trigger_before_cb)(xjr_vm *vm, void *trigger_stub);
typedef void (*xjr_gc_trigger_after_cb)(xjr_vm *vm, void *trigger_stub);

struct xjr_opaque_lib_ctx
{
    xjr_vm *vm;
    xjr_urid_t env;
    xjr_val exports;
    xjr_mp_t *mp;
};

struct xjr_opaque_debugger_ctx
{
    xjr_vm *vm;
    void *stub;

    xjr_bool step;
};

struct xjr_opaque_crash_ctx
{
    xjr_vm *vm;
};

typedef struct
{
    char *name;
    xjr_lib_install_cb cb;
} xjr_vm_lib;

struct xjr_opaque_vm
{
    xjr_xfile xf;

    xjr_u32 opts;

    /* Runtime System */
    struct
    {
        /* Program Counter */
        xjr_offset_t pc;
        /* Running Stack */
        struct
        {
            void *data;
            xjr_size_t size;
            xjr_offset_t sp;
            xjr_offset_t sdp;
        } rstack;
        /* Heap */
        struct
        {
            /* Memory Pool */
            xjr_mp_t *mp;

            void *data;
            xjr_heap_malloc_callback cb_malloc;
            xjr_heap_free_callback cb_free;
        } rheap;
    } rts;

    struct
    {
        xjr_inspect_write_callback cb_write;
    } inspect;

    /* Error */
    xjr_error err;

    struct
    {
        xjr_size_t step;
    } profile;

    /* External */
    xjr_extn extn;

    /* Libraries */
    struct
    {
        xjr_vm_lib libs[XJR_VM_LIB_CAPACITY];
        xjr_size_t count;

        char pool[XJR_VM_LIB_POOL_SIZE + 1];
        xjr_size_t pool_size;
        char *name_p;
    } libs;
    
    /* Debugger */
    struct
    {
        void *data;
        xjr_debugger_cb cb;
    } debugger;

    /* Crash Handler */
    struct
    {
        xjr_crash_cb cb;
    } crash;

    struct
    {
        xjr_val global_object_prototype;
        xjr_val global_function_prototype;
        xjr_val global_number_prototype;
        xjr_val global_string_prototype;
        xjr_val global_array_prototype;
        xjr_val global_uint8array_prototype;
    } fundamental;

    struct
    {
        void *trigger_stub;
        xjr_gc_trigger_before_cb before_cb;
        xjr_gc_trigger_after_cb after_cb;
    } gc;

    /* Distribution */
    void *distribution_data;

    /* Misc */
    struct
    {
        struct
        {
            xjr_val body[XJR_VM_EXTERNAL_VAL_CAPACITY];
            xjr_size_t refcount[XJR_VM_EXTERNAL_VAL_CAPACITY];
            xjr_size_t size;
        } external_vals;

        struct
        {
            struct
            {
                xjr_size_t sp;
                xjr_val *extern_call_dst;
            } frames[XJR_VM_EXTERNAL_CALL_STACK_DEPTH_MAX];
        } external_calls;
        xjr_size_t external_call_depth;
    } misc;

    /* Environment */
    xjr_urid_t env;
};

#endif
