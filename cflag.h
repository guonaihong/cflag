#ifndef __CFLAG_H
#define __CFLAG_H

#if __cplusplus
extern "C" {
#endif

struct cflag_hash_t {
    cflag_hash_node_t  **buckets;
    int               count;
    int               size;
    unsigned (*hash)(unsigned char *key, int *klen);
    void     (*free)(void *arg);
};

struct cflag_hash_node_t {
    const void     *key;
    int             klen;
    void           *val;
    cflag_hash_node_t *next;
};

typedef struct cflag_t {
    void *val;
    char *name;
    char *defvalue;
    char *usage;
    void (*set)(cflagset_t *, const char *val);
} cflag_t;

typedef struct cflagset_t {
    cflag_hash_t formal;
    int          argc;
    char       **argv;
    FILE        *output;
    unsigned     parsed:1;
} cflagset_t;

static inline cflag_int(cflag_t *flag, const char *val) {
}

static inline cflag_str(cflag_t *flag, const char *val) {
}

static inline cflag_double(cflag_t *flag, const char *val) {
}

static inline cflag_time(cflag_t *flag, const char *val) {
}

int cflag_init(cflagset_t *c, int argc, char **argv);

void cflag_parse(cflagset_t *c, cflagset_t *cf);

void cflag_usage(cflagset_t *c);

void cflag_setoutput(cflagset_t *c);

char **cflag_args(cflagset_t *c);

void cflag_free(cflagset_t *c);
#if __cplusplus
}
#endif

#endif
