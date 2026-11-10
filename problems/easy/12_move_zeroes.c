/*
 * Q12. Move Zeroes
 * Difficulty : Easy
 * Topics     : Arrays, Two Pointers
 *
 * Problem
 *   Given an integer array nums, move every 0 to the end of the array in
 *   place, while keeping the relative order of the non-zero elements.
 *
 * Example
 *   nums = [0, 1, 0, 3, 12]   ->   [1, 3, 12, 0, 0]
 *
 * Approaches
 *   1. Extra array               copy non-zeros, then zeros, back  Time O(n)    Space O(n)
 *   2. Bubble zeros right        swap each zero past non-zeros     Time O(n^2)  Space O(1)
 *   3. Compact + fill            pack non-zeros left, pad with 0   Time O(n)    Space O(1)
 *   4. Snowball swap [optimal]   swap each non-zero into place     Time O(n)    Space O(1)
 *
 * Key insight
 *   Keep a write index w = "where the next non-zero belongs". Everything in
 *   nums[w..i-1] is a zero, so when nums[i] is non-zero, one swap puts it in
 *   its final position and pushes the block of zeros one step right.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 12_move_zeroes.c -o move_zeroes && ./move_zeroes
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Extra array
 *   Build the answer in a scratch buffer: first every non-zero in the order
 *   met, then as many zeros as are left. Copy the buffer back. It is the
 *   most obviously correct version, so it is the reference for the tests,
 *   but it breaks the "in place" requirement by using O(n) extra memory.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static void move_zeroes_extra(int *nums, int n)
{
    if (n <= 0)
        return;
    int *tmp = malloc((size_t)n * sizeof *tmp);
    if (tmp == NULL)
        return; /* out of memory: array left unchanged (tests would flag it) */

    int w = 0;
    for (int i = 0; i < n; i++)
        if (nums[i] != 0)
            tmp[w++] = nums[i];
    while (w < n)
        tmp[w++] = 0;

    memcpy(nums, tmp, (size_t)n * sizeof *nums);
    free(tmp);
}

/* -------------------------------------------------------------------------
 * Approach 2: Bubble each zero to the right
 *   Walk from right to left. Invariant: the suffix to the right of i is
 *   already finished (non-zeros in order, then zeros). When nums[i] is a
 *   zero, swap it rightwards past the non-zeros until it touches the block
 *   of zeros or the end. Only adjacent zero/non-zero swaps happen, so no two
 *   non-zeros ever pass each other: their order is kept.
 *   Worst case [0, 0, ..., 0, 1, 1, ..., 1]: n/2 zeros each travel n/2 steps.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static void move_zeroes_bubble(int *nums, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        if (nums[i] != 0)
            continue;
        for (int j = i; j + 1 < n && nums[j + 1] != 0; j++) {
            nums[j] = nums[j + 1];
            nums[j + 1] = 0;
        }
    }
}

/* -------------------------------------------------------------------------
 * Approach 3: Compact, then fill
 *   Pass 1 copies each non-zero to nums[w++]. Because w <= i always, we only
 *   overwrite slots we have already read, so nothing is lost. Pass 2 writes
 *   zeros into nums[w..n-1]. Nothing is swapped; zeros are simply recreated.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void move_zeroes_two_pass(int *nums, int n)
{
    int w = 0;
    for (int i = 0; i < n; i++)
        if (nums[i] != 0)
            nums[w++] = nums[i];
    while (w < n)
        nums[w++] = 0;
}

/* -------------------------------------------------------------------------
 * Approach 4: One-pass swap, the "snowball" (optimal)
 *   Invariant: nums[0..w-1] are the non-zeros seen so far, in order, and
 *   nums[w..i-1] are all zeros (the snowball). When nums[i] is non-zero and
 *   i != w, nums[w] is a zero, so swapping moves the non-zero to its final
 *   slot and the zero to the back of the snowball.
 *   The i != w test skips pointless self-swaps, so writes happen only for a
 *   non-zero that has a zero somewhere before it (two writes each). An
 *   array that is already finished gets zero writes, and the number of
 *   zeros never adds work. Approach 3 always writes all n slots: for
 *   [0, 0, ..., 0, 1] that is n writes against the snowball's 2. (When the
 *   zeros are few and early, approach 3 can write less; both are O(n).)
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void move_zeroes_snowball(int *nums, int n)
{
    int w = 0;
    for (int i = 0; i < n; i++) {
        if (nums[i] == 0)
            continue;
        if (i != w) {
            nums[w] = nums[i];
            nums[i] = 0; /* the swap partner nums[w] was a zero */
        }
        w++;
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

typedef void (*MoveZeroesFn)(int *nums, int n);

static const struct {
    const char *name;
    MoveZeroesFn fn;
} APPROACHES[] = {
    {"extra array", move_zeroes_extra},
    {"bubble zeros right", move_zeroes_bubble},
    {"compact + fill", move_zeroes_two_pass},
    {"snowball swap", move_zeroes_snowball},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 16

typedef struct {
    int in[MAXN];
    int n;
    int expect[MAXN];
} Case;

static int same_array(const int *a, const int *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int main(void)
{
    printf("Q12. Move Zeroes\n");

    const Case cases[] = {
        {{0, 1, 0, 3, 12}, 5, {1, 3, 12, 0, 0}},
        {{0}, 0, {0}},                                  /* empty array */
        {{0}, 1, {0}},
        {{7}, 1, {7}},
        {{1, 2, 3}, 3, {1, 2, 3}},                      /* no zeros */
        {{0, 0, 0}, 3, {0, 0, 0}},                      /* all zeros */
        {{0, 0, 1}, 3, {1, 0, 0}},
        {{1, 0, 0}, 3, {1, 0, 0}},                      /* already done */
        {{-1, 0, -2, 0, 0, 5}, 6, {-1, -2, 5, 0, 0, 0}}, /* negatives */
        {{4, 0, 4, 0, 4}, 5, {4, 4, 4, 0, 0}},          /* duplicates */
        {{INT_MIN, 0, INT_MAX}, 3, {INT_MIN, INT_MAX, 0}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[MAXN];
            memcpy(work, c->in, sizeof work); /* every approach gets a fresh copy */
            APPROACHES[a].fn(work, c->n);
            snprintf(label, sizeof label, "test %d, %s", t + 1, APPROACHES[a].name);
            check(same_array(work, c->expect, c->n), label);
        }
    }

    /* Random cross-check: values in -3..3 so there are plenty of zeros and
     * repeated values. Every approach must match the extra-array version. */
    {
        srand(1212);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN], expect[MAXN];
            int n = rand() % (MAXN + 1);
            for (int k = 0; k < n; k++)
                in[k] = rand() % 7 - 3;

            memcpy(expect, in, (size_t)n * sizeof *in);
            move_zeroes_extra(expect, n);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int work[MAXN];
                memcpy(work, in, (size_t)n * sizeof *in);
                APPROACHES[a].fn(work, n);
                if (!same_array(work, expect, n))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
