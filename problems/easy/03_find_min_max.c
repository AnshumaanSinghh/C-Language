/*
 * Q03. Find Minimum and Maximum
 * Difficulty : Easy
 * Topics     : Arrays, Divide and Conquer, Sorting
 *
 * Problem
 *   Given a non-empty array a of n integers, return both its smallest and
 *   its largest element. Use as few element comparisons as you can.
 *
 * Example
 *   a = [3, -2, 9, 4, 9]   ->   min = -2, max = 9
 *
 * Approaches
 *   1. Sort a copy             sort, take both ends          Time O(n log n)  Space O(n)
 *   2. Single pass             compare x to min and to max   Time O(n)        Space O(1)
 *   3. Pairwise  [optimal]     compare in pairs first        Time O(n)        Space O(1)
 *   4. Divide and conquer      min/max of halves, combine    Time O(n)        Space O(log n)
 *
 *   Element comparisons made (approaches 2-4 do the same number on every
 *   input of size n; this program counts and prints them):
 *     single pass     2(n - 1)          about 2n
 *     pairwise        ceil(3n/2) - 2    about 1.5n, the proven lower bound
 *     divide/conquer  3n/2 - 2 when n is a power of two, a bit more otherwise
 *
 * Key insight
 *   Compare the two elements of a pair with EACH OTHER first. Only the
 *   smaller one can be a new minimum and only the larger one can be a new
 *   maximum, so each pair costs 3 comparisons instead of 4.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 03_find_min_max.c -o find_min_max && ./find_min_max
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Instrumentation: every element-vs-element comparison in this file goes
 * through less() (or the qsort comparator), which bumps this counter. The
 * test harness resets it before each call and reads it afterwards. */
static long comparisons = 0;

static int less(int x, int y)
{
    comparisons++;
    return x < y;
}

/* -------------------------------------------------------------------------
 * Approach 1: Sort a copy
 *   After sorting, the minimum is the first element and the maximum the
 *   last. Correct, but it does far more work than needed: it orders all n
 *   elements when we only want two of them. We sort a copy because the
 *   caller's array is const and must not change.
 *   Pitfall: a comparator written as "return x - y;" overflows for values
 *   like INT_MIN and INT_MAX. (x > y) - (x < y) is always safe.
 *   Time O(n log n), roughly n log2 n comparisons   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int_counted(const void *pa, const void *pb)
{
    int x = *(const int *)pa;
    int y = *(const int *)pb;
    comparisons++; /* one call of the comparator = one comparison */
    return (x > y) - (x < y);
}

static void minmax_sort(const int *a, int n, int *out_min, int *out_max)
{
    int *copy = malloc((size_t)n * sizeof *copy);
    if (copy == NULL)
        return; /* out of memory: outputs left untouched */
    memcpy(copy, a, (size_t)n * sizeof *copy);
    qsort(copy, (size_t)n, sizeof *copy, cmp_int_counted);
    *out_min = copy[0];
    *out_max = copy[n - 1];
    free(copy);
}

/* -------------------------------------------------------------------------
 * Approach 2: Single pass
 *   Start with min = max = a[0]. For every other element, compare it with
 *   the current min and with the current max. One pass, no extra memory.
 *   That is 2 comparisons for each of the n - 1 remaining elements.
 *   (Writing the second test as "else if" saves comparisons on some inputs,
 *   but an increasing array still needs both tests every time.)
 *   Time O(n), exactly 2(n - 1) comparisons   Space O(1)
 * ------------------------------------------------------------------------- */
static void minmax_single_pass(const int *a, int n, int *out_min, int *out_max)
{
    int lo = a[0], hi = a[0];
    for (int i = 1; i < n; i++) {
        if (less(a[i], lo))
            lo = a[i];
        if (less(hi, a[i]))
            hi = a[i];
    }
    *out_min = lo;
    *out_max = hi;
}

