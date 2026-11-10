/*
 * Q29. Search in Rotated Sorted Array
 * Difficulty : Medium
 * Topics     : Arrays, Binary Search
 *
 * Problem
 *   An array of n DISTINCT integers was sorted ascending and then rotated
 *   at an unknown pivot, e.g. [0, 1, 2, 4, 5, 6, 7] -> [4, 5, 6, 7, 0, 1, 2].
 *   Given a target, return its index, or -1 if it is not in the array.
 *
 * Example
 *   nums = [4, 5, 6, 7, 0, 1, 2], target = 0   ->   4
 *   nums = [4, 5, 6, 7, 0, 1, 2], target = 3   ->   -1
 *
 * Approaches
 *   1. Linear scan                 check every element             Time O(n)      Space O(1)
 *   2. Find pivot, then search     binary search min, then a half  Time O(log n)  Space O(1)
 *   3. One-pass binary  [optimal]  one half is always sorted       Time O(log n)  Space O(1)
 *
 * Key insight
 *   Cut a rotated sorted array anywhere and at least one of the two pieces
 *   is plainly sorted. A sorted piece has known bounds (its first and last
 *   elements), so you can tell in O(1) whether the target can be in it,
 *   and throw away the other half, just like ordinary binary search.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 29_search_rotated_array.c -o search_rotated_array && ./search_rotated_array
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Pitfall for every binary search here: compute mid as lo + (hi - lo) / 2.
 * (lo + hi) / 2 overflows int when lo + hi > INT_MAX on huge arrays.
 * Values are compared directly, never subtracted, so INT_MIN / INT_MAX are
 * safe as elements and as targets.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Linear scan
 *   Ignore the structure and look at every element.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int search_linear(const int *nums, int n, int target)
{
    for (int i = 0; i < n; i++)
        if (nums[i] == target)
            return i;
    return -1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Find the pivot, then binary search one sorted half
 *   Step 1: find p = index of the minimum (where the rotation starts).
 *     Compare nums[mid] with nums[hi]: if nums[mid] > nums[hi], the drop
 *     from large to small happens after mid, so the minimum is in
 *     (mid, hi]; otherwise mid..hi is sorted and the minimum is in [lo, mid].
 *     (Comparing with nums[hi] rather than nums[lo] also handles an array
 *     that was not rotated at all.)
 *   Step 2: nums[p..n-1] and nums[0..p-1] are both sorted. Everything in
 *     the first run is smaller than everything in the second, so if
 *     nums[p] <= target <= nums[n-1] search the first, else the second.
 *   Time O(log n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int find_min_index(const int *nums, int n)
{
    int lo = 0, hi = n - 1;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (nums[mid] > nums[hi])
            lo = mid + 1;
        else
            hi = mid;
    }
    return lo;
}

/* Ordinary binary search on the sorted range nums[lo..hi]. */
static int binary_search(const int *nums, int lo, int hi, int target)
{
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (nums[mid] == target)
            return mid;
        if (nums[mid] < target)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}

static int search_pivot(const int *nums, int n, int target)
{
    if (n <= 0)
        return -1;
    int p = find_min_index(nums, n);
    if (nums[p] <= target && target <= nums[n - 1])
        return binary_search(nums, p, n - 1, target);
    return binary_search(nums, 0, p - 1, target);
}

