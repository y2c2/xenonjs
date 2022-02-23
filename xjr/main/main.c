#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "argsparse.h"
#include "ipparse.h"

#define _basename(_fullname) \
    (strrchr(_fullname, '/') ? strrchr(_fullname, '/') + 1 : _fullname)

/* xjr */
#include "xjr.h"

/* Host bindings */
#include "xjr_host.h"

/* xjr libraries */
#include "xjr_lib_os.h"
#include "xjr_lib_buffer.h"

/* Debug Stub */
#include "debugstub.h"

#define XJR_VERSION "0.0.1"
#ifndef XJR_KB
#define XJR_KB(x) (1024 * (x))
#endif
#ifndef XJR_MB
#define XJR_MB(x) (1024 * XJR_KB(x))
#endif
#define XJR_HEAP_SIZE_DEFAULT (XJR_KB(16))
#define XJR_STACK_SIZE_DEFAULT (4096)

static void show_help(void)
{
    const char *info = ""
        "usage: xjr [options] <source.x>\n"
        "  --show-profile   Show profile info\n"
        "  --disable-gc     Disable garbage collector\n"
        "  --gc-mode <mode> Set GC mode\n"
        "    normal         Perform full gc when usage rapidly increased\n"
        "    strict         Perform full gc at every step\n"
        "  --no-stdlib      Do not load standard libraries\n"
        "  --debug-server   Start debug server\n"
        "\n";

    printf("%s", info);
}

