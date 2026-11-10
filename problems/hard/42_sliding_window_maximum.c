/*
 * Q42. Sliding Window Maximum
 * Difficulty : Hard
 * Topics     : Arrays, Sliding Window, Heap, Monotonic Queue
 *
 * Problem
 *   Given an array nums of n integers and a window size k (1 <= k <= n), a
 *   window of k consecutive elements slides from the left end to the right
 *   end one step at a time. Return the maximum of every window: n - k + 1
 *   values in order.
 *
 * Example
 *   nums = [1, 3, -1, -3, 5, 3, 6, 7], k = 3   ->   [3, 3, 5, 5, 6, 7]
 *
 *     [1  3  -1] -3  5  3  6  7     3
 *      1 [3  -1  -3] 5  3  6  7     3
 *      1  3 [-1  -3  5] 3  6  7     5
 *      1  3  -1 [-3  5  3] 6  7     5
 *      1  3  -1  -3 [5  3  6] 7     6
 *      1  3  -1  -3  5 [3  6  7]    7
 *
 * Approaches
 *   1. Brute force                 scan every window                        Time O(n*k)      Space O(1)
 *   2. Max-heap, lazy deletion     heap of (value, index), drop stale top   Time O(n log n)  Space O(n)
 *   3. Block decomposition         prefix/suffix max inside k-blocks        Time O(n)        Space O(n)
 *   4. Monotonic deque [optimal]   deque of indices, values decreasing      Time O(n)        Space O(k)
 *
 * Key insight
 *   Once a newer element x enters the window, every older element <= x can
 *   never be a window maximum again: x is at least as big and stays longer.
 *   Throwing those away leaves a queue whose values decrease from front to
 *   back, so the front is always the current maximum, and each index enters
 *   and leaves the queue once.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 42_sliding_window_maximum.c -o sliding_window_maximum && ./sliding_window_maximum
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* All approaches write n - k + 1 maxima to out and return that count, or
 * return 0 if memory runs out. They require 1 <= k <= n. */

static int max_int(int a, int b) { return a > b ? a : b; }

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   For each of the n - k + 1 windows, scan its k elements for the maximum.
 *   Time O(n*k)   Space O(1)
 * ------------------------------------------------------------------------- */
