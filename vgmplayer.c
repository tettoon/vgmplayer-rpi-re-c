#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <zlib.h>
#include <wiringPi.h>

#include "vgm.h"
#include "raspi_re.h"
#include "modules.h"

static struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"module", required_argument, NULL, 'm'},
    {0, 0, 0, 0}
};

void print_usage(FILE *);
void sigint_handler(int);
int module_id(const char *);
void module_handler(int, int, uint16_t, uint8_t);
void module_init(int);
void module_mute(int);
int vgmplayer_play(FILE *, const char *);
int copy_file(FILE *, FILE *);
int gunzip_file(FILE *, FILE *);

int _module_id;

vgm_t *vgm;

void print_usage(FILE *f) {

    fprintf(f, "Usage: vgmplay [option] <vgm_file>\n");
    fprintf(f, "\n");
    fprintf(f, "-h, --help          Print this message.\n");
    fprintf(f, "\n");
    fprintf(f, "-m <name>, --module=<name>\n");
    fprintf(f, "                    Specify a module name.\n");
    fprintf(f, "                    e.g., AY8910, SN76489, YM2151, YM2413, YM2608, YM2612, YM3526, YM3812\n");
    fprintf(f, "\n");
}

void sigint_handler(int sig) {
    vgm->playing = 0;
}

