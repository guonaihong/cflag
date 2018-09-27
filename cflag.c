#include "cflag.h"

/* the default hash function */
static unsigned hash_func(unsigned char *key, int *klen) {
    int            i;
    unsigned char *p;
    unsigned int   hash;

    hash = 0;

    if (*klen == CFLAG_KEY_STR) {
        for (p = key; *p; p++) {
            hash = hash * 31 + *p;
        }
        *klen = p - key;
    } else {
        for (p = key, i = *klen; i > 0; i--, p++) {
            hash = hash * 31 + *p;
        }
    }

    return hash;
}

int cflag_hash_init(cflag_hash_t *hash, int size, void (*free)(void *arg)) {
    int        i;

    /* i = 2 ^ n - 1 (4 <= n <= 32) */
    for (i = 15; i < INT_MAX && i < size; ) {
        /* select the hash table default length */
        i = i * 2 + 1;
    }

    hash->size    = i;
    hash->buckets = (cflag_hash_node_t **)calloc(hash->size + 1,
                           sizeof(cflag_hash_node_t *));

    if (hash->buckets == NULL) {
        goto fail;
    }

    hash->free = free;
    hash->hash = hash_func;
    return 0;

fail: 
    return 1;
}

cflag_hash_t *cflag_hash_new(int size,
                       void (*free)(void *arg)) {
    int        rv;
    cflag_hash_t *hash;

    hash = (cflag_hash_t *)calloc(1, sizeof(cflag_hash_t));
    if (hash == NULL)
        return hash;

    rv = cflag_hash_init(hash, size, free);
    if (rv != 0) {
        free(hash);
        return NULL;
    }

    return hash;
}

static cflag_hash_node_t **cflag_hash_find(cflag_hash_t  *hash,
                                     const void *key,
                                     int        *klen,
                                     unsigned   *hash_val) {

    cflag_hash_node_t **pp;
    unsigned         hval;
    if (hash_val)
        hval = *hash_val = hash->hash((unsigned char *)key, klen);
    else
        hval = hash->hash((unsigned char *)key, klen);

    pp = &hash->buckets[hval & hash->size];

    for (; pp && *pp; pp = &(*pp)->next) {
        if ((*pp)->klen == *klen &&
            memcmp((*pp)->key, key, *klen) == 0) {
            break;
        }
    }

    return pp;
}

void *cflag_hash_get(cflag_hash_t *hash, const void *key, int klen) {
    cflag_hash_node_t **pp;

    pp = cflag_hash_find(hash, key, &klen, NULL);

    if (*pp != NULL) {
        return (*pp)->val;
    }

    return NULL;
}


void *cflag_hash_put(cflag_hash_t  *hash,
                  const void *key,
                  int         klen,
                  void       *val) {

    void             *prev;
    unsigned          hash_val;
    cflag_hash_node_t  **pp, *newp;

    if (val == NULL) {
        return val;
    }

    prev = NULL;
    pp = cflag_hash_find(hash, key, &klen, &hash_val);

    /* key not exits */
    if (*pp == NULL) {
        newp = (cflag_hash_node_t *)malloc(sizeof(*newp));
        if (newp == NULL) {
            return NULL;
        }

        newp->key  = calloc(1, klen + 1);
        if (newp->key == NULL) {
            return NULL;
        }
        memcpy((void *)newp->key, key, klen);

        newp->val  = val;
        newp->klen = klen;
        newp->next = hash->buckets[hash_val & hash->size];
        hash->buckets[hash_val & hash->size] = newp;
    } else {
        prev = (*pp)->val;
        (*pp)->val = val;
    }

    return prev;
}

void *cflag_hash_del(cflag_hash_t *hash, const void *key, int klen) {
    cflag_hash_node_t **pp, *p;
    void           *val;

    val = NULL;
    pp = cflag_hash_find(hash, key, &klen, NULL);

    if (*pp != NULL) {
        p = *pp;
        *pp = p->next;
        val = p->val;
        free((void *)p->key);
        free(p);
        return val;
    }
    return NULL;
}

void cflag_hash_range(cflag_hash_t *hash,
                      void *user_data,
                      void (*cb)(void *user_data, const void *key, int klen, void *val)) {

    int             i, len;
    cflag_hash_node_t *p, *n;

    if (hash == NULL) {
        return;
    }

    i = 0, len = hash->size + 1;

    for (; i < len; i++) {

        p = hash->buckets[i];

        for (; p; p = n) {
            n  = p->next;

            if (cb) {
                cb(user_data, p->key, p->klen, p->val);
            }
        }
    }
}

void cflag_hash_free(cflag_hash_t *hash) {

    int             i, len;
    cflag_hash_node_t *p, *n;

    if (hash == NULL) {
        return;
    }

    i = 0, len = hash->size + 1;

    for (; i < len; i++) {

        p = hash->buckets[i];

        for (; p; p = n) {
            n  = p->next;

            if (hash->free) {
                hash->free(p->val);
            }

            free((void *)p->key);
            free(p);
        }
    }

    free(hash->buckets);
    free(hash);
}

