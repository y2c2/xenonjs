/* XenonJS : Compiler Driver : Utils
 * Copyright(c) 2017-2018 y2c2 */

#ifndef UTILS_H
#define UTILS_H

#include "platform.h"
#include <stdio.h>

int read_file( \
        char **data_out, size_t *len_out, \
        const char *filename);

int write_file( \
        char *data, size_t len, \
        const char *filename);

int list_dir( \
        char ***filename, \
        int *filenames_count, \
        const char *dirpath);

int fork_path( \
        char *buf, const size_t buf_len, \
        const char *new_ext, \
        const char *filename);

char *realpath_get(const char *pathname);

int dirname_get(const char **dirname_p, size_t *dirname_len, const char *pathname, const size_t pathname_len);
int basename_get(const char **basename_p, size_t *basename_len, const char *pathname, const size_t pathname_len);
int extension_get(const char **extension_p, size_t *extension_len, const char *pathname, const size_t pathname_len);
int mainname_get(const char **mainname_p, size_t *mainname_len, const char *pathname, const size_t pathname_len);

#ifndef PATH_MAX
#define PATH_MAX (512)
#endif

#ifndef OS_SEP
#if defined(PLATFORM_WINDOWS)
#define OS_SEP '\\'
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
#define OS_SEP '/'
#endif
#endif

/* Find the path of current program */
int current_program_path(char *buf, const size_t buf_size);

/* Find the path of current working directory */
int current_working_path(char *buf, const size_t buf_size);

#endif

