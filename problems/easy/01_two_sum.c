/*
 * Q01. Two Sum
 * Difficulty : Easy
 * Topics     : Arrays, Hashing, Two Pointers, Sorting
 *
 * Problem
 *   Given an array nums of n integers and an integer target, return the
 *   indices (i, j) with i < j of the two numbers that add up to target.
 *   You may not use the same element twice.
 *
 * Example
 *   nums = [2, 7, 11, 15], target = 9   ->   (0, 1)   because 2 + 7 = 9
 *
 * Approaches
 *   1. Brute force            check every pair               Time O(n^2)      Space O(1)
 *   2. Sort + two pointers    sort (value, index) pairs      Time O(n log n)  Space O(n)
 *   3. Hash map   [optimal]   look up target - x as you go   Time O(n)        Space O(n)
 *
 * Key insight
 *   For each x you only need to know "have I already seen target - x?".
 *   A hash map answers that in O(1), so a single pass is enough.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 01_two_sum.c -o two_sum && ./two_sum
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Try every pair (i, j) with i < j and stop at the first one whose sum is
 *   target. The sum is computed in long long so large values cannot overflow.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int two_sum_brute(const int *nums, int n, int target, int *out_i, int *out_j)
{
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            if ((long long)nums[i] + nums[j] == target) {
                *out_i = i;
                *out_j = j;
                return 1;
            }
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort + two pointers
 *   Sorting destroys the original positions, so sort (value, index) pairs.
 *   Then move lo/hi inward: if the sum is too small, lo++ makes it bigger;
 *   if it is too big, hi-- makes it smaller.
 *   Time O(n log n)   Space O(n) for the pairs
 * ------------------------------------------------------------------------- */
typedef struct {
    int value;
    int index;
} Pair;

static int cmp_pair(const void *a, const void *b)
{
    const Pair *x = a;
    const Pair *y = b;
    if (x->value != y->value)
        return (x->value > y->value) - (x->value < y->value);
    return (x->index > y->index) - (x->index < y->index);
}

static int two_sum_sorted(const int *nums, int n, int target, int *out_i, int *out_j)
{
    if (n < 2)
        return 0;

    Pair *p = malloc((size_t)n * sizeof *p);
    if (p == NULL)
        return 0;
    for (int i = 0; i < n; i++) {
        p[i].value = nums[i];
        p[i].index = i;
    }
    qsort(p, (size_t)n, sizeof *p, cmp_pair);

    int lo = 0, hi = n - 1, found = 0;
    while (lo < hi) {
        long long sum = (long long)p[lo].value + p[hi].value;
        if (sum == target) {
            int a = p[lo].index, b = p[hi].index;
            *out_i = a < b ? a : b;
            *out_j = a < b ? b : a;
            found = 1;
            break;
        }
        if (sum < target)
            lo++;
        else
            hi--;
    }
    free(p);
    return found;
}

/* -------------------------------------------------------------------------
 * Approach 3: Hash map (optimal)
 *   Walk the array once. For nums[j], look up need = target - nums[j] in a
 *   map of value -> index of everything seen so far. If it is there, done.
 *   Otherwise store nums[j]. C has no built-in map, so this is a small
 *   open-addressing table with linear probing.
 *   Time O(n) expected   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    int *keys;
    int *vals;
    unsigned char *used;
    size_t mask; /* capacity - 1, capacity is a power of two */
} IntMap;

