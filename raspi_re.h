#ifndef __RASPI_RE_H
#define __RASPI_RE_H

#include <stdint.h>

#define PIN_IRQ 29
#define PIN_WR  28
#define PIN_IC  25
#define PIN_CS0 27
#define PIN_RD  26
#define PIN_A0  21
#define PIN_A1  22
#define PIN_A2  23
#define PIN_A3  24
#define PIN_D0   0
#define PIN_D1   1
#define PIN_D2   2
#define PIN_D3   3
#define PIN_D4   4
#define PIN_D5   5
#define PIN_D6   6
#define PIN_D7   7

void re_init();
void re_reset();
void re_address(uint8_t);
void re_write_data(uint8_t);
uint8_t re_read_data();
int re_irq();
void re_wr(int);
void re_ic(int);
void re_cs0(int);
void re_rd(int);

#endif  /* __RASPI_RE_H */

