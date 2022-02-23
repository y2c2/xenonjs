/* XenonJS : Dependency Walk
 * Copyright(c) 2017 y2c2 */

#ifndef XJS_DEPWALK_H
#define XJS_DEPWALK_H

#include "xjs_deptbl.h"

int xjs_dependency_walk( \
        xjs_error_ref err, \
        xjs_dep_table_ref *deptbl_out, \
        xjs_dep_loaditem_list_ref pending_list, \
        const char *entry, \
        xjs_ir_ref ir_stdlib, \
        xjs_load_sys_lib_cb_t load_sys_lib_cb, \
        int generate_debug_info, \
        xjs_ir_dtor_pool_ref ir_dtor_pool);

#endif

