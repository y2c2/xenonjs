/* IP and port Parsing */

#include <string.h>
#include <stdlib.h>
#include "ipparse.h"

int ipparse_parse(ipparse_t *ipparse, const char *s, int len)
{
    const char *p_start;
    const char *p_colon;
    
    if ((len >= 9) && (strncmp("serial://", s, 9) == 0))
    {
        ipparse->type = IPPARSE_TYPE_SERIAL;
        p_start = s + 9;
    }
    else if ((len >= 6) && (strncmp("tcp://", s, 6) == 0))
    {
        ipparse->type = IPPARSE_TYPE_TCP;
        p_start = s + 6;
    }
    else if ((len >= 6) && (strncmp("udp://", s, 6) == 0))
    {
        ipparse->type = IPPARSE_TYPE_UDP;
        p_start = s + 6;
    }
    else
    {
        ipparse->type = IPPARSE_TYPE_TCP;
        p_start = s;
    }

    switch (ipparse->type)
    {
        case IPPARSE_TYPE_SERIAL:
            {
                p_colon = p_start + strlen(p_start);
                if (p_colon - p_start > IPPARSE_SERIAL_LEN) return -1;
                memcpy(ipparse->u.as_serial.device, p_start, (size_t)(p_colon - p_start));
                ipparse->u.as_serial.device[p_colon - p_start] = '\0';
                ipparse->u.as_serial.baudrate = 9600;
                ipparse->u.as_serial.data_bits = 8;
                ipparse->u.as_serial.parity = 'N';
                ipparse->u.as_serial.stop_bits = 1;
            }
            break;

        case IPPARSE_TYPE_TCP:
        case IPPARSE_TYPE_UDP:
            {
                p_colon = strchr(p_start, ':');
                if (p_colon == NULL) return -1;

                if (p_colon - p_start > IPPARSE_IPV4_LEN) return -1;
                memcpy(ipparse->u.as_network.host, p_start, (size_t)(p_colon - p_start));
                ipparse->u.as_network.host[p_colon - p_start] = '\0';

                if (len - (p_colon + 1 - p_start) <= 0) return -1;
                ipparse->u.as_network.port = atoi(p_colon + 1);
            }
            break;
    }

    return 0;
}