static int read_file( \
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
    if (len < 0)
    {
        fclose(fp);
        return -1;
    }
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

static void *xjr_heap_malloc_cb(void *heap, xjr_size_t size)
{
    (void)heap;
    return malloc(size);
}

static void xjr_heap_free_cb(void *heap, void *p)
{
    (void)heap;
    free(p);
}

static void print_vm_error(xjr_vm *vm)
{
    (void)vm;
}

static int simple_write(const char *data, const xjr_size_t len)
{
    fwrite(data, len, 1, stdout);
    return 0;
}

struct xjr_gc_trigger_normal_stub
{
    xjr_size_t latest_usage;
};

static void xjr_gc_trigger_normal_stub_init(struct xjr_gc_trigger_normal_stub *stub)
{
    stub->latest_usage = 1024;
}

static xjr_bool xjr_gc_trigger_normal_before_mark(xjr_vm *vm, void *trigger_stub)
{
    struct xjr_gc_trigger_normal_stub *ts = trigger_stub;
    xjr_size_t usage = xjr_mp_size_capacity(vm->rts.rheap.mp) - xjr_mp_size_free(vm->rts.rheap.mp);
    if (ts->latest_usage * 2 < usage) { return xjr_true; }
    return xjr_false;
}

static void xjr_gc_trigger_normal_after_sweep(xjr_vm *vm, void *trigger_stub)
{
    struct xjr_gc_trigger_normal_stub *ts = trigger_stub;
    xjr_size_t usage = xjr_mp_size_capacity(vm->rts.rheap.mp) - xjr_mp_size_free(vm->rts.rheap.mp);
    ts->latest_usage = usage;
}

static xjr_bool xjr_gc_trigger_strict_before(xjr_vm *vm, void *trigger_stub)
{
    (void)vm;
    (void)trigger_stub;
    return xjr_true;
}

typedef struct
{
    xjr_size_t filename_len;
    char *filename;
    struct
    {
        struct
        {
            int ln, col;
        } start, end;
    } loc;
    struct
    {
        int start, end;
    } range;
} xjs_x_file_dbg_info_item;

#define READ_U32(_dst, _offset) \
    do { \
        _dst = (xjr_u32)(xjr_u8)(*((xjr_u8 *)(_offset))) | \
             (xjr_u32)((xjr_u8)(*(((xjr_u8 *)(_offset)) + 1)) << 8) | \
             (xjr_u32)((xjr_u8)(*(((xjr_u8 *)(_offset)) + 2)) << 16) | \
             (xjr_u32)((xjr_u8)(*(((xjr_u8 *)(_offset)) + 3)) << 24); \
    } while (0)

#define READ_S32(_dst, _offset) \
    do { \
        _dst = (xjr_s32)((xjr_u8)(*((xjr_u8 *)_offset)) | \
                ((xjr_u8)(*(((xjr_u8 *)_offset) + 1)) << 8) | \
                ((xjr_u8)(*(((xjr_u8 *)_offset) + 2)) << 16) | \
                ((xjr_u8)(*(((xjr_u8 *)_offset) + 3)) << 24)); \
    } while (0)

static int dbg_info_load(xjs_x_file_dbg_info_item *item, xjr_vm *vm)
{
    xjr_size_t off, off_orig;
    int pc = -1;

    if ((vm->xf.sections.dbi0 == 0) || (vm->xf.sections.dbg0 == 0))
    { return -1; }
    
    item->filename = NULL;
    item->filename_len = 0;

    off = vm->xf.sections.dbi0 + 4;
    off_orig = 4;
    for (;;)
    {
        READ_S32(pc, vm->xf.bc + off);
        if (pc == vm->err.rts.pc)
        {
            off_orig += 4;
            break;
        }
        off += 8;
        off_orig += 8;
    }
    if (pc != vm->err.rts.pc) { return -1; }

    off = vm->xf.sections.dbi0 + off_orig;
    READ_U32(off, vm->xf.bc + off);
    off = vm->xf.sections.dbg0 + 4 + off;

    /* Filename length */
    READ_U32(item->filename_len, vm->xf.bc + off);
    if (item->filename_len == 0) { return -1; }
    off += 4;

    /* Filename */
    item->filename = vm->xf.bc + off;
    off += item->filename_len + 1;

    /* Location and range */
    READ_S32(item->loc.start.ln, vm->xf.bc + off);
    off += 4;
    READ_S32(item->loc.start.col, vm->xf.bc + off);
    off += 4;
    READ_S32(item->loc.end.ln, vm->xf.bc + off);
    off += 4;
    READ_S32(item->loc.end.col, vm->xf.bc + off);
    off += 4;
    READ_S32(item->range.start, vm->xf.bc + off);
    off += 4;
    READ_S32(item->range.end, vm->xf.bc + off);
    /* off += 4; */

    return 0;
}

static int xjr_crash_callback(xjr_crash_ctx *ctx)
{
    xjr_vm *vm = ctx->vm;
    xjr_error *err = &vm->err;
    xjs_x_file_dbg_info_item dbg_info_item;

    if (dbg_info_load(&dbg_info_item, vm) == 0)
    {
        if (dbg_info_item.filename != NULL)
        {
            fprintf(stderr, "%s:", \
                    dbg_info_item.filename);
        }
        if ((dbg_info_item.loc.start.ln != -1) && (dbg_info_item.loc.start.col != -1))
        {
            fprintf(stderr, "%d:%d:", \
                    dbg_info_item.loc.start.ln, \
                    dbg_info_item.loc.start.col + 1);
        }
        fprintf(stderr, " ");
    }
    else
    {
        /* Filename not resolved */
        if (err->rts.pc != -1) { fprintf(stderr, "pc: 0X%X: ", vm->err.rts.pc); }
    }
    
    /* error description */
    switch (vm->err.code)
    {
        case XJR_ERR_OK:
            break;
        case XJR_ERR_INTERNAL:
            fprintf(stderr, "error: internal error");
            break;
        case XJR_ERR_OOM:
            fprintf(stderr, "error: out of memory");
            break;
        case XJR_ERR_IO:
            fprintf(stderr, "error: I/O error");
            break;
        case XJR_ERR_SOF:
            fprintf(stderr, "error: Stack overflow");
            break;
        case XJR_ERR_XCORRUPT:
            fprintf(stderr, "error: corrupt x file");
            break;
        case XJR_ERR_INT_OUT_OF_BOUND:
            fprintf(stderr, "error: integer out of bound");
            break;
        case XJR_ERR_INVALID_OP:
            fprintf(stderr, "error: invalid opcode %02X", vm->err.rts.pc);
            break;
        case XJR_ERR_INVALID_TYPE:
            fprintf(stderr, "error: invalid operand type");
            break;
        case XJR_ERR_ARGC_EXCEED:
            fprintf(stderr, "error: argument number exceed");
            break;
        case XJR_ERR_NOT_DEFINED:
            if (vm->err.u.as_not_defined.name == xjr_nullptr)
            {
                fprintf(stderr, "error: not defined");
            }
            else
            {
                fprintf(stderr, "error: %s is not defined", vm->err.u.as_not_defined.name);
            }
            break;
        case XJR_ERR_NOT_CONSTRUCTOR:
            fprintf(stderr, "error: not a constructor");
            break;
        case XJR_ERR_MEM_CORRUPTED:
            fprintf(stderr, "error: memory freed before unreachable (loc:%s:%d, urid:%d)", \
                    _basename(vm->err.loc.filename), vm->err.loc.ln, \
                    vm->err.u.as_mem_corrupted.urid);
            break;
    }
    fprintf(stderr, "\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0;
    char *bytecode = NULL;
    size_t bytecode_len = 0;
    xjr_vm vm;
    debugstub_t debugstub;
    char *stack_data = NULL;
    argsparse_t argsparse;
    xjr_bool show_profile = xjr_false;
    xjr_bool disable_gc = xjr_false;
    xjr_bool nostdlib = xjr_false;
    char *debug_server = NULL;
    char *xfile_pathname = NULL;
    char *gc_mode = xjr_nullptr;
    struct xjr_gc_trigger_normal_stub trigger_normal_stub;
    xjr_bool ret_zero = xjr_false;

    argsparse_init(&argsparse, argc, argv);

    if (argsparse_available(&argsparse) == 0)
    {
        fprintf(stderr, "error: no input file\n");
        return -1;
    }

    debugstub_init(&debugstub);
    xjr_vm_init(&vm);

    while (argsparse_available(&argsparse) != 0)
    {
        if (argsparse_match_str(&argsparse, "--help") || \
                argsparse_match_str(&argsparse, "-h"))
        { show_help(); goto done; }
        else if (argsparse_match_str(&argsparse, "--show-profile"))
        {
            show_profile = xjr_true;
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--disable-gc"))
        {
            disable_gc = xjr_true;
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--gc-mode"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            {
                fprintf(stderr, "error: no GC mode\n");
                goto fail;
            }
            gc_mode = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--no-stdlib"))
        {
            nostdlib = xjr_true;
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--ret-zero"))
        {
            ret_zero = xjr_true;
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "--debug-server"))
        {
            argsparse_next(&argsparse);
            if (argsparse_available(&argsparse) == 0)
            {
                fprintf(stderr, "error: no GDB server address\n");
                goto fail;
            }
            debug_server = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else if (argsparse_match_str(&argsparse, "-"))
        {
            xfile_pathname = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
        else
        {
            xfile_pathname = argsparse_fetch(&argsparse);
            argsparse_next(&argsparse);
        }
    }

    if (xfile_pathname == NULL)
    {
        fprintf(stderr, "error: no input file\n");
        goto fail;
    }

    {
        ret = read_file(&bytecode, &bytecode_len, xfile_pathname);
        if (ret != 0)
        {
            fprintf(stderr, "error: failed to read data from %s\n", xfile_pathname);
            goto fail;
        }
    }

    {
        if ((stack_data = (char *)malloc(sizeof(char) * XJR_STACK_SIZE_DEFAULT)) == NULL)
        {
            fprintf(stderr, "error: out of memory\n");
            goto fail;
        }

        xjr_vm_setup_stack(&vm, stack_data, XJR_STACK_SIZE_DEFAULT);
        xjr_vm_setup_heap(&vm, NULL, xjr_heap_malloc_cb, xjr_heap_free_cb);
        xjr_vm_setup_write(&vm, simple_write);
        xjr_vm_extn(&vm)->cb_write = simple_write;
    }

    {
        if (gc_mode != xjr_nullptr)
        {
            if (strcmp(gc_mode, "strict") == 0)
            {
                xjr_vm_set_gc_step_trigger(&vm, \
                        xjr_nullptr, xjr_gc_trigger_strict_before, xjr_nullptr);
            }
            else if (strcmp(gc_mode, "normal") == 0)
            {
                xjr_gc_trigger_normal_stub_init(&trigger_normal_stub);
                xjr_vm_set_gc_step_trigger(&vm, \
                        &trigger_normal_stub, \
                        xjr_gc_trigger_normal_before_mark, \
                        xjr_gc_trigger_normal_after_sweep);
            }
            else
            {
                fprintf(stderr, "error: unsupported GC mode '%s'\n", gc_mode);
                goto fail;
            }
        }
        else
        {
            xjr_gc_trigger_normal_stub_init(&trigger_normal_stub);
            xjr_vm_set_gc_step_trigger(&vm, \
                    &trigger_normal_stub, \
                    xjr_gc_trigger_normal_before_mark, \
                    xjr_gc_trigger_normal_after_sweep);
        }
    }

    {
        xjr_vm_lib_install(&vm, "os", xjr_lib_os);
        xjr_vm_lib_install(&vm, "buffer", xjr_lib_buffer);
    }
    
    if ((ret = xjr_vm_load(&vm, bytecode, (xjr_size_t)bytecode_len)) != 0)
    {
        fprintf(stderr, "error: load bytecode failed: %d\n", ret);
        goto fail;
    }

    xjr_vm_crash_handler_install(&vm, xjr_crash_callback);

    if (debug_server != NULL)
    {
        ipparse_t ipparse;

        if (ipparse_parse(&ipparse, debug_server, (int)strlen(debug_server)) != 0)
        {
            fprintf(stderr, "error: invalid GDB stub address %s, use 'host:port' pattern\n", debug_server);
            goto fail;
        }

        if (debugstub_start(&debugstub, ipparse.host, ipparse.port) != 0)
        {
            fprintf(stderr, "error: start GDB stub failed\n");
            goto fail;
        }

        xjr_vm_debugger_install(&vm, debugstub_cb, &debugstub);
    }

    if (disable_gc == xjr_true)
    {
        xjr_vm_set_options(&vm, XJR_VM_OPTIONS_NOGC);
    }

    if (nostdlib == xjr_true)
    {
        xjr_vm_set_options(&vm, XJR_VM_OPTIONS_NOSTDLIB);
    }

    /* Initial stack frame */
    if (xjr_vm_boot(&vm) != 0) { goto fail; }

    /* Initialize host bindings */
    xjr_host_bindings_init(&vm);

    if ((ret = xjr_vm_start(&vm)) != 0)
    { goto fail; }

    if (show_profile == xjr_true)
    {
        printf("executed instruments: %u\n", (unsigned int)vm.profile.step);
        printf("pool free: %u\n", (unsigned int)xjr_mp_size_free(vm.rts.rheap.mp));
        printf("pool capacity: %u\n", (unsigned int)xjr_mp_size_capacity(vm.rts.rheap.mp));
        printf("pool usage peak: %u\n", (unsigned int)xjr_mp_size_usage_peak(vm.rts.rheap.mp));
        printf("pool capacity peak: %u\n", (unsigned int)xjr_mp_size_capacity_peak(vm.rts.rheap.mp));
        printf("pool block peak: %u\n", (unsigned int)xjr_mp_number_allocated_block_peak(vm.rts.rheap.mp));
        fflush(stdout);
    }

    goto done;
fail:
    if (vm.err.code != 0)
    {
        print_vm_error(&vm);
    }
    ret = -1;
done:
    if (stack_data != NULL) free(stack_data);
    if (bytecode != NULL) free(bytecode);
    xjr_vm_uninit(&vm);
    debugstub_uninit(&debugstub);
    if (ret_zero == xjr_true) ret = 0;
    return ret;
}

