/* XenonJS : Runtime Time System
 * Copyright(c) 2017 y2c2 */

/* Compile with your project */

#ifndef XJR_H
#define XJR_H

#include <stdarg.h>
#include "xjr_dt.h"
#include "xjr_vm_dt.h"

struct xjr_opaque_vm;
typedef struct xjr_opaque_vm xjr_vm;

struct xjr_opaque_debugger_ctx;
typedef struct xjr_opaque_debugger_ctx xjr_debugger_ctx;

typedef xjr_bool (*xjr_gc_trigger_before_cb)(xjr_vm *vm, void *trigger_stub);
typedef void (*xjr_gc_trigger_after_cb)(xjr_vm *vm, void *trigger_stub);

/* Initialize */
int xjr_vm_init(xjr_vm *vm);
int xjr_vm_uninit(xjr_vm *vm);

/* Set options */
void xjr_vm_set_options(xjr_vm *vm, xjr_u32 options);

/* Setup */ 
void xjr_vm_setup_stack(xjr_vm *vm, void *data, xjr_size_t size);
int xjr_vm_setup_heap(xjr_vm *vm, \
        void *data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free);
void xjr_vm_setup_write(xjr_vm *vm, \
        xjr_inspect_write_callback cb_write);

/* Extern */
xjr_extn *xjr_vm_extn(xjr_vm *vm);

/* Others */
int xjr_vm_lib_install(xjr_vm *vm, const char *module_name, xjr_lib_install_cb cb);
void xjr_vm_debugger_install(xjr_vm *vm, xjr_debugger_cb cb, void *stub);
void xjr_vm_crash_handler_install(xjr_vm *vm, xjr_crash_cb cb);

/* Load bytecode */
int xjr_vm_load(xjr_vm *vm, char *bytecode, xjr_size_t bytecode_len);

/* Boot VM */
int xjr_vm_boot(xjr_vm *vm);

/* Start executing */
int xjr_vm_start(xjr_vm *vm);

/* Get stub from debugger context */
void *xjr_debugger_stub_get(xjr_debugger_ctx *ctx);

/* Call function from external */
int xjr_vm_call(xjr_vm *vm, \
        xjr_val *v_dst, \
        xjr_val v_callee, \
        xjr_val v_this, \
        const int argc, \
        xjr_val *argv);
int xjr_vm_call2(xjr_vm *vm, \
        void *heap_data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free, \
        xjr_val *v_dst, \
        xjr_val v_callee, \
        xjr_val v_this, \
        const int argc, \
        ...);

/* Register external objects */
int xjr_vm_register_external_val(xjr_vm *vm, xjr_val v);
/* Unregister external objects */
int xjr_vm_unregister_external_val(xjr_vm *vm, xjr_val v);

/* GC */
void xjr_vm_set_gc_step_trigger(xjr_vm *vm, \
        void *trigger_stub, \
        xjr_gc_trigger_before_cb before_cb, \
        xjr_gc_trigger_after_cb after_cb);


#endif

