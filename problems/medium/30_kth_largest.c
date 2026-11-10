/*
 * Q30. Kth Largest Element
 * Difficulty : Medium
 * Topics     : Sorting, Heap, Quickselect
 *
 * Problem
 *   Given an integer array nums of length n and an integer k with
 *   1 <= k <= n, return the kth largest element. Duplicates count
 *   separately: in [3, 3, 1] the 1st and the 2nd largest are both 3.
 *
 * Example
 *   nums = [3, 2, 1, 5, 6, 4], k = 2   ->   5   (descending: 6, 5, 4, ...)
 *
 * Approaches
 *   1. Sort descending          sort, take index k - 1             Time O(n log n)  Space O(1)
 *   2. Partial selection        k passes of "move the max forward" Time O(n*k)      Space O(1)
 *   3. Min-heap of size k       keep the k largest seen so far     Time O(n log k)  Space O(k)
 *   4. Quickselect  [optimal]   partition, keep only one side      Time O(n) avg    Space O(1)
 *
 *   Approaches 1, 2 and 4 rearrange the array. Here they work on a private
 *   copy so the caller's array is untouched; a real in-place version would
 *   skip that copy, which is why their extra space is listed as O(1).
 *
 * Key insight
 *   You do not need the array sorted, you only need the element that would
 *   land at index n - k of an ascending sort. One partition step puts the
 *   pivot exactly where it belongs and tells you which side the answer is
 *   on, so you can throw the other side away.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 30_kth_largest.c -o kth_largest && ./kth_largest
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int *copy_array(const int *nums, int n)
{
    int *a = malloc((size_t)(n > 0 ? n : 1) * sizeof *a);
    if (a != NULL && n > 0)
        memcpy(a, nums, (size_t)n * sizeof *a);
    return a;
}

static void swap_int(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

/* -------------------------------------------------------------------------
 * Approach 1: Sort descending
 *   Sort a copy from largest to smallest; the answer is at index k - 1.
 *   Pitfall: the comparator must not return x - y, which overflows for
 *   values such as INT_MAX and INT_MIN. Compare instead.
 *   Time O(n log n)   Space O(1) extra (plus the copy)
 * ------------------------------------------------------------------------- */
static int cmp_desc(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x < y) - (x > y);
}

