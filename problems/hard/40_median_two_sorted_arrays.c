/*
 * Q40. Median of Two Sorted Arrays
 * Difficulty : Hard
 * Topics     : Arrays, Binary Search, Divide and Conquer
 *
 * Problem
 *   Given two arrays a (length m) and b (length n), each sorted in
 *   non-decreasing order, return the median of all m + n numbers as a double.
 *   One array may be empty, but not both. The median is the middle element
 *   when m + n is odd, and the average of the two middle elements when it is
 *   even.
 *
 * Example
 *   a = [1, 3], b = [2]       ->   2.0   (merged: 1 2 3)
 *   a = [1, 2], b = [3, 4]    ->   2.5   (merged: 1 2 3 4, (2 + 3) / 2)
 *
 * Approaches
 *   1. Merge                        merge into a new array            Time O(m+n)           Space O(m+n)
 *   2. Walk to the middle           merge on the fly, store nothing   Time O(m+n)           Space O(1)
 *   3. K-th smallest                drop about k/2 elements per step  Time O(log(m+n))      Space O(log(m+n))
 *   4. Partition search  [optimal]  binary-search the cut             Time O(log min(m,n))  Space O(1)
 *
 * Key insight
 *   The median splits the merged sequence into a left half and a right half.
 *   You never need the merged sequence itself, only WHERE each array is cut:
 *   if i elements of a and j = half - i elements of b go to the left, the cut
 *   is right exactly when a[i-1] <= b[j] and b[j-1] <= a[i]. That condition
 *   is monotonic in i, so i can be found by binary search.
 *
 * Exactness
 *   For int inputs the median is an integer or an integer + 0.5 of magnitude
 *   below 2^31, which a double stores exactly. The two middle values are
 *   added in long long (INT_MAX + INT_MAX overflows int), then divided by
 *   2.0. That is why the tests can compare doubles with ==.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 40_median_two_sorted_arrays.c -o median_two_sorted_arrays && ./median_two_sorted_arrays
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* Average of two ints without int overflow; exact in double (see header). */
static double average2(int x, int y)
{
    return ((long long)x + y) / 2.0;
}

/* -------------------------------------------------------------------------
 * Approach 1: Merge into a new array
 *   The standard merge step of merge sort: repeatedly take the smaller front
 *   element. Once everything is in one sorted array, read the middle.
 *   Using <= takes from a on ties, which keeps the merge stable (it does not
 *   matter for the median, but it is the habit to have).
 *   Allocation failure returns 0.0; a real API would report it separately.
 *   Time O(m+n)   Space O(m+n)
 * ------------------------------------------------------------------------- */
static double median_merge(const int *a, int m, const int *b, int n)
{
    int total = m + n;
    int *c = malloc((size_t)total * sizeof *c);
    if (c == NULL)
        return 0.0;

    int i = 0, j = 0, k = 0;
    while (i < m && j < n)
        c[k++] = a[i] <= b[j] ? a[i++] : b[j++];
    while (i < m)
        c[k++] = a[i++];
    while (j < n)
        c[k++] = b[j++];

    double med = total % 2 ? c[total / 2] : average2(c[total / 2 - 1], c[total / 2]);
    free(c);
    return med;
}

/* -------------------------------------------------------------------------
 * Approach 2: Walk two pointers to the middle
 *   Same merge, but nothing is stored: step through the merged order and
 *   remember only the last two values seen. After total/2 + 1 steps,
 *   cur is merged[total/2] and prev is merged[total/2 - 1].
 *   Pitfall: the "take from a" test must check j >= n first, otherwise it
 *   reads b[n] once b is used up.
 *   Time O(m+n)   Space O(1)
 * ------------------------------------------------------------------------- */
static double median_walk(const int *a, int m, const int *b, int n)
{
    int total = m + n;
    int i = 0, j = 0, prev = 0, cur = 0;
    for (int step = 0; step <= total / 2; step++) {
        prev = cur;
        if (j >= n || (i < m && a[i] <= b[j]))
            cur = a[i++];
        else
            cur = b[j++];
    }
    return total % 2 ? cur : average2(prev, cur);
}

