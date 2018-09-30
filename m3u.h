#ifndef M3U_H
#define M3U_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define M3U_STRING_SIZE 256
#define M3U_MAX_ENTRIES 1024

typedef struct m3u_entry_t_ {
    char path[M3U_STRING_SIZE];
    char title[M3U_STRING_SIZE];
} m3u_entry_t;

typedef struct m3u_t_ {
    char *dir;
    int size;
    m3u_entry_t *entries;
    int index;
} m3u_t;

m3u_t *m3u_open(const char *); 
void m3u_close(m3u_t *);
m3u_entry_t *m3u_next(m3u_t *);
void m3u_rewind(m3u_t *);

#ifdef __cplusplus
}
#endif

#endif // M3U_H