static size_t hash_int(int key)
{
    unsigned int x = (unsigned int)key;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

static int map_init(IntMap *m, int expected)
{
    size_t cap = 16;
    while (cap < (size_t)expected * 2)
        cap <<= 1;
    m->keys = malloc(cap * sizeof *m->keys);
    m->vals = malloc(cap * sizeof *m->vals);
    m->used = calloc(cap, 1);
    m->mask = cap - 1;
    return m->keys && m->vals && m->used;
}

static void map_free(IntMap *m)
{
    free(m->keys);
    free(m->vals);
    free(m->used);
}

/* Returns 1 and writes *val if key is present. */
static int map_get(const IntMap *m, int key, int *val)
{
    for (size_t h = hash_int(key) & m->mask; m->used[h]; h = (h + 1) & m->mask) {
        if (m->keys[h] == key) {
            *val = m->vals[h];
            return 1;
        }
    }
    return 0;
}

/* Inserts key if absent. Keeps the first index seen for duplicate values. */
static void map_put(IntMap *m, int key, int val)
{
    size_t h = hash_int(key) & m->mask;
    while (m->used[h]) {
        if (m->keys[h] == key)
            return;
        h = (h + 1) & m->mask;
    }
    m->used[h] = 1;
    m->keys[h] = key;
    m->vals[h] = val;
}

static int two_sum_hash(const int *nums, int n, int target, int *out_i, int *out_j)
{
    IntMap m;
    if (!map_init(&m, n)) {
        map_free(&m);
        return 0;
    }

    int found = 0;
    for (int j = 0; j < n && !found; j++) {
        long long need = (long long)target - nums[j];
        int i;
        if (need >= INT_MIN && need <= INT_MAX && map_get(&m, (int)need, &i)) {
            *out_i = i;
            *out_j = j;
            found = 1;
        }
        map_put(&m, nums[j], j);
    }
    map_free(&m);
    return found;
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

typedef int (*TwoSumFn)(const int *nums, int n, int target, int *out_i, int *out_j);

static const struct {
    const char *name;
    TwoSumFn fn;
} APPROACHES[] = {
    {"brute force", two_sum_brute},
    {"sort + two pointers", two_sum_sorted},
    {"hash map", two_sum_hash},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[8];
    int n;
    int target;
    int expect_i, expect_j;
} Case;

/* A pair is a correct answer if it is two distinct positions that sum to target. */
static int is_valid_pair(const int *nums, int n, int target, int i, int j)
{
    return 0 <= i && i < j && j < n && (long long)nums[i] + nums[j] == target;
}

int main(void)
{
    printf("Q01. Two Sum\n");

    /* Each case has exactly one answer, so every approach must return it. */
    const Case cases[] = {
        {{2, 7, 11, 15}, 4, 9, 0, 1},
        {{3, 2, 4}, 3, 6, 1, 2},
        {{3, 3}, 2, 6, 0, 1},
        {{-1, -2, -3, -4, -5}, 5, -8, 2, 4},
        {{0, 4, 3, 0}, 4, 0, 0, 3},
        {{INT_MAX, -1, 5}, 3, INT_MAX - 1, 0, 1}, /* would overflow in plain int math */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int i = -1, j = -1;
            int found = APPROACHES[a].fn(c->nums, c->n, c->target, &i, &j);
            snprintf(label, sizeof label, "test %d, %-20s -> (%d, %d)", t + 1,
                     APPROACHES[a].name, i, j);
            check(found && i == c->expect_i && j == c->expect_j, label);
        }
    }

    /* No pair exists: every approach must report "not found". */
    {
        const int nums[] = {1, 2, 3};
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int i, j;
            snprintf(label, sizeof label, "no answer, %-20s -> not found", APPROACHES[a].name);
            check(!APPROACHES[a].fn(nums, 3, 100, &i, &j), label);
        }
    }

    /* Random cross-check. Random arrays can have several valid pairs, so each
     * approach must agree with brute force on whether a pair exists and must
     * return a pair that is actually valid. */
    {
        srand(12345);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums[12];
            int n = 2 + rand() % 11;
            for (int k = 0; k < n; k++)
                nums[k] = rand() % 41 - 20;
            int target = rand() % 81 - 40;

            int bi, bj;
            int expect_found = two_sum_brute(nums, n, target, &bi, &bj);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int i = -1, j = -1;
                int found = APPROACHES[a].fn(nums, n, target, &i, &j);
                if (found != expect_found || (found && !is_valid_pair(nums, n, target, i, j)))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
