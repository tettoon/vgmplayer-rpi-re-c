#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if 0
#endif
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

#include "util.h"

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

