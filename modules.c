#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <wiringPi.h>

#include "raspi_re.h"
#include "modules.h"

uint8_t *_y8950_reg_b0[9];

void init_ym2608(int num) {

    write_ym2608(num, 0, 0x29, 0x80); 
}

void mute_ay8910(int num) {

    write_ay8910(num, 0x7, 0x3f);
    write_ay8910(num, 0x8, 0);
    write_ay8910(num, 0x9, 0);
    write_ay8910(num, 0xa, 0);
}

void mute_sn76489(int num) {

    write_sn76489(num, 0x9f);
    write_sn76489(num, 0xbf);
    write_sn76489(num, 0xdf);
    write_sn76489(num, 0xff);
}

void mute_y8950(int num) {

    for (int i = 0; i < 9; i++) {
        write_y8950(num, 0xb0 + i, 0);
    }
}

void mute_ym2151(int num) {

    for (int i = 0; i < 8; i++) {
        write_ym2151(num, 0x08, i);
    }
}

void mute_ym2203(int num) {

    write_ym2203(num, 0x07, 0x3f);
    write_ym2203(num, 0x28, 0);
    write_ym2203(num, 0x28, 1);
    write_ym2203(num, 0x28, 2);
}

void mute_ym2413(int num) {

    for (int i = 0; i < 9; i++) {
        write_ym2413(num, 0x20 + i, 0);
    }
}

void mute_ym2608(int num) {

    write_ym2608(num, 0, 0x07, 0x3f);
    write_ym2608(num, 0, 0x08, 0);
    write_ym2608(num, 0, 0x09, 0);
    write_ym2608(num, 0, 0x0a, 0);

    write_ym2608(num, 0, 0x28, 0);
    write_ym2608(num, 0, 0x28, 1);
    write_ym2608(num, 0, 0x28, 2);
    write_ym2608(num, 0, 0x28, 4);
    write_ym2608(num, 0, 0x28, 5);
    write_ym2608(num, 0, 0x28, 6);

    write_ym2608(num, 1, 0x0b, 0);
}

void mute_ym2612(int num) {

    write_ym2612(num, 0, 0x28, 0);
    write_ym2612(num, 0, 0x28, 1);
    write_ym2612(num, 0, 0x28, 2);
    write_ym2612(num, 0, 0x28, 4);
    write_ym2612(num, 0, 0x28, 5);
    write_ym2612(num, 0, 0x28, 6);
}

void mute_ym3526(int num) {

    for (int i = 0; i < 9; i++) {
        write_ym3526(num, 0xb0 + i, 0);
    }
}

void mute_ym3812(int num) {

    for (int i = 0; i < 9; i++) {
        write_ym3812(num, 0xb0 + i, 0);
    }
}

void write_ay8910(int n, uint8_t aa, uint8_t dd) {

    if (n < 0 || n >= 2) return;

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(20);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(20);
}

void write_sn76489(int n, uint8_t dd) {

    if (n < 0 || n >= 2) return;

/*
    while (re_irq() == 0) {
    }
*/

    re_address(0);
    re_write_data(dd);
    delayMicroseconds(1);
    re_cs0(n & 1);
    delayMicroseconds(1);
    re_wr(0);
    delayMicroseconds(10);
    re_wr(1);
    delayMicroseconds(1);
    re_cs0(!(n & 1));
    delayMicroseconds(20);
}

void write_y8950(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    delayMicroseconds(5);
    re_wr(0);
    delayMicroseconds(5);
    re_wr(1);
    delayMicroseconds(5);
    re_cs0(!(n & 1));
    delayMicroseconds(26);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    delayMicroseconds(5);
    re_wr(0);
    delayMicroseconds(5);
    re_wr(1);
    delayMicroseconds(5);
    re_cs0(!(n & 1));
    if (aa >= 0 && aa <= 0x1a) {
        delayMicroseconds(26);
    } else {
        delayMicroseconds(130);
    }
}

void write_ym2151(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(24);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(24);
}

void write_ym2203(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(10);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(10);
}

void write_ym2413(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_wr(0);
    re_cs0(n & 1);
    delayMicroseconds(1);
    re_cs0(!(n & 1));
    re_wr(1);
    delayMicroseconds(6);

    re_address(1);
    re_write_data(dd);
    re_wr(0);
    re_cs0(n & 1);
    delayMicroseconds(1);
    re_cs0(!(n & 1));
    re_wr(1);
    delayMicroseconds(30);
}

void write_ym2608(int n, int pp, uint8_t aa, uint8_t dd) {

    re_address(pp << 1);
    re_write_data(aa);
    delayMicroseconds(1);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    if (aa >=0x21 && aa <= 0xb6) {
        delayMicroseconds(3);  /* 17 cycles */
    }
    else
    if (pp == 0 && (aa >= 0x10 && aa <= 0x1d)) {
        delayMicroseconds(3);  /* 17 cycles */
    }
    else
    {
        delayMicroseconds(10);
    }

    re_address((pp << 1) | 1);
    re_write_data(dd);
    delayMicroseconds(1);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    if (aa >= 0x21 && aa <= 0x9e) {
        delayMicroseconds(12);  /* 83 cycles */
    }
    else
    if (aa >= 0xa0 && aa <= 0xb6) {
        delayMicroseconds(6);  /* 47 cycles */
    }
    else
    if (pp == 0 && aa == 0x10) {
        delayMicroseconds(75);  /* 576 cycles */
    }
    else
    if (pp == 0 && (aa >= 0x11 && aa <= 0x1d)) {
        delayMicroseconds(12);  /* 83 cycles */
    }
    else
    {
        delayMicroseconds(10);
    }
}

void write_ym2612(int n, int pp, uint8_t aa, uint8_t dd) {

    re_address(pp << 1);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(10);

    re_address((pp << 1) | 1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(10);
}

void write_ym3526(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(6);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    if (aa >= 0 && aa <= 0x1a) {
        delayMicroseconds(6);
    } else {
        delayMicroseconds(30);
    }
}

void write_ym3812(int n, uint8_t aa, uint8_t dd) {

    re_address(0);
    re_write_data(aa);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    delayMicroseconds(6);

    re_address(1);
    re_write_data(dd);
    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
    re_cs0(!(n & 1));
    if (aa >= 0 && aa <= 0x1a) {
        delayMicroseconds(6);
    } else {
        delayMicroseconds(30);
    }
}

