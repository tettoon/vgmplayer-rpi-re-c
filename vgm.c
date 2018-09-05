#include "vgm.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

const char *VGM_IDENT = "Vgm ";

int _vgm_read_uint32(vgm_t *, uint32_t *);
int _vgm_read_uint24(vgm_t *, uint32_t *);
int _vgm_read_uint16(vgm_t *, uint16_t *);
int _vgm_read_uint8(vgm_t *, uint8_t *);
int _vgm_skip(vgm_t *, int);

int _vgm_ay8910(vgm_t *vgm, uint8_t *command);
int _vgm_sn76489(vgm_t *vgm, uint8_t *command);
int _vgm_y8950(vgm_t *vgm, uint8_t *command);
int _vgm_ym2151(vgm_t *vgm, uint8_t *command);
int _vgm_ym2203(vgm_t *vgm, uint8_t *command);
int _vgm_ym2413(vgm_t *vgm, uint8_t *command);
int _vgm_ym2608(vgm_t *vgm, uint8_t *command);
int _vgm_ym2612(vgm_t *vgm, uint8_t *command);
int _vgm_ym3526(vgm_t *vgm, uint8_t *command);
int _vgm_ym3812(vgm_t *vgm, uint8_t *command);
int _vgm_ymf262(vgm_t *vgm, uint8_t *command);
int _vgm_61(vgm_t *vgm, uint8_t *command);
int _vgm_62(vgm_t *vgm, uint8_t *command);
int _vgm_63(vgm_t *vgm, uint8_t *command);
int _vgm_64(vgm_t *vgm, uint8_t *command);
int _vgm_66(vgm_t *vgm, uint8_t *command);
int _vgm_67(vgm_t *vgm, uint8_t *command);
int _vgm_68(vgm_t *vgm, uint8_t *command);
int _vgm_70(vgm_t *vgm, uint8_t *command);
int _vgm_ym2612_2a(vgm_t *vgm, uint8_t *command);
int _vgm_90(vgm_t *vgm, uint8_t *command);
int _vgm_91(vgm_t *vgm, uint8_t *command);
int _vgm_92(vgm_t *vgm, uint8_t *command);
int _vgm_93(vgm_t *vgm, uint8_t *command);
int _vgm_94(vgm_t *vgm, uint8_t *command);
int _vgm_95(vgm_t *vgm, uint8_t *command);
int _vgm_e0(vgm_t *vgm, uint8_t *command);

typedef int (*vgm_processor_t)(vgm_t*, uint8_t*);

