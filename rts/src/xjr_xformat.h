/* XenonJS : Runtime Time System : X Format
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_XFORMAT_H
#define XJR_XFORMAT_H

/* Opcode */

#define XJR_BC_OP_NOP 0x00
#define XJR_BC_OP_HALT 0x01
#define XJR_BC_OP_DEBUG 0x02
#define XJR_BC_OP_INSPECT 0x03
#define XJR_BC_OP_DYNLIB 0x04

#define XJR_BC_OP_BR 0x10
#define XJR_BC_OP_BRC 0x11
#define XJR_BC_OP_MERGE 0x12
#define XJR_BC_OP_RET 0x13
#define XJR_BC_OP_CALL 0x14
#define XJR_BC_OP_CALLT 0x15
#define XJR_BC_OP_NEW 0x16
#define XJR_BC_OP_THIS 0x17
#define XJR_BC_OP_ALLOCREG 0x18
#define XJR_BC_OP_ARG 0x19
#define XJR_BC_OP_DECLVAR 0x1A
#define XJR_BC_OP_LOAD 0x1B
#define XJR_BC_OP_STORE 0x1C
#define XJR_BC_OP_LOAD_UNDEFINED 0x1D
#define XJR_BC_OP_LOAD_NULL 0x1E
#define XJR_BC_OP_LOAD_FALSE 0x1F
#define XJR_BC_OP_LOAD_TRUE 0x20
#define XJR_BC_OP_LOAD_STRING 0x21
#define XJR_BC_OP_LOAD_STRING_EMPTY 0x22
#define XJR_BC_OP_LOAD_NUMBER_S8 0x23
#define XJR_BC_OP_LOAD_NUMBER_S16 0x24
#define XJR_BC_OP_LOAD_NUMBER_S32 0x25
#define XJR_BC_OP_LOAD_NUMBER_U8 0x26
#define XJR_BC_OP_LOAD_NUMBER_U16 0x27
#define XJR_BC_OP_LOAD_NUMBER_U32 0x28
#define XJR_BC_OP_LOAD_NUMBER_F32 0x29
#define XJR_BC_OP_LOAD_NUMBER_F64 0x2A
#define XJR_BC_OP_LOAD_FUNCTION 0x2B
#define XJR_BC_OP_LOAD_ARROWFUNCTION 0x2C
#define XJR_BC_OP_LOAD_OBJECT 0x2D
#define XJR_BC_OP_LOAD_ARRAY 0x2E

/* #define XJR_BC_OP_LOAD_NUMBER_0 0x20 */
/* #define XJR_BC_OP_LOAD_NUMBER_1 0x21 */
/* #define XJR_BC_OP_LOAD_NUMBER_2 0x22 */

#define XJR_BC_OP_BINADD 0x30
#define XJR_BC_OP_BINSUB 0x31
#define XJR_BC_OP_BINMUL 0x32
#define XJR_BC_OP_BINDIV 0x33
#define XJR_BC_OP_BINMOD 0x34
#define XJR_BC_OP_BINE2 0x35
#define XJR_BC_OP_BINNE2 0x36
#define XJR_BC_OP_BINE3 0x37
#define XJR_BC_OP_BINNE3 0x38
#define XJR_BC_OP_BINL 0x39
#define XJR_BC_OP_BINLE 0x3A
#define XJR_BC_OP_BING 0x3B
#define XJR_BC_OP_BINGE 0x3C
#define XJR_BC_OP_BINAND 0x3D
#define XJR_BC_OP_BINOR 0x3E

#define XJR_BC_OP_UNADD 0x40
#define XJR_BC_OP_UNSUB 0x41
#define XJR_BC_OP_UNNOT 0x42
#define XJR_BC_OP_UNBNOT 0x43

#define XJR_BC_OP_OBJSET 0x50
#define XJR_BC_OP_OBJGET 0x51
#define XJR_BC_OP_ARRPUSH 0x52

#define XJR_BC_SECTION_SIGNATURE_LEN (4)
#define XJR_BC_SIZE_HEADER (8)
#define XJR_BC_SIZE_OFFSET_COUNT (4)
#define XJR_BC_SIZE_ADDR (4)
#define XJR_BC_ARGC_MAX (16)
#define XJR_BC_DATASEC_ITEM_TYPE_UTF8STR (1)
#define XJR_BC_DATASEC_ITEM_TYPE_UTF8SYM (2)
#define XJR_BC_ATTR_TYPE_TOPLEVEL (1)

#endif
