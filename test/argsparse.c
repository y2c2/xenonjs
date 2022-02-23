/* Arguments Parsing */

#include <stdio.h>
#include <string.h>

#include "argsparse.h"


int argsparse_init(argsparse_t *argparse, \
        int argc, char **argv)
{
    argparse->idx = 1;
    argparse->argc = argc;
    argparse->argv = argv;

    return 0;
}

int argsparse_request(argsparse_t *argparse, \
        char **p)
{
    if (argparse->idx >= argparse->argc)
    {
        return -1;
    }
    else
    {
        *p = (char *)argparse->argv[(size_t)argparse->idx];
        argparse->idx++;
    }

    return 0;
}


/* If argument available */
int argsparse_available(argsparse_t *argparse)
{
    return (argparse->idx < argparse->argc) ? 1 : 0;
}

/* If specified number of argument available */
int argsparse_available_count(argsparse_t *argparse, int count)
{
    return (argparse->idx + count - 1 < argparse->argc) ? 1 : 0;
}


/* Finished parsing current argument */
int argsparse_next(argsparse_t *argparse)
{
    argparse->idx++;

    return 0;
}


/* Match String */
int argsparse_match_str(argsparse_t *argparse, const char *pat)
{
    char *s;
    size_t s_len, pat_len;

    s = (char *)argparse->argv[(size_t)argparse->idx];
    s_len = strlen(s);
    pat_len = strlen(pat);
    /* Length Comparing */
    if (s_len != pat_len) return 0;
    /* Content Comparing */
    if (strncmp(pat, s, pat_len) != 0) return 0;

    /* Match */
    return 1;
}

/* Fetch whole argument */
char *argsparse_fetch(argsparse_t *argparse)
{
    char *s;
    if (argparse->argc <= argparse->idx) return NULL;
    s = (char *)argparse->argv[(size_t)argparse->idx];
    return s;
}


