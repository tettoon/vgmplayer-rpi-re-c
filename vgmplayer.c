#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/stat.h>

#define _GNU_SOURCE
#include <getopt.h>

#include <zlib.h>
#include <wiringPi.h>

#include "util.h"
#include "m3u.h"
#include "vgm.h"
#include "raspi_re.h"
#include "modules.h"

static struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"playlist", required_argument, NULL, 'l'},
    {"module", required_argument, NULL, 'm'},
    {"repeat", required_argument, NULL, 'r'},
    {"version", no_argument, NULL, 'v'},
    {0, 0, 0, 0}
};

void print_version(void);
void print_usage(FILE *);
void signal_handler(int);
int play(const char *);
int playlist(const char *);
int vgmplayer_play(FILE *, const char *);

int _repeat_count;

int m3u_playing;
vgm_t *vgm;

void print_version(void) {
    printf("VGM Player for RasPi to RE Module Interface Version 1.21\n");
}

void print_usage(FILE *f) {

    fprintf(f, "Usage: vgmplayer [OPTION] [FILE|-]\n");
    fprintf(f, "\n");
    fprintf(f, "Options:\n");
    fprintf(f, "\n");
    fprintf(f, "  -h, --help        Print this message.\n");
    fprintf(f, "\n");
    fprintf(f, "  -l FILE, --playlist=FILE\n");
    fprintf(f, "                    Use a M3U playlist.\n");
    fprintf(f, "\n");
    fprintf(f, "  -m NAME0[,NAME1]  --module=NAME0[,NAME1]\n");
    fprintf(f, "                    Specify connected module names (comma separated).\n");
    fprintf(f, "                    e.g. YM2151,YM2608\n");
    fprintf(f, "\n");
    fprintf(f, "  -r COUNT  --repeat=COUNT\n");
    fprintf(f, "                    Specify a repeat count (>1).\n");
    fprintf(f, "\n");
    fprintf(f, "  -v  --version     Print version.\n");
    fprintf(f, "\n");
    fprintf(f, "Supported module names:\n");
    fprintf(f, "\n");
    fprintf(f, "  AY8910\n");
    fprintf(f, "  SN76489\n");
    fprintf(f, "  YM2151\n");
    fprintf(f, "  YM2413\n");
    fprintf(f, "  YM2608\n");
    fprintf(f, "  YM2612\n");
    fprintf(f, "  YM3526\n");
    fprintf(f, "  YM3812\n");
    fprintf(f, "\n");
}

void signal_handler(int sig) {
    if (m3u_playing != 0 && sig != SIGQUIT) {
        m3u_playing = 0;
    }
    vgm->playing = 0;
}

int main(int argc, char *argv[]) {

    int rc;
    int opt, opterr = 0;
    char *m3u_filename = NULL;
    char *module_name = NULL;
    _repeat_count = -1;

    while ((opt = getopt_long(argc, argv, "hl:m:r:v", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(stdout);
                return EXIT_SUCCESS;
            case 'l':
                m3u_filename = optarg;
                break;
            case 'm':
                module_name = optarg;
                break;
            case 'r':
                if (sscanf(optarg, "%d", &_repeat_count) == EOF) {
                    print_usage(stderr);
                    return EXIT_FAILURE;
                }
                break;
            case 'v':
                print_version();
                return EXIT_SUCCESS;
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

    vgm_handler = module_handler;

    rc = re_init();
    if (rc != 0)  {
        fprintf(stderr, "Initialization failed.\n");
        return EXIT_FAILURE;
    }

    re_reset();
    module_init();

    if (m3u_filename != NULL) {
        if (_repeat_count < 1)
            _repeat_count = 1;
        playlist(m3u_filename);
    } else {
        if (optind == argc) {
            print_usage(stderr);
            return EXIT_FAILURE;
        }
        for (int i = optind; i < argc; i++) {
            play(argv[i]);
        }
    }

    module_mute();

    return EXIT_SUCCESS;
}

int playlist(const char *filename) {

    char *path;
    char *p;
    int dir_len, base_len;

    m3u_t *m3u;
    m3u_entry_t *m3u_entry;
    int rc;

    m3u = m3u_open(filename);
    dir_len = strlen(m3u->dir);
    m3u_playing = 1;
    while (m3u_playing != 0) {
        m3u_entry = m3u_next(m3u);
        if (m3u_entry == NULL) {
            m3u_rewind(m3u);
            continue;
        }

        base_len = strlen(m3u_entry->path);
        path = malloc(sizeof(char) * (dir_len + 1 + base_len + 1));
        p = path;
        strcpy(p, m3u->dir);
        p += dir_len;
        *p = '/';
        p += 1; 
        strcpy(p, m3u_entry->path);
        p += base_len;
        *p = '\0';

        rc = play(path);
        if (rc != VGM_OK) {
            vgm_perror(vgm, path);
            return EXIT_FAILURE;
        }

        module_mute();
    }

    m3u_playing = 0;
    m3u_close(m3u);

    return EXIT_SUCCESS;
}

int play(const char *filename) {

    int rc;
    FILE *f;

    if (strcmp("-", filename) == 0) {
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
        f = fopen(filename, "rb");
    }

    if (!f) {
        perror(filename);
        return EXIT_FAILURE;
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
    }
    else
    if (rc != VGM_OK) {
        vgm_perror(vgm, filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int vgmplayer_play(FILE *f, const char *name) {

    int rc;
    int fd;
    struct stat stbuf;
    uint8_t *vgm_buf;
    struct sigaction act, old;

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

    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &act, &old) < 0) {
        free(vgm_buf);
        perror(name);
        return -1;
    }
    if (sigaction(SIGQUIT, &act, &old) < 0) {
        free(vgm_buf);
        perror(name);
        return -1;
    }

    vgm = malloc(sizeof(vgm_t));
    vgm->repeat = _repeat_count;
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

