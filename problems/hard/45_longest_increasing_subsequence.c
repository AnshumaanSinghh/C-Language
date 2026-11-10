/*
 * Q45. Longest Increasing Subsequence
 * Difficulty : Hard
 * Topics     : Dynamic Programming, Binary Search
 *
 * Problem
 *   Given an array nums of n integers, return the length of the longest
 *   strictly increasing subsequence, and reconstruct one such subsequence.
 *   A subsequence keeps the original order but may skip elements.
 *
 * Example
 *   nums = [10, 9, 2, 5, 3, 7, 101, 18]   ->   4, e.g. [2, 3, 7, 18]
 *
 * Approaches
 *   1. Recursion (take/skip)        try every subsequence             Time O(2^n)      Space O(n)
 *   2. Memoization                  cache f(index, previous index)    Time O(n^2)      Space O(n^2)
 *   3. Bottom-up DP                 best[i] = longest ending at i     Time O(n^2)      Space O(n)
 *   4. Patience sorting [optimal]   tails[] + binary search           Time O(n log n)  Space O(n)
 *
 * Key insight
 *   Of all increasing subsequences of a given length L seen so far, only the
 *   one with the SMALLEST last element matters: any later element that can
 *   extend some length-L subsequence can also extend that one. So keep
 *   tails[L-1] = the smallest possible last element of a length-L increasing
 *   subsequence. tails[] is always strictly increasing, so each new element
 *   finds its slot by binary search, and the number of slots used is the
 *   answer. Remembering, for every element, which element came before it
 *   (a parent index) lets us rebuild one actual LIS at the end.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 45_longest_increasing_subsequence.c -o longest_increasing_subsequence && ./longest_increasing_subsequence
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Every approach has the same contract:
 *   returns the LIS length (0 for an empty array, -1 if memory ran out) and
 *   writes one longest strictly increasing subsequence into out[0..len-1].
 *   out must have room for n ints. nums is never modified.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Recursion (take / skip)
 *   f(i, prev) = length of the longest increasing subsequence that uses only
 *   nums[i..n-1] and whose elements are all greater than nums[prev]
 *   (prev = -1 means "nothing taken yet", so anything may be taken).
 *     skip nums[i]:  f(i + 1, prev)
 *     take nums[i]:  1 + f(i + 1, i)    allowed only if prev == -1 or nums[i] > nums[prev]
 *   Every subsequence is one path of take/skip choices, so taking the better
 *   branch at every step gives the answer f(0, -1). The same (i, prev) pair
 *   is recomputed over and over, which is why this is exponential.
 *
 *   Reconstruction: replay the choices. Walk i = 0..n-1 and take nums[i]
 *   when taking is allowed and 1 + f(i + 1, i) == f(i, prev), i.e. taking is
 *   one of the optimal choices. Every step stays on an optimal path, so the
 *   elements taken form an LIS.
 *   Time O(2^n)   Space O(n) recursion depth. The tests only run it on n <= 15.
 * ------------------------------------------------------------------------- */
static int lis_rec_f(const int *nums, int n, int i, int prev)
{
    if (i == n)
        return 0;
    int best = lis_rec_f(nums, n, i + 1, prev);
    if (prev < 0 || nums[i] > nums[prev]) {
        int take = 1 + lis_rec_f(nums, n, i + 1, i);
        if (take > best)
            best = take;
    }
    return best;
}

static int lis_recursion(const int *nums, int n, int *out)
{
    int len = 0, prev = -1;
    for (int i = 0; i < n; i++) {
        int allowed = prev < 0 || nums[i] > nums[prev];
        if (allowed && 1 + lis_rec_f(nums, n, i + 1, i) == lis_rec_f(nums, n, i, prev)) {
            out[len++] = nums[i];
            prev = i;
        }
    }
    return len;
}

/* -------------------------------------------------------------------------
 * Approach 2: Memoization on (index, previous index)
 *   Same recurrence as approach 1, but each of the n * (n + 1) states
 *   (i, prev) is computed once and stored. Each state does O(1) work besides
 *   its two lookups, so the total is O(n^2).
 *   The table is indexed by prev + 1 because prev can be -1.
 *   Reconstruction replays the decisions exactly like approach 1, but every
 *   f(...) is now a table lookup.
 *   Pitfall: the recursion is still n levels deep, so a huge n would overflow
 *   the call stack; approach 3 removes the recursion entirely.
 *   Time O(n^2)   Space O(n^2) for the table + O(n) recursion depth
 * ------------------------------------------------------------------------- */
typedef struct {
    const int *nums;
    int n;
    int *memo; /* memo[i * (n + 1) + (prev + 1)], -1 means "not computed yet" */
} LisMemo;

static int lis_memo_f(LisMemo *m, int i, int prev)
{
    if (i == m->n)
        return 0;
    int *slot = &m->memo[(size_t)i * (size_t)(m->n + 1) + (size_t)(prev + 1)];
    if (*slot >= 0)
        return *slot;

    int best = lis_memo_f(m, i + 1, prev);
    if (prev < 0 || m->nums[i] > m->nums[prev]) {
        int take = 1 + lis_memo_f(m, i + 1, i);
        if (take > best)
            best = take;
    }
    *slot = best;
    return best;
}

