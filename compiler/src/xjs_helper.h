/* XenonJS : Helper
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_HELPER_H
#define XJS_HELPER_H

#include "xjs_types_error.h"
#include "xjs_error.h"

/* Error Handling */

#ifndef XJS_VNZ_RET
#define XJS_VNZ_RET(x) \
    do { if ((x) == NULL) return NULL; } while (0)
#endif

#ifndef XJS_VNZ_RET_OR
#define XJS_VNZ_RET_OR(x, _or) \
    do { if ((x) == NULL) { _or; return NULL; } } while (0)
#endif

#ifndef XJS_V_ERROR
#define XJS_V_ERROR(x, _e, _errno) \
    do { if (!(x)) { XJS_ERROR(_e, _errno); goto fail; }; } while (0)
#endif

#ifndef XJS_VEZ_ERROR
#define XJS_VEZ_ERROR(x, _e, _errno) \
    do { if ((x) != 0) { XJS_ERROR(_e, _errno); goto fail; }; } while (0)
#endif

#ifndef XJS_VNZ_ERROR
#define XJS_VNZ_ERROR(x, _e, _errno) \
    do { if ((x) == NULL) { XJS_ERROR(_e, _errno); goto fail; }; } while (0)
#endif

#ifndef XJS_VNZ_ERROR_OR
#define XJS_VNZ_ERROR_OR(x, _e, _errno, _or) \
    do { if ((x) == NULL) { _or; XJS_ERROR(_e, _errno); goto fail; }; } while (0)
#endif

#ifndef XJS_VEZ_ERROR_OR
#define XJS_VEZ_ERROR_OR(x, _e, _errno, _or) \
    do { if ((x) != 0) { _or; XJS_ERROR(_e, _errno); goto fail; }; } while (0)
#endif

#define XJS_VNZ_ERROR_MEM(x, _e) XJS_VNZ_ERROR(x, _e, XJS_ERRNO_MEM)
#define XJS_VNZ_ERROR_MEM_OR(x, _e, _or) XJS_VNZ_ERROR_OR(x, _e, XJS_ERRNO_MEM, _or)
#define XJS_VEZ_ERROR_MEM(x, _e) XJS_VEZ_ERROR(x, _e, XJS_ERRNO_MEM)
#define XJS_VEZ_ERROR_MEM_OR(x, _e, _or) XJS_VEZ_ERROR_OR(x, _e, XJS_ERRNO_MEM, _or)
#define XJS_V_ERROR_INTERNAL(x, _e) XJS_V_ERROR(x, _e, XJS_ERRNO_INTERNAL)
#define XJS_VEZ_ERROR_INTERNAL(x, _e) XJS_VEZ_ERROR(x, _e, XJS_ERRNO_INTERNAL)
#define XJS_VEZ_ERROR_INTERNAL_OR(x, _e, _or) XJS_VEZ_ERROR_OR(x, _e, XJS_ERRNO_INTERNAL, _or)
#define XJS_VNZ_ERROR_INTERNAL(x, _e) XJS_VNZ_ERROR(x, _e, XJS_ERRNO_INTERNAL)
#define XJS_VNZ_ERROR_INTERNAL_OR(x, _e, _or) XJS_VNZ_ERROR_OR(x, _e, XJS_ERRNO_INTERNAL, _or)
#define XJS_ERROR_NOTIMP(_e) XJS_VNZ_ERROR(NULL, _e, XJS_ERRNO_NOTIMP)
#define XJS_ERROR_INTERNAL(_e) XJS_VNZ_ERROR(NULL, _e, XJS_ERRNO_INTERNAL)

/* Stringify an identify */
#define XJS_ID_STRINGIFY_IN(x) #x
#define XJS_ID_STRINGIFY(x) XJS_ID_STRINGIFY_IN(x)


#endif


