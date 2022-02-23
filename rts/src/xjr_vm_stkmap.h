/* XenonJS : Runtime Time System : Stack Map
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_VMSTKMAP_H
#define XJR_VMSTKMAP_H

/*
 * Stack Layout
 *
 * sp (Stack Point): Point to the data block that contains the
 * 0. call type : u8
 * 1. restore point : u32
 * 2. environment : u16
 * 3. dst : reg
 * 4. callee : val
 * 5. this : val
 * 6. arguments count
 * 7. vregs count
 * 8. arguments : array of val
 * 9. vregs : array of val
 *
 * sdp (Stack Data Pointer): Base address of the stack frame block
 *
 * ------------ (high)
 * [sdp1]
 * [sdp2]
 * ...
 * [sdpn] <--- sp
 * 
 *
 * <--- sdpn
 * ...
 * ------------ (low)
 */

#define XJR_OFFSET_SHIFT_CALLTYPE (0)
#define XJR_OFFSET_SHIFT_RESTORE_POINT (1)
#define XJR_OFFSET_SHIFT_ENV (5)
#define XJR_OFFSET_SHIFT_DST (7)
#define XJR_OFFSET_SHIFT_CALLEE (8)
#define XJR_OFFSET_SHIFT_THIS (12)
#define XJR_OFFSET_SHIFT_ARGC (16)
#define XJR_OFFSET_SHIFT_VREGC (17)
#define XJR_OFFSET_SHIFT_ARGV (18)

#define XJR_CALLTYPE_CALL (0)
#define XJR_CALLTYPE_NEW (1)

#define XJR_VM_GET_FROM_STACK(_vm, _shift, _type) \
    ((_type)(*((_type *)(&((xjr_u8 *)_vm->rts.rstack.data)[(_vm)->rts.rstack.sdp + (_shift)]))))

#define XJR_VM_ADDR_FROM_STACK(_vm, _shift, _type) \
    ((_type *)(&((xjr_u8 *)_vm->rts.rstack.data)[(_vm)->rts.rstack.sdp + (_shift)]))

#define XJR_VM_SET_TO_STACK(_vm, _shift, _type, _v) \
    do { \
        ((*((_type *)(&((xjr_u8 *)_vm->rts.rstack.data)[(_vm)->rts.rstack.sdp + (_shift)]))) = (_type)(_v)); \
    } while (0)


#define XJR_VM_GET_CALLTYPE(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_CALLTYPE, xjr_u8))
#define XJR_VM_SET_CALLTYPE(_vm, _calltype) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_CALLTYPE, xjr_u8, _calltype)

#define XJR_VM_GET_RESTORE_POINT(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_RESTORE_POINT, xjr_u32))
#define XJR_VM_SET_RESTORE_POINT(_vm, _offset) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_RESTORE_POINT, xjr_u32, _offset)

#define XJR_VM_GET_ENV(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_ENV, xjr_u16))
#define XJR_VM_SET_ENV(_vm, _env) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_ENV, xjr_u16, _env)

#define XJR_VM_GET_DST(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_DST, xjr_u8))
#define XJR_VM_SET_DST(_vm, _dst) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_DST, xjr_u8, _dst)

/* callee */
#define XJR_VM_GET_CALLEE(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_CALLEE, xjr_val))
#define XJR_VM_SET_CALLEE(_vm, _callee) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_CALLEE, xjr_val, _callee)

/* this */
#define XJR_VM_GET_THIS(_vm) \
    (XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_THIS, xjr_val))
#define XJR_VM_SET_THIS(_vm, _this) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_THIS, xjr_val, _this)

/* arguments */
#define XJR_VM_GET_ARGC(_vm) \
    ((xjr_size_t)XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_ARGC, xjr_u8))
#define XJR_VM_SET_ARGC(_vm, _argc) \
    XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_ARGC, xjr_u8, _argc)

#define XJR_VM_GET_ARGV(_vm, _idx) \
    ((_idx < XJR_VM_GET_ARGC(vm)) ? \
     ((xjr_val)XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_ARGV + \
         (_idx) * sizeof(xjr_val), xjr_val)) : \
         XJR_VAL_MAKE_UNDEFINED())
#define XJR_VM_SET_ARGV(_vm, _idx, _val) \
    do { \
        if (_idx < XJR_VM_GET_ARGC(vm)) { \
            XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_ARGV + \
             (_idx) * sizeof(xjr_val), \
             xjr_val, _val); \
        } \
    } while (0)

/* vregc */
#define XJR_VM_GET_VREGC(_vm) \
    ((xjr_size_t)XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_VREGC, xjr_u8))
#define XJR_VM_SET_VREGC(_vm, _vregc) \
    do { \
        XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_VREGC, xjr_u8, _vregc); \
    } while (0)

/* vregv */
#define XJR_VM_GET_VREGV(_vm, _r) \
    (((_r) < XJR_VM_GET_VREGC(_vm)) ? \
     ((xjr_val)XJR_VM_GET_FROM_STACK(_vm, XJR_OFFSET_SHIFT_ARGV + \
         XJR_VM_GET_ARGC(vm) * sizeof(xjr_val) + (_r) * sizeof(xjr_val), xjr_val)) : \
         XJR_VAL_MAKE_UNDEFINED())
#define XJR_VM_SET_VREGV(_vm, _r, _data) \
    do { if ((_r) < XJR_VM_GET_VREGC(_vm)) { \
        XJR_VM_SET_TO_STACK(_vm, XJR_OFFSET_SHIFT_ARGV + \
            XJR_VM_GET_ARGC(vm) * sizeof(xjr_val) + (_r) * sizeof(xjr_val), xjr_val, \
            _data); } \
    } while (0)

/* Stack Frame Size */
#define XJR_VM_GET_SF_SIZE(_vm) \
    (XJR_OFFSET_SHIFT_ARGV + \
     XJR_VM_GET_ARGC(vm) * sizeof(xjr_val) + \
     XJR_VM_GET_VREGC(vm) * sizeof(xjr_val))
#define XJR_VM_STACK_INCREASE_SP(_vm) \
    do { \
        xjr_u8 *stk = _vm->rts.rstack.data; \
        xjr_offset_t *sp_ptr = (xjr_offset_t *)(stk + _vm->rts.rstack.sp); \
        sp_ptr--; \
        *sp_ptr = (xjr_offset_t)_vm->rts.rstack.sdp; \
        _vm->rts.rstack.sp -= sizeof(xjr_offset_t); \
    } while (0)
#define XJR_VM_STACK_DECREASE_SP(_vm) \
    do { \
        xjr_u8 *stk = vm->rts.rstack.data; \
        xjr_offset_t *sp_ptr = (xjr_offset_t *)(stk + vm->rts.rstack.sp); \
        vm->rts.rstack.sdp = *sp_ptr; \
        sp_ptr++; \
        vm->rts.rstack.sp += sizeof(xjr_offset_t); \
    } while (0)


#endif

