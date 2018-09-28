#ifndef __CFLAG_H
#define __CFLAG_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#if __cplusplus
extern "C" {
#endif

#define continue_on_error  0 // Return a descriptive error.
#define exit_on_error      1 // Call os.Exit(2).
#define panic_on_error     2 // Call panic with a descriptive error.
#define CFLAG_KEY_STR     -1

typedef struct cflag_hash_node_t cflag_hash_node_t;
struct cflag_hash_node_t {
    const void        *key;
    int                klen;
    void              *val;
    cflag_hash_node_t *next;
};

typedef struct cflag_hash_t {
    cflag_hash_node_t  **buckets;
    int                  count;
    int                  size;
    unsigned             stack:1;
    unsigned (*hash)(unsigned char *key, int *klen);
    void     (*free)(void *arg);
} cflag_hash_t;

typedef struct cflag_t cflag_t;
struct cflag_t {
    void *val;
    char *name;
    char *defvalue;
    char *usage;
    int (*set)(cflag_t *, const char *val);
    int isbool;
};

typedef struct cflagset_t {
    cflag_hash_t formal;
    int          argc;
    char        *name;
    char       **argv;
    FILE        *output;
    unsigned     error_handling;
    unsigned     parsed:1;
    char         err[512];
} cflagset_t;

static inline int cflag_int(cflag_t *flag, const char *inval) {
    flag->isbool = 0;
    char *endptr = NULL;
    int val = strtol(inval, &endptr, 0);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
        return 1;
    }

    if (endptr == inval) {
        return 1;
    }

    *(int *)flag->val = val;
    return  0;
}

static inline int parse_bool(const char *str, int *err) {
    if (!strcmp(str, "1") || !strcmp(str, "t") || !strcmp(str, "T")) {
        return 1;
    }

    if (!strcmp(str, "true") || !strcmp(str, "TRUE") || !strcmp(str, "True")) {
        return 1;
    }

    if (!strcmp(str, "0") || !strcmp(str, "f") || !strcmp(str, "F")) {
        return 0;
    }

    if (!strcmp(str, "false") || !strcmp(str, "FALSE") || !strcmp(str, "False")) {
        return 0;
    }

    *err = 1;
    return 0;
}

static inline int cflag_bool(cflag_t *flag, const char *val) {
    int  err     = 0;
    flag->isbool = 1;

    *(int *)flag->val = parse_bool(val, &err);

    if (err) {
        return 1;
    }

    return 0;
}

static inline int cflag_str(cflag_t *flag, const char *val) {
    flag->isbool = 0;

    if (*(char **)flag->val != NULL) {
        free(*(char **)flag->val);
    }

    *(char **)flag->val = strdup(val);
    return 0;
}

static inline int cflag_double(cflag_t *flag, const char *inval) {
    flag->isbool = 0;

    char  *endptr = NULL;
    double val   = 0;

    val = strtod(inval, &endptr);

    if (endptr == inval) {
        return 1;
    }

    *(double *)flag->val = val;
    return 0;
}

static inline int cflag_time(cflag_t *flag, const char *val) {
    flag->isbool = 0;
    return 0;
}

int cflag_init(cflagset_t *c, char *name, int error_handling);

int cflag_parse(cflagset_t *c, cflag_t *cf, char **argv);

void cflag_usage(cflagset_t *c);

void cflag_setoutput(cflagset_t *c, FILE *fp);

char **cflag_args(cflagset_t *c);

void cflag_free(cflagset_t *c);
#if __cplusplus
}
#endif

#endif
