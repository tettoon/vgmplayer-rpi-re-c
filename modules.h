#ifndef __MODULES_H
#define __MODULES_H

#include <stdint.h>

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

#endif  /* __MODULES_H */

