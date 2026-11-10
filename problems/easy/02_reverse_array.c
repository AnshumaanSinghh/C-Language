/*
 * Q02. Reverse an Array
 * Difficulty : Easy
 * Topics     : Arrays, Two Pointers
 *
 * Problem
 *   Given an array a of n integers, reverse it in place: the first element
 *   becomes the last, the second becomes the second-to-last, and so on.
 *
 * Example
 *   a = [1, 2, 3, 4, 5]   ->   [5, 4, 3, 2, 1]
 *
 * Approaches
 *   1. Temp array                    copy backwards, copy back       Time O(n)   Space O(n)
 *   2. Two-pointer swap  [optimal]   swap the ends, move inward      Time O(n)   Space O(1)
 *   3. Recursion                     swap the ends, recurse inside   Time O(n)   Space O(n) stack
 *
 * Key insight
 *   Element i belongs at position n - 1 - i. Swapping a[i] with a[n - 1 - i]
 *   puts BOTH of them in their final place at once, so walking two pointers
 *   toward the middle does the whole job in n / 2 swaps with no extra memory.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 02_reverse_array.c -o reverse_array && ./reverse_array
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Temp array
 *   The most direct translation of the definition: tmp[i] = a[n - 1 - i]
 *   for every i, then copy tmp back over a. Easy to get right, but it needs
 *   a second array as big as the input.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static void reverse_temp(int *a, int n)
{
    if (n < 2)
        return; /* nothing to do, and it avoids malloc(0) */

    int *tmp = malloc((size_t)n * sizeof *tmp);
    if (tmp == NULL)
        return; /* out of memory: leave a unchanged */
    for (int i = 0; i < n; i++)
        tmp[i] = a[n - 1 - i];
    memcpy(a, tmp, (size_t)n * sizeof *a);
    free(tmp);
}

/* -------------------------------------------------------------------------
 * Approach 2: Two-pointer swap (optimal)
 *   lo starts at the first element, hi at the last. Swap them, then step
 *   both inward. Each swap fixes two positions for good, so after the
 *   pointers meet (odd n: the middle element stays put) every element is in
 *   place.
 *   Pitfall: stop when the pointers meet. Writing lo <= hi only adds a
 *   harmless self-swap of the middle element, but a loop that runs i over
 *   the WHOLE array (i < n instead of i < n / 2) swaps every pair twice
 *   and leaves the array exactly as it started.
 *   Time O(n), exactly n / 2 swaps   Space O(1)
 * ------------------------------------------------------------------------- */
static void reverse_two_pointer(int *a, int n)
{
    int lo = 0, hi = n - 1; /* n == 0 gives hi = -1, so the loop never runs */
    while (lo < hi) {
        int t = a[lo];
        a[lo] = a[hi];
        a[hi] = t;
        lo++;
        hi--;
    }
}

/* -------------------------------------------------------------------------
 * Approach 3: Recursion
 *   reverse(a[lo..hi]) = swap a[lo] and a[hi], then reverse(a[lo+1..hi-1]).
 *   Base case: a range of 0 or 1 elements is already reversed.
 *   It is the two-pointer loop written as recursion, but every pending call
 *   keeps a stack frame, so it uses O(n) stack. For a few million elements
 *   that can overflow the call stack, which is why the loop is preferred.
 *   Time O(n)   Space O(n) call stack (n / 2 frames)
 * ------------------------------------------------------------------------- */
static void reverse_range(int *a, int lo, int hi)
{
    if (lo >= hi)
        return;
    int t = a[lo];
    a[lo] = a[hi];
    a[hi] = t;
    reverse_range(a, lo + 1, hi - 1);
}

static void reverse_recursive(int *a, int n)
{
    reverse_range(a, 0, n - 1);
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

typedef void (*ReverseFn)(int *a, int n);

static const struct {
    const char *name;
    ReverseFn fn;
} APPROACHES[] = {
    {"temp array", reverse_temp},
    {"two-pointer swap", reverse_two_pointer},
    {"recursion", reverse_recursive},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAX_N 8

typedef struct {
    int in[MAX_N];
    int n;
    int expect[MAX_N];
} Case;

static int same_array(const int *x, const int *y, int n)
{
    for (int i = 0; i < n; i++)
        if (x[i] != y[i])
            return 0;
    return 1;
}

int main(void)
{
    printf("Q02. Reverse an Array\n");

    const Case cases[] = {
        {{0}, 0, {0}},                                     /* empty */
        {{42}, 1, {42}},                                   /* single element */
        {{1, 2}, 2, {2, 1}},                               /* smallest swap */
        {{1, 2, 3, 4, 5}, 5, {5, 4, 3, 2, 1}},             /* odd n: middle stays */
        {{1, 2, 3, 4, 5, 6}, 6, {6, 5, 4, 3, 2, 1}},       /* even n */
        {{7, 7, 7, 7}, 4, {7, 7, 7, 7}},                   /* all equal */
        {{1, 2, 2, 3, 1}, 5, {1, 3, 2, 2, 1}},             /* duplicates */
        {{-3, 0, -1, 8}, 4, {8, -1, 0, -3}},               /* negatives and zero */
        {{INT_MIN, 0, INT_MAX}, 3, {INT_MAX, 0, INT_MIN}}, /* extreme values */
        {{1, 2, 1}, 3, {1, 2, 1}},                         /* palindrome: unchanged */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[MAX_N];
            memcpy(work, c->in, sizeof work); /* each approach gets a fresh copy */
            APPROACHES[a].fn(work, c->n);
            snprintf(label, sizeof label, "test %2d, %-17s n=%d", t + 1, APPROACHES[a].name,
                     c->n);
            check(same_array(work, c->expect, c->n), label);
        }
    }

    /* Reversing twice must give back the original array. */
    {
        const int orig[] = {9, -4, 6, 6, 0, 13, -8};
        const int n = (int)(sizeof orig / sizeof orig[0]);
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[sizeof orig / sizeof orig[0]];
            memcpy(work, orig, sizeof work);
            APPROACHES[a].fn(work, n);
            APPROACHES[a].fn(work, n);
            snprintf(label, sizeof label, "reverse twice, %-17s -> original", APPROACHES[a].name);
            check(same_array(work, orig, n), label);
        }
    }

    /* Random cross-check: every approach must match the temp-array version,
     * and that version must satisfy out[i] == in[n - 1 - i]. */
    {
        srand(2024);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int in[32], expect[32], work[32];
            int n = rand() % 33; /* 0..32, includes empty and single */
            for (int k = 0; k < n; k++)
                in[k] = rand() % 21 - 10;

            memcpy(expect, in, (size_t)n * sizeof *in);
            reverse_temp(expect, n);
            for (int k = 0; k < n; k++)
                if (expect[k] != in[n - 1 - k])
                    bad++;

            for (int a = 0; a < NUM_APPROACHES; a++) {
                memcpy(work, in, (size_t)n * sizeof *in);
                APPROACHES[a].fn(work, n);
                if (!same_array(work, expect, n))
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
