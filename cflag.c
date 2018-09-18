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

int cflag_init(cflagset_t *c, int argc, char **argv) {
    cflag_hash_init();
}

void cflag_parse(cflagset_t *c, cflagset_t *cf) {

}

void cflag_usage(cflagset_t *c) {

}

void cflag_setoutput(cflagset_t *c) {

}

char **cflag_args(cflagset_t *c) {

}

void cflag_free(cflagset_t *c) {

}