/* -------------------------------------------------------------------------
 * Approach 3: Pairwise comparison (optimal)
 *   Take the elements two at a time. First compare the pair with each
 *   other (1 comparison). The smaller of the two is the only candidate for
 *   a new minimum, the larger the only candidate for a new maximum, so test
 *   each against just one of min/max (2 more). 3 comparisons per 2 elements.
 *   Start-up: if n is odd, a[0] alone initialises min and max for free; if
 *   n is even, the first pair initialises them with 1 comparison.
 *   Total: ceil(3n/2) - 2 comparisons, which is a proven lower bound: no
 *   comparison-based algorithm can do better in the worst case.
 *   Time O(n), ceil(3n/2) - 2 comparisons   Space O(1)
 * ------------------------------------------------------------------------- */
static void minmax_pairwise(const int *a, int n, int *out_min, int *out_max)
{
    int lo, hi, i;
    if (n % 2 == 1) {
        lo = hi = a[0];
        i = 1;
    } else if (less(a[0], a[1])) {
        lo = a[0];
        hi = a[1];
        i = 2;
    } else {
        lo = a[1];
        hi = a[0];
        i = 2;
    }

    /* From here the number of remaining elements is even. */
    for (; i + 1 < n; i += 2) {
        int small = a[i], big = a[i + 1];
        if (less(big, small)) {
            small = a[i + 1];
            big = a[i];
        }
        if (less(small, lo))
            lo = small;
        if (less(hi, big))
            hi = big;
    }
    *out_min = lo;
    *out_max = hi;
}

/* -------------------------------------------------------------------------
 * Approach 4: Divide and conquer (tournament)
 *   Split the range in half, find min and max of each half recursively,
 *   then combine: min of the two mins, max of the two maxes (2 comparisons).
 *   Base cases: 1 element costs 0 comparisons, 2 elements cost 1.
 *   Comparisons satisfy T(n) = T(ceil(n/2)) + T(floor(n/2)) + 2, which is
 *   3n/2 - 2 when n is a power of two, the same as pairwise. For other n
 *   it can be a little worse: n = 6 splits into 3 + 3, each half costs 3,
 *   so T(6) = 3 + 3 + 2 = 8, while pairwise needs only 7. Splitting so the
 *   left half has even size would fix that.
 *   Time O(n)   Space O(log n) recursion depth
 * ------------------------------------------------------------------------- */
static void minmax_range(const int *a, int lo, int hi, int *out_min, int *out_max)
{
    if (lo == hi) {
        *out_min = *out_max = a[lo];
        return;
    }
    if (hi == lo + 1) {
        if (less(a[lo], a[hi])) {
            *out_min = a[lo];
            *out_max = a[hi];
        } else {
            *out_min = a[hi];
            *out_max = a[lo];
        }
        return;
    }
    int mid = lo + (hi - lo) / 2; /* not (lo + hi) / 2, which can overflow */
    int min1 = 0, max1 = 0, min2 = 0, max2 = 0;
    minmax_range(a, lo, mid, &min1, &max1);
    minmax_range(a, mid + 1, hi, &min2, &max2);
    *out_min = less(min2, min1) ? min2 : min1;
    *out_max = less(max1, max2) ? max2 : max1;
}

