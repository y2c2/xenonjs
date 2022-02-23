#ifndef CRC32_H
#define CRC32_H

typedef unsigned long crc32_t;

void crc32_init(crc32_t *crc);
void crc32_update(crc32_t *crc, const unsigned char *blk_adr, unsigned long blk_len);
crc32_t crc32_value(crc32_t *crc);

#endif 

