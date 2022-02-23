/* XenonJS : Runtime Time System : Error
 * Copyright(c) 2017 y2c2 */

#ifndef XJR_ERR_H
#define XJR_ERR_H

enum
{
    XJR_ERR_OK = 0,
    XJR_ERR_INTERNAL = 1, /* Internal error */
    XJR_ERR_OOM = 2, /* Out of memory */
    XJR_ERR_IO = 3, /* IO error */
    XJR_ERR_SOF = 4, /* Stack overflow */
    XJR_ERR_XCORRUPT = 5, /* X file corrupt */
    XJR_ERR_INT_OUT_OF_BOUND = 6, /* integer out of bound */
    XJR_ERR_INVALID_OP = 7, /* Invalid opcode */
    XJR_ERR_INVALID_TYPE = 8, /* Invalid operand type */
    XJR_ERR_ARGC_EXCEED = 9, /* Too many arguments */
    XJR_ERR_NOT_DEFINED = 10, /* variable not defined */
    XJR_ERR_NOT_CONSTRUCTOR = 11, /* Not constructor */
    XJR_ERR_MEM_CORRUPTED = 12, /* Memory corrupt */
};

typedef struct
{
    int code;
    union
    {
        struct { char *name; int len; } as_not_defined;
        struct { int urid; } as_mem_corrupted;
    } u;

    /* in .x file */
    struct
    {
        int opcode;
        int pc;
    } rts;

    /* in xjr implementation */
    struct
    {
        char *filename;
        int ln;
    } loc;
} xjr_error;

/* Error */

#define XJR_ERR_UPDATE(_err, _code) \
    do { \
        (_err)->code = _code; \
        (_err)->loc.filename = __FILE__; \
        (_err)->loc.ln = __LINE__; \
    } while (0)

#endif

