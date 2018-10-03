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
#include "s98.h"
#include "raspi_re.h"
#include "modules.h"

static struct option longopts[] = {
    {"help", no_argument, NULL, 'h'},
    {"playlist", required_argument, NULL, 'l'},
    {"module", required_argument, NULL, 'm'},
    {"repeat", required_argument, NULL, 'r'},
    {0, 0, 0, 0}
};

void print_usage(FILE *);
void sigint_handler(int);
int play(const char *);
int playlist(const char *);
int s98player_play(FILE *, const char *);

int _repeat_count;

int m3u_playing;
s98_t *s98;

void print_usage(FILE *f) {

    fprintf(f, "Usage: s98player [OPTION] [FILE|-]\n");
    fprintf(f, "\n");
    fprintf(f, "Options:\n");
    fprintf(f, "\n");
    fprintf(f, "  -h, --help        Print this message.\n");
    fprintf(f, "\n");
    fprintf(f, "  -l FILE, --playlist=FILE\n");
    fprintf(f, "                    Use a M3U playlist.\n");
    fprintf(f, "\n");
    fprintf(f, "  -m NAME0[,NAME1], --module=NAME0[,NAME1]\n");
    fprintf(f, "                    Specify connected module names (comma separated).\n");
    fprintf(f, "                    e.g. YM2151,YM2608\n");
    fprintf(f, "  -r COUNT, --repeat=COUNT\n");
    fprintf(f, "                    Specify a repeat count (>1).\n");
    fprintf(f, "\n");
    fprintf(f, "Supported module names:\n");
    fprintf(f, "\n");
    fprintf(f, "  YM2151\n");
    fprintf(f, "  YM2608\n");
    fprintf(f, "\n");
}

void sigint_handler(int sig) {
    s98->playing = 0;
    m3u_playing = 0;
}

int main(int argc, char *argv[]) {

    int rc;
    int opt, opterr = 0;
    char *m3u_filename = NULL;
    char *module_name = NULL;
    _repeat_count = -1;

    while ((opt = getopt_long(argc, argv, "hl:m:r:", longopts, NULL)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(stdout);
                return 0;
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

    FILE *f;
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

        f = fopen(path, "rb");
        if (!f) {
            perror(path);
            return EXIT_FAILURE;
        }

        rc = s98player_play(f, path);
        if (rc != S98_OK) {
            s98_perror(s98, path);
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

    rc = s98player_play(f, filename);
    if (rc != S98_OK) {
        s98_perror(s98, filename);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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

    fclose(f);

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