/* -------------------------------------------------------------------------
 * Approach 3: K-th smallest by recursive elimination
 *   Find the k-th smallest (k is 1-based) of the two arrays without merging.
 *   Take i = min(m, k/2) from a and j = min(n, k/2) from b, so i + j <= k,
 *   and compare a[i-1] with b[j-1].
 *
 *   Why the smaller one can be thrown away (say a[i-1] <= b[j-1]):
 *   in sorted order (ties broken in favour of a) the only elements that can
 *   come before a[i-1] are a[0..i-2] and some of b[0..j-2], because b[j-1]
 *   and everything after it is >= a[i-1]. So a[i-1] sits at position at most
 *   (i-1) + (j-1) + 1 = i + j - 1 <= k - 1: it and everything before it in
 *   a are strictly before the k-th element. Drop them and look for the
 *   (k-i)-th in what remains. The case b[j-1] < a[i-1] is symmetric.
 *
 *   Each step either removes about k/2 elements (so k halves) or empties
 *   one array, so there are O(log(m+n)) steps. Base cases: one array empty,
 *   or k == 1 (the smaller of the two fronts).
 *   Time O(log(m+n))   Space O(log(m+n)) recursion (O(1) written as a loop)
 * ------------------------------------------------------------------------- */
/* Requires 1 <= k <= m + n. */
static int kth_smallest(const int *a, int m, const int *b, int n, int k)
{
    if (m == 0)
        return b[k - 1];
    if (n == 0)
        return a[k - 1];
    if (k == 1)
        return a[0] < b[0] ? a[0] : b[0];

    int i = m < k / 2 ? m : k / 2;
    int j = n < k / 2 ? n : k / 2;
    if (a[i - 1] <= b[j - 1])
        return kth_smallest(a + i, m - i, b, n, k - i);
    return kth_smallest(a, m, b + j, n - j, k - j);
}

static double median_kth(const int *a, int m, const int *b, int n)
{
    int total = m + n;
    if (total % 2)
        return kth_smallest(a, m, b, n, total / 2 + 1);
    return average2(kth_smallest(a, m, b, n, total / 2),
                    kth_smallest(a, m, b, n, total / 2 + 1));
}

/* -------------------------------------------------------------------------
 * Approach 4: Binary search on the partition of the smaller array (optimal)
 *   Cut a after i elements and b after j elements, with
 *       i + j = half = (m + n + 1) / 2
 *   so the left side has half elements (the extra one when m + n is odd).
 *
 *            a:  a[0] .. a[i-1]  |  a[i] .. a[m-1]
 *            b:  b[0] .. b[j-1]  |  b[j] .. b[n-1]
 *                    LEFT        |      RIGHT
 *
 *   Each array is sorted, so the cut is valid (every left element <= every
 *   right element) exactly when the two cross conditions hold:
 *       a[i-1] <= b[j]   and   b[j-1] <= a[i]
 *   If a[i-1] > b[j], too much of a is on the left: search smaller i.
 *   If b[j-1] > a[i], too little of a is on the left: search larger i.
 *   As i grows a[i-1] grows and b[j] shrinks, so once the first condition
 *   fails it fails for every larger i: that monotonicity makes binary
 *   search correct. At the valid cut:
 *       odd total  -> median = max(a[i-1], b[j-1])
 *       even total -> median = (max(a[i-1], b[j-1]) + min(a[i], b[j])) / 2
 *
 *   Why search the SMALLER array: with m <= n, j = half - i stays inside
 *   [0, n] for every i in [0, m], so b[j-1] / b[j] are never out of range.
 *
 *   Sentinels: a missing neighbour (i == 0, i == m, j == 0, j == n) is
 *   replaced by INT_MIN on the left and INT_MAX on the right. That stays
 *   correct even when the data itself contains INT_MIN or INT_MAX:
 *     - the tests use strict ">", and "INT_MIN > x" or "x > INT_MAX" is never
 *       true, so a sentinel never forces a wrong move;
 *     - the left half is never empty (half >= 1), so at least one of
 *       a[i-1], b[j-1] is a real element and max() returns a real value
 *       (or an equal one); likewise the right half is non-empty whenever the
 *       total is even.
 *   The values are held in long long so the final sum cannot overflow.
 *   Time O(log min(m,n))   Space O(1)
 * ------------------------------------------------------------------------- */
