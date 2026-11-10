/*
 * Q39. Trapping Rain Water
 * Difficulty : Hard
 * Topics     : Arrays, Two Pointers, Stack
 *
 * Problem
 *   Given n non-negative integers height[0..n-1] describing an elevation map
 *   in which every bar has width 1, compute how many units of rain water are
 *   trapped between the bars after it rains.
 *
 * Example
 *   height = [0,1,0,2,1,0,1,3,2,1,2,1]   ->   6
 *
 *       level 3   .......#....
 *       level 2   ...#~~~##~#.          # = bar, ~ = trapped water
 *       level 1   .#~##~######
 *
 * Approaches
 *   1. Per-bar scan              scan both sides of every bar for the max   Time O(n^2)  Space O(1)
 *   2. Prefix/suffix max         precompute left-max and right-max arrays   Time O(n)    Space O(n)
 *   3. Monotonic stack           fill a dip layer by layer when it closes   Time O(n)    Space O(n)
 *   4. Two pointers  [optimal]   settle the side with the smaller max       Time O(n)    Space O(1)
 *
 * Key insight
 *   The water above bar i is min(highest bar on its left, highest bar on its
 *   right) - height[i], counting bar i itself on both sides. Every approach
 *   is a different way to get those two maxima. Two pointers notice that you
 *   only need the SMALLER of the two exactly: if the best wall seen so far on
 *   the left is lower than one already seen on the right, the left wall is
 *   the bottleneck, whatever the unseen bars in the middle look like.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 39_trapping_rain_water.c -o trapping_rain_water && ./trapping_rain_water
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int max_int(int a, int b) { return a > b ? a : b; }
static int min_int(int a, int b) { return a < b ? a : b; }

/* -------------------------------------------------------------------------
 * Approach 1: Per-bar scan (brute force)
 *   Apply the formula literally: for every bar, walk left to find the
 *   highest bar in h[0..i] and walk right to find the highest in h[i..n-1].
 *   Including bar i in both scans means min(left, right) >= h[i], so the
 *   amount added is never negative.
 *   The total is a long long: n bars of height up to INT_MAX can hold far
 *   more than INT_MAX units of water.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long trap_scan(const int *h, int n)
{
    long long total = 0;
    for (int i = 0; i < n; i++) {
        int left = 0, right = 0;
        for (int l = 0; l <= i; l++)
            left = max_int(left, h[l]);
        for (int r = i; r < n; r++)
            right = max_int(right, h[r]);
        total += min_int(left, right) - h[i];
    }
    return total;
}

/* -------------------------------------------------------------------------
 * Approach 2: Prefix max + suffix max arrays
 *   Approach 1 recomputes the same maxima over and over. Compute them once:
 *     left_max[i]  = max(h[0..i])    = max(left_max[i-1], h[i])
 *     right_max[i] = max(h[i..n-1])  = max(right_max[i+1], h[i])
 *   Then the answer is the sum of min(left_max[i], right_max[i]) - h[i].
 *   Fewer than 3 bars can never hold water, which also lets us skip the
 *   allocation (malloc(0) may legally return NULL).
 *   Returns -1 if memory runs out.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static long long trap_prefix_suffix(const int *h, int n)
{
    if (n < 3)
        return 0;

    int *left_max = malloc((size_t)n * sizeof *left_max);
    int *right_max = malloc((size_t)n * sizeof *right_max);
    if (left_max == NULL || right_max == NULL) {
        free(left_max);
        free(right_max);
        return -1;
    }

    left_max[0] = h[0];
    for (int i = 1; i < n; i++)
        left_max[i] = max_int(left_max[i - 1], h[i]);
    right_max[n - 1] = h[n - 1];
    for (int i = n - 2; i >= 0; i--)
        right_max[i] = max_int(right_max[i + 1], h[i]);

    long long total = 0;
    for (int i = 0; i < n; i++)
        total += min_int(left_max[i], right_max[i]) - h[i];

    free(left_max);
    free(right_max);
    return total;
}

/* -------------------------------------------------------------------------
 * Approach 3: Monotonic stack
 *   Instead of counting water column by column, count it in horizontal
 *   layers. Keep a stack of indices whose heights never increase from bottom
 *   to top (a "staircase going down"). When a bar h[i] is taller than the
 *   top of the stack, the top is the floor of a dip that is now closed on
 *   the right by i and on the left by the next index on the stack:
 *
 *       left wall  = st[top-1]   floor = popped index   right wall = i
 *       width      = i - left - 1
 *       depth      = min(h[left], h[i]) - h[floor]
 *       water     += width * depth
 *
 *   Each pop fills exactly the layer between the floor and the lower wall;
 *   deeper layers were filled by earlier pops. If the stack becomes empty
 *   there is no left wall and the water would spill out, so stop.
 *   Pitfall: width * depth is an int * int that can exceed INT_MAX; cast
 *   to long long BEFORE multiplying.
 *   Every index is pushed and popped at most once.
 *   Time O(n)   Space O(n) for the stack
 * ------------------------------------------------------------------------- */