vgm_processor_t _processors[] = {
    /* 0x00 - 0x3f */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0x40 - 0x4f */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0x50 - 0x5f */
    _vgm_ym2413, _vgm_ym2612, _vgm_ym2612, _vgm_ym2151, _vgm_ym2203, _vgm_ym2608, _vgm_ym2608,
    NULL, NULL, _vgm_ym3812, _vgm_ym3526, _vgm_y8950, NULL, _vgm_ymf262, _vgm_ymf262,

    /* 0x60 - 0x6f */
    NULL, _vgm_61, _vgm_62, _vgm_63, _vgm_64, NULL, _vgm_66, _vgm_67,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0x70 - 0x7f */
    _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70,
    _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70, _vgm_70,

    /* 0x80 - 0x8f */
    _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a,
    _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a, _vgm_ym2612_2a,

    /* 0x90 - 0x9f */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xa0 - 0xaf */
    _vgm_ay8910, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xb0 - 0xbf */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xc0 - 0xcf */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xd0 - 0xdf */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xe0 - 0xef */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,

    /* 0xf0 - 0xff */
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

void (*vgm_handler)(int module, int module_num, uint16_t aa, uint8_t dd);

uint8_t *_vgm_datablock;
uint32_t _vgm_datablock_size;
uint8_t *_vgm_datablock_current;

void* _vgm_play_thread(vgm_t *vgm);
int _vgm_seterr(vgm_t *, int);

int _vgm_check_ident(vgm_t *vgm) {

    if (strncmp(VGM_IDENT, (const char *) vgm->buf, 4) != 0) {
        return _vgm_seterr(vgm, VGM_EIDENT);
    }
    return VGM_OK;
}

void _vgm_init_pos(vgm_t *vgm) {
    vgm_header_t *header = (vgm_header_t *) vgm->buf;
    if (header->version < 0x150) {
        vgm->pos = 0x40;
    } else {
        vgm->pos = 0x34 + header->vgm_data_offset;
    }
}

double _timespec_double(struct timespec *tp) {
    return tp->tv_sec + tp->tv_nsec / 1000000000.0;
}

double _timespec_span(struct timespec *tp1, struct timespec *tp2) {
    double t1, t2;
    t1 = _timespec_double(tp1);
    t2 = _timespec_double(tp2);
    return t2 - t1;
}

void vgm_load(void *buf, size_t bufsz, vgm_t *vgm) {

    vgm->buf = (uint8_t *) buf;
    vgm->buf_sz = bufsz;
    vgm->header = (vgm_header_t *) vgm->buf;
    vgm->pos = 0;
    vgm->frequency = 44100;
    vgm->samples = 0;
    vgm->samples_62 = 735;
    vgm->samples_63 = 882;
    _vgm_seterr(vgm, VGM_OK);
}

int vgm_play(vgm_t *vgm) {

    int rc;
    void *status = NULL;
    pthread_t thread;

    if ((rc = _vgm_check_ident(vgm)) != VGM_OK) return rc;

    vgm->playing = 1;
    vgm->_repeat = vgm->repeat;

    if ((rc = pthread_create(&thread, NULL, (void*) _vgm_play_thread, (void*) vgm)) != 0) {
        _vgm_seterr(vgm, VGM_EPTHREAD);
        return rc;
    }

    if ((rc = pthread_join(thread, &status)) != 0) {
        _vgm_seterr(vgm, VGM_EPTHREAD);
        return rc;
    }

    if (status != NULL) {
        rc = * (int *) status;
    } else {
        rc = -1;
    }
    free(status);

    return rc;
}

int _vgm_read_uint32(vgm_t *vgm, uint32_t *data) {

    if (vgm->pos >= vgm->buf_sz - 4)
        return _vgm_seterr(vgm, VGM_EOVERFLOW);

    *data = 0;
    *data |= vgm ->buf[vgm->pos++];
    *data |= (vgm ->buf[vgm->pos++]) << 8;
    *data |= (vgm ->buf[vgm->pos++]) << 16;
    *data |= (vgm ->buf[vgm->pos++]) << 24;

    return VGM_OK;
}

int _vgm_read_uint24(vgm_t *vgm, uint32_t *data) {

    if (vgm->pos >= vgm->buf_sz - 3)
        return _vgm_seterr(vgm, VGM_EOVERFLOW);

    *data = 0;
    *data |= vgm ->buf[vgm->pos++];
    *data |= (vgm ->buf[vgm->pos++]) << 8;
    *data |= (vgm ->buf[vgm->pos++]) << 16;

    return VGM_OK;
}

int _vgm_read_uint16(vgm_t *vgm, uint16_t *data) {

    if (vgm->pos >= vgm->buf_sz - 2)
        return _vgm_seterr(vgm, VGM_EOVERFLOW);

    *data = 0;
    *data |= vgm ->buf[vgm->pos++];
    *data |= (vgm ->buf[vgm->pos++]) << 8;

    return VGM_OK;
}

int _vgm_read_uint8(vgm_t *vgm, uint8_t *data) {

    if (vgm->pos >= vgm->buf_sz - 1)
        return _vgm_seterr(vgm, VGM_EOVERFLOW);

    *data = vgm->buf[vgm->pos++];

    return VGM_OK;
}

int _vgm_skip(vgm_t *vgm, int len) {

    if (vgm->pos >= vgm->buf_sz - len)
        return _vgm_seterr(vgm, VGM_EOVERFLOW);

    vgm->pos += len;

    return VGM_OK;
}

int _vgm_process_command(vgm_t *vgm, uint8_t *command) {

    // printf("Command: 0x%02X\n", *command);

    switch (*command) {
        case 0x4f:
            return _vgm_skip(vgm, 1);
        case 0x50:
            return _vgm_sn76489(vgm, command);
        case 0x51:
            return _vgm_ym2413(vgm, command);
        case 0x52:
        case 0x53:
            return _vgm_ym2612(vgm, command);
        case 0x54:
            return _vgm_ym2151(vgm, command);
        case 0x55:
            return _vgm_ym2203(vgm, command);
        case 0x56:
        case 0x57:
            return _vgm_ym2608(vgm, command);
        case 0x5a:
            return _vgm_ym3812(vgm, command);
        case 0x5b:
            return _vgm_ym3526(vgm, command);
        case 0x5c:
            return _vgm_y8950(vgm, command);
        case 0x5e:
        case 0x5f:
            return _vgm_ymf262(vgm, command);
        case 0x61:
            return _vgm_61(vgm, command);
        case 0x62:
            return _vgm_62(vgm, command);
        case 0x63:
            return _vgm_63(vgm, command);
        case 0x64:
            return _vgm_64(vgm, command);
        case 0x66:
            return _vgm_66(vgm, command);
        case 0x67:
            return _vgm_67(vgm, command);
        case 0x68:
            // printf("Command: 0x%02x\n", *command);
            return _vgm_68(vgm, command);
        case 0x90:
            return _vgm_90(vgm, command);
        case 0x91:
            return _vgm_91(vgm, command);
        case 0x92:
            return _vgm_92(vgm, command);
        case 0x93:
            return _vgm_93(vgm, command);
        case 0x94:
            return _vgm_94(vgm, command);
        case 0x95:
            return _vgm_95(vgm, command);
        case 0xa0:
            return _vgm_ay8910(vgm, command);
        case 0xe0:
            return _vgm_e0(vgm, command);
    }

    switch (*command & 0xf0) {
        case 0x30:
            return _vgm_skip(vgm, 1);
        case 0x40:
        case 0x50:
            return _vgm_skip(vgm, 2);
        case 0x70:
            return _vgm_70(vgm, command);
        case 0x80:
            return _vgm_ym2612_2a(vgm, command);
        case 0xa0:
        case 0xb0:
            return _vgm_skip(vgm, 2);
        case 0xc0:
        case 0xd0:
            return _vgm_skip(vgm, 3);
        case 0xe0:
        case 0xf0:
            return _vgm_skip(vgm, 4);
        default:
            return _vgm_seterr(vgm, VGM_ECOMMAND);
    }

/*
    vgm_processor_t processor = _processors[(int) command];
    if (processor == NULL) {
        return 0;
    }

    printf("Found processor\n");
    rc = processor(vgm, command);

    return rc;
*/
}

void* _vgm_play_thread(vgm_t *vgm) {

    int rc;
    int *status;
    uint8_t command;

    _vgm_init_pos(vgm);

    struct timespec tp_origin;
    struct timespec tp_current;
    double elapsed_time = 0.0;
    double next_time = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &tp_origin);

    while (vgm->playing != 0) {
        if ((rc = _vgm_read_uint8(vgm, &command)) != VGM_OK) break;

        if ((rc = _vgm_process_command(vgm, &command)) != VGM_OK) break;
        
        next_time = vgm->samples * 1.0 / vgm->frequency;
        while (elapsed_time == 0.0 || elapsed_time < next_time) {
            clock_gettime(CLOCK_MONOTONIC, &tp_current);
            elapsed_time = _timespec_span(&tp_origin, &tp_current);
        }

        if (command == 0x67) {
            clock_gettime(CLOCK_MONOTONIC, &tp_origin);
            vgm->samples = 0;
        }
    }

    status = malloc(sizeof(int));
    *status = rc;

    return status;
}

