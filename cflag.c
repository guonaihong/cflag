#include "cflag.h"

/* the default hash function */
static unsigned hash_func(unsigned char *key, int *klen) {
    int            i;
    unsigned char *p;
    unsigned int   hash;

    hash = 0;

    if (*klen == TP_KEY_STR) {
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
        return NULL
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

void cflag_hash_range(cflag_hash_t *hash) {
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

            if (hash->cb) {
                hash->cb(p->key, p->klen, p->val);
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

int cflag_init(cflagset_t *c, char **argv) {

    c->argc = 0;
    c->argv = argv;

    if (cflag_hash_init(&c->formal, 30, NULL) != 0) {
        return -1;
    }
}

int cflag_parse_one(cflagset_t *c, char *err, int err_len) {
    char   *name = c->argv;
    int     len = strlen(name);
    int     num_minuses;
    cflag_t flag;

    if (len == 0) {
        return 0;
    }

    if (len < 2 || name[0] != '-') {
        return 0;
    }

    num_minuses = 1;
    if (name[1] == '-') {
        if (len == 2) {
            c->argc++;
            return 0;
        }
        num_minuses++;
    }

    char *name0 = strdup(name);
    name        = name0;
    name        = name + num_minuses;
    len         = len  - num_minuses;

    if (len == 0 || name[0] == '-' || name[0] == '=') {
        snprintf(err, sizeof(err), "bad flag syntax: %s", c->argv);
        goto fail;
    }

    int   has_value = 0;
    char *value;
    char *pos = strchr(name, '=');

    if (pos != NULL) {
        *pos = '\0';
        has_value = 1;
        value = pos +1;
    }

    flag = cflag_hash_get(c->formal, name[num_minuses], len - num_minuses);
    if (flag == NULL) {
        if (!strcmp(name, "h") || !strcmp(name, "help")) {
        }
    }
    return 1;

fail:
    free(name0);
}

void cflag_parse(cflagset_t *c, cflagset_t *cf) {

    cflag_t *cfp     = cf;
    cflag_t *cfp2    = NULL;
    char     err[512]={0};

    while(*cfp) {
        cfp2 = malloc(sizeof(cflag_t));
        if (cfp2 == NULL) {
            continue
        }

        memcpy(cfp2, cfp, sizeof(cflag_t));
        cflag_hash_put(c->formal, cfp->name, strlen(cfp->name), cfp2, sizeof(cflag_t));
    }
    
    for(;;) {

        if (cflag_parse_one(c, err, sizeof(err))) {
            continue;
        }

        break;
    }

    if (strlen(err) > 0 ) {
        printf("%s\n", err);
    }
}

void cflag_usage(cflagset_t *c) {
    cflag_hash_range(c->formal);

}

void cflag_setoutput(cflagset_t *c, FILE *fp) {
    c->output = c;
}

char **cflag_args(cflagset_t *c) {
    return c->argv;
}

void cflag_free(cflagset_t *c) {

}
