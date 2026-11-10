/*
 * Q23. Sort Colors
 * Difficulty : Medium
 * Topics     : Arrays, Two Pointers, Sorting, Counting
 *
 * Problem
 *   An array of n integers contains only 0s, 1s and 2s (red, white, blue).
 *   Sort it in place so all 0s come first, then all 1s, then all 2s.
 *   This is Dijkstra's Dutch National Flag problem.
 *
 * Example
 *   nums = [2, 0, 2, 1, 1, 0]   ->   [0, 0, 1, 1, 2, 2]
 *
 * Approaches
 *   1. qsort                       general comparison sort         Time O(n log n)  Space O(log n)
 *   2. Counting, two passes        count 0/1/2, then overwrite     Time O(n)        Space O(1)
 *   3. Dutch flag    [optimal]     low/mid/high, one pass          Time O(n)        Space O(1)
 *
 * Key insight
 *   With only three values you never need comparisons between elements:
 *   keep three regions (0s, 1s, 2s) and grow them from the ends while one
 *   pointer scans the unknown middle. Each element is looked at once.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 23_sort_colors.c -o sort_colors && ./sort_colors
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

/* -------------------------------------------------------------------------
 * Approach 1: qsort
 *   Ignore the special structure and use the library sort. Correct for any
 *   integers, but pays for comparisons it does not need.
 *   The comparator uses (x > y) - (x < y) rather than x - y, which can
 *   overflow for general ints (a habit worth keeping even here).
 *   Time O(n log n)   Space O(log n) typical qsort stack
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void sort_colors_qsort(int *nums, int n)
{
    if (n > 1)
        qsort(nums, (size_t)n, sizeof *nums, cmp_int);
}

/* -------------------------------------------------------------------------
 * Approach 2: Counting sort, two passes
 *   Pass 1 counts how many 0s, 1s and 2s there are. Pass 2 overwrites the
 *   array with that many 0s, then 1s, then 2s. Simple and linear, but it
 *   reads the array twice and rewrites every slot; it also only works
 *   because the values are plain numbers (it would not keep attached data,
 *   e.g. if each colour were a struct with a payload).
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void sort_colors_counting(int *nums, int n)
{
    int count[3] = {0, 0, 0};
    for (int i = 0; i < n; i++)
        count[nums[i]]++;
    int pos = 0;
    for (int colour = 0; colour < 3; colour++)
        for (int c = 0; c < count[colour]; c++)
            nums[pos++] = colour;
}

/* -------------------------------------------------------------------------
 * Approach 3: Dutch National Flag, one pass (optimal)
 *   Three indices split the array into four regions. At every step:
 *
 *       [0, low)      all 0s
 *       [low, mid)    all 1s
 *       [mid, high]   not looked at yet
 *       (high, n-1]   all 2s
 *
 *   Initially low = mid = 0 and high = n - 1, so everything is "unknown"
 *   and the invariant holds trivially. Look at nums[mid]:
 *     0 -> swap it with nums[low] (the first 1, or mid itself if the 1s
 *          region is empty), then low++ and mid++. Both regions shift right.
 *     1 -> already in the right place: mid++.
 *     2 -> swap it with nums[high] and high--. Do NOT advance mid: the
 *          value that came back from high has not been examined yet.
 *   Each step shrinks the unknown region by one, so the loop ends after at
 *   most n steps with mid > high, leaving exactly 0s, 1s, 2s.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void sort_colors_dnf(int *nums, int n)
{
    int low = 0, mid = 0, high = n - 1;
    while (mid <= high) {
        if (nums[mid] == 0) {
            swap_int(&nums[low], &nums[mid]);
            low++;
            mid++;
        } else if (nums[mid] == 1) {
            mid++;
        } else {
            swap_int(&nums[mid], &nums[high]);
            high--;
        }
    }
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

typedef void (*SortFn)(int *nums, int n);

static const struct {
    const char *name;
    SortFn fn;
} APPROACHES[] = {
    {"qsort", sort_colors_qsort},
    {"counting", sort_colors_counting},
    {"dutch flag", sort_colors_dnf},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 24

typedef struct {
    int in[MAXN];
    int n;
    int expect[MAXN];
} Case;

static int same(const int *a, const int *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int main(void)
{
    printf("Q23. Sort Colors\n");

    /* Expected outputs written by hand from the colour counts. */
    const Case cases[] = {
        {{2, 0, 2, 1, 1, 0}, 6, {0, 0, 1, 1, 2, 2}},
        {{2, 0, 1}, 3, {0, 1, 2}},
        {{0}, 0, {0}},                                 /* empty */
        {{1}, 1, {1}},                                 /* single */
        {{2}, 1, {2}},
        {{2, 2, 2}, 3, {2, 2, 2}},                     /* all equal */
        {{0, 0, 0}, 3, {0, 0, 0}},
        {{2, 1, 0}, 3, {0, 1, 2}},                     /* reversed */
        {{1, 0}, 2, {0, 1}},
        {{2, 0, 0, 1, 2, 1, 0}, 7, {0, 0, 0, 1, 1, 2, 2}},
        {{2, 2, 0, 0}, 4, {0, 0, 2, 2}},               /* no 1s */
        {{1, 1, 1, 0}, 4, {0, 1, 1, 1}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[MAXN];
            memcpy(work, c->in, sizeof work); /* each approach sorts its own copy */
            APPROACHES[a].fn(work, c->n);
            snprintf(label, sizeof label, "test %2d, %-10s n=%d", t + 1, APPROACHES[a].name, c->n);
            check(same(work, c->expect, c->n), label);
        }
    }

    /* Random cross-check against qsort. */
    {
        srand(2323);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN] = {0}, expect[MAXN];
            int n = rand() % (MAXN + 1);
            for (int i = 0; i < n; i++)
                in[i] = rand() % 3;
            memcpy(expect, in, sizeof expect);
            sort_colors_qsort(expect, n);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int work[MAXN];
                memcpy(work, in, sizeof work);
                APPROACHES[a].fn(work, n);
                if (!same(work, expect, n))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
