/*
 * Q20. Merge Sorted Array
 * Difficulty : Easy
 * Topics     : Arrays, Two Pointers, Sorting
 *
 * Problem
 *   nums1 has room for m + n values: its first m are real and sorted in
 *   non-decreasing order, the last n are placeholders (zeros). nums2 holds
 *   n sorted values. Merge nums2 into nums1 so that all m + n values end up
 *   in nums1 in sorted order. (LeetCode 88)
 *
 * Example
 *   nums1 = [1, 2, 3, 0, 0, 0], m = 3, nums2 = [2, 5, 6], n = 3
 *   ->  nums1 = [1, 2, 2, 3, 5, 6]
 *
 * Approaches
 *   1. Insert + shift          insertion-sort each nums2 value   Time O(m*n)             Space O(1)
 *   2. Append + qsort          copy nums2 to the tail, sort      Time O((m+n) log(m+n))  Space O(1) *
 *   3. Buffer merge            two-pointer merge into new array  Time O(m+n)             Space O(m+n)
 *   4. Back merge  [optimal]   fill nums1 from right to left     Time O(m+n)             Space O(1)
 *   * plus whatever qsort uses internally, which the C standard leaves open
 *
 * Key insight
 *   The free space is at the END of nums1, so merge from the end: always
 *   place the larger of the two last values in the last free slot. The
 *   write position is ahead of every nums1 value not yet read, so nothing
 *   is overwritten before it is used.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 20_merge_sorted_arrays.c -o merge_sorted_arrays && ./merge_sorted_arrays
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Insert each nums2 value by shifting
 *   Treat nums1[0..len-1] as a sorted array that grows by one each time.
 *   For each v in nums2, shift every element greater than v one slot right
 *   (scanning from the back) and drop v into the gap, like one step of
 *   insertion sort.
 *   Why O(m*n) and not O((m+n)*n): nums2 is sorted, so the nums2 values
 *   already inserted are all <= v and the backward scan stops at them. Only
 *   original nums1 values (at most m) are ever shifted for each v.
 *   Time O(m*n + n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void merge_insert(int *nums1, int m, const int *nums2, int n)
{
    int len = m;
    for (int k = 0; k < n; k++) {
        int v = nums2[k];
        int j = len - 1;
        while (j >= 0 && nums1[j] > v) {
            nums1[j + 1] = nums1[j];
            j--;
        }
        nums1[j + 1] = v;
        len++;
    }
}

/* -------------------------------------------------------------------------
 * Approach 2: Copy nums2 into the tail, then qsort everything
 *   Short and hard to get wrong, but it throws away the fact that both
 *   halves are already sorted, so it pays a full sort.
 *   Pitfall: the comparator must not return x - y. For x = INT_MAX and
 *   y = INT_MIN that overflows (undefined behaviour) and usually gives the
 *   wrong sign. (x > y) - (x < y) is always -1, 0 or 1.
 *   Time O((m+n) log(m+n))   Space O(1) beyond what qsort uses internally
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static void merge_qsort(int *nums1, int m, const int *nums2, int n)
{
    for (int k = 0; k < n; k++)
        nums1[m + k] = nums2[k];
    if (m + n > 1)
        qsort(nums1, (size_t)(m + n), sizeof *nums1, cmp_int);
}

/* -------------------------------------------------------------------------
 * Approach 3: Two pointers into a new buffer, then copy back
 *   The merge step of merge sort: repeatedly take the smaller front value
 *   of the two arrays; when one runs out, copy the rest of the other.
 *   We need the buffer because writing into nums1 from the front would
 *   overwrite nums1 values that have not been read yet.
 *   Using <= takes from nums1 on ties, which keeps the merge stable.
 *   If the buffer cannot be allocated, nums1 is left unchanged.
 *   Time O(m+n)   Space O(m+n)
 * ------------------------------------------------------------------------- */
static void merge_buffer(int *nums1, int m, const int *nums2, int n)
{
    if (n == 0)
        return; /* nums1 is already the answer */
    int *out = malloc((size_t)(m + n) * sizeof *out);
    if (out == NULL)
        return;

    int i = 0, j = 0, w = 0;
    while (i < m && j < n)
        out[w++] = nums1[i] <= nums2[j] ? nums1[i++] : nums2[j++];
    while (i < m)
        out[w++] = nums1[i++];
    while (j < n)
        out[w++] = nums2[j++];

    memcpy(nums1, out, (size_t)(m + n) * sizeof *nums1);
    free(out);
}

