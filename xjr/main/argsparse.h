/* Arguments Parsing */

#ifndef ARGSPARSE_H
#define ARGSPARSE_H

/* Data structure for storing the parsing state */
typedef struct 
{
    char **argv;
    int argc;
    int idx;
} argsparse_t;

/* Initialize argument parsing */
int argsparse_init(argsparse_t *argparse, \
        int argc, char **argv);
/* Request an argument in char* style */
int argsparse_request(argsparse_t *argparse, \
        char **p);

/* If argument available */
int argsparse_available(argsparse_t *argparse);
/* If specified number of argument available */
int argsparse_available_count(argsparse_t *argparse, int count);
/* Finished parsing current argument */
int argsparse_next(argsparse_t *argparse);

/* Match String */
int argsparse_match_str(argsparse_t *argparse, const char *pat);
/* Fetch whole argument */
char *argsparse_fetch(argsparse_t *argparse);


#endif