/* -------------------------------------------------------------------------
 * Approach 3: One-pass modified binary search (optimal)
 *   At each step, at least one of nums[lo..mid] and nums[mid..hi] is
 *   sorted (the rotation point can only be in one of them):
 *     nums[lo] <= nums[mid] -> the left half is sorted. If
 *         nums[lo] <= target < nums[mid] the target can only be there:
 *         hi = mid - 1. Otherwise it can only be on the right: lo = mid + 1.
 *     otherwise             -> the right half is sorted. If
 *         nums[mid] < target <= nums[hi], go right; else go left.
 *   Use <= in "nums[lo] <= nums[mid]": when lo == mid the one-element left
 *   half counts as sorted. Using < there is a classic off-by-one bug
 *   (e.g. [3, 1] searching for 1).
 *   Time O(log n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int search_one_pass(const int *nums, int n, int target)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (nums[mid] == target)
            return mid;
        if (nums[lo] <= nums[mid]) {
            if (nums[lo] <= target && target < nums[mid])
                hi = mid - 1;
            else
                lo = mid + 1;
        } else {
            if (nums[mid] < target && target <= nums[hi])
                lo = mid + 1;
            else
                hi = mid - 1;
        }
    }
    return -1;
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

typedef int (*SearchFn)(const int *nums, int n, int target);

static const struct {
    const char *name;
    SearchFn fn;
} APPROACHES[] = {
    {"linear scan", search_linear},
    {"pivot + search", search_pivot},
    {"one-pass binary", search_one_pass},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 24

typedef struct {
    int nums[MAXN];
    int n;
    int target;
    int expect;
} Case;

int main(void)
{
    printf("Q29. Search in Rotated Sorted Array\n");

    /* Expected indices read off by hand (values are distinct, so the answer
     * is unique). */
    const Case cases[] = {
        {{4, 5, 6, 7, 0, 1, 2}, 7, 0, 4},
        {{4, 5, 6, 7, 0, 1, 2}, 7, 3, -1},             /* absent, inside range */
        {{4, 5, 6, 7, 0, 1, 2}, 7, 4, 0},              /* first element */
        {{4, 5, 6, 7, 0, 1, 2}, 7, 2, 6},              /* last element */
        {{1}, 1, 1, 0},                                /* n = 1, present */
        {{1}, 1, 0, -1},                               /* n = 1, absent */
        {{0}, 0, 5, -1},                               /* empty */
        {{1, 3, 5, 7, 9}, 5, 7, 3},                    /* not rotated */
        {{1, 3, 5, 7, 9}, 5, 1, 0},
        {{1, 3, 5, 7, 9}, 5, 10, -1},                  /* above every value */
        {{5, 1, 2, 3, 4}, 5, 5, 0},                    /* [1..5] rotated by n-1 */
        {{5, 1, 2, 3, 4}, 5, 4, 4},
        {{5, 1, 2, 3, 4}, 5, 1, 1},
        {{2, 3, 4, 5, 1}, 5, 1, 4},                    /* rotated by 1 */
        {{3, 1}, 2, 1, 1},                             /* lo == mid trap */
        {{3, 1}, 2, 3, 0},
        {{3, 1}, 2, 2, -1},
        {{-1, 2, -10, -7, -3}, 5, -7, 3},              /* negatives */
        {{-1, 2, -10, -7, -3}, 5, 2, 1},
        {{INT_MAX, INT_MIN, 0}, 3, INT_MIN, 1},        /* extreme values */
        {{INT_MAX, INT_MIN, 0}, 3, INT_MAX, 0},
        {{INT_MAX, INT_MIN, 0}, 3, 1, -1},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->nums, c->n, c->target);
            snprintf(label, sizeof label, "test %2d, %-15s target %d -> %d", t + 1,
                     APPROACHES[a].name, c->target, got);
            check(got == c->expect, label);
        }
    }

    /* Random cross-check: build a sorted array of distinct values (random
     * gaps of 1..4, so absent values sit between present ones), rotate it by
     * a random amount 0..n-1, then search for every value from min - 2 to
     * max + 2. Every approach must agree with the linear scan. */
    {
        srand(2929);
        int bad = 0, searches = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int sorted[MAXN], nums[MAXN];
            int n = 1 + rand() % MAXN;
            sorted[0] = rand() % 41 - 30;
            for (int i = 1; i < n; i++)
                sorted[i] = sorted[i - 1] + 1 + rand() % 4;
            int shift = rand() % n;
            for (int i = 0; i < n; i++)
                nums[i] = sorted[(i + shift) % n]; /* rotate left by shift */

            for (int target = sorted[0] - 2; target <= sorted[n - 1] + 2; target++) {
                int expect = search_linear(nums, n, target);
                for (int a = 0; a < NUM_APPROACHES; a++)
                    if (APPROACHES[a].fn(nums, n, target) != expect)
                        bad++;
                searches++;
            }
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d arrays, %d searches, all approaches agree", rounds,
                 searches);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