static long long trap_stack(const int *h, int n)
{
    if (n < 3)
        return 0;

    int *st = malloc((size_t)n * sizeof *st);
    if (st == NULL)
        return -1;

    int top = 0; /* number of indices on the stack */
    long long total = 0;
    for (int i = 0; i < n; i++) {
        while (top > 0 && h[i] > h[st[top - 1]]) {
            int floor_idx = st[--top];
            if (top == 0)
                break;
            int left = st[top - 1];
            int width = i - left - 1;
            int depth = min_int(h[left], h[i]) - h[floor_idx];
            total += (long long)width * depth;
        }
        st[top++] = i;
    }
    free(st);
    return total;
}

/* -------------------------------------------------------------------------
 * Approach 4: Two pointers (optimal)
 *   Keep lo and hi at the two ends, with
 *     left_max  = max(h[0..lo])     (everything the left pointer has seen)
 *     right_max = max(h[hi..n-1])   (everything the right pointer has seen)
 *   and settle one bar per step:
 *
 *   If left_max < right_max, bar lo is settled:
 *     its true left max is exactly left_max, and its true right max is
 *     max(h[lo..n-1]) >= max(h[hi..n-1]) = right_max > left_max.
 *     So min(true left, true right) = left_max no matter what the unseen
 *     bars between lo and hi are. Add left_max - h[lo] and move lo right.
 *   Otherwise (right_max <= left_max) the mirror argument settles bar hi.
 *
 *   That is why moving the smaller side is safe: the smaller running max is
 *   already the bottleneck, and the unseen bars can only make the OTHER side
 *   taller, never this side shorter. Each bar is settled exactly once (the
 *   loop runs while lo <= hi), and only four ints of extra memory are used.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long trap_two_pointers(const int *h, int n)
{
    long long total = 0;
    int lo = 0, hi = n - 1;
    int left_max = 0, right_max = 0;
    while (lo <= hi) {
        left_max = max_int(left_max, h[lo]);
        right_max = max_int(right_max, h[hi]);
        if (left_max < right_max) {
            total += left_max - h[lo];
            lo++;
        } else {
            total += right_max - h[hi];
            hi--;
        }
    }
    return total;
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

typedef long long (*TrapFn)(const int *h, int n);

static const struct {
    const char *name;
    TrapFn fn;
} APPROACHES[] = {
    {"per-bar scan", trap_scan},
    {"prefix/suffix max", trap_prefix_suffix},
    {"monotonic stack", trap_stack},
    {"two pointers", trap_two_pointers},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int h[12];
    int n;
    long long expect;
} Case;

int main(void)
{
    printf("Q39. Trapping Rain Water\n");

    /* Expected values worked out by hand with min(left max, right max) - h[i]. */
    const Case cases[] = {
        {{0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1}, 12, 6}, /* 1+1+2+1+1 */
        {{4, 2, 0, 3, 2, 5}, 6, 9},                    /* 2+4+1+2 */
        {{0}, 0, 0},                                   /* empty */
        {{5}, 1, 0},                                   /* single bar */
        {{4, 1}, 2, 0},                                /* two bars hold nothing */
        {{3, 3, 3, 3}, 4, 0},                          /* all equal */
        {{1, 2, 3, 4, 5}, 5, 0},                       /* rising: water runs off left */
        {{5, 4, 3, 2, 1}, 5, 0},                       /* falling: water runs off right */
        {{0, 0, 0}, 3, 0},                             /* flat ground */
        {{3, 0, 3}, 3, 3},
        {{2, 0, 2, 0, 2}, 5, 4},                       /* two pools, shared wall */
        {{5, 2, 1, 2, 1, 5}, 6, 14},                   /* 3+4+3+4 */
        {{2, 1, 0, 1, 3}, 5, 4},                       /* level 2, set by the left wall */
        {{5, 0, 0, 1, 0, 0, 3}, 7, 14},                /* level 3, set by the lower right wall */
        {{INT_MAX, 0, 0, INT_MAX}, 4, 2LL * INT_MAX},  /* total does not fit in an int */
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

    /* Random cross-check: small bars with many repeats, every approach must
     * agree with the per-bar scan. No approach modifies its input. */
    {
        srand(39);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int h[16];
            int n = rand() % 17;
            for (int k = 0; k < n; k++)
                h[k] = rand() % 10;

            long long expect = trap_scan(h, n);
            for (int a = 1; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(h, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