static int lis_memo(const int *nums, int n, int *out)
{
    if (n == 0)
        return 0;
    size_t cells = (size_t)n * (size_t)(n + 1);
    LisMemo m = {nums, n, malloc(cells * sizeof(int))};
    if (m.memo == NULL)
        return -1;
    for (size_t c = 0; c < cells; c++)
        m.memo[c] = -1;

    int len = 0, prev = -1;
    for (int i = 0; i < n; i++) {
        int allowed = prev < 0 || nums[i] > nums[prev];
        if (allowed && 1 + lis_memo_f(&m, i + 1, i) == lis_memo_f(&m, i, prev)) {
            out[len++] = nums[i];
            prev = i;
        }
    }
    free(m.memo);
    return len;
}

/* -------------------------------------------------------------------------
 * Approach 3: Bottom-up DP with parent pointers
 *   best[i] = length of the longest increasing subsequence that ENDS at i.
 *   It is 1 (nums[i] alone) or 1 + best[j] for some earlier j with
 *   nums[j] < nums[i] (append nums[i] to that subsequence). parent[i] records
 *   which j gave the maximum, so following parent[] from the index with the
 *   largest best[] walks one LIS backwards.
 *   Fixing the END of the subsequence (instead of "anything after prev") is
 *   what shrinks the state from two indices to one.
 *   Time O(n^2)   Space O(n)
 * ------------------------------------------------------------------------- */
static int lis_dp(const int *nums, int n, int *out)
{
    if (n == 0)
        return 0;
    int *best = malloc((size_t)n * sizeof *best);
    int *parent = malloc((size_t)n * sizeof *parent);
    if (best == NULL || parent == NULL) {
        free(best);
        free(parent);
        return -1;
    }

    int end = 0;
    for (int i = 0; i < n; i++) {
        best[i] = 1;
        parent[i] = -1;
        for (int j = 0; j < i; j++) {
            if (nums[j] < nums[i] && best[j] + 1 > best[i]) {
                best[i] = best[j] + 1;
                parent[i] = j;
            }
        }
        if (best[i] > best[end])
            end = i;
    }

    int len = best[end];
    for (int k = len - 1, i = end; k >= 0; k--, i = parent[i])
        out[k] = nums[i];
    free(best);
    free(parent);
    return len;
}

/* -------------------------------------------------------------------------
 * Approach 4: Patience sorting with binary search (optimal)
 *   tail_idx[L-1] = index of the smallest last element among all increasing
 *   subsequences of length L seen so far (see Key insight). For each x:
 *     - binary search the first slot whose tail is >= x (a lower bound).
 *       Using >= and not > is what makes the subsequence STRICTLY
 *       increasing: an equal value replaces a tail, it never extends it.
 *     - x becomes that slot's new tail; if the slot is one past the end,
 *       the LIS just got one longer.
 *   Why tails stay sorted: x is placed right after the largest tail < x, so
 *   it is smaller than or equal to the tail it replaces and still larger than
 *   the tail before it.
 *
 *   Reconstruction: when x lands in slot L, the subsequence it ends extends
 *   the current tail of slot L - 1, so parent[i] = tail_idx[L - 1]. That
 *   link stays valid even after slot L - 1 gets a newer tail. Follow parents
 *   from the final tail of the last slot.
 *   Pitfall: the tails array itself is usually NOT an increasing
 *   subsequence of nums. For [3, 4, 1] tails end up as [1, 4], but 1 comes
 *   after 4. The parent links are what give a real answer ([3, 4]).
 *   Time O(n log n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int lis_patience(const int *nums, int n, int *out)
{
    if (n == 0)
        return 0;
    int *tail_idx = malloc((size_t)n * sizeof *tail_idx);
    int *parent = malloc((size_t)n * sizeof *parent);
    if (tail_idx == NULL || parent == NULL) {
        free(tail_idx);
        free(parent);
        return -1;
    }

    int len = 0;
    for (int i = 0; i < n; i++) {
        int lo = 0, hi = len;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (nums[tail_idx[mid]] < nums[i])
                lo = mid + 1;
            else
                hi = mid;
        }
        parent[i] = lo > 0 ? tail_idx[lo - 1] : -1;
        tail_idx[lo] = i;
        if (lo == len)
            len++;
    }

    for (int k = len - 1, i = tail_idx[len - 1]; k >= 0; k--, i = parent[i])
        out[k] = nums[i];
    free(tail_idx);
    free(parent);
    return len;
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

typedef int (*LisFn)(const int *nums, int n, int *out);

/* max_n: largest input the approach is run on (recursion is exponential,
 * the O(n^2) ones are kept off the 100000-element test). */
