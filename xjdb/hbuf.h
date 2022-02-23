#ifndef HBUF_H
#define HBUF_H

#define HBUF_NULL ((void*)0)

#define HBUF_BUFFER_LEN 20
#define HBUF_INIT_CAPACITY 64
#define HBUF_EXTRA_CAPACITY 64

struct hbuf;
typedef struct hbuf hbuf_t;

hbuf_t *hbuf_new(void);
void hbuf_destroy(hbuf_t *hb);

void hbuf_clear(hbuf_t *hb);

int hbuf_append(hbuf_t *hb, const unsigned char *s, unsigned int len);
int hbuf_append_char(hbuf_t *hb, unsigned char ch);

unsigned char *hbuf_buf(hbuf_t *hb);
unsigned int hbuf_size(hbuf_t *hb);

void hbuf_left_strip(hbuf_t *hb, unsigned char ch);
void hbuf_shift(hbuf_t *hb, unsigned int n);
void hbuf_shift_one(hbuf_t *hb);

int hbuf_find(hbuf_t *hb, unsigned char ch);

#endif

