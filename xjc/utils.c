/* XenonJS : Compiler Driver : Utils
 * Copyright(c) 2017-2018 y2c2 */

#include "platform.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#elif defined(PLATFORM_FREEBSD)
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/sysctl.h>
#elif defined(PLATFORM_MACOS)
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <mach-o/dyld.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

int read_file( \
        char **data_out, size_t *len_out, \
        const char *filename)
{
    FILE *fp;
    char *data;
    long len;
    int from_terminal = ((strlen(filename) == 1) && (strncmp(filename, "-", 1) == 0));

    if (from_terminal)
    {
        fp = stdin;
    }
    else
    {
        if ((fp = fopen(filename, "rb")) == NULL)
        { return -1; }
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if ((data = (char *)malloc(sizeof(char) * ((size_t)len + 1))) == NULL)
    { 
        if (!from_terminal) fclose(fp);
        return -1;
    }
    fread(data, (size_t)len, 1, fp);
    data[len] = '\0';

    *data_out = data;
    *len_out = (size_t)len;

    if (!from_terminal) fclose(fp);

    return 0;
}

int write_file( \
        char *data, size_t len, \
        const char *filename)
{
    if ((filename == NULL) || \
            ((strlen(filename) == 1) && (strncmp(filename, "-", 1) == 0)))
    {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
        fwrite(data, len, 1, stdout);
#elif defined(PLATFORM_WINDOWS)
        HANDLE handle;
        DWORD i;
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (handle == INVALID_HANDLE_VALUE)
        { return -1; }
        if (!WriteConsoleA(handle, data, len, &i, NULL))
        { return -1; }
#endif
    }
    else
    {
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
        FILE *fp;
        if ((fp = fopen(filename, "wb+")) == NULL)
        { return -1; }
        fwrite(data, len, 1, fp);
        fclose(fp);
#elif defined(PLATFORM_WINDOWS)
        HANDLE handle;
        DWORD i;
        handle = CreateFile(filename, \
                    GENERIC_WRITE,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
        if (!WriteFile(handle, data, len, &i, NULL))
        {
            CloseHandle(handle);
            return -1;
        }
        CloseHandle(handle);
#endif
    }

    return 0;
}


#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
int list_dir( \
        char ***filenames_out, \
        int *filenames_count_out, \
        const char *dirpath)
{
    int ret = 0;
    DIR *d = NULL;
    struct dirent *dp;
    int count = 0, idx = 0;
    char **filenames = NULL;

    if ((d = opendir(dirpath)) == NULL) { return -1; }
    while ((dp = readdir(d)) != NULL)
    {
        char fullpath[PATH_MAX];
        struct stat st;
        snprintf(fullpath, PATH_MAX, "%s%c%s", dirpath, '/', dp->d_name);
        if (lstat(fullpath, &st) != 0) continue;
        if (!(st.st_mode & S_IFMT)) continue;
        if (!(st.st_mode & S_IFREG)) continue;
        count++;
    }
    closedir(d); d = NULL;

    if (count == 0)
    {
        goto fail;
    }

    if ((filenames = (char **)malloc(sizeof(char *) * (size_t)count)) == NULL)
    { goto fail; }

    if ((d = opendir(dirpath)) == NULL) { goto fail; } 
    while ((dp = readdir(d)) != NULL)
    {
        char fullpath[PATH_MAX];
        struct stat st;
        snprintf(fullpath, PATH_MAX, "%s%c%s", dirpath, '/', dp->d_name);
        if (lstat(fullpath, &st) != 0) continue;
        if (!(st.st_mode & S_IFMT)) continue;
        if (!(st.st_mode & S_IFREG)) continue;

        {
            size_t name_len;
            name_len = strlen(dp->d_name);
            if ((filenames[idx] = (char *)malloc(sizeof(char) * (name_len + 1))) == NULL)
            {
                idx--;
                while (idx >= 0) { free(filenames[idx]); idx--; }
                goto fail;
            }
            memcpy(filenames[idx], dp->d_name, name_len);
            filenames[idx][name_len] = '\0';
        }

        idx++;
    }
    closedir(d); d = NULL;

    *filenames_out = filenames; filenames = NULL;
    *filenames_count_out = count;

    goto done;
fail:
    ret = -1;
done:
    if (d != NULL) closedir(d);
    if (filenames != NULL) free(filenames);
    return ret;
}
#elif defined(PLATFORM_WINDOWS)
int list_dir( \
        char ***filenames_out, \
        int *filenames_count_out, \
        const char *dirpath)
{
    (void)filenames_out;
    (void)filenames_count_out;
    (void)dirpath;
    return -1;
}
#endif

int fork_path( \
        char *buf, const size_t buf_len, \
        const char *new_ext, \
        const char *filename)
{
    const char *p_dot;
    size_t base_len, new_ext_len; 

    if (filename == NULL) return -1;
    if (strlen(filename) == 0) return -1;
    new_ext_len = strlen(new_ext);
    p_dot = strrchr(filename, '.');
    if (p_dot == NULL)
    {
        base_len = strlen(filename);
    }
    else
    {
        base_len = (size_t)(p_dot - filename);
    }
    if (base_len + 1 + new_ext_len >= buf_len) { return -1; }
    memcpy(buf, filename, base_len);
    buf[base_len] = '.';
    memcpy(buf + base_len + 1, new_ext, new_ext_len);
    buf[base_len + 1 + new_ext_len] = '\0';
    return 0;
}

char *realpath_get(const char *pathname)
{
#if defined(PLATFORM_WINDOWS)
    char buffer[512];
    char *out;
    DWORD len = GetFullPathName(pathname, 512, buffer, NULL);
    if (len == 0 && len > 512) return NULL;
    out = (char *)malloc(sizeof(char) * (len + 1));
    if (out == NULL) return NULL;
    memcpy(out, buffer, len);
    out[len] = '\0';
    return out;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return realpath(pathname, NULL);
#endif
}

/* Extract directory part of a path 
 * "/lib/libc.so" -> "/lib/" */
int dirname_get(const char **dirname_p, size_t *dirname_len, const char *pathname, const size_t pathname_len)
{
    const char *pathname_p;

    *dirname_p = NULL;
    *dirname_len = 0;

    if (pathname_len == 0) return 0;

    pathname_p = pathname + pathname_len;
    do
    {
        pathname_p--;
        if ((*pathname_p == '/') || (*pathname_p == '\\'))
        {
            *dirname_p = pathname;
            *dirname_len = (size_t)(pathname_p - pathname) + 1;
            return 0;
        }
    } while (pathname_p != pathname);

    return 0;
}

/* Extract filename part of a path 
 * "/lib/libc.so" -> "libc.so" */
int basename_get(const char **basename_p, size_t *basename_len, const char *pathname, const size_t pathname_len)
{
    const char *pathname_p;

    if (pathname_len == 0)
    {
        *basename_p = NULL;
        *basename_len = 0;
        return 0;
    }

    if ((strchr(pathname, '/') == NULL) && (strchr(pathname, '\\') == NULL))
    {
        *basename_p = pathname;
        *basename_len = (size_t)pathname_len;
        return 0;
    }

    *basename_p = NULL;
    *basename_len = 0;
    pathname_p = pathname + pathname_len;
    do
    {
        pathname_p--;
        if ((*pathname_p == '/') || (*pathname_p == '\\'))
        {
            *basename_p = pathname_p + 1;
            *basename_len = (size_t)((pathname + pathname_len) - pathname_p - 1); /* Is it safe? */
            return 0;
        }
    } while (pathname_p != pathname);
    return 0;
}

/* Extract extension of a path
 * "libc.so" -> "so" */
int extension_get(const char **extension_p, size_t *extension_len, const char *pathname, const size_t pathname_len)
{
    const char *pathname_p, *pathname_endp;

    if (pathname_len == 0) return 0;

    *extension_p = NULL;
    *extension_len = 0;
    pathname_p = pathname;
    pathname_endp = pathname_p + pathname_len;
    while (pathname_p != pathname_endp)
    {
        if (*pathname_p == '.')
        {
            *extension_p = pathname_p + 1;
            *extension_len = 0;
        }
        else if ((*pathname_p == '/') || (*pathname_p == '\\'))
        {
            *extension_p = NULL;
            *extension_len = 0;
        }
        pathname_p++;

        if (*extension_p != NULL) *extension_len += 1;
    }
    *extension_len -= 1;
    return 0;
}

/* Extract mainname part of a path 
 * "/lib/libc.so" -> "libc" */
int mainname_get(const char **mainname_p, size_t *mainname_len, const char *pathname, const size_t pathname_len)
{
    const char *basename_p;
    size_t basename_len;
    const char *extension_p;
    size_t extension_len;

    basename_get(&basename_p, &basename_len, pathname, pathname_len);
    if (basename_len == 0)
    {
        *mainname_p = NULL;
        *mainname_len = 0;
    }
    else
    {
        extension_get(&extension_p, &extension_len, basename_p, basename_len);
        *mainname_p = basename_p;
        *mainname_len = basename_len - extension_len - 1;
    }

    return 0;
}

/* Find the path of current program */
int current_program_path(char *buf, const size_t buf_size)
{
#if defined(PLATFORM_WINDOWS)
    HMODULE hModule = GetModuleHandle(NULL);
    GetModuleFileName(hModule, buf, buf_size);
    return 0;
#elif defined(PLATFORM_LINUX)
    ssize_t ret = 0;
    ret = readlink("/proc/self/exe", buf, buf_size);
    if (ret < 0) return -1;
    buf[ret] = '\0';
    return 0;
#elif defined(PLATFORM_FREEBSD)
    int mib[4];
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PATHNAME;
    mib[3] = -1;
    size_t cb = buf_size;
    sysctl(mib, 4, buf, &cb, NULL, 0);
    return 0;
#elif defined(PLATFORM_MACOS)
    uint32_t len_uint32 = (uint32_t)buf_size;
    int ret = _NSGetExecutablePath(buf, &len_uint32);
    if (ret < 0) return -1;
    return 0;
#endif
}

/* Find the path of current working directory */
int current_working_path(char *buf, const size_t buf_size)
{
#if defined(PLATFORM_WINDOWS)
    return GetCurrentDirectory(buf_size, buf) == 0 ? -1 : 0;
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return getcwd(buf, buf_size) != NULL ? 0 : -1;
#endif
}

