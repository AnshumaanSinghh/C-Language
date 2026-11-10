/*
 * Q49. LRU Cache
 * Difficulty : Hard
 * Topics     : Design, Hashing, Linked List
 *
 * Problem
 *   Design a Least Recently Used (LRU) cache with a fixed capacity:
 *     get(key)        returns the value stored for key, or -1 if it is absent.
 *     put(key, value) inserts key or updates its value. If the cache then
 *                     holds more than capacity keys, it evicts the key that
 *                     was used least recently.
 *   A successful get and every put (insert or update) count as a "use".
 *   Values are non-negative, so -1 is never a real value. capacity >= 1.
 *
 * Example
 *   capacity 2: put(1,1) put(2,2) get(1)=1 put(3,3) [evicts 2] get(2)=-1
 *               put(4,4) [evicts 1] get(1)=-1 get(3)=3 get(4)=4
 *
 * Approaches
 *   1. Timestamps                 scan for the key / the oldest stamp   Time O(cap) per op  Space O(cap)
 *   2. Recency-ordered array      move to the front by shifting         Time O(cap) per op  Space O(cap)
 *   3. Hash map + DLL  [optimal]  map finds the node, list keeps order  Time O(1) per op    Space O(cap)
 *
 * Key insight
 *   An LRU cache needs two things to be fast: finding a key, and changing or
 *   reading the recency order. A hash map finds a key in O(1). A doubly
 *   linked list ordered from most to least recent can unlink a node from
 *   anywhere and push it to the front in O(1), and the eviction victim is
 *   simply the last node. Store pointers to list nodes in the map and every
 *   operation is O(1).
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 49_lru_cache.c -o lru_cache && ./lru_cache
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Common interface: each implementation provides create/get/put/destroy on
 * an opaque pointer, so the tests can drive all three through one table.
 * create returns NULL if capacity < 1 or memory runs out.
 */
typedef struct {
    const char *name;
    void *(*create)(int capacity);
    int (*get)(void *cache, int key);
    void (*put)(void *cache, int key, int value);
    void (*destroy)(void *cache);
} LruImpl;

/* -------------------------------------------------------------------------
 * Approach 1: Array of entries with a "last used" timestamp
 *   Keep a global clock that ticks on every use and stamp the entry with it.
 *   get/put find the key by a linear scan. When a new key arrives and the
 *   array is full, the entry with the smallest stamp is the least recently
 *   used one, so overwrite it. Easy to get right, but every operation scans.
 *   The clock is unsigned long long so it cannot realistically wrap around.
 *   Time O(capacity) per operation   Space O(capacity)
 * ------------------------------------------------------------------------- */
typedef struct {
    int key, value;
    unsigned long long stamp;
} StampEntry;

typedef struct {
    StampEntry *e;
    int size, cap;
    unsigned long long clock;
} StampCache;

static void *stamp_create(int capacity)
{
    if (capacity < 1)
        return NULL;
    StampCache *c = malloc(sizeof *c);
    if (c == NULL)
        return NULL;
    c->e = malloc((size_t)capacity * sizeof *c->e);
    if (c->e == NULL) {
        free(c);
        return NULL;
    }
    c->size = 0;
    c->cap = capacity;
    c->clock = 0;
    return c;
}

static int stamp_find(const StampCache *c, int key)
{
    for (int i = 0; i < c->size; i++)
        if (c->e[i].key == key)
            return i;
    return -1;
}

static int stamp_get(void *cache, int key)
{
    StampCache *c = cache;
    int i = stamp_find(c, key);
    if (i < 0)
        return -1;
    c->e[i].stamp = ++c->clock;
    return c->e[i].value;
}

static void stamp_put(void *cache, int key, int value)
{
    StampCache *c = cache;
    int i = stamp_find(c, key);
    if (i < 0) {
        if (c->size < c->cap) {
            i = c->size++;
        } else {
            i = 0; /* evict: the oldest stamp is the least recently used */
            for (int j = 1; j < c->size; j++)
                if (c->e[j].stamp < c->e[i].stamp)
                    i = j;
        }
        c->e[i].key = key;
    }
    c->e[i].value = value;
    c->e[i].stamp = ++c->clock;
}

static void stamp_destroy(void *cache)
{
    StampCache *c = cache;
    if (c != NULL)
        free(c->e);
    free(c);
}

