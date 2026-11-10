/*
 * Q48. First Missing Positive
 * Difficulty : Hard
 * Topics     : Arrays, Hashing, Cyclic Sort, Sorting
 *
 * Problem
 *   Given an unsorted array nums of n integers, return the smallest positive
 *   integer (1, 2, 3, ...) that does not appear in nums. The array may hold
 *   zeros, negatives, duplicates and huge values.
 *
 * Example
 *   nums = [3, 4, -1, 1]   ->   2
 *
 * Approaches
 *   1. Brute force                try 1, 2, 3, ... with a scan each      Time O(n^2)      Space O(1)
 *   2. Sort a copy                sort, then walk up from 1              Time O(n log n)  Space O(n)
 *   3. Boolean array              seen[v] for every v in 1..n            Time O(n)        Space O(n)
 *   4. Cyclic sort    [optimal]   swap each v to index v - 1             Time O(n)        Space O(1)
 *   5. Sign marking               negative nums[v - 1] means "v seen"    Time O(n)        Space O(1)
 *
 * Key insight
 *   The answer is always in 1..n+1. An array of n numbers can hold at most n
 *   different values from 1..n. If it holds all of them, the answer is n+1;
 *   otherwise some value in 1..n is missing and the smallest such is the
 *   answer. So every value <= 0 or > n is irrelevant, and the array itself,
 *   whose indices are 0..n-1, has exactly one slot for each candidate 1..n.
 *   Using those slots as the "seen" table gives O(n) time with O(1) extra
 *   space.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 48_first_missing_positive.c -o first_missing_positive && ./first_missing_positive
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Every approach takes int *nums so they all fit one function-pointer type.
 * Approaches 4 and 5 rearrange or overwrite nums (that is how they get O(1)
 * space); 1-3 only read it. The harness gives each call its own copy.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   For k = 1, 2, 3, ... scan the whole array for k; the first k not found
 *   is the answer. By the key insight the loop stops by k = n + 1.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int first_missing_brute(int *nums, int n)
{
    for (int k = 1;; k++) {
        int found = 0;
        for (int i = 0; i < n && !found; i++)
            if (nums[i] == k)
                found = 1;
        if (!found)
            return k;
    }
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort a copy, then scan
 *   In sorted order the positives appear smallest first. Keep `expect`, the
 *   smallest positive not seen yet (starting at 1). A value equal to expect
 *   moves it up by one; values below expect (negatives, zeros, duplicates)
 *   change nothing; the first value above expect proves expect is missing.
 *   Pitfall: the comparator must not compute x - y; INT_MAX - INT_MIN
 *   overflows.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static int first_missing_sort(int *nums, int n)
{
    if (n == 0)
        return 1;
    int *copy = malloc((size_t)n * sizeof *copy);
    if (copy == NULL)
        return -1;
    memcpy(copy, nums, (size_t)n * sizeof *copy);
    qsort(copy, (size_t)n, sizeof *copy, cmp_int);

    int expect = 1;
    for (int i = 0; i < n; i++) {
        if (copy[i] == expect)
            expect++;
        else if (copy[i] > expect)
            break;
    }
    free(copy);
    return expect;
}

/* -------------------------------------------------------------------------
 * Approach 3: Boolean array of size n + 1
 *   seen[v] = 1 for every v in 1..n that occurs; values outside 1..n can be
 *   ignored (key insight). Then the first v in 1..n with seen[v] == 0 is
 *   the answer, or n + 1 if there is none.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int first_missing_seen(int *nums, int n)
{
    unsigned char *seen = calloc((size_t)n + 1, 1);
    if (seen == NULL)
        return -1;
    for (int i = 0; i < n; i++)
        if (nums[i] >= 1 && nums[i] <= n)
            seen[nums[i]] = 1;

    int answer = n + 1;
    for (int v = 1; v <= n; v++) {
        if (!seen[v]) {
            answer = v;
            break;
        }
    }
    free(seen);
    return answer;
}

/* -------------------------------------------------------------------------
 * Approach 4: Cyclic sort (optimal)
 *   Goal: put every value v in 1..n at index v - 1. At index i, while
 *   nums[i] = v is in range and its home slot does not already hold v, swap
 *   v into its home. The value swapped back into i is then examined too.
 *   Afterwards, the first index i with nums[i] != i + 1 gives the answer
 *   i + 1; if every slot is correct the answer is n + 1.
 *   Why O(n): every swap puts one value into its home slot for good, so
 *   there are at most n swaps in total across all i.
 *   Pitfall: the test must be nums[v - 1] != v, not i != v - 1. With
 *   duplicates ([1, 1]) the home slot already holds v, and the second test
 *   would swap the two equal values forever.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int first_missing_cyclic(int *nums, int n)
{
    for (int i = 0; i < n; i++) {
        while (nums[i] >= 1 && nums[i] <= n && nums[nums[i] - 1] != nums[i]) {
            int home = nums[i] - 1;
            int tmp = nums[home];
            nums[home] = nums[i];
            nums[i] = tmp;
        }
    }
    for (int i = 0; i < n; i++)
        if (nums[i] != i + 1)
            return i + 1;
    return n + 1;
}

/* -------------------------------------------------------------------------
 * Approach 5: Sign marking
 *   Use the SIGN of nums[v - 1] as the "seen v" flag, keeping the value
 *   itself readable through its absolute value.
 *     1. Replace every value outside 1..n with n + 1. Now everything is
 *        positive, so a minus sign can only mean "marked". This also gets
 *        rid of INT_MIN, whose negation would overflow.
 *     2. For each |nums[i]| = v in 1..n, make nums[v - 1] negative (only if
 *        it is still positive, so duplicates do not flip it back).
 *     3. The first index still positive is the first missing value.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int first_missing_sign(int *nums, int n)
{
    for (int i = 0; i < n; i++)
        if (nums[i] < 1 || nums[i] > n)
            nums[i] = n + 1;
    for (int i = 0; i < n; i++) {
        int v = nums[i] < 0 ? -nums[i] : nums[i];
        if (v <= n && nums[v - 1] > 0)
            nums[v - 1] = -nums[v - 1];
    }
    for (int i = 0; i < n; i++)
        if (nums[i] > 0)
            return i + 1;
    return n + 1;
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

typedef int (*FirstMissingFn)(int *nums, int n);

/* quadratic: kept off the large test (n = 100000 would take ~10^10 steps). */
static const struct {
    const char *name;
    FirstMissingFn fn;
    int quadratic;
} APPROACHES[] = {
    {"brute force", first_missing_brute, 1},
    {"sort a copy", first_missing_sort, 0},
    {"boolean array", first_missing_seen, 0},
    {"cyclic sort", first_missing_cyclic, 0},
    {"sign marking", first_missing_sign, 0},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[10];
    int n;
    int expect;
} Case;