static int kth_sort(const int *nums, int n, int k)
{
    int *a = copy_array(nums, n);
    if (a == NULL)
        return INT_MIN;
    qsort(a, (size_t)n, sizeof *a, cmp_desc);
    int result = a[k - 1];
    free(a);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 2: Partial selection
 *   This is selection sort stopped after k passes. Pass i finds the largest
 *   of a[i..n-1] and swaps it to position i, so after pass i the prefix
 *   a[0..i] holds the i + 1 largest values in descending order. After k
 *   passes a[k - 1] is the answer.
 *   Good when k is tiny (k = 1 is just "find the max"); terrible when k is
 *   close to n. For k > n / 2 you could instead select the (n - k + 1)th
 *   smallest with the same loop.
 *   Time O(n*k)   Space O(1) extra (plus the copy)
 * ------------------------------------------------------------------------- */
static int kth_partial(const int *nums, int n, int k)
{
    int *a = copy_array(nums, n);
    if (a == NULL)
        return INT_MIN;
    for (int i = 0; i < k; i++) {
        int best = i;
        for (int j = i + 1; j < n; j++)
            if (a[j] > a[best])
                best = j;
        swap_int(&a[i], &a[best]);
    }
    int result = a[k - 1];
    free(a);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Min-heap of size k
 *   Keep the k largest values seen so far in a min-heap. Its root is the
 *   smallest of those k, i.e. the kth largest so far. For each new value x:
 *     x <= root : x cannot be among the k largest, skip it;
 *     x >  root : the root drops out, x takes its place, sift down.
 *   After the whole array the root is the kth largest overall.
 *   Why a MIN-heap for the kth LARGEST: the element we must be able to kick
 *   out quickly is the weakest member of the top k, which is the minimum.
 *   Works on a stream too: it never needs more than k values in memory.
 *   Time O(n log k)   Space O(k)
 * ------------------------------------------------------------------------- */
static void sift_down_min(int *h, int size, int i)
{
    for (;;) {
        int l = 2 * i + 1, r = l + 1, m = i;
        if (l < size && h[l] < h[m])
            m = l;
        if (r < size && h[r] < h[m])
            m = r;
        if (m == i)
            return;
        swap_int(&h[i], &h[m]);
        i = m;
    }
}

static int kth_heap(const int *nums, int n, int k)
{
    int *h = malloc((size_t)k * sizeof *h);
    if (h == NULL)
        return INT_MIN;
    memcpy(h, nums, (size_t)k * sizeof *h);
    for (int i = k / 2 - 1; i >= 0; i--) /* heapify the first k in O(k) */
        sift_down_min(h, k, i);
    for (int j = k; j < n; j++) {
        if (nums[j] > h[0]) {
            h[0] = nums[j];
            sift_down_min(h, k, 0);
        }
    }
    int result = h[0];
    free(h);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 4: Quickselect with a random pivot (optimal on average)
 *   The kth largest sits at index target = n - k of an ascending sort.
 *   Pick a pivot and 3-way partition a[lo..hi] into
 *       [ < pivot | == pivot | > pivot ]
 *        lo..lt-1   lt..gt     gt+1..hi
 *   Every pivot copy is now at its final sorted position. If target is in
 *   lt..gt the answer is the pivot; otherwise continue on the one side that
 *   contains target and forget the other side completely.
 *
 *   Why it is O(n) on average: a random pivot lands in the middle half of
 *   the range half the time, so the range shrinks geometrically and the
 *   total work is about n + 3n/4 + (3/4)^2 n + ... = O(n).
 *
 *   Worst case: if every pivot happens to be the smallest or largest value
 *   of its range, each step only removes one element and the total work is
 *   n + (n-1) + ... = O(n^2). A fixed pivot choice (say, always a[lo]) hits
 *   this on already-sorted input, which is common in practice. A random
 *   pivot makes the bad case astronomically unlikely but not impossible.
 *   If you need a guarantee, median-of-medians picks a pivot in O(n) that
 *   is always near the middle, giving O(n) worst case at a larger constant;
 *   introselect starts with quickselect and switches to it when progress
 *   stalls.
 *
 *   Why 3-way partitioning: with a 2-way (Lomuto) partition, an array of
 *   all-equal values puts everything on one side every time -> O(n^2).
 *   Grouping the "== pivot" block finishes that case in one pass.
 *
 *   The pivot index comes from a private xorshift generator so this
 *   approach does not disturb rand(), which the test harness uses.
 *   Time O(n) average, O(n^2) worst   Space O(1) extra (plus the copy)
 * ------------------------------------------------------------------------- */
static unsigned int qs_state = 2463534242u;

static unsigned int qs_rand(void)
{
    qs_state ^= qs_state << 13;
    qs_state ^= qs_state >> 17;
    qs_state ^= qs_state << 5;
    return qs_state;
}

static int kth_quickselect(const int *nums, int n, int k)
{
    int *a = copy_array(nums, n);
    if (a == NULL)
        return INT_MIN;

    int target = n - k;
    int lo = 0, hi = n - 1;
    while (lo < hi) {
        int pivot = a[lo + (int)(qs_rand() % (unsigned int)(hi - lo + 1))];
        int lt = lo, i = lo, gt = hi;
        while (i <= gt) {
            if (a[i] < pivot)
                swap_int(&a[lt++], &a[i++]);
            else if (a[i] > pivot)
                swap_int(&a[i], &a[gt--]); /* do not advance i: new a[i] is unseen */
            else
                i++;
        }
        if (target < lt)
            hi = lt - 1;
        else if (target > gt)
            lo = gt + 1;
        else
            break; /* a[target] == pivot, already in its final place */
    }
    int result = a[target];
    free(a);
    return result;
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

typedef int (*KthFn)(const int *nums, int n, int k);

static const struct {
    const char *name;
    KthFn fn;
} APPROACHES[] = {
    {"sort descending", kth_sort},
    {"partial selection", kth_partial},
    {"min-heap of size k", kth_heap},
    {"quickselect", kth_quickselect},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[10];
    int n;
    int k;
    int expect;
} Case;

int main(void)
{
    printf("Q30. Kth Largest Element\n");

    /* Expected values worked out by sorting each array by hand. */
    const Case cases[] = {
        {{3, 2, 1, 5, 6, 4}, 6, 2, 5},             /* 6 5 4 3 2 1 */
        {{3, 2, 3, 1, 2, 4, 5, 5, 6}, 9, 4, 4},    /* 6 5 5 4 3 3 2 2 1 */
        {{7}, 1, 1, 7},                            /* single element */
        {{1, 2, 2}, 3, 2, 2},                      /* duplicates count separately */
        {{5, 5, 5, 5}, 4, 3, 5},                   /* all equal */
        {{-1, -5, -3}, 3, 1, -1},                  /* negatives, k = 1 */
        {{-1, -5, -3}, 3, 3, -5},                  /* negatives, k = n */
        {{1, 2, 3, 4, 5, 6, 7, 8}, 8, 8, 1},       /* k = n is the minimum */
        {{8, 7, 6, 5, 4, 3, 2, 1}, 8, 1, 8},       /* k = 1 is the maximum */
        {{INT_MAX, INT_MIN, 0}, 3, 1, INT_MAX},    /* x - y comparator would overflow */
        {{INT_MAX, INT_MIN, 0}, 3, 3, INT_MIN},
        {{0, INT_MIN, INT_MIN, -7}, 4, 2, -7},     /* 0 -7 MIN MIN */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->nums, c->n, c->k);
            snprintf(label, sizeof label, "test %2d, %-20s k=%d -> %d", t + 1,
                     APPROACHES[a].name, c->k, got);
            check(got == c->expect, label);
        }
    }

    /* Large inputs that break careless quickselects: sorted input (bad for a
     * fixed first-element pivot) and all-equal input (bad for a 2-way
     * partition). For 1..N the kth largest is N - k + 1. Partial selection
     * is skipped here because O(n*k) would be 10^8 steps. */
    {
        enum { N = 20000 };
        int *asc = malloc(N * sizeof *asc);
        int *same = malloc(N * sizeof *same);
        if (asc != NULL && same != NULL) {
            for (int i = 0; i < N; i++) {
                asc[i] = i + 1;
                same[i] = 42;
            }
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (APPROACHES[a].fn == kth_partial)
                    continue;
                int ok = APPROACHES[a].fn(asc, N, 1) == N &&
                         APPROACHES[a].fn(asc, N, N) == 1 &&
                         APPROACHES[a].fn(asc, N, 5000) == N - 5000 + 1 &&
                         APPROACHES[a].fn(same, N, N / 2) == 42;
                snprintf(label, sizeof label, "n=%d sorted + all-equal, %-20s", N,
                         APPROACHES[a].name);
                check(ok, label);
            }
        } else {
            check(0, "allocation for large inputs");
        }
        free(asc);
        free(same);
    }

    /* The input array must be left exactly as it was. */
    {
        const int orig[] = {9, -2, 7, 7, 0, 3};
        int arr[6];
        memcpy(arr, orig, sizeof arr);
        for (int a = 0; a < NUM_APPROACHES; a++)
            (void)APPROACHES[a].fn(arr, 6, 3);
        check(memcmp(arr, orig, sizeof arr) == 0, "input array unchanged by every approach");
    }

    /* Random cross-check against the sort approach on small arrays with many
     * duplicates (values -10..10). */
    {
        srand(30);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums[20];
            int n = 1 + rand() % 20;
            for (int i = 0; i < n; i++)
                nums[i] = rand() % 21 - 10;
            int k = 1 + rand() % n;
            int expect = kth_sort(nums, n, k);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(nums, n, k) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