/* -------------------------------------------------------------------------
 * Approach 2: Array kept in recency order
 *   e[0] is the most recently used entry and e[size-1] the least. Using a
 *   key moves its entry to the front: save it, shift e[0..i-1] one step to
 *   the right, and put it at e[0]. Eviction just drops the last entry.
 *   No timestamps are needed because the position IS the recency, but the
 *   shift is still O(capacity). memmove is required (not memcpy) because
 *   the source and destination ranges overlap.
 *   Time O(capacity) per operation   Space O(capacity)
 * ------------------------------------------------------------------------- */
typedef struct {
    int key, value;
} KeyValue;

typedef struct {
    KeyValue *e;
    int size, cap;
} OrderCache;

static void *order_create(int capacity)
{
    if (capacity < 1)
        return NULL;
    OrderCache *c = malloc(sizeof *c);
    if (c == NULL)
        return NULL;
    c->e = malloc((size_t)capacity * sizeof *c->e);
    if (c->e == NULL) {
        free(c);
        return NULL;
    }
    c->size = 0;
    c->cap = capacity;
    return c;
}

static int order_find(const OrderCache *c, int key)
{
    for (int i = 0; i < c->size; i++)
        if (c->e[i].key == key)
            return i;
    return -1;
}

static void order_to_front(OrderCache *c, int i)
{
    KeyValue item = c->e[i];
    memmove(&c->e[1], &c->e[0], (size_t)i * sizeof *c->e);
    c->e[0] = item;
}

static int order_get(void *cache, int key)
{
    OrderCache *c = cache;
    int i = order_find(c, key);
    if (i < 0)
        return -1;
    order_to_front(c, i);
    return c->e[0].value;
}

static void order_put(void *cache, int key, int value)
{
    OrderCache *c = cache;
    int i = order_find(c, key);
    if (i >= 0) {
        c->e[i].value = value;
        order_to_front(c, i);
        return;
    }
    if (c->size == c->cap)
        c->size--; /* evict the last (least recently used) entry */
    memmove(&c->e[1], &c->e[0], (size_t)c->size * sizeof *c->e);
    c->e[0].key = key;
    c->e[0].value = value;
    c->size++;
}

static void order_destroy(void *cache)
{
    OrderCache *c = cache;
    if (c != NULL)
        free(c->e);
    free(c);
}

/* -------------------------------------------------------------------------
 * Approach 3: Hash map + doubly linked list (optimal)
 *   - The list runs head <-> most recent <-> ... <-> least recent <-> tail.
 *     head and tail are sentinel nodes that hold no data, so unlinking or
 *     inserting never has to check for NULL neighbours or empty lists.
 *   - The hash map (separate chaining) maps key -> list node. Each node
 *     carries a `chain` pointer to the next node in its bucket.
 *   get: find the node, move it to the front, return its value.
 *   put: if the key exists, update and move to the front. Otherwise take a
 *        fresh node from the pool, or, when full, recycle the least recent
 *        node (tail.prev): unlink it from the list AND from its bucket.
 *   All capacity nodes are allocated once up front, so steady-state puts
 *   never call malloc or free.
 *   The bucket count is a power of two >= capacity, which keeps chains
 *   short (load factor <= 1) and lets `hash & mask` replace `hash % size`.
 *   Pitfall: forgetting to remove the evicted key from the map leaves a
 *   pointer to a node that now holds a different key.
 *   Time O(1) expected per operation   Space O(capacity)
 * ------------------------------------------------------------------------- */
typedef struct LNode {
    int key, value;
    struct LNode *prev, *next; /* recency list */
    struct LNode *chain;       /* next node in the same hash bucket */
} LNode;

typedef struct {
    LNode head, tail; /* sentinels */
    LNode **buckets;
    size_t mask;      /* bucket count - 1 */
    LNode *pool;      /* capacity nodes, handed out in order */
    int used, cap;
} HashCache;

