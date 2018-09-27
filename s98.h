#ifndef __S98_H
#define __S98_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define S98_OK           0
#define S98_EIDENT       1
#define S98_EOVERFLOW    2
#define S98_EPTHREAD     3
#define S98_ECOMMAND     4
#define S98_EDATATYPE    5

#define S98_NONE    0
#define S98_YM2149  1
#define S98_YM2203  2
#define S98_YM2612  3
#define S98_YM2608  4
#define S98_YM2151  5
#define S98_YM2413  6
#define S98_YM3526  7
#define S98_YM3812  8
#define S98_YMF262  9
#define S98_AY8910  15
#define S98_SN76489 16

enum s98_device_type {
    NONE = 0,
    YM2149,
    YM2203,
    YM2612,
    YM2608,
    YM2151,
    YM2413,
    YM3526,
    YM3812,
    YMF262,
    AY8910 = 15,
    SN76489
};

typedef struct s98_device_info_t_ {
    uint32_t device_type;              /* 0x00: */
    uint32_t clock;                    /* 0x04: */
    uint32_t pan;                      /* 0x08: */
    uint32_t _reserve;
} s98_device_info_t;

typedef struct s98_header_t_ {
    uint8_t  ident[3];                 /* 0x00: "S98" file identification */
    uint8_t  version;                  /* 0x03: Format version */
    uint32_t timer_info;               /* 0x04: sync numerator */
    uint32_t timer_info2;              /* 0x08: sync denominator */
    uint32_t compressing;              /* 0x0C: The value is 0 always */
    uint32_t offset_tag;               /* 0x10: */
    uint32_t offset_dump_data;         /* 0x14: */
    uint32_t offset_loop_point;        /* 0x18: */
    uint32_t device_count;             /* 0x1C: */
    s98_device_info_t device_info[];   /* 0x20: */
} s98_header_t;

typedef struct s98_t_ {
    int             s98_errno;
    char            *s98_errmsg;
    uint8_t         *buf;
    int             buf_sz;
    int             pos;
    s98_header_t    *header;
    int             repeat;
    int             _repeat;
    int             playing;
    uint32_t        frequency;
    uint32_t        samples;
    struct timespec start_tp;
    uint8_t         opna_reg100[2];
} s98_t;

extern void (*s98_handler)(int, int, uint16_t aa, uint8_t dd);

void s98_load(void *, size_t, s98_t *);
int s98_play(s98_t *);
void s98_clearerr(s98_t *);
int s98_error(s98_t *);
void s98_perror(s98_t *, const char *);
char * s98_strerror(int);

#endif /* __S98_H */

