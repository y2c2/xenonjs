#include <ec_alloc.h>
#include "sourcelist.h"

static void xjs_source_ctor(void *data)
{
    xjc_source_ref r = data;
    r->filetype = xjc_filetype_auto;
    r->source_path = NULL;
}

static void xjs_source_dtor(void *data)
{
    xjc_source_ref r = data;
    (void)r;
}

xjc_source_ref xjc_source_new(const char *source_path, const xjc_filetype filetype)
{
    xjc_source_ref r;
    if ((r = ec_newcd(xjc_source, \
                    xjs_source_ctor, xjs_source_dtor)) == NULL) { return NULL; }
    r->source_path = source_path;
    r->filetype = filetype;
    return r;
}

static void sourcelist_node_dtor(xjc_source *node)
{
    ec_delete(node);
}

ect_list_define_declared(sourcelist, xjc_source *, sourcelist_node_dtor);