static const struct {
    const char *name;
    LisFn fn;
    int max_n;
} APPROACHES[] = {
    {"recursion", lis_recursion, 15},
    {"memoization", lis_memo, 1000},
    {"bottom-up DP", lis_dp, 1000},
    {"patience sorting", lis_patience, 100000},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[16];
    int n;
    int expect;
} Case;

/* A reconstruction is correct if it has the expected length, is strictly
 * increasing, and appears in nums in order (greedy matching is enough to
 * test "is a subsequence"). */
static int is_valid_lis(const int *nums, int n, const int *seq, int len, int expect_len)
{
    if (len != expect_len)
        return 0;
    for (int k = 1; k < len; k++)
        if (seq[k - 1] >= seq[k])
            return 0;
    int k = 0;
    for (int i = 0; i < n && k < len; i++)
        if (nums[i] == seq[k])
            k++;
    return k == len;
}

/* Runs every approach whose max_n allows it on a large array. */
static void check_large(const char *title, const int *nums, int n, int expect)
{
    char label[160];
    int *out = malloc((size_t)n * sizeof *out);
    for (int a = 0; a < NUM_APPROACHES; a++) {
        if (n > APPROACHES[a].max_n)
            continue;
        int len = out ? APPROACHES[a].fn(nums, n, out) : -1;
        snprintf(label, sizeof label, "%s, %-18s -> %d", title, APPROACHES[a].name, len);
        check(out != NULL && is_valid_lis(nums, n, out, len, expect), label);
    }
    free(out);
}

int main(void)
{
    printf("Q45. Longest Increasing Subsequence\n");

    /* Expected lengths worked out by hand; an example LIS is in each comment. */
    const Case cases[] = {
        {{10, 9, 2, 5, 3, 7, 101, 18}, 8, 4},     /* 2 3 7 18                     */
        {{0, 1, 0, 3, 2, 3}, 6, 4},               /* 0 1 2 3                      */
        {{7, 7, 7, 7, 7, 7, 7}, 7, 1},            /* all equal: strict, so 1      */
        {{0}, 0, 0},                              /* empty array                  */
        {{5}, 1, 1},                              /* single element               */
        {{1, 2, 3, 4, 5}, 5, 5},                  /* already sorted               */
        {{5, 4, 3, 2, 1}, 5, 1},                  /* strictly decreasing          */
        {{2, 2, 3, 3, 4, 4}, 6, 3},               /* duplicates: 2 3 4            */
        {{-2, -1, -5, 0, -3, 1}, 6, 4},           /* negatives: -2 -1 0 1         */
        {{INT_MIN, INT_MAX, 0}, 3, 2},            /* extremes: INT_MIN 0          */
        {{3, 4, 1}, 3, 2},                        /* tails[] ends as [1,4]; LIS 3 4 */
        {{1, 3, 6, 7, 9, 4, 10, 5, 6}, 9, 6},     /* 1 3 6 7 9 10                 */
        {{3, 5, 6, 2, 5, 4, 19, 5, 6, 7, 12}, 11, 6}, /* 3 4 5 6 7 12             */
        {{4, 10, 4, 3, 8, 9}, 6, 3},              /* 4 8 9                        */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int out[16];
            int len = APPROACHES[a].fn(c->nums, c->n, out);
            snprintf(label, sizeof label, "test %2d, %-18s -> %d", t + 1, APPROACHES[a].name, len);
            check(is_valid_lis(c->nums, c->n, out, len, c->expect), label);
        }
    }

    /* n = 1000: values 0,0,1,1,...,499,499. Only 500 distinct values, each
     * pair is equal, so the strict LIS is 0,1,...,499 -> 500. */
    {
        const int n = 1000;
        int *nums = malloc((size_t)n * sizeof *nums);
        if (nums != NULL) {
            for (int i = 0; i < n; i++)
                nums[i] = i / 2;
            check_large("n = 1000 pairs", nums, n, 500);
        } else {
            check(0, "n = 1000 pairs: out of memory");
        }
        free(nums);
    }

    /* n = 100000: the block 0..999 repeated 100 times. Only 1000 distinct
     * values exist, and the first block alone is increasing -> 1000.
     * Only the O(n log n) approach runs here. */
    {
        const int n = 100000;
        int *nums = malloc((size_t)n * sizeof *nums);
        if (nums != NULL) {
            for (int i = 0; i < n; i++)
                nums[i] = i % 1000;
            check_large("n = 100000 blocks", nums, n, 1000);
        } else {
            check(0, "n = 100000 blocks: out of memory");
        }
        free(nums);
    }

    /* Random cross-check: lengths must match brute force, and every
     * reconstruction must be a valid LIS (there can be several). n <= 12
     * keeps the exponential recursion fast. */
    {
        srand(4545);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int nums[12] = {0}, out[12];
            int n = rand() % 13;
            for (int k = 0; k < n; k++)
                nums[k] = rand() % 21 - 10;

            int expect = lis_recursion(nums, n, out);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int len = APPROACHES[a].fn(nums, n, out);
                if (!is_valid_lis(nums, n, out, len, expect))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
