/*
 * Q41. Largest Rectangle in Histogram
 * Difficulty : Hard
 * Topics     : Arrays, Stack, Monotonic Stack, Divide and Conquer
 *
 * Problem
 *   Given n non-negative integers heights[0..n-1], the heights of the bars
 *   of a histogram in which every bar has width 1, return the area of the
 *   largest rectangle that fits entirely inside the histogram. The answer is
 *   a long long, because height * width can exceed INT_MAX.
 *
 * Example
 *   heights = [2, 1, 5, 6, 2, 3]   ->   10   (bars 5 and 6, height 5, width 2)
 *
 *               #
 *             @ @
 *             @ @
 *             @ @   #
 *         #   @ @ # #
 *         # # @ @ # #        @ = the area-10 rectangle
 *
 * Approaches
 *   1. Every pair (i, j)          running min over i..j             Time O(n^2)                  Space O(1)
 *   2. Divide and conquer         split at the lowest bar           Time O(n log n) avg, O(n^2)  Space O(n)
 *   3. Nearest smaller arrays     two stack passes                  Time O(n)                    Space O(n)
 *   4. One-pass stack  [optimal]  pop when the right edge is found  Time O(n)                    Space O(n)
 *
 * Key insight
 *   The best rectangle is as tall as some bar k and stretches from k until
 *   the first bar lower than h[k] on each side. So for every bar you only
 *   need its nearest smaller neighbour on the left and on the right, and a
 *   monotonic (increasing) stack hands you both in a single pass: when a bar
 *   is popped, the bar popping it is its right limit and the bar below it on
 *   the stack is its left limit.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 41_largest_rectangle_histogram.c -o largest_rectangle_histogram && ./largest_rectangle_histogram
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Approach 1: Every pair (i, j) with a running minimum
 *   A rectangle spanning bars i..j can be at most as tall as the lowest bar
 *   in that range. Fix i, extend j to the right and keep the minimum as you
 *   go, so each range costs O(1) instead of O(j - i).
 *   Pitfall: min_h * width is int * int; cast to long long first.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long rect_pairs(const int *h, int n)
{
    long long best = 0;
    for (int i = 0; i < n; i++) {
        int min_h = INT_MAX;
        for (int j = i; j < n; j++) {
            if (h[j] < min_h)
                min_h = h[j];
            long long area = (long long)min_h * (j - i + 1);
            if (area > best)
                best = area;
        }
    }
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 2: Divide and conquer on the minimum bar
 *   In the range lo..hi let m be the lowest bar. Any rectangle in the range
 *   either
 *     - contains bar m: then its height is at most h[m], and the best such
 *       rectangle is the full range at height h[m]; or
 *     - does not contain m: then it lies entirely in lo..m-1 or m+1..hi.
 *   Solve both sides recursively and take the best of the three.
 *   Finding m is a linear scan, so the cost behaves like quicksort: about
 *   O(n log n) when the minimum splits the range evenly, O(n^2) (and n
 *   levels of recursion) for sorted input, where the minimum is always at
 *   an end. A segment tree for range-minimum queries would fix the worst
 *   case at O(n log n); the stack approaches below do better anyway.
 *   Time O(n log n) average, O(n^2) worst   Space O(n) recursion worst case
 * ------------------------------------------------------------------------- */
static long long rect_dc_range(const int *h, int lo, int hi)
{
    if (lo > hi)
        return 0;

    int m = lo;
    for (int k = lo + 1; k <= hi; k++)
        if (h[k] < h[m])
            m = k;

    long long best = (long long)h[m] * (hi - lo + 1);
    long long left = rect_dc_range(h, lo, m - 1);
    long long right = rect_dc_range(h, m + 1, hi);
    if (left > best)
        best = left;
    if (right > best)
        best = right;
    return best;
}

static long long rect_divide_conquer(const int *h, int n)
{
    return rect_dc_range(h, 0, n - 1);
}

