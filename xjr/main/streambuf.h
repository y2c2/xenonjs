/* Stream Buffer */

#ifndef STREAMBUF_H
#define STREAMBUF_H

#include "xjr_dt.h"

#define SB_BUFFER_LEN 20
#define SB_INIT_CAPACITY 64
#define SB_EXTRA_CAPACITY 64

struct sb;
typedef struct sb sb_t;

sb_t *sb_new(void);
void sb_destroy(sb_t *hb);

void sb_clear(sb_t *hb);

int sb_append(sb_t *hb, const char *s, xjr_size_t len);
int sb_append_char(sb_t *hb, const char ch);

char *sb_buf(sb_t *hb);
xjr_size_t sb_size(sb_t *hb);

void sb_left_strip(sb_t *hb, char ch);
void sb_shift(sb_t *hb, xjr_size_t n);
void sb_shift_one(sb_t *hb);

int sb_find(sb_t *hb, char ch);
xjr_u8 checksum(sb_t *hb, xjr_size_t len);


#endif