static void minmax_divide_conquer(const int *a, int n, int *out_min, int *out_max)
{
    minmax_range(a, 0, n - 1, out_min, out_max);
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

typedef void (*MinMaxFn)(const int *a, int n, int *out_min, int *out_max);

static const struct {
    const char *name;
    MinMaxFn fn;
} APPROACHES[] = {
    {"sort a copy", minmax_sort},
    {"single pass", minmax_single_pass},
    {"pairwise", minmax_pairwise},
    {"divide and conquer", minmax_divide_conquer},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int a[8];
    int n;
    int expect_min, expect_max;
} Case;

int main(void)
{
    printf("Q03. Find Minimum and Maximum\n");

    const Case cases[] = {
        {{5}, 1, 5, 5},                                /* single element */
        {{3, 9}, 2, 3, 9},                             /* two, ascending */
        {{9, 3}, 2, 3, 9},                             /* two, descending */
        {{4, 4, 4, 4}, 4, 4, 4},                       /* all equal */
        {{-5, -1, -9, -3}, 4, -9, -1},                 /* all negative */
        {{2, 8, 8, -1, 2, -1}, 6, -1, 8},              /* duplicates of min and max */
        {{1, 2, 3, 4, 5, 6, 7}, 7, 1, 7},              /* sorted, odd n */
        {{7, 6, 5, 4, 3, 2, 1}, 7, 1, 7},              /* reverse sorted */
        {{INT_MAX, INT_MIN, 0}, 3, INT_MIN, INT_MAX},  /* x - y comparator would overflow */
        {{INT_MIN, INT_MIN}, 2, INT_MIN, INT_MIN},     /* extreme duplicates */
        {{0, 10, -10, 20, -20, 30, -30, 5}, 8, -30, 30}, /* alternating, even n */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int mn = 0, mx = 0;
            APPROACHES[a].fn(c->a, c->n, &mn, &mx);
            snprintf(label, sizeof label, "test %2d, %-19s -> min %d, max %d", t + 1,
                     APPROACHES[a].name, mn, mx);
            check(mn == c->expect_min && mx == c->expect_max, label);
        }
    }

    /* Comparison counts. Approaches 2-4 always make the same number of
     * comparisons for a given n, whatever the values, so the expected
     * numbers below follow from the formulas in the header:
     *   n            1    2    3    6    1024
     *   single pass  0    2    4   10    2046   = 2(n - 1)
     *   pairwise     0    1    3    7    1534   = ceil(3n/2) - 2
     *   divide/conq  0    1    3    8    1534   (T(6) = 3 + 3 + 2)
     * Sorting depends on the data and on the qsort implementation, so its
     * count is printed but only the answer is checked. */
    {
        enum { NUM_SIZES = 5, MAX_SIZE = 1024 };
        const int sizes[NUM_SIZES] = {1, 2, 3, 6, MAX_SIZE};
        const long expect[NUM_APPROACHES][NUM_SIZES] = {
            {-1, -1, -1, -1, -1}, /* sort: not fixed */
            {0, 2, 4, 10, 2046},
            {0, 1, 3, 7, 1534},
            {0, 1, 3, 8, 1534},
        };
        static int data[MAX_SIZE];
        srand(7);
        for (int s = 0; s < NUM_SIZES; s++) {
            int n = sizes[s];
            int true_min = INT_MAX, true_max = INT_MIN;
            for (int k = 0; k < n; k++) {
                data[k] = rand() % 100001 - 50000;
                if (data[k] < true_min)
                    true_min = data[k];
                if (data[k] > true_max)
                    true_max = data[k];
            }
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int mn = 0, mx = 0;
                comparisons = 0;
                APPROACHES[a].fn(data, n, &mn, &mx);
                int ok = mn == true_min && mx == true_max;
                if (expect[a][s] >= 0) {
                    ok = ok && comparisons == expect[a][s];
                    snprintf(label, sizeof label, "n=%-4d %-19s -> %5ld comparisons (expected %ld)",
                             n, APPROACHES[a].name, comparisons, expect[a][s]);
                } else {
                    snprintf(label, sizeof label, "n=%-4d %-19s -> %5ld comparisons (data-dependent)",
                             n, APPROACHES[a].name, comparisons);
                }
                check(ok, label);
            }
        }
    }

    /* Random cross-check against the sorting approach. */
    {
        srand(12345);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int a[20];
            int n = 1 + rand() % 20;
            for (int k = 0; k < n; k++)
                a[k] = rand() % 101 - 50;

            int emin = 0, emax = 0;
            minmax_sort(a, n, &emin, &emax);
            for (int k = 0; k < n; k++)
                if (a[k] < emin || a[k] > emax)
                    bad++; /* the reference itself must bound every element */
            for (int ap = 0; ap < NUM_APPROACHES; ap++) {
                int mn = 0, mx = 0;
                APPROACHES[ap].fn(a, n, &mn, &mx);
                if (mn != emin || mx != emax)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