static int window_max_brute(const int *nums, int n, int k, int *out)
{
    int count = 0;
    for (int i = 0; i + k <= n; i++) {
        int best = nums[i];
        for (int j = i + 1; j < i + k; j++)
            best = max_int(best, nums[j]);
        out[count++] = best;
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Max-heap of (value, index) with lazy deletion
 *   Push every element into a max-heap. The top is the largest value pushed
 *   so far, but it may have slid out of the window already. Removing an
 *   arbitrary element from a binary heap is awkward, so do not try: only
 *   when a stale element reaches the top (index <= i - k) pop it. Stale
 *   elements buried deeper do no harm until they surface.
 *   The heap can hold up to n items (for increasing input nothing is ever
 *   popped), and each push/pop costs O(log n).
 *   Time O(n log n)   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    int value;
    int index;
} HeapItem;

/* Higher value wins; on ties the newer index wins (it stays valid longer). */
static int heap_above(HeapItem x, HeapItem y)
{
    return x.value > y.value || (x.value == y.value && x.index > y.index);
}

static void heap_push(HeapItem *heap, int *size, HeapItem item)
{
    int c = (*size)++;
    while (c > 0) {
        int p = (c - 1) / 2;
        if (!heap_above(item, heap[p]))
            break;
        heap[c] = heap[p];
        c = p;
    }
    heap[c] = item;
}

static void heap_pop(HeapItem *heap, int *size)
{
    HeapItem last = heap[--(*size)];
    int n = *size, p = 0;
    for (;;) {
        int c = 2 * p + 1;
        if (c >= n)
            break;
        if (c + 1 < n && heap_above(heap[c + 1], heap[c]))
            c++;
        if (!heap_above(heap[c], last))
            break;
        heap[p] = heap[c];
        p = c;
    }
    if (n > 0)
        heap[p] = last;
}

static int window_max_heap(const int *nums, int n, int k, int *out)
{
    HeapItem *heap = malloc((size_t)n * sizeof *heap);
    if (heap == NULL)
        return 0;

    int size = 0, count = 0;
    for (int i = 0; i < n; i++) {
        heap_push(heap, &size, (HeapItem){nums[i], i});
        if (i >= k - 1) {
            /* Never empties: item i itself is inside the window. */
            while (heap[0].index <= i - k)
                heap_pop(heap, &size);
            out[count++] = heap[0].value;
        }
    }
    free(heap);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: Block decomposition (prefix max + suffix max)
 *   Cut the array into blocks of k: [0..k-1], [k..2k-1], ... (the last one
 *   may be shorter). Inside each block precompute
 *       prefix[i] = max from the start of i's block up to i
 *       suffix[i] = max from i up to the end of i's block
 *   A window [i, i+k-1] has length k, so it covers the tail of the block
 *   holding i and the head of the block holding i+k-1 (if i is a block
 *   start, both are the same block). Therefore
 *       max(window) = max(suffix[i], prefix[i + k - 1]).
 *
 *       nums    1  3 -1 | -3  5  3 |  6  7        (k = 3)
 *       prefix  1  3  3 | -3  5  5 |  6  7
 *       suffix  3  3 -1 |  5  5  3 |  7  7
 *       window [1..3] = max(suffix[1], prefix[3]) = max(3, -3) = 3
 *
 *   Pitfall: the suffix scan must restart at n - 1, which ends a partial
 *   last block, not only at multiples of k.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int window_max_blocks(const int *nums, int n, int k, int *out)
{
    int *prefix = malloc((size_t)n * sizeof *prefix);
    int *suffix = malloc((size_t)n * sizeof *suffix);
    if (prefix == NULL || suffix == NULL) {
        free(prefix);
        free(suffix);
        return 0;
    }

    for (int i = 0; i < n; i++)
        prefix[i] = i % k == 0 ? nums[i] : max_int(prefix[i - 1], nums[i]);
    for (int i = n - 1; i >= 0; i--)
        suffix[i] = (i == n - 1 || (i + 1) % k == 0) ? nums[i] : max_int(suffix[i + 1], nums[i]);

    int count = 0;
    for (int i = 0; i + k <= n; i++)
        out[count++] = max_int(suffix[i], prefix[i + k - 1]);

    free(prefix);
    free(suffix);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 4: Monotonic deque of indices (optimal)
 *   Keep a double-ended queue of indices whose values strictly decrease from
 *   front to back. For each new index i:
 *     1. If the front index is i - k, it just left the window: drop it.
 *        (At most one index leaves per step, and it is always the oldest,
 *        i.e. the front.)
 *     2. Pop from the BACK every index whose value is <= nums[i]. They are
 *        older and not larger, so they can never be a maximum again.
 *     3. Push i at the back.
 *     4. Once the first window is complete, the front is the maximum.
 *   Each index is pushed once and popped at most once, so the whole loop is
 *   O(n) even though step 2 is a loop.
 *   Only indices inside the current window are stored, so a ring buffer of
 *   k slots is enough: position t of the deque lives at dq[(head + t) % k].
 *   Time O(n)   Space O(k)
 * ------------------------------------------------------------------------- */
static int window_max_deque(const int *nums, int n, int k, int *out)
{
    int *dq = malloc((size_t)k * sizeof *dq);
    if (dq == NULL)
        return 0;

    int head = 0, size = 0, count = 0;
    for (int i = 0; i < n; i++) {
        if (size > 0 && dq[head] <= i - k) {
            head = (head + 1) % k;
            size--;
        }
        while (size > 0 && nums[dq[(head + size - 1) % k]] <= nums[i])
            size--;
        dq[(head + size) % k] = i;
        size++;
        if (i >= k - 1)
            out[count++] = nums[dq[head]];
    }
    free(dq);
    return count;
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

typedef int (*WindowMaxFn)(const int *nums, int n, int k, int *out);

static const struct {
    const char *name;
    WindowMaxFn fn;
} APPROACHES[] = {
    {"brute force", window_max_brute},
    {"max-heap", window_max_heap},
    {"block decomposition", window_max_blocks},
    {"monotonic deque", window_max_deque},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAX_N 20

typedef struct {
    int nums[MAX_N];
    int n;
    int k;
    int expect[MAX_N];
} Case;

static int same(const int *x, const int *y, int len)
{
    return memcmp(x, y, (size_t)len * sizeof *x) == 0;
}

/* Writes "[v0 v1 ...]" into buf; INT_MIN / INT_MAX are shown by name. */
static void format_list(char *buf, size_t cap, const int *v, int len)
{
    size_t used = (size_t)snprintf(buf, cap, "[");
    for (int i = 0; i < len && used < cap; i++) {
        const char *sep = i ? " " : "";
        int w;
        if (v[i] == INT_MIN)
            w = snprintf(buf + used, cap - used, "%sMIN", sep);
        else if (v[i] == INT_MAX)
            w = snprintf(buf + used, cap - used, "%sMAX", sep);
        else
            w = snprintf(buf + used, cap - used, "%s%d", sep, v[i]);
        if (w < 0)
            return;
        used += (size_t)w;
    }
    if (used < cap)
        snprintf(buf + used, cap - used, "]");
}

int main(void)
{
    printf("Q42. Sliding Window Maximum\n");

    /* Expected maxima worked out window by window by hand. */
    const Case cases[] = {
        {{1, 3, -1, -3, 5, 3, 6, 7}, 8, 3, {3, 3, 5, 5, 6, 7}},
        {{1}, 1, 1, {1}},                                         /* single element */
        {{1, -1}, 2, 1, {1, -1}},                                 /* k = 1 */
        {{3, 1, 4, 1, 5}, 5, 1, {3, 1, 4, 1, 5}},                 /* k = 1: output = input */
        {{9, 11}, 2, 2, {11}},                                    /* k = n */
        {{4, -2}, 2, 2, {4}},                                     /* k = n, max first */
        {{3, 1, 4, 1, 5, 9, 2, 6}, 8, 8, {9}},                    /* k = n */
        {{5, 5, 5, 5}, 4, 2, {5, 5, 5}},                          /* all equal */
        {{1, 2, 3, 4, 5}, 5, 2, {2, 3, 4, 5}},                    /* increasing */
        {{5, 4, 3, 2, 1}, 5, 3, {5, 4, 3}},                       /* decreasing */
        {{-1, -3, -2, -5, -4}, 5, 2, {-1, -2, -2, -4}},           /* all negative */
        {{1, 3, 1, 2, 0, 5}, 6, 3, {3, 3, 2, 5}},
        {{7, 2, 4, 6, 1, 8, 3, 9, 5, 0}, 10, 4, {7, 6, 8, 8, 9, 9, 9}}, /* partial last block */
        {{INT_MIN, INT_MIN, INT_MAX, INT_MIN}, 4, 2, {INT_MIN, INT_MAX, INT_MAX}}, /* extremes */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        int want = c->n - c->k + 1;
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int out[MAX_N];
            int got = APPROACHES[a].fn(c->nums, c->n, c->k, out);
            char shown[96];
            format_list(shown, sizeof shown, out, got);
            snprintf(label, sizeof label, "test %2d (k=%d), %-19s -> %s", t + 1, c->k,
                     APPROACHES[a].name, shown);
            check(got == want && same(out, c->expect, want), label);
        }
    }

    /* Random cross-check: every (n, k) shape with 1 <= k <= n <= 20 is
     * possible, values -10..10 so ties are common. Every approach must match
     * brute force element for element. No approach modifies its input. */
    {
        srand(42);
        int bad = 0;
        const int rounds = 1000;
        for (int r = 0; r < rounds; r++) {
            int nums[MAX_N], expect[MAX_N];
            int n = 1 + rand() % MAX_N;
            int k = 1 + rand() % n;
            for (int i = 0; i < n; i++)
                nums[i] = rand() % 21 - 10;

            int want = window_max_brute(nums, n, k, expect);
            for (int a = 1; a < NUM_APPROACHES; a++) {
                int out[MAX_N];
                int got = APPROACHES[a].fn(nums, n, k, out);
                if (got != want || !same(out, expect, want))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
