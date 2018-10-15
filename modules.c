#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <wiringPi.h>

#include "raspi_re.h"
#include "modules.h"

/* Static variables */

static module_info_t _module_info[MODULE_COUNT];

#if 0
static uint8_t *_y8950_reg_b0[9];
#endif

/* Functions */

void module_prepare_info(char *module_names) {

    int i, count;
    char* p;
    int slot_num = 0;

    p = strtok(module_names, ",");
    _module_info[0].slot_num  = 0;
    _module_info[0].module_id = module_name_to_id(p);
    _module_info[0].num       = 0;

    for (slot_num = 1; slot_num < MODULE_COUNT; slot_num++) {
        _module_info[slot_num].slot_num  = slot_num;
        p = strtok(NULL, ",");
        if (p == NULL) {
            _module_info[slot_num].module_id = -1;
            _module_info[slot_num].num       = -1;
            continue;
        }
        count = 0;
        _module_info[slot_num].module_id = module_name_to_id(p);
        for (i = 0; i < slot_num - 1; i++) {
            if (_module_info[i].module_id == _module_info[slot_num].module_id) {
                count++;
            }
        }
        _module_info[slot_num].num       = count;
    }

/*
    for (slot_num = 0; slot_num < MODULE_COUNT; slot_num++) {
        printf("#%d: module_id=$%02X (%d)\n", slot_num, _module_info[slot_num].module_id, _module_info[slot_num].num);
    }
*/
}

int module_name_to_id(const char *module_name) {

    int module = -1;

    if (module_name == NULL) {
        module = 0;
    } else if (strcmp("AY8910", module_name) == 0) {
        module = AY8910;
    } else if (strcmp("SN76489", module_name) == 0) {
        module = SN76489;
    } else if (strcmp("Y8950", module_name) == 0) {
        module = Y8950;
    } else if (strcmp("YM2151", module_name) == 0) {
        module = YM2151;
    } else if (strcmp("YM2203", module_name) == 0) {
        module = YM2203;
    } else if (strcmp("YM2413", module_name) == 0) {
        module = YM2413;
    } else if (strcmp("YM2608", module_name) == 0) {
        module = YM2608;
    } else if (strcmp("YM2612", module_name) == 0) {
        module = YM2612;
    } else if (strcmp("YM3526", module_name) == 0) {
        module = YM3526;
    } else if (strcmp("YM3812", module_name) == 0) {
        module = YM3812;
    }

    return module;
}

int module_name_to_slot(int module_id, int num) {
    int i, m_id;
    int count = 0;
    for (i = 0; i < MODULE_COUNT; i++) {
        m_id = _module_info[i].module_id;
        if (m_id >= 0 && m_id == module_id) {
            if (count++ == num) return i;
        }
    }
    return -1;
}

void module_handler(int module, int num, uint16_t ppaa, uint8_t dd) {

    int pp = ppaa >> 8;
    uint8_t aa = ppaa & 0xff;

    int i, slot_num;
    slot_num = -1;
    for (i = 0; i < MODULE_COUNT; i++) {
        if (_module_info[i].module_id == module && _module_info[i].num == num) {
            slot_num = i;
            break;
        }
    }

    if (slot_num < 0) return;

    switch (module) {
        case AY8910:
            write_ay8910(slot_num, aa, dd);
            break;
        case SN76489:
            write_sn76489(slot_num, dd);
            break;
        case Y8950:
            write_y8950(slot_num, aa, dd);
            break;
        case YM2151:
            write_ym2151(slot_num, aa, dd);
            break;
        case YM2203:
            write_ym2203(slot_num, aa, dd);
            break;
        case YM2413:
            write_ym2413(slot_num, aa, dd);
            break;
        case YM2608:
            write_ym2608(slot_num, pp, aa, dd);
            break;
        case YM2612:
            write_ym2612(slot_num, pp, aa, dd);
            break;
        case YM3526:
            write_ym3526(slot_num, aa, dd);
            break;
        case YM3812:
            write_ym3812(slot_num, aa, dd);
            break;
    }
}

void module_init(void) {

    int slot_num;
    for (slot_num = 0; slot_num < MODULE_COUNT; slot_num++) {
        if (_module_info[slot_num].module_id <= 0)
            continue;
        switch (_module_info[slot_num].module_id) {
            case SN76489:
                mute_sn76489(slot_num);
            case YM2608:
                init_ym2608(slot_num); 
                break;
        }
    }

}

void module_mute(void) {

    int slot_num;

    for (slot_num = 0; slot_num < MODULE_COUNT; slot_num++) {
        if (_module_info[slot_num].module_id <= 0)
            continue;

        switch (_module_info[slot_num].module_id) {
            case AY8910:
                mute_ay8910(slot_num);
                break;
            case SN76489:
                mute_sn76489(slot_num);
                break;
            case Y8950:
                mute_y8950(slot_num);
                break;
            case YM2151:
                mute_ym2151(slot_num);
                break;
            case YM2203:
                mute_ym2203(slot_num);
                break;
            case YM2413:
                mute_ym2413(slot_num);
                break;
            case YM2608:
                mute_ym2608(slot_num);
                break;
            case YM2612:
                mute_ym2612(slot_num);
                break;
            case YM3526:
                mute_ym3526(slot_num);
                break;
            case YM3812:
                mute_ym3812(slot_num);
                break;
        }
    }
}

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
//    re_cs0(!(n & 1));
    delayMicroseconds(20);

    re_address(1);
    re_write_data(dd);
//    re_cs0(n & 1);
    re_wr(0);
    delayMicroseconds(1);
    re_wr(1);
//    re_cs0(!(n & 1));
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
//    re_cs0(!(n & 1));
    re_wr(1);
    delayMicroseconds(6);

    re_address(1);
    re_write_data(dd);
    re_wr(0);
//    re_cs0(n & 1);
    delayMicroseconds(1);
//    re_cs0(!(n & 1));
    re_wr(1);
    delayMicroseconds(30);
}

void write_ym2608(int n, int pp, uint8_t aa, uint8_t dd) {

    re_cs0(n & 1);
    delayMicroseconds(1);

    re_address(pp << 1);
    re_write_data(aa);
    delayMicroseconds(2);
    re_wr(0);
    delayMicroseconds(10);
    re_wr(1);
    delayMicroseconds(5);
    if (aa >=0x21 && aa <= 0xb6) {
        delayMicroseconds(3);  /* FM: 17 cycles */
    }
    else
    if (pp == 0) {
        if (aa >= 0x10 && aa <= 0x1d) {
            delayMicroseconds(3);  /* Rhythm: 17 cycles */
        }
    }

    re_address((pp << 1) | 1);
    re_write_data(dd);
    delayMicroseconds(2);
    re_wr(0);
    delayMicroseconds(10);
    re_wr(1);
    delayMicroseconds(5);
    if (aa >= 0x21 && aa <= 0x9e) {
        delayMicroseconds(12);  /* FM: 83 cycles */
    }
    else
    if (aa >= 0xa0 && aa <= 0xb6) {
        delayMicroseconds(6);  /* FM: 47 cycles */
    }
    else
    if (pp == 0) {
        if (aa == 0x10) {
            // delayMicroseconds(75);  /* Rhythm: 576 cycles */
            delayMicroseconds(100);  /* Rhythm: 576 cycles */
        }
        else
        if (aa >= 0x11 && aa <= 0x1d) {
            // delayMicroseconds(12);  /* Rhythm: 83 cycles */
            delayMicroseconds(18);  /* Rhythm: 83 cycles */
        }
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

