/* IP and port Parsing */

#ifndef IPPARSE_H
#define IPPARSE_H

#define IPPARSE_IPV4_LEN 17

typedef struct
{
    char host[IPPARSE_IPV4_LEN + 1];
    int port;
} ipparse_t;

int ipparse_parse(ipparse_t *ipparse, const char *s, int len);

#endif

