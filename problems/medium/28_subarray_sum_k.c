/*
 * Q28. Subarray Sum Equals K
 * Difficulty : Medium
 * Topics     : Arrays, Prefix Sums, Hashing, Sliding Window
 *
 * Problem
 *   Given an array nums of n integers (negatives allowed) and an integer k,
 *   count the contiguous, non-empty subarrays whose sum is exactly k.
 *
 * Example
 *   nums = [1, 2, 3], k = 3   ->   2   ([1, 2] and [3])
 *
 * Approaches
 *   1. Triple loop                sum every subarray from scratch   Time O(n^3)  Space O(1)
 *   2. Running sum                extend each start one step        Time O(n^2)  Space O(1)
 *   3. Prefix + hash  [optimal]   count earlier prefixes P - k      Time O(n)    Space O(n)
 *   4. Sliding window             atMost(k) - atMost(k - 1), valid  Time O(n)    Space O(1)
 *                                 ONLY if no element is negative
 *
 * Key insight
 *   With prefix sums P[0] = 0, P[j] = nums[0] + ... + nums[j-1], the sum
 *   of nums[i..j-1] is P[j] - P[i]. So "subarrays ending here with sum k"
 *   = "earlier prefixes equal to P[j] - k". A hash map of prefix counts
 *   answers that in O(1) per step.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 28_subarray_sum_k.c -o subarray_sum_k && ./subarray_sum_k
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Every approach returns the count as long long (there are n(n+1)/2
 * subarrays, which outgrows int quickly) or -1 if it ran out of memory.
 * Sums are long long: INT_MAX + INT_MAX does not fit in int.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Triple loop
 *   For every (i, j) with i <= j, add nums[i..j] from scratch and compare.
 *   Time O(n^3)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long count_triple(const int *nums, int n, int k)
{
    long long count = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            long long sum = 0;
            for (int t = i; t <= j; t++)
                sum += nums[t];
            if (sum == k)
                count++;
        }
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Running sum (double loop)
 *   For a fixed start i, sum(i..j) = sum(i..j-1) + nums[j], so keep a
 *   running total while j moves right. Note we can NOT stop early when the
 *   sum passes k: a later negative number may bring it back down.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long count_running(const int *nums, int n, int k)
{
    long long count = 0;
    for (int i = 0; i < n; i++) {
        long long sum = 0;
        for (int j = i; j < n; j++) {
            sum += nums[j];
            if (sum == k)
                count++;
        }
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: Prefix sums + hash map of prefix counts (optimal)
 *   Walk left to right keeping prefix = sum of everything so far, and a map
 *   seen[p] = how many earlier prefixes equal p. The map starts with
 *   seen[0] = 1 for the empty prefix, so subarrays that start at index 0
 *   are counted too. At each element:
 *       prefix += nums[j]
 *       count  += seen[prefix - k]   (each earlier match starts a subarray
 *                                     ending at j with sum k)
 *       seen[prefix]++
 *   Order matters: look up BEFORE inserting the current prefix, otherwise
 *   k == 0 would count the empty subarray.
 *   The map is an open-addressing table with long long keys (prefix sums
 *   leave int range), at least 2(n + 1) slots, so it never fills.
 *   Time O(n) expected   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    long long *keys;
    long long *counts;
    unsigned char *used;
    size_t mask; /* capacity - 1, capacity is a power of two */
} PrefixMap;

static size_t hash_ll(long long key)
{
    /* splitmix64 finaliser: mixes all 64 bits so nearby sums spread out */
    unsigned long long x = (unsigned long long)key;
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return (size_t)x;
}

static int pmap_init(PrefixMap *m, int expected)
{
    size_t cap = 16;
    while (cap < ((size_t)expected + 1) * 2)
        cap <<= 1;
    m->keys = malloc(cap * sizeof *m->keys);
    m->counts = malloc(cap * sizeof *m->counts);
    m->used = calloc(cap, 1);
    m->mask = cap - 1;
    return m->keys && m->counts && m->used;
}

static void pmap_free(PrefixMap *m)
{
    free(m->keys);
    free(m->counts);
    free(m->used);
}

/* Returns the slot for key: either where it lives or the empty slot where
 * it would be inserted. */
static size_t pmap_slot(const PrefixMap *m, long long key)
{
    size_t h = hash_ll(key) & m->mask;
    while (m->used[h] && m->keys[h] != key)
        h = (h + 1) & m->mask;
    return h;
}

static long long pmap_get(const PrefixMap *m, long long key)
{
    size_t h = pmap_slot(m, key);
    return m->used[h] ? m->counts[h] : 0;
}

static void pmap_increment(PrefixMap *m, long long key)
{
    size_t h = pmap_slot(m, key);
    if (!m->used[h]) {
        m->used[h] = 1;
        m->keys[h] = key;
        m->counts[h] = 0;
    }
    m->counts[h]++;
}