int _vgm_ay8910(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(AY8910, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_sn76489(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(SN76489, 0, 0, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_y8950(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(Y8950, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym2151(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(YM2151, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym2203(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(YM2203, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym2413(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(YM2413, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym2608(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    int port = (*command == 0x56) ? 0 : 1;
    uint16_t ppaa = port << 8 | aa;

    vgm_handler(YM2608, 0, ppaa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym2612(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    int port = (*command == 0x52) ? 0 : 1;
    uint16_t ppaa = port << 8 | aa;

    vgm_handler(YM2612, 0, ppaa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym3526(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(YM3526, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ym3812(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t aa = 0;
    uint8_t dd = 0;

    if ((rc = _vgm_read_uint8(vgm, &aa)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &dd)) != VGM_OK) return rc;

    vgm_handler(YM3812, 0, aa, dd);

    vgm->samples++;
    return VGM_OK;
}

int _vgm_ymf262(vgm_t *vgm, uint8_t *command) {
    vgm->samples++;
    return VGM_OK;
}

int _vgm_61(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint16_t nnnn = 0;

    if ((rc = _vgm_read_uint16(vgm, &nnnn)) != VGM_OK) return rc;
    vgm->samples += nnnn;

    return VGM_OK;
}

int _vgm_62(vgm_t *vgm, uint8_t *command) {
    vgm->samples += vgm->samples_62;
    return VGM_OK;
}

int _vgm_63(vgm_t *vgm, uint8_t *command) {
    vgm->samples += vgm->samples_63;
    return VGM_OK;
}

int _vgm_64(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t cc = 0;
    uint16_t nnnn = 0;

    if ((rc = _vgm_read_uint8(vgm, &cc)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint16(vgm, &nnnn)) != VGM_OK) return rc;

    switch (cc) {
        case 0x62:
            vgm->samples_62 = nnnn;
            return VGM_OK;
        case 0x63:
            vgm->samples_63 = nnnn;
            return VGM_OK;
        default:
            return _vgm_seterr(vgm, VGM_ECOMMAND);
    }
}

int _vgm_66(vgm_t *vgm, uint8_t *command) {

    if (vgm->header->loop_offset == 0) {
        vgm->playing = 0;
    }
    else
    if (vgm->repeat > 0 && --vgm->_repeat <= 0) {
        vgm->playing = 0;
    }
    else
    {
        vgm->pos = vgm->header->loop_offset + 0xc;
    }
    return VGM_OK;
}

int _vgm_67(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint8_t tt = 0;
    uint32_t sz = 0;

    if ((rc = _vgm_skip(vgm, 1)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint8(vgm, &tt)) != VGM_OK) return rc;
    if ((rc = _vgm_read_uint32(vgm, &sz)) != VGM_OK) return rc;

    // printf("Data block type: 0x%02X\n", tt);
    // printf("Size of data: 0x%08X\n", sz);

    if (tt == 0x00) {
        _vgm_datablock = &vgm->buf[vgm->pos];
        _vgm_datablock_size = sz;
        _vgm_datablock_current = _vgm_datablock;

        if ((rc = _vgm_skip(vgm, sz)) != VGM_OK) return rc;
        return VGM_OK;
    }
    else
    if (tt >= 0x80 && tt <= 0xbf) {
        uint32_t rom_size = 0;
        uint32_t rom_start = 0;
        uint32_t rom_end;
        uint8_t  rom_data = 0;

        if ((rc = _vgm_read_uint32(vgm, &rom_size)) != VGM_OK) return rc;
        if ((rc = _vgm_read_uint32(vgm, &rom_start)) != VGM_OK) return rc;
        rom_end = rom_start + sz - 8 - 1;

        // printf("ROM size: %08X\n", rom_size);
        // printf("ROM start address: %08X\n", rom_start);
        // printf("ROM end address  : %08X\n", rom_end);

        if (tt == 0x81) {
            uint32_t start_addr = rom_start >> 2;
            uint32_t   end_addr = rom_end   >> 2;

            vgm_handler(YM2608, 0, 0x100, 0x00);
            vgm_handler(YM2608, 0, 0x100, 0x01);
            vgm_handler(YM2608, 0, 0x101, 0x00);
            vgm_handler(YM2608, 0, 0x102, start_addr & 0xff);
            vgm_handler(YM2608, 0, 0x103, (start_addr >> 8) & 0xff);
            vgm_handler(YM2608, 0, 0x104, end_addr & 0xff);
            vgm_handler(YM2608, 0, 0x105, (end_addr >> 8) & 0xff);
            vgm_handler(YM2608, 0, 0x10c, 0xff);
            vgm_handler(YM2608, 0, 0x10d, 0xff);
            vgm_handler(YM2608, 0, 0x110, 0x1f);
            vgm_handler(YM2608, 0, 0x100, 0x60);

            for (int i = 0; i < sz - 8; i++) {
                if ((rc = _vgm_read_uint8(vgm, &rom_data)) != VGM_OK) return rc;
                vgm_handler(YM2608, 0, 0x108, rom_data);
            }

            vgm_handler(YM2608, 0, 0x100, 0x00);
            vgm_handler(YM2608, 0, 0x110, 0x80);

            return VGM_OK;
        }
        if (tt == 0x188) {
            uint32_t start_addr = rom_start >> 2;
            uint32_t   end_addr = rom_end   >> 2;

            vgm_handler(Y8950, 0, 0x07, 0x00);
            vgm_handler(Y8950, 0, 0x07, 0x01);
            vgm_handler(Y8950, 0, 0x07, 0x00);

            vgm_handler(Y8950, 0, 0x04, 0x78);
            //vgm_handler(Y8950, 0, 0x04, 0x00);
            vgm_handler(Y8950, 0, 0x04, 0x80);
            vgm_handler(Y8950, 0, 0x07, 0x60);
            vgm_handler(Y8950, 0, 0x08, 0x00);
            vgm_handler(Y8950, 0, 0x09, start_addr & 0xff);
            vgm_handler(Y8950, 0, 0x0a, (start_addr >> 8) & 0xff);
            vgm_handler(Y8950, 0, 0x0b, end_addr & 0xff);
            vgm_handler(Y8950, 0, 0x0c, (end_addr >> 8) & 0xff);

            for (int i = 0; i < sz - 8; i++) {
                if ((rc = _vgm_read_uint8(vgm, &rom_data)) != VGM_OK) return rc;
                vgm_handler(Y8950, 0, 0x0f, rom_data);
            }

            vgm_handler(Y8950, 0, 0x07, 0x00);
            vgm_handler(Y8950, 0, 0x04, 0x78);
            vgm_handler(Y8950, 0, 0x04, 0x80);

            return VGM_OK;
        }

        if ((rc = _vgm_skip(vgm, sz - 8)) != VGM_OK) return rc;

        return VGM_OK;
    }

    if ((rc = _vgm_skip(vgm, sz)) != VGM_OK) return rc;

    return VGM_OK;
}

int _vgm_68(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 11);
    return VGM_OK;
}

int _vgm_70(vgm_t *vgm, uint8_t *command) {

    vgm->samples += *command - 0x6f;
    return VGM_OK;
}

int _vgm_ym2612_2a(vgm_t *vgm, uint8_t *command) {

    uint8_t dd = 0;
    dd = *_vgm_datablock_current;
    _vgm_datablock_current++;

    vgm_handler(YM2612, 0, 0x2a, dd);

    vgm->samples += *command - 0x80;
    return VGM_OK;
}

int _vgm_90(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 4);
    return VGM_OK;
}

int _vgm_91(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 4);
    return VGM_OK;
}

int _vgm_92(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 5);
    return VGM_OK;
}

int _vgm_93(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 10);
    return VGM_OK;
}

int _vgm_94(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 1);
    return VGM_OK;
}

int _vgm_95(vgm_t *vgm, uint8_t *command) {

    _vgm_skip(vgm, 4);
    return VGM_OK;
}

int _vgm_e0(vgm_t *vgm, uint8_t *command) {

    int rc;
    uint32_t offset = 0;

    rc = _vgm_read_uint32(vgm, &offset);
    if (rc != 0) return rc;

    _vgm_datablock_current = _vgm_datablock + offset;
    return VGM_OK;
}

void vgm_clearerr(vgm_t *vgm) {
    _vgm_seterr(vgm, VGM_OK);
}

int _vgm_seterr(vgm_t *vgm, int errnum) {
    vgm->vgm_errno = errnum;
    vgm->vgm_errmsg = vgm_strerror(vgm->vgm_errno);
    return errnum;
}

int vgm_error(vgm_t *vgm) {
    return vgm->vgm_errno;
}

void vgm_perror(vgm_t *vgm, const char *msg) {

    if (msg == NULL) {
        fprintf(stderr, "%s\n", vgm->vgm_errmsg);
    } else {
        fprintf(stderr, "%s: %s\n", msg, vgm->vgm_errmsg);
    }
}

char * vgm_strerror(int errnum) {
    switch(errnum) {
        case VGM_OK:
            return "Success";
        case VGM_EIDENT:
            return "Invalid VGM identifier";
        case VGM_EOVERFLOW:
            return "Overflow";
        case VGM_EPTHREAD:
            return "Thread error";
        case VGM_ECOMMAND:
            return "Invalid command";
        case VGM_EDATATYPE:
            return "Invalid or unsupported data type";
        default:
            return "Undefined error";
    }
}

