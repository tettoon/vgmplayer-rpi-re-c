#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

int copy_file(FILE *, FILE *);
int gunzip_file(FILE *, FILE *);

#ifdef __cplusplus
}
#endif

#endif  /* UTIL_H */

