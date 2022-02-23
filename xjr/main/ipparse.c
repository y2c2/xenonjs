/* IP and port Parsing */

#include <string.h>
#include <stdlib.h>
#include "ipparse.h"

int ipparse_parse(ipparse_t *ipparse, const char *s, int len)
{
    char *p_colon = strchr(s, ':');
    if (p_colon == 0) return -1;

    if (p_colon - s > IPPARSE_IPV4_LEN) return -1;
    memcpy(ipparse->host, s, (size_t)(p_colon - s));
    ipparse->host[p_colon - s] = '\0';

    if (len - (p_colon + 1 - s) <= 0) return -1;
    ipparse->port = atoi(p_colon + 1);

    return 0;
}

