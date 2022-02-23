/* IP and port Parsing */

#ifndef IPPARSE_H
#define IPPARSE_H

#define IPPARSE_IPV4_LEN 17
#define IPPARSE_SERIAL_LEN 17

typedef enum
{
    IPPARSE_TYPE_SERIAL,
    IPPARSE_TYPE_TCP,
    IPPARSE_TYPE_UDP,
} ipparse_type;

typedef struct
{
    ipparse_type type;

    union
    {
        struct
        {
            char host[IPPARSE_IPV4_LEN + 1];
            int port;
        } as_network;
        struct
        {
            char device[IPPARSE_SERIAL_LEN + 1];
            int baudrate; /* 9600 */
            int data_bits; /* 8 */
            int parity; /* N */
            int stop_bits; /* 1 */
        } as_serial;
    } u;
} ipparse_t;

int ipparse_parse(ipparse_t *ipparse, const char *s, int len);

#endif

