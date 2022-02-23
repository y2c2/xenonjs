#include "platform.h"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

#include <stdlib.h>
#include <ec_encoding.h>
#include "xjs.h"
#include "print_error.h"

void print_error(xjs_error_ref err)
{
    if (err->error_no == 0) return;

    if (err->desc == NULL)
    {
        fprintf(stderr, "%s:%d: error: ", err->loc.filename, (int)err->loc.ln);
    }

    if (err->desc != NULL)
    {
        ec_encoding_t enc;
        char *encoded_bytes;
        ec_size_t encoded_bytes_len;

        ec_encoding_utf8_init(&enc);
        ec_encoding_encode(&enc, (ec_byte_t **)&encoded_bytes, &encoded_bytes_len, err->desc);

#if defined(PLATFORM_WINDOWS)
        {
            HANDLE handle;
            DWORD i;
            handle = GetStdHandle(STD_ERROR_HANDLE);
            if (handle == INVALID_HANDLE_VALUE)
            { return; }
            if (!WriteFile(handle, encoded_bytes, encoded_bytes_len, &i, NULL))
            { return; }
        }
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
        fwrite(encoded_bytes, encoded_bytes_len, 1, stderr);
#endif

        free(encoded_bytes);
    }
    else
    {
        switch (err->error_no)
        {
            case XJS_ERRNO_INTERNAL:
                fprintf(stderr, "internal error");
                break;
            case XJS_ERRNO_MEM:
                fprintf(stderr, "out of memory");
                break;
            case XJS_ERRNO_NOTIMP:
                fprintf(stderr, "not implemented");
                break;
            case XJS_ERRNO_IO:
                fprintf(stderr, "I/O error");
                break;
            case XJS_ERRNO_LEX:
                fprintf(stderr, "tokenize error");
                break;
            case XJS_ERRNO_PARSE:
                fprintf(stderr, "parse error");
                break;
            case XJS_ERRNO_RUNTIME:
                fprintf(stderr, "runtime error");
                break;
        }
    }

    fprintf(stderr, "\n");

}


