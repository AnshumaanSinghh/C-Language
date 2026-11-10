/*
 * Q38. Find the Duplicate Number
 * Difficulty : Medium
 * Topics     : Arrays, Binary Search, Two Pointers, Cycle Detection
 *
 * Problem
 *   An array nums holds n + 1 integers, each in the range [1, n]. Exactly
 *   one value is repeated, possibly more than twice (so some other values
 *   may be missing). Return the repeated value. Ideally do it without
 *   modifying nums and with O(1) extra space.
 *
 * Example
 *   nums = [1, 3, 4, 2, 2]   ->   2
 *   nums = [3, 1, 3, 4, 2]   ->   3
 *
 * Approaches
 *   1. Brute force pairs        compare every pair                 Time O(n^2)      Space O(1)
 *   2. Sort a copy              duplicates end up adjacent         Time O(n log n)  Space O(n)
 *   3. Seen array               mark each value, stop on a repeat  Time O(n)        Space O(n)
 *   4. Negative marking         use the sign of nums[v] as a flag  Time O(n)        Space O(1)  (writes to nums)
 *   5. Binary search on values  count how many are <= mid          Time O(n log n)  Space O(1)
 *   6. Floyd cycle  [optimal]   i -> nums[i] is a linked list      Time O(n)        Space O(1)
 *
 * Key insight
 *   Read the array as a function on indices 0..n: from index i go to index
 *   nums[i]. The duplicate value is a node with two incoming arrows, which
 *   is exactly where a linked list's tail joins its cycle. Floyd's
 *   tortoise and hare finds that node in O(n) time and O(1) space.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 38_find_duplicate_number.c -o find_duplicate && ./find_duplicate
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Every approach takes the array length len = n + 1. The array is not
 * const only because approach 4 writes to it temporarily; every approach
 * must leave it exactly as it found it, and the tests check that. */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force pairs
 *   Compare every pair (i, j), i < j; the first equal pair is the answer.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int dup_brute(int *nums, int len)
{
    for (int i = 0; i < len; i++)
        for (int j = i + 1; j < len; j++)
            if (nums[i] == nums[j])
                return nums[i];
    return -1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort a copy
 *   After sorting, equal values are next to each other, so one pass over
 *   neighbours finds the repeat. Sorting nums itself would break the
 *   "do not modify" rule, hence the copy.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

static int dup_sort(int *nums, int len)
{
    int *a = malloc((size_t)len * sizeof *a);
    if (a == NULL)
        return -1;
    memcpy(a, nums, (size_t)len * sizeof *a);
    qsort(a, (size_t)len, sizeof *a, cmp_int);
    int result = -1;
    for (int i = 1; i < len; i++) {
        if (a[i] == a[i - 1]) {
            result = a[i];
            break;
        }
    }
    free(a);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Seen array
 *   Values are in [1, n], so a flag array indexed by value replaces a hash
 *   set: the first value whose flag is already set is the duplicate.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int dup_seen(int *nums, int len)
{
    unsigned char *seen = calloc((size_t)len, 1); /* indices 0..n */
    if (seen == NULL)
        return -1;
    int result = -1;
    for (int i = 0; i < len; i++) {
        if (seen[nums[i]]) {
            result = nums[i];
            break;
        }
        seen[nums[i]] = 1;
    }
    free(seen);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 4: Negative marking
 *   Use the array itself as the seen array: every value is positive, so
 *   the SIGN of nums[v] is a free bit. For each element, let v = |nums[i]|
 *   (it may already have been negated). If nums[v] is already negative, v
 *   was seen before: it is the duplicate. Otherwise negate nums[v].
 *   Index v is valid because 1 <= v <= n < len.
 *   It modifies the input, which the problem forbids and which surprises
 *   callers, so restore every sign before returning. Even with the restore
 *   it is not safe if another thread reads nums meanwhile, or if nums is
 *   read-only memory.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int dup_negate(int *nums, int len)
{
    int result = -1;
    for (int i = 0; i < len; i++) {
        int v = abs(nums[i]);
        if (nums[v] < 0) {
            result = v;
            break;
        }
        nums[v] = -nums[v];
    }
    for (int i = 0; i < len; i++)
        nums[i] = abs(nums[i]);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 5: Binary search on the value range
 *   Search over VALUES 1..n, not indices. Let count(x) = how many elements
 *   are <= x, and d the duplicate.
 *     x < d : each of 1..x appears at most once, so count(x) <= x.
 *     x >= d: every value above x except d appears at most once, so at
 *             most n - x elements are > x, so count(x) >= (n + 1) - (n - x)
 *             = x + 1 > x.
 *   So "count(x) > x" is false below d and true from d on, and d is the
 *   smallest x where it is true: a classic binary search for a boundary.
 *   This is the pigeonhole principle applied to the lower half of values.
 *   Each step is one O(n) counting pass, and there are log n steps.
 *   Time O(n log n)   Space O(1), array untouched
 * ------------------------------------------------------------------------- */
static int dup_binary_search(int *nums, int len)
{
    int lo = 1, hi = len - 1; /* the answer is in [lo, hi] */
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int count = 0;
        for (int i = 0; i < len; i++)
            if (nums[i] <= mid)
                count++;
        if (count > mid)
            hi = mid; /* duplicate is <= mid */
        else
            lo = mid + 1;
    }
    return lo;
}

/* -------------------------------------------------------------------------
 * Approach 6: Floyd's cycle detection (optimal)
 *   Why the array is a linked list with a cycle:
 *     Treat each index i in 0..n as a node whose "next" is nums[i]. Every
 *     node has exactly one next, and next is always in 1..n, so following
 *     next from any node can never stop and, with only n + 1 nodes, must
 *     eventually repeat a node: there is a cycle.
 *     No element points to index 0 (values are >= 1), so 0 is not on any
 *     cycle: starting from 0 we walk a tail and then enter the cycle.
 *     The node where the tail joins the cycle has two incoming arrows: one
 *     from the last tail node and one from the last cycle node. Two
 *     different indices i != j with nums[i] == nums[j] means that node's
 *     index is a repeated VALUE. Only one value repeats, so the entrance
 *     of the cycle is the duplicate.
 *   Finding the entrance is Floyd's algorithm (see Q32 for the proof):
 *     phase 1: slow = nums[slow], fast = nums[nums[fast]] until they meet;
 *     phase 2: restart one pointer at 0, move both one step at a time;
 *              they meet at the entrance.
 *   Time O(n)   Space O(1), array untouched
 * ------------------------------------------------------------------------- */
static int dup_floyd(int *nums, int len)
{
    (void)len; /* the walk never leaves 0..n, so len is not needed */
    int slow = 0, fast = 0;
    do {
        slow = nums[slow];
        fast = nums[nums[fast]];
    } while (slow != fast);

    int p = 0;
    while (p != slow) {
        p = nums[p];
        slow = nums[slow];
    }
    return p;
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

typedef int (*DupFn)(int *nums, int len);

/* max_len: largest array an approach is run on here (brute force is
 * quadratic, so it stays off the big input). */
static const struct {
    const char *name;
    DupFn fn;
    int max_len;
} APPROACHES[] = {
    {"brute pairs", dup_brute, 1000},
    {"sort a copy", dup_sort, 1 << 30},
    {"seen array", dup_seen, 1 << 30},
    {"negative marking", dup_negate, 1 << 30},
    {"binary search", dup_binary_search, 1 << 30},
    {"Floyd", dup_floyd, 1 << 30},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

/* Runs one approach on a private copy and also reports whether the copy
 * came back unchanged. */
static int run_on_copy(DupFn fn, const int *nums, int len, int *unchanged)
{
    int *a = malloc((size_t)len * sizeof *a);
    if (a == NULL) {
        *unchanged = 0;
        return -1;
    }
    memcpy(a, nums, (size_t)len * sizeof *a);
    int result = fn(a, len);
    *unchanged = memcmp(a, nums, (size_t)len * sizeof *a) == 0;
    free(a);
    return result;
}

typedef struct {
    const char *what;
    int nums[12];
    int len;
    int expect;
} Case;

int main(void)
{
    printf("Q38. Find the Duplicate Number\n");

    /* Expected answers read off each array by hand. */
    const Case cases[] = {
        {"example 1", {1, 3, 4, 2, 2}, 5, 2},
        {"example 2", {3, 1, 3, 4, 2}, 5, 3},
        {"smallest, n = 1", {1, 1}, 2, 1},
        {"n = 2", {1, 1, 2}, 3, 1},
        {"n = 2, other order", {2, 2, 1}, 3, 2},
        {"repeated many times", {2, 2, 2, 2, 2}, 5, 2},
        {"all equal to n", {3, 3, 3, 3}, 4, 3},
        {"three copies, one missing", {2, 5, 9, 6, 9, 3, 8, 9, 7, 1}, 10, 9},
        {"duplicate at both ends", {4, 1, 2, 3, 4}, 5, 4},
        {"duplicate is 1", {1, 2, 3, 4, 5, 6, 7, 8, 9, 1}, 10, 1},
        {"three copies of the largest", {1, 4, 4, 2, 4}, 5, 4},
        {"duplicate at the start", {7, 7, 1, 2, 3, 4, 5, 6}, 8, 7},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int unchanged;
            int got = run_on_copy(APPROACHES[a].fn, c->nums, c->len, &unchanged);
            snprintf(label, sizeof label, "test %2d %-28s, %-16s -> %d", t + 1, c->what,
                     APPROACHES[a].name, got);
            check(got == c->expect && unchanged, label);
        }
    }

    /* Big input: 1..N plus one extra 77777, shuffled (n = N = 100000).
     * Brute force is skipped (5 * 10^9 comparisons). */
    {
        enum { N = 100000, D = 77777 };
        int *big = malloc((N + 1) * sizeof *big);
        if (big != NULL) {
            for (int i = 0; i < N; i++)
                big[i] = i + 1;
            big[N] = D;
            srand(380);
            for (int i = N; i > 0; i--) { /* Fisher-Yates shuffle */
                int j = rand() % (i + 1);
                int tmp = big[i];
                big[i] = big[j];
                big[j] = tmp;
            }
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (N + 1 > APPROACHES[a].max_len)
                    continue;
                int unchanged;
                int got = run_on_copy(APPROACHES[a].fn, big, N + 1, &unchanged);
                snprintf(label, sizeof label, "n=%d shuffled, %-16s -> %d", N, APPROACHES[a].name,
                         got);
                check(got == D && unchanged, label);
            }
        } else {
            check(0, "allocation for big input");
        }
        free(big);
    }

    /* Random cross-check. Build a valid input: 1..n plus one extra d, then
     * overwrite some other values with d (so d can repeat many times and
     * those values go missing), then shuffle. The answer is d by
     * construction; every approach must return it, agree with brute force,
     * and leave its input unchanged. */
    {
        srand(38);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums[16];
            int n = 1 + rand() % 15, len = n + 1;
            int d = 1 + rand() % n;
            for (int i = 0; i < n; i++)
                nums[i] = i + 1;
            nums[n] = d;
            int extra = rand() % n; /* how many more values to replace by d */
            for (int e = 0; e < extra; e++) {
                int i = rand() % n;
                nums[i] = d; /* may hit d again; still one repeated value */
            }
            for (int i = len - 1; i > 0; i--) {
                int j = rand() % (i + 1);
                int tmp = nums[i];
                nums[i] = nums[j];
                nums[j] = tmp;
            }
            int brute_unchanged;
            int expect = run_on_copy(dup_brute, nums, len, &brute_unchanged);
            if (expect != d || !brute_unchanged)
                bad++;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int unchanged;
                if (run_on_copy(APPROACHES[a].fn, nums, len, &unchanged) != expect || !unchanged)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
