/* XenonJS : Runtime Time System : VM
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_VM_H
#define XJR_VM_H

#include "xjr_dt.h"
#include "xjr_vm_dt.h"


/* Initialize */
int xjr_vm_init(xjr_vm *vm);
int xjr_vm_uninit(xjr_vm *vm);

/* Set options */
void xjr_vm_set_options(xjr_vm *vm, xjr_u32 options);

/* Distribution Data */
void *xjr_vm_get_distribution_data(xjr_vm *vm);
void xjr_vm_set_distribution_data(xjr_vm *vm, void *data);

/* Setup */ 
void xjr_vm_setup_stack(xjr_vm *vm, void *data, xjr_size_t size);
int xjr_vm_setup_heap(xjr_vm *vm, \
        void *data, \
        xjr_heap_malloc_callback cb_malloc, \
        xjr_heap_free_callback cb_free);
void xjr_vm_setup_write(xjr_vm *vm, \
        xjr_inspect_write_callback cb_write);

/* Memory Record callbacks */ 
void xjr_vm_set_malloc_callback(xjr_vm *vm, xjr_heap_malloc_record_callback cb);
void xjr_vm_set_free_callback(xjr_vm *vm, xjr_heap_free_record_callback cb);

/* Extern */
xjr_extn *xjr_vm_extn(xjr_vm *vm);

/* Libraries */
int xjr_vm_lib_install(xjr_vm *vm, const char *module_name, xjr_lib_install_cb cb);
/* Debugger */
void xjr_vm_debugger_install(xjr_vm *vm, xjr_debugger_cb cb, void *stub);
/* Crash Handler */
void xjr_vm_crash_handler_install(xjr_vm *vm, xjr_crash_cb cb);

/* Get stub from debugger context */
void *xjr_debugger_stub_get(xjr_debugger_ctx *ctx);

/* Load bytecode */
int xjr_vm_load(xjr_vm *vm, char *bytecode, xjr_size_t bytecode_len);

/* Initialize stack */
void xjr_vm_stack_init_on_boot(xjr_vm *vm, xjr_urid_t env, xjr_val global);
int xjr_vm_stack_init_on_call(xjr_vm *vm, xjr_urid_t env, \
        xjr_val callee, \
        xjr_val _this, \
        const int argc, \
        xjr_val *argv);

/* Boot VM */
int xjr_vm_boot(xjr_vm *vm);

/* Start executing */
int xjr_vm_step(xjr_vm *vm, int *to_halt);

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


#endif