static long long count_prefix_hash(const int *nums, int n, int k)
{
    PrefixMap m;
    if (!pmap_init(&m, n)) {
        pmap_free(&m);
        return -1;
    }
    long long count = 0, prefix = 0;
    pmap_increment(&m, 0); /* the empty prefix */
    for (int j = 0; j < n; j++) {
        prefix += nums[j];
        count += pmap_get(&m, prefix - k);
        pmap_increment(&m, prefix);
    }
    pmap_free(&m);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 4: Sliding window (valid ONLY when every nums[i] >= 0)
 *   With no negatives, extending a window can only increase its sum and
 *   shrinking it from the left can only decrease it. That monotonicity
 *   lets us count windows with sum <= limit in one pass: for each right
 *   end, move left forward while the sum is too big; then every start in
 *   [left, right] gives a sum <= limit, which is right - left + 1 windows.
 *   Exactly k = (sum <= k) minus (sum <= k - 1). Counting "exactly k"
 *   directly with one window miscounts zeros: in [0, 0, 0] with k = 0 all
 *   six subarrays qualify, but a single window sees only one per end.
 *
 *   Why it breaks with negatives: the "shrink while too big" step assumes
 *   that dropping the left element makes the sum smaller and that a start
 *   which is too far left for this right end is too far left for every
 *   later right end too. A negative element breaks both: removing it makes
 *   the sum BIGGER, and a later negative can bring a discarded start back
 *   under the limit. The counts are then simply wrong (see the demo test).
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long count_at_most(const int *nums, int n, long long limit)
{
    long long count = 0, sum = 0;
    int left = 0;
    for (int right = 0; right < n; right++) {
        sum += nums[right];
        while (left <= right && sum > limit)
            sum -= nums[left++];
        count += right - left + 1; /* 0 if the window emptied */
    }
    return count;
}

static long long count_sliding_window(const int *nums, int n, int k)
{
    return count_at_most(nums, n, k) - count_at_most(nums, n, (long long)k - 1);
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

typedef long long (*CountFn)(const int *nums, int n, int k);

static const struct {
    const char *name;
    CountFn fn;
    int nonneg_only; /* 1: only correct when every element is >= 0 */
} APPROACHES[] = {
    {"triple loop", count_triple, 0},
    {"running sum", count_running, 0},
    {"prefix + hash", count_prefix_hash, 0},
    {"sliding window", count_sliding_window, 1},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 12

typedef struct {
    int nums[MAXN];
    int n;
    int k;
    long long expect;
} Case;

static int all_non_negative(const int *nums, int n)
{
    for (int i = 0; i < n; i++)
        if (nums[i] < 0)
            return 0;
    return 1;
}

int main(void)
{
    printf("Q28. Subarray Sum Equals K\n");

    /* Expected counts found by listing the subarrays by hand. The sliding
     * window is run only on the cases with no negative numbers. */
    const Case cases[] = {
        {{1, 1, 1}, 3, 2, 2},                     /* [1,1] twice */
        {{1, 2, 3}, 3, 3, 2},                     /* [1,2], [3] */
        {{1, -1, 0}, 3, 0, 3},                    /* [1,-1], [0], [1,-1,0] */
        {{0, 0, 0}, 3, 0, 6},                     /* every subarray */
        {{0}, 0, 0, 0},                           /* empty array: nothing to count */
        {{5}, 1, 5, 1},                           /* single element, hit */
        {{5}, 1, 0, 0},                           /* single element, miss */
        {{-1, -1, 1}, 3, 0, 1},                   /* [-1,1] */
        {{3, 4, 7, 2, -3, 1, 4, 2}, 8, 7, 4},     /* [3,4] [7] [7,2,-3,1] [1,4,2] */
        {{2, 2, 2, 2}, 4, 4, 3},                  /* all equal */
        {{-2, -3, 5, -5}, 4, -5, 3},              /* negative k */
        {{0, 1, 0}, 3, 0, 2},                     /* the two [0]s */
        {{0, 1, 0}, 3, 1, 4},                     /* [1] [0,1] [1,0] [0,1,0] */
        {{1, 2, 1, 2, 1}, 5, 3, 4},
        {{INT_MAX, INT_MAX, -INT_MAX}, 3, INT_MAX, 3}, /* MAX+MAX overflows int */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        int nonneg = all_non_negative(c->nums, c->n);
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (APPROACHES[a].nonneg_only && !nonneg)
                continue;
            long long got = APPROACHES[a].fn(c->nums, c->n, c->k);
            snprintf(label, sizeof label, "test %2d, %-14s k=%d -> %lld", t + 1,
                     APPROACHES[a].name, c->k, got);
            check(got == c->expect, label);
        }
    }

    /* Demo, not part of the cross-check: the sliding window on an array WITH
     * a negative. nums = [-1, 1], k = 0. The subarrays are [-1] = -1,
     * [1] = 1 and [-1, 1] = 0, so the true count is 1. By hand, the window
     * computes atMost(0) = 3: the window [-1, 1] has sum 0, so it also
     * counts the shorter window [1], assuming shorter means smaller, but
     * [1] = 1 > 0. With atMost(-1) = 1 it answers 3 - 1 = 2. */
    {
        const int nums[] = {-1, 1};
        long long truth = count_triple(nums, 2, 0);
        long long window = count_sliding_window(nums, 2, 0);
        snprintf(label, sizeof label,
                 "demo: window on [-1, 1], k=0 gives %lld, true count %lld (wrong, as expected)",
                 window, truth);
        check(truth == 1 && window == 2, label);
    }

    /* Random cross-check against the triple loop. Every round tests one
     * array that may contain negatives (approaches 1-3) and one array with
     * no negatives (all approaches, including the sliding window). n <= 12
     * keeps the O(n^3) reference tiny. */
    {
        srand(2828);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int any[MAXN], nonneg[MAXN];
            int n = rand() % (MAXN + 1);
            for (int i = 0; i < n; i++) {
                any[i] = rand() % 11 - 5;
                nonneg[i] = rand() % 6;
            }
            int k_any = rand() % 17 - 8;
            int k_nonneg = rand() % 13 - 2; /* includes negative k: answer 0 */

            long long expect_any = count_triple(any, n, k_any);
            long long expect_nonneg = count_triple(nonneg, n, k_nonneg);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (!APPROACHES[a].nonneg_only && APPROACHES[a].fn(any, n, k_any) != expect_any)
                    bad++;
                if (APPROACHES[a].fn(nonneg, n, k_nonneg) != expect_nonneg)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d x 2 arrays, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
