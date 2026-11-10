/*
 * Q22. Rotate Array
 * Difficulty : Medium
 * Topics     : Arrays, Two Pointers, Math
 *
 * Problem
 *   Rotate an array of n integers to the right by k steps, in place.
 *   k >= 0 and may be 0 or larger than n. Rotating right by one moves the
 *   last element to the front and shifts everything else one place right.
 *
 * Example
 *   nums = [1, 2, 3, 4, 5, 6, 7], k = 3   ->   [5, 6, 7, 1, 2, 3, 4]
 *
 * Approaches
 *   1. Rotate by one, k times     shift right once, repeat         Time O(n*k)  Space O(1)
 *   2. Extra array                copy i to (i + k) % n            Time O(n)    Space O(n)
 *   3. Reversal       [optimal]   reverse all, then both parts     Time O(n)    Space O(1)
 *   4. Juggling (cycles)          follow gcd(n, k) cycles          Time O(n)    Space O(1)
 *
 * Key insight
 *   Rotating by n is the identity, so only k % n matters. Right-rotation
 *   turns A B (B = last k elements) into B A. Reversing the whole array
 *   gives rev(B) rev(A); reversing each part again gives B A.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 22_rotate_array.c -o rotate_array && ./rotate_array
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Common pitfall for every approach: k % n with n == 0 is a division by
 * zero (undefined behaviour), so arrays of length 0 or 1 return early.
 * They are unchanged by any rotation anyway.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Rotate by one, k times
 *   One right-rotation: save the last element, shift everything right by
 *   one place (walking from the back so nothing is overwritten before it is
 *   moved), and put the saved element at index 0. Repeat k % n times;
 *   without the % a huge k like 10^9 would take forever for no reason.
 *   Time O(n * k)   Space O(1)
 * ------------------------------------------------------------------------- */
static void rotate_by_one(int *nums, int n, int k)
{
    if (n <= 1)
        return;
    k %= n;
    for (int step = 0; step < k; step++) {
        int last = nums[n - 1];
        for (int i = n - 1; i > 0; i--)
            nums[i] = nums[i - 1];
        nums[0] = last;
    }
}

/* -------------------------------------------------------------------------
 * Approach 2: Extra array
 *   After a right rotation by k, the element at index i lands at index
 *   (i + k) % n. Write every element straight to its final slot in a
 *   scratch array, then copy the scratch array back.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static void rotate_extra_array(int *nums, int n, int k)
{
    if (n <= 1)
        return;
    k %= n;
    int *tmp = malloc((size_t)n * sizeof *tmp);
    if (tmp == NULL)
        return; /* out of memory: leave the array unchanged */
    for (int i = 0; i < n; i++)
        tmp[(i + k) % n] = nums[i];
    memcpy(nums, tmp, (size_t)n * sizeof *nums);
    free(tmp);
}