/* Copies nums so approaches that modify their input cannot affect others. */
static int run_on_copy(FirstMissingFn fn, const int *nums, int n)
{
    int *copy = malloc((size_t)(n > 0 ? n : 1) * sizeof *copy);
    if (copy == NULL)
        return -1;
    if (n > 0)
        memcpy(copy, nums, (size_t)n * sizeof *copy);
    int result = fn(copy, n);
    free(copy);
    return result;
}

int main(void)
{
    printf("Q48. First Missing Positive\n");

    /* Expected answers worked out by hand. */
    const Case cases[] = {
        {{1, 2, 0}, 3, 3},
        {{3, 4, -1, 1}, 4, 2},
        {{7, 8, 9, 11, 12}, 5, 1},           /* nothing in 1..n             */
        {{0}, 0, 1},                         /* empty array                 */
        {{1}, 1, 2},
        {{2}, 1, 1},
        {{0}, 1, 1},
        {{1, 1}, 2, 2},                      /* duplicates                  */
        {{2, 2, 2, 2}, 4, 1},                /* all equal                   */
        {{-1, -2, -3}, 3, 1},                /* all negative                */
        {{INT_MIN, INT_MAX}, 2, 1},          /* extremes, none in range     */
        {{INT_MIN, 1, INT_MAX, 2}, 4, 3},    /* negating INT_MIN would overflow */
        {{1, 2, 3, 4, 5}, 5, 6},             /* already 1..n -> n + 1       */
        {{5, 4, 3, 2, 1}, 5, 6},             /* 1..n reversed               */
        {{2, 3, 1, 5, 5}, 5, 4},             /* duplicate hides a gap       */
        {{1, 1000000000}, 2, 2},             /* huge value out of range     */
        {{3, 1, 2, 3, 1, 2}, 6, 4},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = run_on_copy(APPROACHES[a].fn, c->nums, c->n);
            snprintf(label, sizeof label, "test %2d, %-14s -> %d", t + 1, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Large: n = 100000 holding 1..n in reverse order, except the slot that
     * held 777 now holds 0. So 1..776 are present and 777 is missing. */
    {
        const int n = 100000;
        int *nums = malloc((size_t)n * sizeof *nums);
        if (nums != NULL) {
            for (int i = 0; i < n; i++)
                nums[i] = n - i == 777 ? 0 : n - i;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (APPROACHES[a].quadratic)
                    continue;
                int got = run_on_copy(APPROACHES[a].fn, nums, n);
                snprintf(label, sizeof label, "n = 100000, %-14s -> %d", APPROACHES[a].name, got);
                check(got == 777, label);
            }
        } else {
            check(0, "n = 100000: out of memory");
        }
        free(nums);
    }

    /* Random cross-check against brute force. Values in -5..15 with n <= 12
     * give a good mix of in-range values, duplicates and out-of-range junk. */
    {
        srand(4848);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int nums[12] = {0};
            int n = rand() % 13;
            for (int k = 0; k < n; k++)
                nums[k] = rand() % 21 - 5;

            int expect = run_on_copy(first_missing_brute, nums, n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (run_on_copy(APPROACHES[a].fn, nums, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
