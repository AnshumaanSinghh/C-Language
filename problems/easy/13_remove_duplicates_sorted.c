/*
 * Q13. Remove Duplicates from Sorted Array
 * Difficulty : Easy
 * Topics     : Arrays, Two Pointers
 *
 * Problem
 *   Given an integer array nums sorted in non-decreasing order, remove the
 *   duplicates in place so that each unique value appears once. Return k,
 *   the number of unique values; nums[0..k-1] must hold them in their
 *   original order. What is left beyond index k - 1 does not matter.
 *   (LeetCode 26. A variant at the end keeps up to 2 copies: LeetCode 80.)
 *
 * Example
 *   nums = [0, 0, 1, 1, 1, 2, 2, 3, 3, 4]   ->   k = 5, nums = [0, 1, 2, 3, 4, ...]
 *
 * Approaches
 *   1. Extra array              copy each new value, copy back    Time O(n)    Space O(n)
 *   2. Shift left               close the gap after a duplicate   Time O(n^2)  Space O(1)
 *   3. Two pointers [optimal]   write index trails read index     Time O(n)    Space O(1)
 *
 * Key insight
 *   Because the array is sorted, equal values sit next to each other, so a
 *   value is new exactly when it differs from the last value kept. A slow
 *   write index and a fast read index do the whole job in one pass.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 13_remove_duplicates_sorted.c -o remove_duplicates_sorted && ./remove_duplicates_sorted
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Extra array
 *   Walk the input and append a value to a scratch buffer whenever it
 *   differs from the last value appended. Copy the buffer back.
 *   Simple and clearly correct, but uses O(n) extra memory.
 *   Returns -1 if the buffer cannot be allocated.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int remove_dups_extra(int *nums, int n)
{
    if (n == 0)
        return 0;
    int *tmp = malloc((size_t)n * sizeof *tmp);
    if (tmp == NULL)
        return -1;

    int k = 0;
    for (int i = 0; i < n; i++)
        if (k == 0 || nums[i] != tmp[k - 1])
            tmp[k++] = nums[i];

    memcpy(nums, tmp, (size_t)k * sizeof *nums);
    free(tmp);
    return k;
}

/* -------------------------------------------------------------------------
 * Approach 2: Shift everything left on each duplicate
 *   Keep a live length len. When nums[i] equals nums[i-1], delete nums[i]
 *   by shifting nums[i+1..len-1] one step left and shrinking len. Do not
 *   advance i after a delete: the value that slid into slot i may also be a
 *   duplicate. This is how you delete from an array without extra memory,
 *   and why it is slow: an all-equal array pays one O(n) shift per element.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int remove_dups_shift(int *nums, int n)
{
    int len = n;
    int i = 1;
    while (i < len) {
        if (nums[i] == nums[i - 1]) {
            for (int j = i; j + 1 < len; j++)
                nums[j] = nums[j + 1];
            len--;
        } else {
            i++;
        }
    }
    return len;
}

/* -------------------------------------------------------------------------
 * Approach 3: Two pointers (optimal)
 *   k is the write index: nums[0..k-1] is the deduplicated prefix built so
 *   far. i reads ahead. Since the input is sorted, nums[i] is a new value
 *   exactly when it differs from nums[k-1], the last value kept. k <= i
 *   always holds, so writing nums[k] never destroys anything still unread.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int remove_dups_two_pointers(int *nums, int n)
{
    if (n == 0)
        return 0;
    int k = 1; /* nums[0] is always kept */
    for (int i = 1; i < n; i++)
        if (nums[i] != nums[k - 1])
            nums[k++] = nums[i];
    return k;
}

/* -------------------------------------------------------------------------
 * Variant: keep at most m copies of each value (m = 2 is LeetCode 80)
 *   Same two pointers, one change: nums[i] may be kept unless the last m
 *   kept values already equal it. The kept prefix is sorted, so those m
 *   values are all equal to nums[i] exactly when nums[k-m] == nums[i]; one
 *   comparison is enough. While k < m there is no room for m copies yet, so
 *   everything is kept. With m = 1 this is exactly approach 3.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int remove_dups_at_most(int *nums, int n, int m)
{
    int k = 0;
    for (int i = 0; i < n; i++)
        if (k < m || nums[i] != nums[k - m])
            nums[k++] = nums[i];
    return k;
}

/* The obvious way to do the variant, used to cross-check it: count the
 * length of the current run of equal values and copy while it is <= m. */