static size_t hash_int(int key)
{
    unsigned int x = (unsigned int)key;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

static void list_unlink(LNode *nd)
{
    nd->prev->next = nd->next;
    nd->next->prev = nd->prev;
}

static void list_push_front(HashCache *c, LNode *nd)
{
    nd->prev = &c->head;
    nd->next = c->head.next;
    c->head.next->prev = nd;
    c->head.next = nd;
}

/* Returns the address of the pointer that points at key's node, or at the
 * NULL ending the bucket's chain if key is absent. Holding the address of
 * the link (instead of the node) lets removal be one assignment, with no
 * special case for the first node of a bucket. */
static LNode **bucket_link(HashCache *c, int key)
{
    LNode **link = &c->buckets[hash_int(key) & c->mask];
    while (*link != NULL && (*link)->key != key)
        link = &(*link)->chain;
    return link;
}

static void *hash_create(int capacity)
{
    if (capacity < 1)
        return NULL;
    HashCache *c = malloc(sizeof *c);
    if (c == NULL)
        return NULL;
    size_t nb = 1;
    while (nb < (size_t)capacity)
        nb <<= 1;
    c->buckets = calloc(nb, sizeof *c->buckets);
    c->pool = malloc((size_t)capacity * sizeof *c->pool);
    if (c->buckets == NULL || c->pool == NULL) {
        free(c->buckets);
        free(c->pool);
        free(c);
        return NULL;
    }
    c->mask = nb - 1;
    c->used = 0;
    c->cap = capacity;
    c->head.prev = NULL;
    c->head.next = &c->tail;
    c->tail.prev = &c->head;
    c->tail.next = NULL;
    return c;
}

static int hash_get(void *cache, int key)
{
    HashCache *c = cache;
    LNode *nd = *bucket_link(c, key);
    if (nd == NULL)
        return -1;
    list_unlink(nd);
    list_push_front(c, nd);
    return nd->value;
}

static void hash_put(void *cache, int key, int value)
{
    HashCache *c = cache;
    LNode *nd = *bucket_link(c, key);
    if (nd != NULL) {
        nd->value = value;
        list_unlink(nd);
        list_push_front(c, nd);
        return;
    }

    if (c->used < c->cap) {
        nd = &c->pool[c->used++];
    } else {
        nd = c->tail.prev; /* least recently used */
        list_unlink(nd);
        LNode **link = bucket_link(c, nd->key);
        *link = nd->chain;
    }
    nd->key = key;
    nd->value = value;
    /* Insert at the head of the bucket. Looking the bucket up again here,
     * after the eviction, matters: the eviction may have edited this very
     * chain, so a link found before it could now be stale. */
    LNode **bucket = &c->buckets[hash_int(key) & c->mask];
    nd->chain = *bucket;
    *bucket = nd;
    list_push_front(c, nd);
}

static void hash_destroy(void *cache)
{
    HashCache *c = cache;
    if (c != NULL) {
        free(c->buckets);
        free(c->pool);
    }
    free(c);
}

/* ------------------------------------------------------------------------- */
/* Test harness                                                              */
/* ------------------------------------------------------------------------- */
static int tests_run = 0, tests_failed = 0;

static void check(int ok, const char *label)
{
    tests_run++;
    if (!ok)
        tests_failed++;
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", label);
}

static const LruImpl APPROACHES[] = {
    {"timestamps", stamp_create, stamp_get, stamp_put, stamp_destroy},
    {"recency-ordered array", order_create, order_get, order_put, order_destroy},
    {"hash map + DLL", hash_create, hash_get, hash_put, hash_destroy},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

/* 'P': put(key, arg).  'G': get(key) must return arg. */
typedef struct {
    char op;
    int key, arg;
} Op;

typedef struct {
    const char *title;
    int capacity;
    int nops;
    Op ops[16];
} Scenario;

/* Runs a scripted scenario; returns the 1-based index of the first get that
 * returned the wrong value, 0 if all were right, -1 if create failed. */
static int run_scenario(const LruImpl *impl, const Scenario *sc)
{
    void *cache = impl->create(sc->capacity);
    if (cache == NULL)
        return -1;
    int first_bad = 0;
    for (int i = 0; i < sc->nops && first_bad == 0; i++) {
        const Op *op = &sc->ops[i];
        if (op->op == 'P')
            impl->put(cache, op->key, op->arg);
        else if (impl->get(cache, op->key) != op->arg)
            first_bad = i + 1;
    }
    impl->destroy(cache);
    return first_bad;
}

int main(void)
{
    printf("Q49. LRU Cache\n");

    /* Every expected get result traced by hand from the LRU rules. */
    const Scenario scenarios[] = {
        {"LeetCode 146 example", 2, 10,
         {{'P', 1, 1}, {'P', 2, 2}, {'G', 1, 1}, {'P', 3, 3}, /* evicts 2 */
          {'G', 2, -1}, {'P', 4, 4},                          /* evicts 1 */
          {'G', 1, -1}, {'G', 3, 3}, {'G', 4, 4}, {'G', 3, 3}}},
        {"capacity 1", 1, 8,
         {{'P', 1, 10}, {'G', 1, 10}, {'P', 2, 20}, /* evicts 1 */
          {'G', 1, -1}, {'G', 2, 20}, {'P', 2, 30}, /* update, no eviction */
          {'G', 2, 30}, {'G', 3, -1}}},
        {"update refreshes recency", 2, 7,
         {{'P', 1, 1}, {'P', 2, 2}, {'P', 1, 10}, /* 1 is now most recent */
          {'P', 3, 3},                            /* so 2 is evicted     */
          {'G', 2, -1}, {'G', 1, 10}, {'G', 3, 3}}},
        {"gets refresh recency", 3, 10,
         {{'P', 1, 1}, {'P', 2, 2}, {'P', 3, 3}, {'G', 1, 1}, {'G', 2, 2},
          {'P', 4, 4}, /* 3 is least recent */
          {'G', 3, -1}, {'G', 1, 1}, {'G', 2, 2}, {'G', 4, 4}}},
        {"missed get changes nothing", 2, 6,
         {{'P', 1, 1}, {'P', 2, 2}, {'G', 5, -1}, {'P', 3, 3}, /* evicts 1 */
          {'G', 1, -1}, {'G', 2, 2}}},
        {"negative, zero, extreme keys", 2, 7,
         {{'P', INT_MIN, 7}, {'P', 0, 0}, {'G', INT_MIN, 7}, {'P', INT_MAX, 9}, /* evicts 0 */
          {'G', 0, -1}, {'G', INT_MAX, 9}, {'G', INT_MIN, 7}}},
        {"fill, then evict in order", 3, 12,
         {{'P', 1, 1}, {'P', 2, 2}, {'P', 3, 3}, {'P', 4, 4}, /* evicts 1 */
          {'P', 5, 5},                                        /* evicts 2 */
          {'G', 1, -1}, {'G', 2, -1}, {'G', 3, 3}, {'G', 4, 4}, {'G', 5, 5},
          {'P', 6, 6},                                        /* evicts 3 */
          {'G', 3, -1}}},
    };
    const int num_scenarios = (int)(sizeof scenarios / sizeof scenarios[0]);

    char label[160];
    for (int s = 0; s < num_scenarios; s++) {
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int bad = run_scenario(&APPROACHES[a], &scenarios[s]);
            snprintf(label, sizeof label, "%-28s  %-21s -> %s", scenarios[s].title,
                     APPROACHES[a].name, bad == 0 ? "all gets correct" : "wrong get");
            check(bad == 0, label);
        }
    }

    /* capacity 0 is outside the problem's range: create must refuse it. */
    for (int a = 0; a < NUM_APPROACHES; a++) {
        void *cache = APPROACHES[a].create(0);
        snprintf(label, sizeof label, "capacity 0 rejected, %s", APPROACHES[a].name);
        check(cache == NULL, label);
        if (cache != NULL)
            APPROACHES[a].destroy(cache);
    }

    /* Random cross-check: random put/get sequences, and all three caches
     * must return identical results for every single get. Most rounds use a
     * small capacity (lots of evictions); every 10th uses up to 64 entries
     * so the hash map's chains and the long shifts get exercised too. */
    {
        srand(4949);
        int bad = 0, created = 1;
        const int rounds = 300, ops_per_round = 200;
        for (int r = 0; r < rounds && created; r++) {
            int capacity = 1 + rand() % (r % 10 == 0 ? 64 : 6);
            int key_range = 3 * capacity + 2;
            void *caches[NUM_APPROACHES];
            for (int a = 0; a < NUM_APPROACHES; a++) {
                caches[a] = APPROACHES[a].create(capacity);
                if (caches[a] == NULL)
                    created = 0;
            }
            for (int i = 0; i < ops_per_round && created; i++) {
                int key = rand() % key_range - capacity; /* includes negative keys */
                if (rand() % 2) {
                    int value = rand() % 1000;
                    for (int a = 0; a < NUM_APPROACHES; a++)
                        APPROACHES[a].put(caches[a], key, value);
                } else {
                    int first = APPROACHES[0].get(caches[0], key);
                    for (int a = 1; a < NUM_APPROACHES; a++)
                        if (APPROACHES[a].get(caches[a], key) != first)
                            bad++;
                }
            }
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (caches[a] != NULL)
                    APPROACHES[a].destroy(caches[a]);
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d op sequences, all implementations agree", rounds);
        check(bad == 0 && created, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
