/*
 * Q16. Single Number
 * Difficulty : Easy
 * Topics     : Arrays, Hashing, Sorting, Bit Manipulation
 *
 * Problem
 *   Given a non-empty array nums in which every value appears exactly twice
 *   except for one value that appears once, return that single value.
 *   Here every approach returns 1 and writes *out when it finds the value,
 *   returns 0 if there is none, and returns -1 if it could not allocate.
 *
 * Example
 *   nums = [4, 1, 2, 1, 2]   ->   4
 *
 * Approaches
 *   1. Brute force         count each value's occurrences      Time O(n^2)      Space O(1)
 *   2. Sort                sort a copy, find the broken pair   Time O(n log n)  Space O(n)
 *   3. Hash map            count occurrences in a table        Time O(n)        Space O(n)
 *   4. XOR    [optimal]    XOR everything, pairs cancel        Time O(n)        Space O(1)
 *
 * Key insight
 *   x ^ x = 0 and x ^ 0 = x, and XOR is commutative and associative, so the
 *   order of the elements does not matter. XOR of the whole array is
 *   (a ^ a) ^ (b ^ b) ^ ... ^ s = 0 ^ 0 ^ ... ^ s = s.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 16_single_number.c -o single_number && ./single_number
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   For each element, count how many times its value occurs in the whole
 *   array. The one that occurs exactly once is the answer.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int single_brute(const int *nums, int n, int *out)
{
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++)
            if (nums[j] == nums[i])
                count++;
        if (count == 1) {
            *out = nums[i];
            return 1;
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort a copy and check pairs
 *   After sorting, the copies of each value are adjacent, so the array
 *   reads (a, a), (b, b), ... in pairs starting at even indices, until the
 *   single value shifts everything by one. Check a[0] vs a[1], a[2] vs
 *   a[3], ...: the first pair that does not match starts with the single
 *   value. If every pair matches, the single value is the last element.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y); /* not x - y: INT_MIN - 1 overflows */
}

static int single_sort(const int *nums, int n, int *out)
{
    if (n == 0)
        return 0;
    int *a = malloc((size_t)n * sizeof *a);
    if (a == NULL)
        return -1;
    memcpy(a, nums, (size_t)n * sizeof *a);
    qsort(a, (size_t)n, sizeof *a, cmp_int);

    int i = 0;
    while (i + 1 < n && a[i] == a[i + 1])
        i += 2;
    /* Either a pair broke at i, or i is the last index. i == n can only
     * happen if every value was paired (invalid input): never read a[n]. */
    int found = i < n;
    if (found)
        *out = a[i];
    free(a);
    return found;
}

/* -------------------------------------------------------------------------
 * Approach 3: Hash map of counts
 *   Count every value in one pass, then return the value whose count is 1.
 *   C has no built-in map, so this is a small open-addressing table with
 *   linear probing, the same design as Q01. Capacity is at least 2n, so the
 *   table is never more than half full and probe chains stay short.
 *   Time O(n) expected   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    int *keys;
    int *counts;
    unsigned char *used;
    size_t mask; /* capacity - 1, capacity is a power of two */
} CountMap;

