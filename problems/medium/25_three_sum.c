/*
 * Q25. 3Sum
 * Difficulty : Medium
 * Topics     : Arrays, Two Pointers, Sorting, Hashing
 *
 * Problem
 *   Given an array nums of n integers, return every UNIQUE triplet of
 *   values [a, b, c] taken from three different positions with
 *   a + b + c == 0. Two triplets are the same if they hold the same values.
 *   Output here is normalised so approaches can be compared exactly: each
 *   triplet is in ascending order and the list is sorted lexicographically.
 *
 * Example
 *   nums = [-1, 0, 1, 2, -1, -4]   ->   [[-1, -1, 2], [-1, 0, 1]]
 *
 * Approaches
 *   1. Brute force               every i<j<k, sort + dedupe       Time O(n^3)      Space O(answers)
 *   2. Sort + hash set           fix a, look up c for each b      Time O(n^2)      Space O(n)
 *   3. Two pointers  [optimal]   sort, fix a, squeeze b and c     Time O(n^2)      Space O(1) extra
 *
 * Key insight
 *   After sorting, fixing the smallest value a turns the rest into Two Sum
 *   on a sorted suffix (find b + c == -a), which two pointers solve in one
 *   linear sweep. Sorting also puts equal values next to each other, so
 *   duplicates can be skipped instead of filtered afterwards.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 25_three_sum.c -o three_sum && ./three_sum
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
/* A growable list of triplets, shared by all approaches.                    */
/* ------------------------------------------------------------------------- */
typedef struct {
    int v[3];
} Triplet;

typedef struct {
    Triplet *items;
    int count;
    int cap;
} TripletList;

static void list_init(TripletList *list)
{
    list->items = NULL;
    list->count = 0;
    list->cap = 0;
}

static void list_free(TripletList *list)
{
    free(list->items);
    list_init(list);
}

/* Appends (a, b, c), which the caller passes already in ascending order.
 * Returns 0 if memory runs out. */
static int list_push(TripletList *list, int a, int b, int c)
{
    if (list->count == list->cap) {
        int new_cap = list->cap ? list->cap * 2 : 8;
        Triplet *grown = realloc(list->items, (size_t)new_cap * sizeof *grown);
        if (grown == NULL)
            return 0; /* the old block is still owned by list, freed later */
        list->items = grown;
        list->cap = new_cap;
    }
    Triplet *t = &list->items[list->count++];
    t->v[0] = a;
    t->v[1] = b;
    t->v[2] = c;
    return 1;
}

static int cmp_triplet(const void *pa, const void *pb)
{
    const Triplet *x = pa;
    const Triplet *y = pb;
    for (int i = 0; i < 3; i++)
        if (x->v[i] != y->v[i])
            return (x->v[i] > y->v[i]) - (x->v[i] < y->v[i]);
    return 0;
}

/* Sort the list lexicographically and drop repeated triplets. */
static void list_normalise(TripletList *list)
{
    if (list->count < 2)
        return;
    qsort(list->items, (size_t)list->count, sizeof *list->items, cmp_triplet);
    int w = 1;
    for (int r = 1; r < list->count; r++)
        if (cmp_triplet(&list->items[r], &list->items[w - 1]) != 0)
            list->items[w++] = list->items[r];
    list->count = w;
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

/* Returns a sorted malloc'd copy of nums (NULL on failure or n == 0). */
static int *sorted_copy(const int *nums, int n)
{
    if (n <= 0)
        return NULL;
    int *a = malloc((size_t)n * sizeof *a);
    if (a == NULL)
        return NULL;
    memcpy(a, nums, (size_t)n * sizeof *a);
    qsort(a, (size_t)n, sizeof *a, cmp_int);
    return a;
}

/*
 * Every approach: fills *out (which must be initialised and empty) with the
 * normalised answer and returns 1, or returns 0 if memory ran out.
 * Sums are computed in long long: INT_MAX + INT_MAX + 2 wraps to 0 in int
 * arithmetic (undefined behaviour, and a false "triplet").
 */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Try every triple of positions i < j < k. When the sum is 0, sort the
 *   three values (so [2, -1, -1] and [-1, 2, -1] look identical) and
 *   append. Duplicates are removed at the end by sorting the list and
 *   dropping repeats.
 *   Time O(n^3) (+ sorting the answers)   Space O(number of hits)
 * ------------------------------------------------------------------------- */
static void sort3(int *a, int *b, int *c)
{
    int t;
    if (*a > *b) { t = *a; *a = *b; *b = t; }
    if (*b > *c) { t = *b; *b = *c; *c = t; }
    if (*a > *b) { t = *a; *a = *b; *b = t; }
}

static int three_sum_brute(const int *nums, int n, TripletList *out)
{
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            for (int k = j + 1; k < n; k++) {
                if ((long long)nums[i] + nums[j] + nums[k] != 0)
                    continue;
                int a = nums[i], b = nums[j], c = nums[k];
                sort3(&a, &b, &c);
                if (!list_push(out, a, b, c))
                    return 0;
            }
        }
    }
    list_normalise(out);
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort + hash set for the third value
 *   Sort a copy. Fix the first element a = s[i] (skipping repeated values
 *   of a). Then scan j = i+1 .. n-1 and keep a set of the values s[i+1..j-1]
 *   seen so far. For each b = s[j], the missing value is c = -(a + b); if
 *   it is in the set, (a, c, b) is a triplet. Because the array is sorted,
 *   a <= c <= b, so the triplet is already in ascending order.
 *   The same triplet can be found more than once (e.g. many equal b's), so
 *   the list is normalised at the end.
 *   The set is a small open-addressing table, cleared for each i. Clearing
 *   costs O(n) per i, which is within the O(n^2) budget.
 *   Time O(n^2) expected   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    int *keys;
    unsigned char *used;
    size_t cap;
} IntSet;

