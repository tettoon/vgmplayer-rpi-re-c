#ifndef __VGM_H
#define __VGM_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define VGM_OK           0
#define VGM_EIDENT       1
#define VGM_EOVERFLOW    2
#define VGM_EPTHREAD     3
#define VGM_ECOMMAND     4
#define VGM_EDATATYPE    5

#define SN76489   0x50
#define YM2413    0x51
#define YM2612    0x52
#define YM2151    0x54
#define YM2203    0x55
#define YM2608    0x56
#define YM3812    0x5a
#define YM3526    0x5b
#define Y8950     0x5c
#define YMF262    0x5e
#define AY8910    0xa0
#define OKIM6258  0xb7

typedef struct vgm_header_t_ {
    uint32_t ident;                    /* 0x00: "Vgm " file identification */
    uint32_t eof_offset;               /* 0x04: Eof offset */
    uint32_t version;                  /* 0x08: Version number */
    uint32_t sn76489_clock;            /* 0x0C: SN76489 clock */
    uint32_t ym2413_clock;             /* 0x10: YM2413 clock */
    uint32_t gd3_offset;               /* 0x14: GD3 offset */
    uint32_t total_samples;            /* 0x18: Total # samples */
    uint32_t loop_offset;              /* 0x1C: Loop offset */
    uint32_t loop_samples;             /* 0x20: Loop # samples */
    uint32_t rate;                     /* 0x24: Rate */
    uint16_t sn76489_feedback;         /* 0x28: SN76489 feedback */
    uint8_t  sn76489_shift_reg_width;  /* 0x2A: SN76489 shift register width */
    uint8_t  sn76489_flags;            /* 0x2B: SN76489 flags */
    uint32_t ym2612_clock;             /* 0x2C: YM2612 clock */
    uint32_t ym2151_clock;             /* 0x30: YM2151 clock */
    uint32_t vgm_data_offset;          /* 0x34: VGM data offset */
    uint32_t sega_pcm_clock;           /* 0x38 */
    uint32_t sega_pcm_interface_reg;   /* 0x3C */
    uint32_t rf5c48_clock;             /* 0x40 */
    uint32_t ym2203_clock;             /* 0x44: YM2203 clock */
    uint32_t ym2608_clock;             /* 0x48: YM2608 clock */
    uint32_t ym2610_clock;             /* 0x4C: YM2610/YM2610B clock */
    uint32_t ym3812_clock;             /* 0x50: YM3812 clock */
    uint32_t ym3526_clock;             /* 0x54: YM3526 clock */
    uint32_t y8950_clock;              /* 0x58: Y8950 clock */
    uint32_t ymf262_clock;             /* 0x5C: YMF262 clock */
    uint32_t ymf278b_clock;            /* 0x60 */
    uint32_t ymf271_clock;             /* 0x64 */
    uint32_t ymz280b_clock;            /* 0x68 */
    uint32_t rf5c164_clock;            /* 0x6C */
    uint32_t pwm_clock;                /* 0x70 */
    uint32_t ay8910_clock;             /* 0x74: AY8910 clock */
    uint8_t  ay8910_chip_type;         /* 0x78: AY8910 chip type */
    uint8_t  ay8910_flags;             /* 0x79: AY8910 flags */
    uint8_t  ym2203_ay8910_flags;      /* 0x7A: YM2203/AY8910 flags */
    uint8_t  ym2608_ay8910_flags;      /* 0x7B: YM2608/AY8910 flags */
    uint8_t  volume_modifier;          /* 0x7C: Volume modifier */
    uint8_t  _reserved_7d;             /* 0x7D: Reserved */
    uint8_t  loop_base;                /* 0x7E: Loop base */
    uint8_t  _reserved_7f;             /* 0x7F: Reserved */
} vgm_header_t;

typedef struct vgm_t_ {
    int             vgm_errno;
    char            *vgm_errmsg;
    uint8_t         *buf;
    int             buf_sz;
    int             pos;
    vgm_header_t    *header;
    int             playing;
    uint32_t        frequency;
    uint32_t        samples;
    uint16_t        samples_62;
    uint16_t        samples_63;
    struct timespec start_tp;
} vgm_t;

extern void (*vgm_handler)(int, int, uint16_t aa, uint8_t dd);

void vgm_load(void *, size_t, vgm_t *);
int vgm_play(vgm_t *);
void vgm_clearerr(vgm_t *);
int vgm_error(vgm_t *);
void vgm_perror(vgm_t *, const char *);
char * vgm_strerror(int);

#endif /* __VGM_H */