int cflag_init(cflagset_t *c, char *name, int error_handling) {

    c->argc           = 0;
    c->argv           = NULL;
    c->name           = strdup(name);
    c->output         = NULL;
    c->error_handling = error_handling;

    if (cflag_hash_init(&c->formal, 30, NULL) != 0) {
        goto fail;
    }

    return 0;
fail:
    free(c->name);
    return 1;
}

// return 0 = ok
// return 1 = false
int cflag_parse_one(cflagset_t *c, char *err, int err_len) {
    char    *name = c->argv[0];
    int      len;
    int      num_minuses;

    if (name == NULL) {
        return 1;
    }

    cflag_t *flag;
    len = strlen(name);
    if (len == 0) {
        return 1;
    }

    if (len < 2 || name[0] != '-') {
        return 1;
    }

    num_minuses = 1;
    if (name[1] == '-') {
        if (len == 2) {
            c->argc++;
            return 1;
        }
        num_minuses++;
    }

    char *name0 = strdup(name);
    name        = name0;
    name        = name + num_minuses;
    len         = len  - num_minuses;

    if (len == 0 || name[0] == '-' || name[0] == '=') {
        snprintf(err, sizeof(err), "bad flag syntax: %s", c->argv[0]);
        goto fail;
    }

    c->argv++;
    int   has_value = 0;
    char *value;
    char *pos = strchr(name, '=');

    if (pos != NULL) {
        *pos = '\0';
        has_value = 1;
        value = pos +1;
        len = pos - name;
    }

    flag = cflag_hash_get(&c->formal, name, len);
    if (flag == NULL) {
        if (!strcmp(name, "h") || !strcmp(name, "help")) {
            cflag_usage(c);
            snprintf(err, err_len, "flag: help requested");
            goto fail;
        }
        snprintf(err, err_len, "cflag provided but not defined: -%s", name);
        goto fail;
    }

    //TODO check error
    if (flag->isbool) {
        if (has_value) {
            flag->set(flag, value);
        } else {
            flag->set(flag, "true");
        }
    } else {
        if (!has_value && c->argv[0]) {
            has_value = 1;
            value = c->argv[0];
            c->argv++;
        }

        if (!has_value) {
            snprintf(err, sizeof(err), "cflag needs an argument: -%s", name);
            goto fail;
        }

        flag->set(flag, value);
    }
    free(name0);
    return 0;

fail:
    free(name0);
    return 1;
}

int cflag_parse(cflagset_t *c, cflag_t *cf, char **argv) {

    cflag_t *cfp     = cf;
    cflag_t *cfp2    = NULL;
    c->argv          = argv;

    memset(c->err, '\0', sizeof(c->err));
    for (;;) {
        cfp2 = malloc(sizeof(cflag_t));
        if (cfp2 == NULL) {
            continue;
        }

        memset(cfp2, 0, sizeof(cflag_t));

        if (cfp->name == NULL && cfp->defvalue == NULL && cfp->usage == NULL) {
            break;
        }

        memcpy(cfp2, cfp, sizeof(cflag_t));

        cfp2->set(cfp2, cfp2->defvalue);
        cflag_hash_put(&c->formal, cfp->name, strlen(cfp->name), cfp2);
        cfp++;
    }
    
    for(;;) {

        if (cflag_parse_one(c, c->err, sizeof(c->err)) == 0) {
            continue;
        }

        if (c->err[0] == '\0') {
            break;
        }

        switch (c->error_handling) {
            case continue_on_error:
                return 1;
            case exit_on_error:
                exit(2);
                break;
            case panic_on_error:
                abort();
        }
    }
    return 0;
}

FILE *cflag_output(cflagset_t *c) {
    if (c->output == NULL) {
        return stderr;
    }
    return c->output;
}

static void print_flag(void *user_data, const void *key, int klen, void *val) {

    if (val == NULL) {
        return;
    }

    cflag_t    *flag = (cflag_t *)val;
    cflagset_t *c    = (cflagset_t *)user_data;

    //TODO sort
    //
    if (strlen(c->name) == 0) {
        return;
    }

    fprintf(cflag_output(c), "  -%s\n", flag->name);
    fprintf(cflag_output(c), "     %s", flag->usage);
    fprintf(cflag_output(c), "\n");
}

void cflag_usage(cflagset_t *c) {
    fprintf(cflag_output(c), "Usage of %s\n", c->name);
    cflag_hash_range(&c->formal, c, print_flag);
}

void cflag_setoutput(cflagset_t *c, FILE *fp) {
    c->output = fp;
}

char **cflag_args(cflagset_t *c) {
    return c->argv;
}

void cflag_free(cflagset_t *c) {

}
