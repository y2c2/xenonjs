#ifndef SOURCELIST_H
#define SOURCELIST_H

#include <ec_string.h>
#include <ec_list.h>

typedef enum
{
    xjc_filetype_auto,
    xjc_filetype_js,
    xjc_filetype_mjs,
    xjc_filetype_x,
} xjc_filetype;

typedef struct
{
    const char *source_path;
    xjc_filetype filetype;
} xjc_source;

typedef xjc_source *xjc_source_ref;

xjc_source_ref xjc_source_new(const char *source_path, const xjc_filetype filetype);

ect_list_declare(sourcelist, xjc_source *);
typedef sourcelist *sourcelist_ref;

#endif