/* -------------------------------------------------------------------------
 * Approach 3: Reversal algorithm (optimal)
 *   Write the array as A B where B is the last k elements. We want B A.
 *     reverse everything      -> rev(B) rev(A)
 *     reverse the first k     -> B rev(A)
 *     reverse the remaining   -> B A
 *   Every element is swapped at most twice, with no extra memory.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void reverse_range(int *nums, int lo, int hi)
{
    while (lo < hi) {
        int t = nums[lo];
        nums[lo] = nums[hi];
        nums[hi] = t;
        lo++;
        hi--;
    }
}

static void rotate_reversal(int *nums, int n, int k)
{
    if (n <= 1)
        return;
    k %= n;
    reverse_range(nums, 0, n - 1);
    reverse_range(nums, 0, k - 1);
    reverse_range(nums, k, n - 1);
}

/* -------------------------------------------------------------------------
 * Approach 4: Juggling / cyclic replacement
 *   Pick up nums[start], drop it at (start + k) % n, pick up what was
 *   there, drop it k further on, and so on until we come back to start.
 *   Starting from s, the walk visits s, s+k, s+2k, ... (mod n). Those are
 *   exactly the indices congruent to s modulo g = gcd(n, k), and there are
 *   n / g of them. So the array splits into g independent cycles, one for
 *   each start 0 .. g-1, and together they move every element exactly once.
 *   (If you forgot the gcd and only ran one cycle, n = 6, k = 2 would only
 *   move the even indices.)
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int gcd(int a, int b)
{
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return a;
}

static void rotate_juggling(int *nums, int n, int k)
{
    if (n <= 1)
        return;
    k %= n;
    if (k == 0)
        return;
    int cycles = gcd(n, k);
    for (int start = 0; start < cycles; start++) {
        int carry = nums[start];
        int cur = start;
        do {
            int next = (cur + k) % n;
            int t = nums[next];
            nums[next] = carry;
            carry = t;
            cur = next;
        } while (cur != start);
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

typedef void (*RotateFn)(int *nums, int n, int k);

static const struct {
    const char *name;
    RotateFn fn;
} APPROACHES[] = {
    {"rotate by one", rotate_by_one},
    {"extra array", rotate_extra_array},
    {"reversal", rotate_reversal},
    {"juggling", rotate_juggling},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 16

typedef struct {
    int in[MAXN];
    int n;
    int k;
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
    printf("Q22. Rotate Array\n");

    /* Expected outputs worked out by hand. */
    const Case cases[] = {
        {{1, 2, 3, 4, 5, 6, 7}, 7, 3, {5, 6, 7, 1, 2, 3, 4}},
        {{-1, -100, 3, 99}, 4, 2, {3, 99, -1, -100}},
        {{1, 2, 3}, 3, 0, {1, 2, 3}},                   /* k = 0 */
        {{1, 2, 3}, 3, 3, {1, 2, 3}},                   /* k = n */
        {{1, 2, 3}, 3, 4, {3, 1, 2}},                   /* k > n: same as k = 1 */
        {{7}, 1, 5, {7}},                               /* single element */
        {{0}, 0, 3, {0}},                               /* empty: must not divide by 0 */
        {{1, 1, 2, 2}, 4, 1, {2, 1, 1, 2}},             /* duplicates */
        {{1, 2, 3, 4, 5, 6}, 6, 2, {5, 6, 1, 2, 3, 4}}, /* gcd 2: two cycles */
        {{1, 2, 3, 4, 5, 6}, 6, 4, {3, 4, 5, 6, 1, 2}}, /* gcd 2 */
        {{1, 2, 3, 4, 5, 6}, 6, 3, {4, 5, 6, 1, 2, 3}}, /* gcd 3 */
        {{1, 2, 3, 4, 5}, 5, 1000000007, {4, 5, 1, 2, 3}}, /* 10^9+7 % 5 = 2 */
        {{9, 9, 9}, 3, 2, {9, 9, 9}},                   /* all equal */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[MAXN];
            memcpy(work, c->in, sizeof work); /* each approach rotates its own copy */
            APPROACHES[a].fn(work, c->n, c->k);
            snprintf(label, sizeof label, "test %2d, %-14s n=%d k=%d", t + 1, APPROACHES[a].name,
                     c->n, c->k);
            check(same(work, c->expect, c->n), label);
        }
    }

    /* Random cross-check against rotate-by-one, including n = 0 and k > n.
     * k <= 40 and n <= 16 keep the O(n*k) reference trivial. */
    {
        srand(2222);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN] = {0}, expect[MAXN];
            int n = rand() % (MAXN + 1);
            int k = rand() % 41;
            for (int i = 0; i < n; i++)
                in[i] = rand() % 21 - 10;
            memcpy(expect, in, sizeof expect);
            rotate_by_one(expect, n, k);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int work[MAXN];
                memcpy(work, in, sizeof work);
                APPROACHES[a].fn(work, n, k);
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
