/*
 * Q14. Second Largest Distinct Element
 * Difficulty : Easy
 * Topics     : Arrays, Sorting
 *
 * Problem
 *   Given an array nums of n integers, return the second largest DISTINCT
 *   value: the largest value that is strictly smaller than the maximum.
 *   If there is none (n < 2, or every element is equal), report "none".
 *   Here every approach returns 1 and writes *out when the answer exists,
 *   returns 0 for "none", and returns -1 if it could not allocate memory.
 *
 * Example
 *   nums = [12, 35, 1, 10, 34, 1]   ->   34
 *   nums = [10, 5, 10]              ->   5     (the second 10 does not count)
 *   nums = [7, 7, 7]                ->   none
 *
 * Approaches
 *   1. Sort descending         sort a copy, first value != max  Time O(n log n)  Space O(n)
 *   2. Two passes              find max, then max below it      Time O(n)        Space O(1)
 *   3. One pass    [optimal]   track largest and second         Time O(n)        Space O(1)
 *
 * Key insight
 *   "None" must be reported with a separate found flag. Starting second at a
 *   sentinel like INT_MIN cannot tell "no answer" from "the answer is
 *   INT_MIN": both [INT_MIN, 5] and [5, 5] would leave second == INT_MIN.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 14_second_largest.c -o second_largest && ./second_largest
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Sort a copy in descending order
 *   After sorting, a[0] is the maximum. Skip every copy of it; the first
 *   different value is the second largest distinct value. If the scan runs
 *   off the end, all values were equal.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_desc(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x < y) - (x > y); /* reversed for descending; never y - x (overflow) */
}

static int second_sort(const int *nums, int n, int *out)
{
    if (n < 2)
        return 0;
    int *a = malloc((size_t)n * sizeof *a);
    if (a == NULL)
        return -1;
    memcpy(a, nums, (size_t)n * sizeof *a);
    qsort(a, (size_t)n, sizeof *a, cmp_desc);

    int found = 0;
    for (int i = 1; i < n && !found; i++) {
        if (a[i] != a[0]) {
            *out = a[i];
            found = 1;
        }
    }
    free(a);
    return found;
}

/* -------------------------------------------------------------------------
 * Approach 2: Two passes
 *   Pass 1 finds the maximum. Pass 2 finds the largest value strictly below
 *   it, which is the definition of the answer. "found" records whether pass
 *   2 saw any such value at all.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int second_two_pass(const int *nums, int n, int *out)
{
    if (n < 2)
        return 0;
    int max = nums[0];
    for (int i = 1; i < n; i++)
        if (nums[i] > max)
            max = nums[i];

    int found = 0, best = 0;
    for (int i = 0; i < n; i++) {
        if (nums[i] < max && (!found || nums[i] > best)) {
            best = nums[i];
            found = 1;
        }
    }
    if (found)
        *out = best;
    return found;
}

/* -------------------------------------------------------------------------
 * Approach 3: One pass (optimal)
 *   Keep the largest value seen and the largest value strictly below it,
 *   each with its own "have" flag. For each x:
 *     x >  largest : the old largest becomes second, x becomes largest
 *     x == largest : a duplicate of the maximum, ignore it
 *     second < x < largest (or no second yet) : x becomes second
 *   Pitfall: forgetting the x == largest case makes [10, 10] answer 10.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int second_one_pass(const int *nums, int n, int *out)
{
    int have_largest = 0, have_second = 0;
    int largest = 0, second = 0;
    for (int i = 0; i < n; i++) {
        int x = nums[i];
        if (!have_largest || x > largest) {
            if (have_largest) {
                second = largest;
                have_second = 1;
            }
            largest = x;
            have_largest = 1;
        } else if (x < largest && (!have_second || x > second)) {
            second = x;
            have_second = 1;
        }
    }
    if (have_second)
        *out = second;
    return have_second;
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

typedef int (*SecondFn)(const int *nums, int n, int *out);

static const struct {
    const char *name;
    SecondFn fn;
} APPROACHES[] = {
    {"sort descending", second_sort},
    {"two passes", second_two_pass},
    {"one pass", second_one_pass},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[8];
    int n;
    int expect_found;
    int expect; /* only meaningful when expect_found is 1 */
} Case;

int main(void)
{
    printf("Q14. Second Largest Distinct Element\n");

    const Case cases[] = {
        {{12, 35, 1, 10, 34, 1}, 6, 1, 34},
        {{10, 5, 10}, 3, 1, 5},                 /* duplicate maximum */
        {{1, 2}, 2, 1, 1},
        {{2, 1, 2, 1}, 4, 1, 1},
        {{-5, -1, -3}, 3, 1, -3},               /* all negative */
        {{INT_MIN, INT_MAX}, 2, 1, INT_MIN},    /* answer equals the usual sentinel */
        {{5, INT_MIN, 5}, 3, 1, INT_MIN},
        {{3, 3, 3, 2, 3}, 5, 1, 2},
        {{7, 7, 7}, 3, 0, 0},                   /* all equal -> none */
        {{INT_MIN, INT_MIN}, 2, 0, 0},
        {{42}, 1, 0, 0},                        /* n < 2 -> none */
        {{0}, 0, 0, 0},                         /* empty -> none */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = 0;
            int found = APPROACHES[a].fn(c->nums, c->n, &got);
            if (found == 1)
                snprintf(label, sizeof label, "test %2d, %-15s -> %d", t + 1, APPROACHES[a].name, got);
            else
                snprintf(label, sizeof label, "test %2d, %-15s -> none", t + 1, APPROACHES[a].name);
            check(found == c->expect_found && (!found || got == c->expect), label);
        }
    }

    /* Random cross-check: small values so duplicates and all-equal arrays
     * are common, plus an occasional INT_MIN / INT_MAX to stress the found
     * flag. Every approach must agree with the sort-based version. */
    {
        srand(1414);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums[10];
            int n = rand() % 11;
            for (int k = 0; k < n; k++) {
                int pick = rand() % 12;
                nums[k] = pick == 0 ? INT_MIN : pick == 1 ? INT_MAX : rand() % 7 - 3;
            }

            int expect = 0;
            int expect_found = second_sort(nums, n, &expect);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int got = 0;
                int found = APPROACHES[a].fn(nums, n, &got);
                if (found != expect_found || (found && got != expect))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