static int at_most_reference(const int *nums, int n, int m, int *out)
{
    int k = 0, run = 0;
    for (int i = 0; i < n; i++) {
        run = (i > 0 && nums[i] == nums[i - 1]) ? run + 1 : 1;
        if (run <= m)
            out[k++] = nums[i];
    }
    return k;
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

typedef int (*RemoveDupsFn)(int *nums, int n);

static const struct {
    const char *name;
    RemoveDupsFn fn;
} APPROACHES[] = {
    {"extra array", remove_dups_extra},
    {"shift left", remove_dups_shift},
    {"two pointers", remove_dups_two_pointers},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 20

typedef struct {
    int in[MAXN];
    int n;
    int expect_k;
    int expect[MAXN];
} Case;

typedef struct {
    int in[MAXN];
    int n;
    int m;
    int expect_k;
    int expect[MAXN];
} AtMostCase;

static int same_array(const int *a, const int *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

/* Fills nums[0..n-1] with a random non-decreasing sequence. Steps of 0, 1
 * or 2 make runs of duplicates common. */
static void random_sorted(int *nums, int n)
{
    int v = rand() % 11 - 5;
    for (int i = 0; i < n; i++) {
        nums[i] = v;
        v += rand() % 3;
    }
}

int main(void)
{
    printf("Q13. Remove Duplicates from Sorted Array\n");

    const Case cases[] = {
        {{1, 1, 2}, 3, 2, {1, 2}},
        {{0, 0, 1, 1, 1, 2, 2, 3, 3, 4}, 10, 5, {0, 1, 2, 3, 4}},
        {{0}, 0, 0, {0}},                                   /* empty array */
        {{7}, 1, 1, {7}},                                   /* single element */
        {{3, 3, 3, 3}, 4, 1, {3}},                          /* all equal */
        {{-3, -1, 0, 2}, 4, 4, {-3, -1, 0, 2}},             /* already unique */
        {{-2, -2, -1, -1, -1}, 5, 2, {-2, -1}},             /* negatives */
        {{INT_MIN, INT_MIN, -1, INT_MAX, INT_MAX}, 5, 3, {INT_MIN, -1, INT_MAX}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[MAXN];
            memcpy(work, c->in, sizeof work); /* every approach gets a fresh copy */
            int k = APPROACHES[a].fn(work, c->n);
            snprintf(label, sizeof label, "test %d, %-12s -> k = %d", t + 1, APPROACHES[a].name, k);
            check(k == c->expect_k && same_array(work, c->expect, c->expect_k), label);
        }
    }

    /* Random cross-check against the extra-array version. */
    {
        srand(1313);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN], expect[MAXN];
            int n = rand() % (MAXN + 1);
            random_sorted(in, n);

            memcpy(expect, in, (size_t)n * sizeof *in);
            int expect_k = remove_dups_extra(expect, n);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int work[MAXN];
                memcpy(work, in, (size_t)n * sizeof *in);
                int k = APPROACHES[a].fn(work, n);
                if (k != expect_k || !same_array(work, expect, k))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    /* Variant: at most m copies of each value (LeetCode 80 is m = 2). */
    {
        const AtMostCase vcases[] = {
            {{1, 1, 1, 2, 2, 3}, 6, 2, 5, {1, 1, 2, 2, 3}},
            {{0, 0, 1, 1, 1, 1, 2, 3, 3}, 9, 2, 7, {0, 0, 1, 1, 2, 3, 3}},
            {{0}, 0, 2, 0, {0}},                            /* empty array */
            {{5}, 1, 2, 1, {5}},
            {{2, 2, 2, 2}, 4, 2, 2, {2, 2}},                /* all equal */
            {{-1, -1, -1, 0}, 4, 1, 2, {-1, 0}},            /* m = 1 is Q13 itself */
            {{1, 1, 1, 2, 2, 2}, 6, 3, 6, {1, 1, 1, 2, 2, 2}}, /* nothing removed */
        };
        const int num_vcases = (int)(sizeof vcases / sizeof vcases[0]);
        for (int t = 0; t < num_vcases; t++) {
            const AtMostCase *c = &vcases[t];
            int work[MAXN];
            memcpy(work, c->in, sizeof work);
            int k = remove_dups_at_most(work, c->n, c->m);
            snprintf(label, sizeof label, "variant test %d, m = %d -> k = %d", t + 1, c->m, k);
            check(k == c->expect_k && same_array(work, c->expect, c->expect_k), label);
        }

        srand(8080);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN], expect[MAXN], work[MAXN];
            int n = rand() % (MAXN + 1);
            int m = 1 + rand() % 3;
            random_sorted(in, n);
            int expect_k = at_most_reference(in, n, m, expect);
            memcpy(work, in, (size_t)n * sizeof *in);
            int k = remove_dups_at_most(work, n, m);
            if (k != expect_k || !same_array(work, expect, k))
                bad++;
        }
        snprintf(label, sizeof label, "variant cross-check: %d arrays, m in 1..3, matches run counting", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
