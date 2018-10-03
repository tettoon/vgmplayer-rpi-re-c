#ifndef MODULES_H
#define MODULES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

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

#define MODULE_COUNT 2

typedef struct module_info_t_ {
    int slot_num;
    int module_id;
    int num;
} module_info_t;

void module_prepare_info(char *);
int module_name_to_id(const char *);
int module_name_to_slot(int, int);
void module_handler(int, int, uint16_t, uint8_t);
void module_init(void);
void module_mute(void);

void init_ym2608(int);

void mute_ay8910(int);
void mute_sn76489(int);
void mute_y8950(int);
void mute_ym2151(int);
void mute_ym2203(int);
void mute_ym2413(int);
void mute_ym2608(int);
void mute_ym2612(int);
void mute_ym3526(int);
void mute_ym3812(int);

void write_ay8910(int, uint8_t, uint8_t);
void write_sn76489(int, uint8_t);
void write_y8950(int, uint8_t, uint8_t);
void write_ym2151(int, uint8_t, uint8_t);
void write_ym2203(int, uint8_t, uint8_t);
void write_ym2413(int, uint8_t, uint8_t);
void write_ym2608(int, int, uint8_t, uint8_t);
void write_ym2612(int, int, uint8_t, uint8_t);
void write_ym3526(int, uint8_t, uint8_t);
void write_ym3812(int, uint8_t, uint8_t);

#ifdef __cplusplus
}
#endif

#endif  /* MODULES_H */

