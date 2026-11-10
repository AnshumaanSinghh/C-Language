/*
 * Q21. Maximum Subarray Sum
 * Difficulty : Medium
 * Topics     : Arrays, DP, Divide & Conquer
 *
 * Problem
 *   Given an array nums of n >= 1 integers, find the contiguous, non-empty
 *   subarray with the largest sum. Return that sum and the start and end
 *   indices (inclusive) of one subarray that achieves it.
 *
 * Example
 *   nums = [-2, 1, -3, 4, -1, 2, 1, -5, 4]   ->   6, indices 3..6
 *   because 4 + (-1) + 2 + 1 = 6
 *
 * Approaches
 *   1. Triple loop               sum every subarray from scratch    Time O(n^3)      Space O(1)
 *   2. Running sum               reuse the sum while extending      Time O(n^2)      Space O(1)
 *   3. Divide & conquer          best of left, right, crossing      Time O(n log n)  Space O(log n)
 *   4. Kadane       [optimal]    best subarray ending at i          Time O(n)        Space O(1)
 *
 * Key insight
 *   The best subarray ending at position i is either nums[i] alone or
 *   nums[i] glued onto the best subarray ending at i - 1. A running sum that
 *   has gone negative can only hurt whatever comes next, so throw it away
 *   and restart. One pass is enough.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 21_max_subarray_sum.c -o max_subarray_sum && ./max_subarray_sum
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Every approach has the same contract:
 *   - n >= 1 (the subarray must be non-empty, so an empty array has no answer)
 *   - returns the maximum sum as long long, because adding two ints can
 *     overflow int (INT_MAX + INT_MAX does not fit)
 *   - writes the inclusive bounds of one best subarray to *out_l, *out_r
 * Several subarrays can tie for the best sum (e.g. [1, -1, 1]), so different
 * approaches may legitimately report different indices.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Triple loop
 *   Enumerate every pair (l, r) with l <= r and add nums[l..r] from scratch
 *   with a third loop. Obviously correct: it looks at every candidate.
 *   The best sum starts as LLONG_MIN, not 0, so all-negative arrays work.
 *   Time O(n^3)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long max_sub_triple(const int *nums, int n, int *out_l, int *out_r)
{
    long long best = LLONG_MIN;
    int bl = 0, br = 0;
    for (int l = 0; l < n; l++) {
        for (int r = l; r < n; r++) {
            long long sum = 0;
            for (int k = l; k <= r; k++)
                sum += nums[k];
            if (sum > best) {
                best = sum;
                bl = l;
                br = r;
            }
        }
    }
    *out_l = bl;
    *out_r = br;
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 2: Running sum (double loop)
 *   sum(l..r) = sum(l..r-1) + nums[r], so for a fixed start l we can extend
 *   r one step at a time and reuse the previous sum instead of re-adding.
 *   That removes the innermost loop.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long max_sub_running(const int *nums, int n, int *out_l, int *out_r)
{
    long long best = LLONG_MIN;
    int bl = 0, br = 0;
    for (int l = 0; l < n; l++) {
        long long sum = 0;
        for (int r = l; r < n; r++) {
            sum += nums[r];
            if (sum > best) {
                best = sum;
                bl = l;
                br = r;
            }
        }
    }
    *out_l = bl;
    *out_r = br;
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 3: Divide and conquer
 *   Split [lo, hi] at mid. The best subarray is in exactly one of three
 *   places:
 *     - entirely in the left half  [lo, mid]      -> recurse
 *     - entirely in the right half [mid+1, hi]    -> recurse
 *     - crossing the middle: it must contain both nums[mid] and nums[mid+1],
 *       so it is (best suffix of the left half ending at mid) +
 *       (best prefix of the right half starting at mid+1). Both are found
 *       with one linear scan outward from the middle.
 *   The work per level is O(n) and there are log n levels, like merge sort.
 *   Time O(n log n)   Space O(log n) recursion depth
 * ------------------------------------------------------------------------- */
typedef struct {
    long long sum;
    int l, r;
} Range;

static Range max_sub_dc_rec(const int *nums, int lo, int hi)
{
    if (lo == hi) {
        Range single = {nums[lo], lo, lo};
        return single;
    }
    int mid = lo + (hi - lo) / 2;
    Range left = max_sub_dc_rec(nums, lo, mid);
    Range right = max_sub_dc_rec(nums, mid + 1, hi);

    /* Best sum of nums[i..mid], scanning i leftwards from mid. */
    long long run = 0, best_left = LLONG_MIN;
    int cross_l = mid;
    for (int i = mid; i >= lo; i--) {
        run += nums[i];
        if (run > best_left) {
            best_left = run;
            cross_l = i;
        }
    }
    /* Best sum of nums[mid+1..j], scanning j rightwards from mid+1. */
    run = 0;
    long long best_right = LLONG_MIN;
    int cross_r = mid + 1;
    for (int j = mid + 1; j <= hi; j++) {
        run += nums[j];
        if (run > best_right) {
            best_right = run;
            cross_r = j;
        }
    }
    /* Both halves are non-empty, so best_left and best_right are real sums. */
    Range cross = {best_left + best_right, cross_l, cross_r};

    Range best = left;
    if (right.sum > best.sum)
        best = right;
    if (cross.sum > best.sum)
        best = cross;
    return best;
}