/* -------------------------------------------------------------------------
 * Approach 4: Merge from the back, in place (optimal)
 *   i = last real value of nums1, j = last value of nums2, w = last slot.
 *   Put the larger of nums1[i] and nums2[j] at nums1[w] and step back.
 *   Why it is safe: w = i + j + 1, and while j >= 0 that is > i, so the
 *   slot we write is never a nums1 value we still need to read.
 *   The loop runs only while nums2 has values left. Once j < 0, the
 *   remaining nums1[0..i] are already in their final places.
 *   Time O(m+n)   Space O(1)
 * ------------------------------------------------------------------------- */
static void merge_from_back(int *nums1, int m, const int *nums2, int n)
{
    int i = m - 1, j = n - 1, w = m + n - 1;
    while (j >= 0) {
        if (i >= 0 && nums1[i] > nums2[j])
            nums1[w--] = nums1[i--];
        else
            nums1[w--] = nums2[j--];
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

typedef void (*MergeFn)(int *nums1, int m, const int *nums2, int n);

static const struct {
    const char *name;
    MergeFn fn;
} APPROACHES[] = {
    {"insert + shift", merge_insert},
    {"append + qsort", merge_qsort},
    {"buffer merge", merge_buffer},
    {"back merge", merge_from_back},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums1[8];
    int m;
    int nums2[4];
    int n;
    int expect[8];
} Case;

static int same_array(const int *a, const int *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

/* Fills a[0..n-1] with random sorted values in -10..10. */
static void random_sorted(int *a, int n)
{
    for (int k = 0; k < n; k++)
        a[k] = rand() % 21 - 10;
    if (n > 1)
        qsort(a, (size_t)n, sizeof *a, cmp_int);
}

int main(void)
{
    printf("Q20. Merge Sorted Array\n");

    const Case cases[] = {
        {{1, 2, 3, 0, 0, 0}, 3, {2, 5, 6}, 3, {1, 2, 2, 3, 5, 6}},
        {{0}, 0, {1}, 1, {1}},                                    /* m = 0 */
        {{1}, 1, {0}, 0, {1}},                                    /* n = 0 */
        {{0}, 0, {0}, 0, {0}},                                    /* both empty */
        {{0, 0, 0}, 0, {-5, 0, 5}, 3, {-5, 0, 5}},                /* m = 0, n > 1 */
        {{4, 5, 6, 0, 0, 0}, 3, {1, 2, 3}, 3, {1, 2, 3, 4, 5, 6}}, /* nums2 all first */
        {{1, 2, 3, 0, 0, 0}, 3, {4, 5, 6}, 3, {1, 2, 3, 4, 5, 6}}, /* nums2 all last */
        {{1, 5, 9, 0, 0}, 3, {3, 7}, 2, {1, 3, 5, 7, 9}},          /* interleaved */
        {{-3, -1, -1, 0, 0, 0}, 3, {-2, -1, 5}, 3, {-3, -2, -1, -1, -1, 5}},
        {{2, 2, 0, 0}, 2, {2, 2}, 2, {2, 2, 2, 2}},               /* all equal */
        {{0, 0, 0, 0}, 2, {-1, 1}, 2, {-1, 0, 0, 1}},             /* real zeros vs placeholders */
        {{INT_MIN, INT_MAX, 0, 0}, 2, {INT_MIN, INT_MAX}, 2, {INT_MIN, INT_MIN, INT_MAX, INT_MAX}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int work[8];
            memcpy(work, c->nums1, sizeof work); /* every approach gets a fresh nums1 */
            APPROACHES[a].fn(work, c->m, c->nums2, c->n);
            snprintf(label, sizeof label, "test %2d, %-14s (m = %d, n = %d)", t + 1,
                     APPROACHES[a].name, c->m, c->n);
            check(same_array(work, c->expect, c->m + c->n), label);
        }
    }

    /* Random cross-check against insert + shift. The problem says the
     * placeholders are zeros; here they are a junk value instead, which
     * proves no approach ever reads a placeholder as data. */
    {
        srand(2020);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums1[20], nums2[10], expect[20];
            int m = rand() % 11, n = rand() % 11;
            random_sorted(nums1, m);
            random_sorted(nums2, n);
            for (int k = m; k < m + n; k++)
                nums1[k] = 77777;

            memcpy(expect, nums1, (size_t)(m + n) * sizeof *nums1);
            merge_insert(expect, m, nums2, n);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int work[20];
                memcpy(work, nums1, (size_t)(m + n) * sizeof *nums1);
                APPROACHES[a].fn(work, m, nums2, n);
                if (!same_array(work, expect, m + n))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d merges, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
