#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <zlib.h>
#include <wiringPi.h>

#include "s98.h"
#include "raspi_re.h"
#include "modules.h"

static struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"module", required_argument, NULL, 'm'},
    {"repeat", required_argument, NULL, 'r'},
    {0, 0, 0, 0}
};

void print_usage(FILE *);
void sigint_handler(int);
int s98player_play(FILE *, const char *);
int copy_file(FILE *, FILE *);
#if 0
int gunzip_file(FILE *, FILE *);
#endif

int _repeat_count;

s98_t *s98;

void print_usage(FILE *f) {

    fprintf(f, "Usage: s98player [option] <file>\n");
    fprintf(f, "\n");
    fprintf(f, "Options:\n");
    fprintf(f, "\n");
    fprintf(f, "  -                 Read from STDIN.\n");
    fprintf(f, "\n");
    fprintf(f, "  -h, --help        Print this message.\n");
    fprintf(f, "\n");
    fprintf(f, "  -m <name>, --module=<name>\n");
    fprintf(f, "                    Specify connected module names (comma separated).\n");
    // fprintf(f, "                    e.g., AY8910, SN76489, YM2151, YM2203, YM2413, YM2608, YM2612, YM3526, YM3812, YMF262\n");
    fprintf(f, "                    e.g. YM2151,YM2608\n");
    fprintf(f, "  -r <count>, --repeat=<count>\n");
    fprintf(f, "                    Specify a repeat count (>1).\n");
    fprintf(f, "\n");
    fprintf(f, "Supported modules:\n");
    fprintf(f, "\n");
    fprintf(f, "  YM2151\n");
    fprintf(f, "  YM2608\n");
    fprintf(f, "\n");
}

void sigint_handler(int sig) {
    s98->playing = 0;
}

int main(int argc, char *argv[]) {

    int rc;
    int opt, opterr = 0;
    char *module_name = NULL;
    _repeat_count = -1;

    while ((opt = getopt_long(argc, argv, "hm:r:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(stdout);
                return 0;
            case 'm':
                module_name = optarg;
                break;
            case 'r':
                if (sscanf(optarg, "%d", &_repeat_count) == EOF) {
                    print_usage(stderr);
                    return EXIT_FAILURE;
                }
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

    if (module_name == NULL) {
        fprintf(stderr, "Module name must be specified.\n");
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    module_prepare_info(module_name);

    if (_repeat_count != -1 && _repeat_count < 1) {
        fprintf(stderr, "Repeat count must be greater than or equal to 1.\n");
        print_usage(stderr);
        return EXIT_FAILURE;
    }

    s98_handler = module_handler;

    rc = re_init();
    if (rc != 0)  {
        fprintf(stderr, "Initialization failed.\n");
        return EXIT_FAILURE;
    }

    re_reset();
    module_init();

    if (optind == argc) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }

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

        rc = s98player_play(f, filename);
#if 0
        if (rc == S98_EIDENT) {
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

            rc = s98player_play(f2, filename);
            if (rc != S98_OK) {
                fclose(f2);
                return EXIT_FAILURE;
            }

            fclose(f2);
        }
#endif

        if (rc != S98_OK) {
            s98_perror(s98, "xxx");
            return EXIT_FAILURE;
        }
    }

    module_mute();

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

#if 0
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
#endif

int s98player_play(FILE *f, const char *name) {

    int rc;
    int fd;
    struct stat stbuf;
    uint8_t *s98_buf;

    fd = fileno(f);
    fstat(fd, &stbuf);

    s98_buf = malloc(stbuf.st_size);

    fread(s98_buf, 1, stbuf.st_size, f);
    if (ferror(f) != 0) {
        free(s98_buf);
        perror(name);
        return -1;
    }

    // fclose(f);

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        free(s98_buf);
        perror(name);
        return -1;
    }

    s98 = malloc(sizeof(s98_t));
    s98->repeat = _repeat_count;
    s98_load(s98_buf, stbuf.st_size, s98);

    rc = s98_play(s98);
    if (rc != 0) {
        free(s98);
        free(s98_buf);
        return rc;
    }

    free(s98);
    free(s98_buf);

    return 0;
}