int module_id(const char *module_name) {

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

void module_handler(int module, int num, uint16_t ppaa, uint8_t dd) {

    int pp = ppaa >> 8;
    uint8_t aa = ppaa & 0xff;

    if (module != _module_id) return;

    switch (module) {
        case AY8910:
            write_ay8910(num, aa, dd);
            break;
        case SN76489:
            write_sn76489(num, dd);
            break;
        case Y8950:
            write_y8950(num, aa, dd);
            break;
        case YM2151:
            write_ym2151(num, aa, dd);
            break;
        case YM2203:
            write_ym2203(num, aa, dd);
            break;
        case YM2413:
            write_ym2413(num, aa, dd);
            break;
        case YM2608:
            write_ym2608(num, pp, aa, dd);
            break;
        case YM2612:
            write_ym2612(num, pp, aa, dd);
            break;
        case YM3526:
            write_ym3526(num, aa, dd);
            break;
        case YM3812:
            write_ym3812(num, aa, dd);
            break;
    }
}

void module_init(int module) {

    if (module == 0) return;

    if (module == YM2608) {
        init_ym2608(0); 
    }
}

void module_mute(int module) {

    if (module == 0) return;

    if (module == AY8910) {
        mute_ay8910(0);
    }
    else
    if (module == SN76489) {
        mute_sn76489(0);
    }
    else
    if (module == Y8950) {
        mute_y8950(0);
    }
    else
    if (module == YM2151) {
        mute_ym2151(0);
    }
    else
    if (module == YM2203) {
        mute_ym2203(0);
    }
    else
    if (module == YM2413) {
        mute_ym2413(0);
    }
    else
    if (module == YM2608) {
        mute_ym2608(0);
    }
    else
    if (module == YM2612) {
        mute_ym2612(0);
    }
    else
    if (module == YM3526) {
        mute_ym3526(0);
    }
    else
    if (module == YM3812) {
        mute_ym3812(0);
    }
}

int main(int argc, char *argv[]) {

    int rc;
    int opt, opterr = 0;
    char *module_name = NULL;

    while ((opt = getopt_long(argc, argv, "hm:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(stdout);
                return 0;
            case 'm':
                module_name = optarg;
                break;
            default:
                opterr = 1;
                break;
        }
    }

    if (opterr != 0) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    _module_id = module_id(module_name);
    if (_module_id == 0) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    else
    if (_module_id == -1) {
        fprintf(stderr, "Invalid or unsupported module name\n");
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    vgm_handler = module_handler;

    re_reset();
    module_init(_module_id);

    if (optind == argc) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    re_init();

    FILE *f;
    char *filename;
    for (int i = optind; i < argc; i++) {
        if (strcmp("-", argv[i]) == 0) {
            filename = "tmpfile()";

            f = tmpfile();
            if (!f) {
                perror(filename);
                return EXIT_FAILURE;
            }

            copy_file(stdin, f);
            if (ferror(stdin) != 0) {
                perror("stdin");
                return EXIT_FAILURE;
            }
            else
            if (ferror(f) != 0) {
                perror(filename);
                return EXIT_FAILURE;
            }
        } else {
            filename = argv[i];

            f = fopen(filename, "rb");
            if (!f) {
                perror(filename);
                return EXIT_FAILURE;
            }
        }

        rc = vgmplayer_play(f, filename);
        if (rc == VGM_EIDENT) {
            rewind(f);

            FILE *f2 = tmpfile();

            rc = gunzip_file(f, f2);
            if (ferror(f) != 0) {
                perror(filename);
                return EXIT_FAILURE;
            }
            else
            if (ferror(f2) != 0) {
                perror(filename);
                return EXIT_FAILURE;
            }

            fclose(f);

            rc = vgmplayer_play(f2, filename);
            if (rc != VGM_OK) {
                fclose(f2);
                return EXIT_FAILURE;
            }

            fclose(f2);
        } else if (rc != VGM_OK) {
            vgm_perror(vgm, "xxx");
            return EXIT_FAILURE;
        }
    }

    module_mute(_module_id);

    return EXIT_SUCCESS;
}

int copy_file(FILE *f1, FILE *f2) {

    uint8_t *fbuf;
    size_t sz;

    fbuf = (uint8_t *) malloc(65536);

    while(1) {
        if (feof(f1) != 0)
            break;

        sz = fread(fbuf, 1, 65536, f1);
        if (ferror(f1) != 0)
            break;

        sz = fwrite(fbuf, 1, sz, f2);
        if (ferror(f2) != 0)
            break;
    }

    if (fflush(f2) != 0) {
        free(fbuf);
        return -1;
    }

    free(fbuf);

    if (ferror(f1) != 0 || ferror(f2) != 0)
        return -1;

    rewind(f2);

    return 0;
}

int gunzip_file(FILE *f1, FILE *f2) {

    uint8_t *fbuf;
    int rc;
    int fd1;
    gzFile gzf;

    fbuf = (uint8_t *) malloc(8192);
    if (fbuf == NULL) {
        fprintf(stderr, "malloc() failed\n");
        return -1;
    }

    fd1 = fileno(f1);
    lseek(fd1, 0, SEEK_SET);
    gzf = gzdopen(fd1, "rb");
    if (gzf == NULL) {
        printf("gzf == NULL\n");
        free(fbuf);
        return -1;
    }

    while(1) {
        if ((rc = gzread(gzf, fbuf, 8192)) == -1) {
            const char *msg = gzerror(gzf, &rc);
            if (rc == Z_ERRNO)
                msg = strerror(rc);
            fprintf(stderr, "zlib error: %s\n", msg);
            break;
        }
        else
        if (rc == 0) {
            break;
        }

        fwrite(fbuf, 1, rc, f2);
        if (ferror(f2) != 0) {
            return -1;
        }
    }

    if ((rc = gzclose(gzf)) != Z_OK) {
        free(fbuf);
        const char *msg = gzerror(gzf, &rc);
        if (rc == Z_ERRNO)
            msg = strerror(rc);
        fprintf(stderr, "zlib error: %s\n", msg);
        return -1;
    }

    if (fflush(f2) != 0) {
        free(fbuf);
        return -1;
    }

    free(fbuf);

    if (ferror(f1) != 0 || ferror(f2) != 0)
        return -1;

    rewind(f2);

    return 0;
}

int vgmplayer_play(FILE *f, const char *name) {

    int rc;
    int fd;
    struct stat stbuf;
    uint8_t *vgm_buf;

    fd = fileno(f);
    fstat(fd, &stbuf);

    vgm_buf = malloc(stbuf.st_size);

    fread(vgm_buf, 1, stbuf.st_size, f);
    if (ferror(f) != 0) {
        free(vgm_buf);
        perror(name);
        return -1;
    }

    // fclose(f);

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        free(vgm_buf);
        perror(name);
        return -1;
    }

    vgm = malloc(sizeof(vgm_t));
    vgm_load(vgm_buf, stbuf.st_size, vgm);

    rc = vgm_play(vgm);
    if (rc != 0) {
        free(vgm);
        free(vgm_buf);
        return rc;
    }

    free(vgm);
    free(vgm_buf);

    return 0;
}