/* -------------------------------------------------------------------------
 * Approach 3: Nearest smaller to the left / right, via stacks
 *   For each bar k, the widest rectangle of height h[k] runs from
 *       left[k]  = index of the nearest bar to the LEFT  lower than h[k] (or -1)
 *       right[k] = index of the nearest bar to the RIGHT lower than h[k] (or n)
 *   exclusive, so its area is h[k] * (right[k] - left[k] - 1). The best
 *   rectangle overall has the height of its lowest bar k, so trying every k
 *   is enough.
 *   Computing left[]: scan left to right with a stack of indices whose
 *   heights strictly increase. Before pushing k, pop every bar >= h[k]: those
 *   can never be the "nearest lower bar" for k or anything after k, because
 *   k is closer and at least as low. Whatever is left on top is left[k].
 *   right[] is the same scan from right to left.
 *   Each index is pushed and popped once per pass.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static long long rect_nearest_smaller(const int *h, int n)
{
    if (n == 0)
        return 0;

    int *left = malloc((size_t)n * sizeof *left);
    int *right = malloc((size_t)n * sizeof *right);
    int *st = malloc((size_t)n * sizeof *st);
    if (left == NULL || right == NULL || st == NULL) {
        free(left);
        free(right);
        free(st);
        return -1;
    }

    int top = 0;
    for (int k = 0; k < n; k++) {
        while (top > 0 && h[st[top - 1]] >= h[k])
            top--;
        left[k] = top > 0 ? st[top - 1] : -1;
        st[top++] = k;
    }

    top = 0;
    for (int k = n - 1; k >= 0; k--) {
        while (top > 0 && h[st[top - 1]] >= h[k])
            top--;
        right[k] = top > 0 ? st[top - 1] : n;
        st[top++] = k;
    }

    long long best = 0;
    for (int k = 0; k < n; k++) {
        long long area = (long long)h[k] * (right[k] - left[k] - 1);
        if (area > best)
            best = area;
    }

    free(left);
    free(right);
    free(st);
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 4: Single-pass monotonic stack (optimal)
 *   Merge the two passes of approach 3 into one. Keep a stack of indices
 *   whose heights strictly increase. When bar i arrives and the top bar t
 *   has h[t] >= h[i], bar t cannot extend past i, so its rectangle is
 *   finished: pop it and compute
 *       right limit = i                (first bar to the right that is <= h[t])
 *       left limit  = new top, or -1   (nearest bar to the left lower than h[t])
 *       area        = h[t] * (i - left - 1)
 *   A virtual bar of height 0 at position n pops everything that is left.
 *
 *   Equal heights: with ">=" a bar is popped by an EQUAL bar to its right,
 *   so its width is cut short. That is harmless: the last bar of a run of
 *   equal heights is popped by a truly lower bar, and its left limit is the
 *   same as the first bar's, so the full-width rectangle is still counted.
 *   Each index is pushed once and popped once.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static long long rect_stack(const int *h, int n)
{
    if (n == 0)
        return 0;

    int *st = malloc((size_t)n * sizeof *st);
    if (st == NULL)
        return -1;

    int top = 0;
    long long best = 0;
    for (int i = 0; i <= n; i++) {
        int cur = i < n ? h[i] : 0;
        while (top > 0 && h[st[top - 1]] >= cur) {
            int height = h[st[--top]];
            int left = top > 0 ? st[top - 1] : -1;
            long long area = (long long)height * (i - left - 1);
            if (area > best)
                best = area;
        }
        if (i < n)
            st[top++] = i;
    }
    free(st);
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

typedef long long (*RectFn)(const int *h, int n);

static const struct {
    const char *name;
    RectFn fn;
} APPROACHES[] = {
    {"every pair", rect_pairs},
    {"divide and conquer", rect_divide_conquer},
    {"nearest smaller", rect_nearest_smaller},
    {"one-pass stack", rect_stack},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int h[8];
    int n;
    long long expect;
} Case;

int main(void)
{
    printf("Q41. Largest Rectangle in Histogram\n");

    /* Expected areas worked out by hand: for each candidate height, the
     * widest run of bars at least that tall. */
    const Case cases[] = {
        {{2, 1, 5, 6, 2, 3}, 6, 10},                /* 5 * 2 */
        {{2, 4}, 2, 4},                             /* 4 * 1 = 2 * 2 */
        {{0}, 0, 0},                                /* empty */
        {{7}, 1, 7},                                /* single bar */
        {{0}, 1, 0},                                /* single bar of height 0 */
        {{0, 0, 0}, 3, 0},
        {{3, 3, 3, 3}, 4, 12},                      /* all equal: full width */
        {{1, 2, 3, 4, 5}, 5, 9},                    /* 3 * 3 */
        {{5, 4, 3, 2, 1}, 5, 9},                    /* 3 * 3 */
        {{6, 2, 5, 4, 5, 1, 6}, 7, 12},             /* 4 * 3 over 5 4 5 */
        {{2, 0, 2}, 3, 2},                          /* a zero bar splits the histogram */
        {{2, 1, 2}, 3, 3},                          /* 1 * 3 beats 2 * 1 */
        {{INT_MAX, INT_MAX}, 2, 2LL * INT_MAX},     /* area does not fit in an int */
        {{INT_MAX, INT_MAX, INT_MAX}, 3, 3LL * INT_MAX},
        {{1, INT_MAX, 1}, 3, INT_MAX},              /* tall thin bar wins */
        {{1000000000, 2000000000, 1500000000}, 3, 3000000000LL}, /* 1e9*3 = 1.5e9*2 */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            long long got = APPROACHES[a].fn(c->h, c->n);
            snprintf(label, sizeof label, "test %2d, %-18s -> %lld", t + 1,
                     APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Larger input: 2000 equal bars of height 1,500,000, area 3e9. Equal bars
     * are also the worst case for divide and conquer (2000 levels deep). */
    {
        enum { BIG_N = 2000 };
        static int big[BIG_N];
        for (int k = 0; k < BIG_N; k++)
            big[k] = 1500000;
        for (int a = 0; a < NUM_APPROACHES; a++) {
            long long got = APPROACHES[a].fn(big, BIG_N);
            snprintf(label, sizeof label, "2000 equal bars, %-18s -> %lld",
                     APPROACHES[a].name, got);
            check(got == 3000000000LL, label);
        }
    }

    /* Random cross-check: short histograms with heights 0..9 (lots of equal
     * heights and zeros); every approach must agree with every-pair. */
    {
        srand(41);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int h[16];
            int n = rand() % 17;
            for (int k = 0; k < n; k++)
                h[k] = rand() % 10;

            long long expect = rect_pairs(h, n);
            for (int a = 1; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(h, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d histograms, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
