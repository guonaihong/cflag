#ifndef __CFLAG_H
#define __CFLAG_H

#if __cplusplus
extern "C" {
#endif

typedef struct  cflag_t {
    char *name;
    char *usage;
    char *defvalue;
    void (*set)(cflagset_t *, const char *val);
} cflag_t;

typedef struct cflagset_t {

    FILE *output;
} cflagset_t;

static inline cflag_int(cflag_t *flag, const char *val) {
}

static inline cflag_str(cflag_t *flag, const char *val) {
}

static inline clfag_double(cflag_t *flag, const char *val) {
}

void cflag_init(cflagset_t *c, int argc, char **argv);

void cflag_parse(cflagset_t *c, cflagset_t *cf);

void cflag_usage(cflagset_t *c);

void cflag_setoutput(cflagset_t *c);

void cflag_free(cflagset_t *c);
#if __cplusplus
}
#endif

#endif