static long long max_sub_dc(const int *nums, int n, int *out_l, int *out_r)
{
    Range best = max_sub_dc_rec(nums, 0, n - 1);
    *out_l = best.l;
    *out_r = best.r;
    return best.sum;
}

/* -------------------------------------------------------------------------
 * Approach 4: Kadane's algorithm (optimal)
 *   Let cur be the best sum of a subarray that ENDS at index i. Then
 *       cur(i) = max(nums[i], cur(i-1) + nums[i])
 *   i.e. extend the previous run unless it is negative, in which case it
 *   only drags nums[i] down and we start a new run at i. The answer is the
 *   largest cur seen. To report indices, remember where the current run
 *   started and copy (start, i) whenever cur beats the best.
 *   Pitfall: initialise from nums[0], not 0, or an all-negative array would
 *   wrongly answer 0 (the empty subarray, which is not allowed).
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long max_sub_kadane(const int *nums, int n, int *out_l, int *out_r)
{
    long long cur = nums[0], best = nums[0];
    int start = 0, bl = 0, br = 0;
    for (int i = 1; i < n; i++) {
        if (cur < 0) {
            cur = nums[i];
            start = i;
        } else {
            cur += nums[i];
        }
        if (cur > best) {
            best = cur;
            bl = start;
            br = i;
        }
    }
    *out_l = bl;
    *out_r = br;
    return best;
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

typedef long long (*MaxSubFn)(const int *nums, int n, int *out_l, int *out_r);

static const struct {
    const char *name;
    MaxSubFn fn;
} APPROACHES[] = {
    {"triple loop", max_sub_triple},
    {"running sum", max_sub_running},
    {"divide & conquer", max_sub_dc},
    {"kadane", max_sub_kadane},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[10];
    int n;
    long long expect_sum;
} Case;

/* The reported indices are valid if they bound a non-empty subarray whose
 * sum really is the reported sum. */
static int indices_give_sum(const int *nums, int n, int l, int r, long long sum)
{
    if (l < 0 || l > r || r >= n)
        return 0;
    long long s = 0;
    for (int k = l; k <= r; k++)
        s += nums[k];
    return s == sum;
}

int main(void)
{
    printf("Q21. Maximum Subarray Sum\n");

    /* Expected sums worked out by hand. Indices are checked for validity
     * rather than exact values because of ties. */
    const Case cases[] = {
        {{-2, 1, -3, 4, -1, 2, 1, -5, 4}, 9, 6},          /* 4 -1 2 1 */
        {{1}, 1, 1},                                       /* single element */
        {{-5}, 1, -5},                                     /* single negative */
        {{-3, -1, -2}, 3, -1},                             /* all negative: best single */
        {{5, 4, -1, 7, 8}, 5, 23},                         /* whole array */
        {{0, 0, 0}, 3, 0},                                 /* all zero */
        {{1, -1, 1}, 3, 1},                                /* ties: [0..0], [2..2], [0..2] */
        {{2, -1, 2}, 3, 3},                                /* worth crossing the dip */
        {{-2, -2, -2, -2}, 4, -2},                         /* all equal negative */
        {{INT_MAX, INT_MAX, -1}, 3, 2LL * INT_MAX},        /* overflows int */
        {{INT_MIN, INT_MIN}, 2, INT_MIN},                  /* most negative values */
        {{-1, 3, -1, -1, 3, -1}, 6, 4},                    /* 3 -1 -1 3 */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int l = -1, r = -1;
            long long sum = APPROACHES[a].fn(c->nums, c->n, &l, &r);
            snprintf(label, sizeof label, "test %2d, %-16s -> sum %lld at [%d..%d]", t + 1,
                     APPROACHES[a].name, sum, l, r);
            check(sum == c->expect_sum && indices_give_sum(c->nums, c->n, l, r, sum), label);
        }
    }

    /* Random cross-check: every approach must match the triple loop's sum and
     * return indices that really produce it. n <= 14 keeps O(n^3) tiny. */
    {
        srand(2121);
        int bad = 0;
        const int rounds = 400;
        for (int round = 0; round < rounds; round++) {
            int nums[14];
            int n = 1 + rand() % 14;
            for (int k = 0; k < n; k++)
                nums[k] = rand() % 41 - 20;

            int bl, br;
            long long expect = max_sub_triple(nums, n, &bl, &br);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int l = -1, r = -1;
                long long sum = APPROACHES[a].fn(nums, n, &l, &r);
                if (sum != expect || !indices_give_sum(nums, n, l, r, sum))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