static double median_partition(const int *a, int m, const int *b, int n)
{
    if (m > n)
        return median_partition(b, n, a, m);

    int half = (m + n + 1) / 2;
    int lo = 0, hi = m;
    while (lo <= hi) {
        int i = lo + (hi - lo) / 2;
        int j = half - i;
        long long a_left = i > 0 ? a[i - 1] : INT_MIN;
        long long a_right = i < m ? a[i] : INT_MAX;
        long long b_left = j > 0 ? b[j - 1] : INT_MIN;
        long long b_right = j < n ? b[j] : INT_MAX;

        if (a_left > b_right) {
            hi = i - 1;
        } else if (b_left > a_right) {
            lo = i + 1;
        } else {
            long long left_max = a_left > b_left ? a_left : b_left;
            if ((m + n) % 2)
                return (double)left_max;
            long long right_min = a_right < b_right ? a_right : b_right;
            return (left_max + right_min) / 2.0;
        }
    }
    return 0.0; /* unreachable when both inputs are sorted */
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

typedef double (*MedianFn)(const int *a, int m, const int *b, int n);

static const struct {
    const char *name;
    MedianFn fn;
} APPROACHES[] = {
    {"merge", median_merge},
    {"walk to the middle", median_walk},
    {"k-th smallest", median_kth},
    {"partition search", median_partition},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int a[8];
    int m;
    int b[8];
    int n;
    double expect;
} Case;

static int cmp_int(const void *x, const void *y)
{
    int p = *(const int *)x, q = *(const int *)y;
    return (p > q) - (p < q);
}

int main(void)
{
    printf("Q40. Median of Two Sorted Arrays\n");

    /* Expected medians worked out by merging by hand. */
    const Case cases[] = {
        {{1, 3}, 2, {2}, 1, 2.0},
        {{1, 2}, 2, {3, 4}, 2, 2.5},
        {{0}, 0, {1}, 1, 1.0},                           /* a empty, single element */
        {{2}, 1, {0}, 0, 2.0},                           /* b empty */
        {{0}, 0, {2, 3}, 2, 2.5},                        /* a empty, even total */
        {{1, 2, 3, 4, 5, 6, 7, 8}, 8, {0}, 0, 4.5},      /* b empty, longer a */
        {{0, 0}, 2, {0, 0}, 2, 0.0},
        {{1, 1, 1}, 3, {1, 1}, 2, 1.0},                  /* all equal */
        {{1, 2}, 2, {1, 2, 3}, 3, 2.0},                  /* duplicates across arrays: 1 1 2 2 3 */
        {{-5, -3, -1}, 3, {-2}, 1, -2.5},                /* negatives: -5 -3 -2 -1 */
        {{1, 2, 3, 4, 5}, 5, {6, 7, 8, 9, 10}, 5, 5.5},  /* disjoint ranges */
        {{1, 3, 5, 7}, 4, {2, 4, 6}, 3, 4.0},            /* interleaved */
        {{4}, 1, {1, 2, 3, 5, 6}, 5, 3.5},               /* 1 2 3 4 5 6 */
        {{100}, 1, {1, 2, 3, 4}, 4, 3.0},                /* 1 2 3 4 100 */
        {{5, 6, 7, 8, 9}, 5, {1}, 1, 6.5},               /* longer first: 1 5 6 7 8 9 */
        {{INT_MAX}, 1, {INT_MAX}, 1, 2147483647.0},      /* sum overflows int */
        {{INT_MAX - 1}, 1, {INT_MAX}, 1, 2147483646.5},  /* exact .5 near the top */
        {{INT_MIN}, 1, {INT_MAX}, 1, -0.5},
        {{INT_MIN}, 1, {INT_MIN, INT_MAX}, 2, -2147483648.0},  /* data equals sentinel */
        {{INT_MIN, INT_MIN}, 2, {INT_MAX, INT_MAX}, 2, -0.5},  /* both sentinels in data */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            double got = APPROACHES[a].fn(c->a, c->m, c->b, c->n);
            snprintf(label, sizeof label, "test %2d, %-18s -> %.1f", t + 1,
                     APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Random cross-check: sorted arrays of length 0..8 (never both empty)
     * with many duplicates; every approach must return exactly the same
     * median as the merge. No approach modifies its input. */
    {
        srand(40);
        int bad = 0;
        const int rounds = 1000;
        for (int r = 0; r < rounds; r++) {
            int a[8], b[8];
            int m = rand() % 9, n = rand() % 9;
            if (m + n == 0)
                n = 1;
            for (int k = 0; k < m; k++)
                a[k] = rand() % 41 - 20;
            for (int k = 0; k < n; k++)
                b[k] = rand() % 41 - 20;
            qsort(a, (size_t)m, sizeof a[0], cmp_int);
            qsort(b, (size_t)n, sizeof b[0], cmp_int);

            double expect = median_merge(a, m, b, n);
            for (int k = 1; k < NUM_APPROACHES; k++)
                if (APPROACHES[k].fn(a, m, b, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d array pairs, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
