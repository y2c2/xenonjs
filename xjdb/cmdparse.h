#ifndef CMDPARSE_H
#define CMDPARSE_H

#define CMDPARSE_ARGC_MAX 16

typedef struct
{
    const char *s;
    int len;
} cmdparse_argv_t;

typedef struct
{
    cmdparse_argv_t argv[CMDPARSE_ARGC_MAX];
    int argc;
} cmdparse_t;

int cmdparse_parse(cmdparse_t *cmdparse, const char *s, int len);

#endif

