#include <string.h>
#include <stdlib.h>
#include "cmdparse.h"

typedef enum
{
    cmdparse_parse_state_init,
    cmdparse_parse_state_element,
    cmdparse_parse_state_element_str,
    cmdparse_parse_state_escape
} cmdparse_parse_state;

int cmdparse_parse(cmdparse_t *cmdparse, const char *s, int len)
{
    const char *p = s;
    cmdparse_argv_t *cur_argv;
    cmdparse_parse_state state = cmdparse_parse_state_init;
    int idx = 0;

    cmdparse->argc = 0;
    cur_argv = &cmdparse->argv[0];

    while (len != 0)
    {
        char ch = *p;

        switch (state)
        {
            case cmdparse_parse_state_init:
                if (ch == '\"')
                {
                    cur_argv->s = p;
                    cur_argv->len = 1;
                    state = cmdparse_parse_state_element_str;
                    p++;
                    len--;
                }
                else if ((ch == ' ') || (ch == '\t'))
                {
                    p++;
                    len--;
                }
                else
                {
                    cur_argv->s = p;
                    cur_argv->len = 1;
                    state = cmdparse_parse_state_element;
                    p++;
                    len--;
                }
                break;
            case cmdparse_parse_state_element:
                if (ch == '\"')
                {
                    /* Error */
                    return -1;
                }
                else if ((ch == ' ') || (ch == '\t'))
                {
                    cmdparse->argc++;
                    idx++;
                    state = cmdparse_parse_state_init;
                    p++;
                    len--;
                }
                else
                {
                    cmdparse->argv[idx].len++;
                    p++;
                    len--;
                }
                break;
            case cmdparse_parse_state_element_str:
                if (ch == '\"')
                {
                    cur_argv->len++;
                    cmdparse->argc++;
                    idx++;
                    state = cmdparse_parse_state_init;
                    p++;
                    len--;
                }
                else
                {
                    cur_argv->len++;
                    p++;
                    len--;
                }
                break;
            case cmdparse_parse_state_escape:
                break;
        }
    }

    if (state == cmdparse_parse_state_element)
    {
        cmdparse->argc++;
    }
    else if (state == cmdparse_parse_state_element_str)
    {
        return -1;
    }

    return 0;
}

