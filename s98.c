#include "s98.h"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "modules.h"

const char *S98_IDENT = "S98";

int _s98_read_uint32(s98_t *, uint32_t *);
int _s98_read_uint24(s98_t *, uint32_t *);
int _s98_read_uint16(s98_t *, uint16_t *);
int _s98_read_uint8(s98_t *, uint8_t *);
int _s98_skip(s98_t *, int);
int _s98_fe(s98_t*);
int _s98_fd(s98_t*);

int _s98_ay8910(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_sn76489(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym2151(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym2203(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym2413(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym2608(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym2612(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym3526(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ym3812(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);
int _s98_ymf262(s98_t*, uint8_t, uint8_t, uint8_t, uint8_t);

void (*s98_handler)(int module, int module_num, uint16_t aa, uint8_t dd);

void* _s98_play_thread(s98_t *s98);
int _s98_seterr(s98_t *, int);

static int _reset_time;

int _s98_check_ident(s98_t *s98) {

    if (strncmp(S98_IDENT, (const char *) s98->buf, 0) != 0) {
        return _s98_seterr(s98, S98_EIDENT);
    }
    return S98_OK;
}

void _s98_init_pos(s98_t *s98) {
    s98->pos = s98->header->offset_dump_data;
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

void s98_load(void *buf, size_t bufsz, s98_t *s98) {

    double timer_info, timer_info2;

    s98->buf = (uint8_t *) buf;
    s98->buf_sz = bufsz;
    s98->header = (s98_header_t *) s98->buf;
    s98->pos = 0;
    s98->samples = 0;

    timer_info = s98->header->timer_info;
    if (timer_info == 0) timer_info = 10;

    timer_info2 = s98->header->timer_info2;
    if (timer_info2 == 0) timer_info2 = 1000;

    s98->frequency = (uint32_t) (1.0 / (timer_info / timer_info2));

    _s98_seterr(s98, S98_OK);
}

int s98_play(s98_t *s98) {

    int rc;
    void *status = NULL;
    pthread_t thread;

    if ((rc = _s98_check_ident(s98)) != S98_OK) return rc;

    _reset_time = 0;
    s98->playing = 1;
    s98->_repeat = s98->repeat;

    if ((rc = pthread_create(&thread, NULL, (void*) _s98_play_thread, (void*) s98)) != 0) {
        _s98_seterr(s98, S98_EPTHREAD);
        return rc;
    }

    if ((rc = pthread_join(thread, &status)) != 0) {
        _s98_seterr(s98, S98_EPTHREAD);
        if (status != NULL)
            free(status);
        return rc;
    }

    if (status != NULL) {
        rc = * (int *) status;
        _s98_seterr(s98, rc);
    } else {
        rc = -1;
    }

    free(status);

    return rc;
}

int _s98_read_uint32(s98_t *s98, uint32_t *data) {

    if (s98->pos >= s98->buf_sz - 3)
        return _s98_seterr(s98, S98_EOVERFLOW);

    *data = 0;
    *data |= s98 ->buf[s98->pos++];
    *data |= (s98 ->buf[s98->pos++]) << 8;
    *data |= (s98 ->buf[s98->pos++]) << 16;
    *data |= (s98 ->buf[s98->pos++]) << 24;

    return S98_OK;
}

int _s98_read_uint24(s98_t *s98, uint32_t *data) {

    if (s98->pos >= s98->buf_sz - 2)
        return _s98_seterr(s98, S98_EOVERFLOW);

    *data = 0;
    *data |= s98 ->buf[s98->pos++];
    *data |= (s98 ->buf[s98->pos++]) << 8;
    *data |= (s98 ->buf[s98->pos++]) << 16;

    return S98_OK;
}

int _s98_read_uint16(s98_t *s98, uint16_t *data) {

    if (s98->pos >= s98->buf_sz - 1)
        return _s98_seterr(s98, S98_EOVERFLOW);

    *data = 0;
    *data |= s98 ->buf[s98->pos++];
    *data |= (s98 ->buf[s98->pos++]) << 8;

    return S98_OK;
}

int _s98_read_uint8(s98_t *s98, uint8_t *data) {

    if (s98->pos >= s98->buf_sz)
        return _s98_seterr(s98, S98_EOVERFLOW);

    *data = s98->buf[s98->pos++];

    return S98_OK;
}

int _s98_device_info(s98_t *s98, int num) {
    if (num < s98->header->device_count) {
    }
    return S98_NONE;
}

int _s98_skip(s98_t *s98, int len) {

    if (s98->pos >= s98->buf_sz - len)
        return _s98_seterr(s98, S98_EOVERFLOW);

    s98->pos += len;

    return S98_OK;
}

int _s98_process_command(s98_t *s98, uint8_t *command) {

    // printf("Command: 0x%02X\n", *command);

    uint8_t device_num, extend;
    uint8_t aa = 0, dd = 0;
    uint32_t *device_count, *device_type;
    s98_device_info_t *device_info;
    int i, rc;

    switch (*command) {
        case 0xff:
            s98->samples++;
            break;
        case 0xfe:
            _s98_fe(s98);
            break;
        case 0xfd:
            _s98_fd(s98);
            break;
        default:
            if ((rc = _s98_read_uint8(s98, &aa)) != S98_OK) return rc;
            if ((rc = _s98_read_uint8(s98, &dd)) != S98_OK) return rc;

            device_count = &s98->header->device_count;
            device_num = (*command >> 1) & 0x7f;
            if (*device_count != 0 && device_num >= *device_count) {
                return S98_ECOMMAND;
            }
            extend = *command & 1;

            if (*device_count == 0)
            {
                _s98_ym2608(s98, 0, extend, aa, dd);
            }
            else
            {
                uint8_t num = 0;
                device_info = &s98->header->device_info[device_num];
                device_type = &(*device_info).device_type;
                for (i = 0; i < device_num - 1; i++) {
                    if ((s98->header->device_info[i]).device_type == *device_type)
                        num++;
                }
                switch(*device_type)
                {
                    case 2:
                        _s98_ym2203(s98, num, extend, aa, dd);
                        break;
                    case 4:
                        _s98_ym2608(s98, num, extend, aa, dd);
                        break;
                    case 5:
                        _s98_ym2151(s98, num, extend, aa, dd);
                        break;
                }
            }
    }

    return S98_OK;
}

void* _s98_play_thread(s98_t *s98) {

    int rc = 0;
    int *status;
    uint8_t command;

    _s98_init_pos(s98);

    struct timespec tp_origin;
    struct timespec tp_current;
    double elapsed_time = 0.0;
    double next_time = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &tp_origin);
    // clock_gettime(CLOCK_MONOTONIC, &s98->start_tp);

    while (s98->playing != 0) {
        if ((rc = _s98_read_uint8(s98, &command)) != S98_OK) break;

        if ((rc = _s98_process_command(s98, &command)) != S98_OK) break;

        if (_reset_time != 0) {
            s98->samples = 0;
            // clock_gettime(CLOCK_MONOTONIC, &s98->start_tp);
            clock_gettime(CLOCK_MONOTONIC, &tp_origin);
            _reset_time = 0;
            continue;
        }
        
        next_time = s98->samples * 1.0 / s98->frequency;
        while (elapsed_time == 0.0 || elapsed_time < next_time) {
            clock_gettime(CLOCK_MONOTONIC, &tp_current);
            // elapsed_time = _timespec_span(&s98->start_tp, &tp_current);
            elapsed_time = _timespec_span(&tp_origin, &tp_current);
        }

    }

    status = malloc(sizeof(int));
    *status = rc;

    return status;
}

int _s98_ay8910(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(AY8910, num, aa, dd);
    }

    return S98_OK;
}

int _s98_sn76489(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(SN76489, 0, 0, dd);
    }

    return S98_OK;
}

int _s98_ym2151(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(YM2151, num, aa, dd);
    }

    return S98_OK;
}

int _s98_ym2203(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(YM2203, num, aa, dd);
    }

    return S98_OK;
}

int _s98_ym2413(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(YM2413, num, aa, dd);
    }

    return S98_OK;
}

int _s98_ym2608(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    uint16_t ppaa = extend << 8 | aa;

    s98_handler(YM2608, num, ppaa, dd);

    if (extend != 0) {
        if (aa == 0)
            s98->opna_reg100[num] = dd;
        // printf("s98->opna_reg100[%d] = %02X\n", num, s98->opna_reg100[num]);
        if ((int) (s98->opna_reg100[num] & 0x60) == 0x60) {
            _reset_time = 1;
        }
    }

    return S98_OK;
}

int _s98_ym2612(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    uint16_t ppaa = extend << 8 | aa;

    s98_handler(YM2612, num, ppaa, dd);

    return S98_OK;
}

int _s98_ym3526(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(YM3526, num, aa, dd);
    }

    return S98_OK;
}

int _s98_ym3812(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    if (extend == 0)
    {
        s98_handler(YM3812, num, aa, dd);
    }

    return S98_OK;
}

int _s98_ymf262(s98_t *s98, uint8_t num, uint8_t extend, uint8_t aa, uint8_t dd) {

    uint16_t ppaa = extend << 8 | aa;

    s98_handler(YMF262, num, ppaa, dd);

    return S98_OK;
}

int _s98_fe(s98_t *s98) {
    
    uint8_t dd = 0;
    int rc = 0;

    int s = 0, n = 0;

    do
    {
        if ((rc = _s98_read_uint8(s98, &dd)) != S98_OK) return rc;
        n |= (dd & 0x7f) << s;
        s += 7;
    }
    while (dd & 0x80);

    s98->samples += n + 2;

    return S98_OK;
}

int _s98_fd(s98_t *s98) {

    if (s98->header->offset_loop_point == 0) {
        s98->playing = 0;
    }
    else
    if (s98->repeat > 0 && --s98->_repeat <= 0) {
        s98->playing = 0;
    }
    else
    {
        s98->pos = s98->header->offset_loop_point;
    }

    return S98_OK;
}

void s98_clearerr(s98_t *s98) {
    _s98_seterr(s98, S98_OK);
}

int _s98_seterr(s98_t *s98, int errnum) {
    s98->s98_errno = errnum;
    s98->s98_errmsg = s98_strerror(s98->s98_errno);
    return errnum;
}

int s98_error(s98_t *s98) {
    return s98->s98_errno;
}

void s98_perror(s98_t *s98, const char *msg) {

    if (msg == NULL) {
        fprintf(stderr, "%s\n", s98->s98_errmsg);
    } else {
        fprintf(stderr, "%s: %s\n", msg, s98->s98_errmsg);
    }
}

char * s98_strerror(int errnum) {
    switch(errnum) {
        case S98_OK:
            return "Success";
        case S98_EIDENT:
            return "Invalid S98 identifier";
        case S98_EOVERFLOW:
            return "Overflow";
        case S98_EPTHREAD:
            return "Thread error";
        case S98_ECOMMAND:
            return "Invalid command";
        case S98_EDATATYPE:
            return "Invalid or unsupported data type";
        default:
            return "Undefined error";
    }
}