static size_t hash_int(int key)
{
    unsigned int x = (unsigned int)key;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

static int set_init(IntSet *s, int expected)
{
    s->cap = 16;
    while (s->cap < (size_t)expected * 2)
        s->cap <<= 1;
    s->keys = malloc(s->cap * sizeof *s->keys);
    s->used = calloc(s->cap, 1);
    return s->keys && s->used;
}

static void set_free(IntSet *s)
{
    free(s->keys);
    free(s->used);
}

static void set_clear(IntSet *s)
{
    memset(s->used, 0, s->cap);
}

static int set_has(const IntSet *s, int key)
{
    size_t mask = s->cap - 1;
    for (size_t h = hash_int(key) & mask; s->used[h]; h = (h + 1) & mask)
        if (s->keys[h] == key)
            return 1;
    return 0;
}

static void set_add(IntSet *s, int key)
{
    size_t mask = s->cap - 1;
    size_t h = hash_int(key) & mask;
    while (s->used[h]) {
        if (s->keys[h] == key)
            return;
        h = (h + 1) & mask;
    }
    s->used[h] = 1;
    s->keys[h] = key;
}

static int three_sum_hash(const int *nums, int n, TripletList *out)
{
    if (n < 3)
        return 1;
    int *s = sorted_copy(nums, n);
    if (s == NULL)
        return 0;
    IntSet seen;
    if (!set_init(&seen, n)) {
        set_free(&seen); /* frees whichever half did get allocated */
        free(s);
        return 0;
    }

    int ok = 1;
    for (int i = 0; i < n - 2 && ok; i++) {
        if (i > 0 && s[i] == s[i - 1])
            continue; /* same a as last time: same triplets */
        set_clear(&seen);
        for (int j = i + 1; j < n; j++) {
            long long need = -((long long)s[i] + s[j]);
            if (need >= INT_MIN && need <= INT_MAX && set_has(&seen, (int)need)) {
                if (!list_push(out, s[i], (int)need, s[j])) {
                    ok = 0;
                    break;
                }
            }
            set_add(&seen, s[j]);
        }
    }
    set_free(&seen);
    free(s);
    if (ok)
        list_normalise(out);
    return ok;
}

/* -------------------------------------------------------------------------
 * Approach 3: Sort + two pointers (optimal)
 *   Sort a copy. For each i (the smallest value a = s[i]) look for b + c ==
 *   -a in s[i+1..n-1] with lo at the left end and hi at the right end:
 *     sum < 0  -> need something bigger: lo++
 *     sum > 0  -> need something smaller: hi--
 *     sum == 0 -> record it, then move BOTH pointers past every copy of
 *                 s[lo] and s[hi], otherwise the same triplet is recorded
 *                 again.
 *   Skip i when s[i] == s[i-1] for the same reason. If s[i] > 0 every value
 *   to its right is also > 0, so no more triplets are possible: stop.
 *   Output comes out ascending and lexicographically sorted with no repeats,
 *   so no normalising pass is needed. Many-zero inputs like [0, 0, 0, 0]
 *   are the classic trap: without the skip on i they give [0, 0, 0] twice
 *   (once for i = 0, again for i = 1); [-1, -1, -1, 2, 2, 2] needs the
 *   pointer skips.
 *   Time O(n^2) (+ O(n log n) sort)   Space O(1) extra beyond the sorted copy
 *   (sort nums itself if you are allowed to modify the input).
 * ------------------------------------------------------------------------- */
static int three_sum_two_pointers(const int *nums, int n, TripletList *out)
{
    if (n < 3)
        return 1;
    int *s = sorted_copy(nums, n);
    if (s == NULL)
        return 0;

    for (int i = 0; i < n - 2; i++) {
        if (s[i] > 0)
            break;
        if (i > 0 && s[i] == s[i - 1])
            continue;
        int lo = i + 1, hi = n - 1;
        while (lo < hi) {
            long long sum = (long long)s[i] + s[lo] + s[hi];
            if (sum < 0) {
                lo++;
            } else if (sum > 0) {
                hi--;
            } else {
                if (!list_push(out, s[i], s[lo], s[hi])) {
                    free(s);
                    return 0;
                }
                lo++;
                hi--;
                while (lo < hi && s[lo] == s[lo - 1])
                    lo++;
                while (lo < hi && s[hi] == s[hi + 1])
                    hi--;
            }
        }
    }
    free(s);
    return 1;
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

typedef int (*ThreeSumFn)(const int *nums, int n, TripletList *out);

static const struct {
    const char *name;
    ThreeSumFn fn;
} APPROACHES[] = {
    {"brute force", three_sum_brute},
    {"sort + hash set", three_sum_hash},
    {"two pointers", three_sum_two_pointers},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 12
#define MAXT 4

typedef struct {
    int nums[MAXN];
    int n;
    int expect[MAXT][3]; /* normalised: ascending triplets, sorted list */
    int expect_count;
} Case;

static int list_equals(const TripletList *list, const int expect[][3], int count)
{
    if (list->count != count)
        return 0;
    for (int t = 0; t < count; t++)
        for (int i = 0; i < 3; i++)
            if (list->items[t].v[i] != expect[t][i])
                return 0;
    return 1;
}

static int lists_equal(const TripletList *x, const TripletList *y)
{
    if (x->count != y->count)
        return 0;
    for (int t = 0; t < x->count; t++)
        if (cmp_triplet(&x->items[t], &y->items[t]) != 0)
            return 0;
    return 1;
}

int main(void)
{
    printf("Q25. 3Sum\n");

    /* Expected lists worked out by hand. */
    const Case cases[] = {
        {{-1, 0, 1, 2, -1, -4}, 6, {{-1, -1, 2}, {-1, 0, 1}}, 2},
        {{0, 1, 1}, 3, {{0}}, 0},
        {{0, 0, 0}, 3, {{0, 0, 0}}, 1},
        {{0, 0, 0, 0}, 4, {{0, 0, 0}}, 1},                   /* many zeros: one triplet */
        {{0}, 0, {{0}}, 0},                                  /* empty */
        {{1}, 1, {{0}}, 0},                                  /* single */
        {{0, 0}, 2, {{0}}, 0},                               /* two elements */
        {{-2, 0, 1, 1, 2}, 5, {{-2, 0, 2}, {-2, 1, 1}}, 2},
        {{3, 0, -2, -1, 1, 2}, 6, {{-2, -1, 3}, {-2, 0, 2}, {-1, 0, 1}}, 3},
        {{-1, -1, -1, 2, 2, 2}, 6, {{-1, -1, 2}}, 1},        /* heavy duplicates */
        {{1, 2, 3, 4}, 4, {{0}}, 0},                         /* all positive */
        {{INT_MAX, INT_MIN, 1}, 3, {{INT_MIN, 1, INT_MAX}}, 1},
        /* In int arithmetic MAX+MAX+2 and MIN+MIN+0 both wrap to 0; the real
         * sums are not 0, so there is no triplet. */
        {{INT_MAX, INT_MAX, 2, INT_MIN, INT_MIN, 0}, 6, {{0}}, 0},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            TripletList out;
            list_init(&out);
            int ok = APPROACHES[a].fn(c->nums, c->n, &out);
            snprintf(label, sizeof label, "test %2d, %-15s -> %d triplet(s)", t + 1,
                     APPROACHES[a].name, out.count);
            check(ok && list_equals(&out, c->expect, c->expect_count), label);
            list_free(&out);
        }
    }

    /* Random cross-check against brute force. Values in [-6, 6] give lots of
     * duplicates and zero sums; n <= 12 keeps the O(n^3) brute force tiny. */
    {
        srand(2525);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int nums[MAXN];
            int n = rand() % (MAXN + 1);
            for (int i = 0; i < n; i++)
                nums[i] = rand() % 13 - 6;

            TripletList expect;
            list_init(&expect);
            if (!three_sum_brute(nums, n, &expect))
                bad++;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                TripletList out;
                list_init(&out);
                if (!APPROACHES[a].fn(nums, n, &out) || !lists_equal(&out, &expect))
                    bad++;
                list_free(&out);
            }
            list_free(&expect);
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