static size_t hash_int(int key)
{
    unsigned int x = (unsigned int)key;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

static int map_init(CountMap *m, int expected)
{
    size_t cap = 16;
    while (cap < (size_t)expected * 2)
        cap <<= 1;
    m->keys = malloc(cap * sizeof *m->keys);
    m->counts = malloc(cap * sizeof *m->counts);
    m->used = calloc(cap, 1);
    m->mask = cap - 1;
    return m->keys && m->counts && m->used;
}

static void map_free(CountMap *m)
{
    free(m->keys);
    free(m->counts);
    free(m->used);
}

/* Adds 1 to key's count, inserting it with count 1 if absent. */
static void map_add(CountMap *m, int key)
{
    size_t h = hash_int(key) & m->mask;
    while (m->used[h]) {
        if (m->keys[h] == key) {
            m->counts[h]++;
            return;
        }
        h = (h + 1) & m->mask;
    }
    m->used[h] = 1;
    m->keys[h] = key;
    m->counts[h] = 1;
}

static int single_hash(const int *nums, int n, int *out)
{
    CountMap m;
    if (!map_init(&m, n)) {
        map_free(&m);
        return -1;
    }
    for (int i = 0; i < n; i++)
        map_add(&m, nums[i]);

    int found = 0;
    for (size_t h = 0; h <= m.mask && !found; h++) {
        if (m.used[h] && m.counts[h] == 1) {
            *out = m.keys[h];
            found = 1;
        }
    }
    map_free(&m);
    return found;
}

/* -------------------------------------------------------------------------
 * Approach 4: XOR of all elements (optimal)
 *   XOR works bit by bit: in each bit position, a value that appears twice
 *   contributes its bit twice, and 1 ^ 1 = 0, so it vanishes. The single
 *   value contributes its bit once, so what is left is exactly that value.
 *   Order is irrelevant because XOR is commutative and associative.
 *   It also works for negative numbers, since only bit patterns matter, and
 *   unlike a sum trick (2 * sum of distinct - sum of all) it cannot overflow.
 *   It relies on the "exactly twice" promise: for an invalid input it still
 *   returns something, it just cannot tell you the input was invalid.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int single_xor(const int *nums, int n, int *out)
{
    if (n == 0)
        return 0;
    int x = 0;
    for (int i = 0; i < n; i++)
        x ^= nums[i];
    *out = x;
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

typedef int (*SingleFn)(const int *nums, int n, int *out);

static const struct {
    const char *name;
    SingleFn fn;
} APPROACHES[] = {
    {"brute force", single_brute},
    {"sort", single_sort},
    {"hash map", single_hash},
    {"xor", single_xor},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[9];
    int n;
    int expect;
} Case;

int main(void)
{
    printf("Q16. Single Number\n");

    const Case cases[] = {
        {{2, 2, 1}, 3, 1},
        {{4, 1, 2, 1, 2}, 5, 4},
        {{1}, 1, 1},                                     /* single element */
        {{-1, -1, -2}, 3, -2},                           /* negatives */
        {{0, 7, 7}, 3, 0},                               /* the answer is 0 */
        {{3, 3, 9, 8, 8, 9, -4}, 7, -4},
        {{5, 6, 6}, 3, 5},                               /* single is the smallest */
        {{1, 1, 2, 2, 9}, 5, 9},                         /* single sorts to the end */
        {{INT_MIN, 5, 5}, 3, INT_MIN},
        {{INT_MAX, INT_MIN, INT_MAX}, 3, INT_MIN},
        {{INT_MAX, INT_MAX, INT_MIN, INT_MIN, 0}, 5, 0}, /* a sum would overflow */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = 0;
            int found = APPROACHES[a].fn(c->nums, c->n, &got);
            snprintf(label, sizeof label, "test %2d, %-11s -> %d", t + 1, APPROACHES[a].name, got);
            check(found == 1 && got == c->expect, label);
        }
    }

    /* Random cross-check: pick 1 + pairs distinct values from -50..50, put
     * each "pair" value in twice and the single value once, shuffle. Every
     * approach must find the planted single value, as brute force does. */
    {
        srand(1616);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int distinct[8] = {0}, nums[15];
            int pairs = rand() % 8; /* 0..7 pairs, so n = 2 * pairs + 1 <= 15 */
            for (int k = 0; k <= pairs; k++) {
                int v, dup;
                do { /* draw until v differs from every value chosen so far */
                    v = rand() % 101 - 50;
                    dup = 0;
                    for (int q = 0; q < k; q++)
                        dup |= distinct[q] == v;
                } while (dup);
                distinct[k] = v;
            }
            int n = 0;
            nums[n++] = distinct[0]; /* the single one */
            for (int k = 1; k <= pairs; k++) {
                nums[n++] = distinct[k];
                nums[n++] = distinct[k];
            }
            for (int k = n - 1; k > 0; k--) { /* Fisher-Yates shuffle */
                int j = rand() % (k + 1);
                int tmp = nums[k];
                nums[k] = nums[j];
                nums[j] = tmp;
            }

            int expect = 0;
            if (single_brute(nums, n, &expect) != 1 || expect != distinct[0])
                bad++;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int got = 0;
                if (APPROACHES[a].fn(nums, n, &got) != 1 || got != expect)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
