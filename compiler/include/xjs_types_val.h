/* XenonJS : Types : Value
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_TYPES_VAL_H
#define XJS_TYPES_VAL_H

#include "xjs_dt.h"

/* JavaScript String (16-bit, UTF16) */

/* Object */
struct xjs_opaque_val_object
{
    int dummy;
};
typedef struct xjs_opaque_val_object xjs_val_object;
typedef struct xjs_opaque_val_object *xjs_val_object_ref;


/* Value */
typedef enum
{
    xjs_val_type_unknown,
    xjs_val_type_undefined,
    xjs_val_type_null,
    xjs_val_type_boolean,
    xjs_val_type_string,
    xjs_val_type_number,
    xjs_val_type_object,
} xjs_val_type_t;

struct xjs_opaque_val
{
    xjs_val_type_t type;
    union
    {
        xjs_bool as_boolean;
        double as_number;
    } u;
};
typedef struct xjs_opaque_val xjs_val;
typedef struct xjs_opaque_val *xjs_val_ref;

/* Undefined */
/* Null */
/* Boolean */
/* String */
/* Number */
/* Object */

#endif

