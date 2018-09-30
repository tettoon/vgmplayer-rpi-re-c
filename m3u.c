#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "m3u.h"

char* get_dirname(const char *path) {

    char *p;
    char *s;
    int len;

    len = strlen(path);
    s = malloc(sizeof(char) * (len + 1));
    strncpy(s, path, len + 1);

    p = s;
    p = p + len;
    while (p >= s && *p != '/') {
        p--;
    }
    *p = '\0';

    return s;
}

m3u_t *m3u_open(const char *filename) {

    FILE *f;
    char line[1024];
    int size;
    m3u_t *m3u;
    m3u_entry_t *entry;

    f = fopen(filename, "r");
    if (!f) {
        return NULL;
    }

    m3u = malloc(sizeof(m3u_t));
    m3u->dir = get_dirname(filename);
    m3u->size = 0;
    m3u->entries = malloc(sizeof(m3u_entry_t) * M3U_MAX_ENTRIES);
    m3u->index = -1;

    while (fgets(line, M3U_STRING_SIZE + 1, f) != NULL) {
        if (m3u->size > M3U_MAX_ENTRIES)
            break;
        size = strlen(line);
        while (size > 0 && (line[size - 1] == '\0' || line[size - 1] == '\n' || line[size - 1] == '\r')) {
            line[--size] = '\0';
        }
        entry = &m3u->entries[m3u->size++];
        strncpy(entry->path, line, M3U_STRING_SIZE);
        entry->title[0] = '\0';
    }

    fclose(f);
        
    return m3u;
}

void m3u_close(m3u_t *m3u) {

    if (m3u != NULL)
        return;

    free(m3u->dir);
    free(m3u->entries);

    free(m3u);
}

void m3u_rewind(m3u_t *m3u) {

    m3u->index = -1;
}

m3u_entry_t *m3u_next(m3u_t *m3u) {

    if (m3u->index >= m3u->size - 1)
        return NULL;

    return &m3u->entries[++m3u->index];
}

