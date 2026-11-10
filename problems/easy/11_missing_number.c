/*
 * Q11. Missing Number
 * Difficulty : Easy
 * Topics     : Arrays, Math, Bit Manipulation
 *
 * Problem
 *   You are given an array nums of n distinct integers, each one in the range
 *   0..n. That range holds n + 1 numbers, so exactly one of them is absent.
 *   Return the missing number.
 *
 * Example
 *   nums = [3, 0, 1]   ->   2    because the range is 0..3 and only 2 is absent
 *
 * Approaches
 *   1. Linear search          look for every k in 0..n          Time O(n^2)      Space O(1)
 *   2. Sort                   sort a copy, first a[i] != i      Time O(n log n)  Space O(n)
 *   3. Seen array             mark each value, find the gap     Time O(n)        Space O(n)
 *   4. Sum formula            0 + 1 + ... + n minus the sum     Time O(n)        Space O(1)
 *   5. XOR        [optimal]   XOR every index and every value   Time O(n)        Space O(1)
 *
 * Key insight
 *   You already know the complete range 0..n, so the answer is "range minus
 *   array". Addition can compute that difference, but the running total grows
 *   like n^2 / 2 and overflows int. XOR cancels values the same way (x ^ x = 0)
 *   and never leaves 32 bits, so it cannot overflow at all.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 11_missing_number.c -o missing_number && ./missing_number
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Linear search (brute force)
 *   Ask "is k in the array?" for k = 0, 1, ..., n. The first k that is not
 *   found is the answer. It is correct by definition, which makes it the
 *   reference the other approaches are tested against.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int missing_linear(const int *nums, int n)
{
    for (int k = 0; k <= n; k++) {
        int found = 0;
        for (int i = 0; i < n && !found; i++)
            found = nums[i] == k;
        if (!found)
            return k;
    }
    return -1; /* unreachable for valid input */
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort a copy
 *   After sorting, a complete prefix looks like 0, 1, 2, ... so a[i] == i.
 *   The first index where a[i] != i is the gap. If there is no gap, every
 *   value 0..n-1 is present and the missing one is n.
 *   We sort a copy because the input is const and callers may not expect
 *   their array to be reordered.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y); /* not x - y: that overflows for big gaps */
}

static int missing_sort(const int *nums, int n)
{
    if (n == 0)
        return 0; /* range is just {0} and the array is empty */

    int *a = malloc((size_t)n * sizeof *a);
    if (a == NULL)
        return -1;
    memcpy(a, nums, (size_t)n * sizeof *a);
    qsort(a, (size_t)n, sizeof *a, cmp_int);

    int i = 0;
    while (i < n && a[i] == i)
        i++;
    free(a);
    return i;
}

/* -------------------------------------------------------------------------
 * Approach 3: Seen array
 *   Keep one flag per possible value 0..n (that is n + 1 flags, not n).
 *   Mark every value in the array, then return the one flag left unmarked.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int missing_seen(const int *nums, int n)
{
    unsigned char *seen = calloc((size_t)n + 1, 1);
    if (seen == NULL)
        return -1;

    for (int i = 0; i < n; i++)
        if (nums[i] >= 0 && nums[i] <= n) /* guard: never write out of bounds */
            seen[nums[i]] = 1;

    int k = 0;
    while (k < n && seen[k])
        k++;
    free(seen);
    return k;
}

/* -------------------------------------------------------------------------
 * Approach 4: Gauss sum formula
 *   0 + 1 + ... + n = n(n + 1) / 2. Subtract the array's sum from it and the
 *   only thing left is the missing value.
 *   Pitfall: both totals must be long long. In int, n * (n + 1) already
 *   overflows at n = 46,341 (even though the halved result would still fit),
 *   and the total itself passes INT_MAX at n = 65,536. Signed overflow is
 *   undefined behaviour in C, not just a wrong answer.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int missing_sum(const int *nums, int n)
{
    long long expected = (long long)n * ((long long)n + 1) / 2;
    long long actual = 0;
    for (int i = 0; i < n; i++)
        actual += nums[i];
    return (int)(expected - actual);
}

/* -------------------------------------------------------------------------
 * Approach 5: XOR (optimal)
 *   XOR has three useful rules: x ^ x = 0, x ^ 0 = x, and the order of the
 *   operands does not matter. XOR together all of 0..n and all of nums:
 *   every present value shows up twice and cancels, the missing one shows up
 *   once and survives. Indices 0..n-1 plus the extra n give us 0..n for free.
 *   No sums are formed, so nothing can overflow, whatever n is.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int missing_xor(const int *nums, int n)
{
    int x = n;
    for (int i = 0; i < n; i++)
        x ^= i ^ nums[i];
    return x;
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

typedef int (*MissingFn)(const int *nums, int n);

static const struct {
    const char *name;
    MissingFn fn;
    int quadratic; /* 1 = too slow for the large test */
} APPROACHES[] = {
    {"linear search", missing_linear, 1},
    {"sort", missing_sort, 0},
    {"seen array", missing_seen, 0},
    {"sum formula", missing_sum, 0},
    {"xor", missing_xor, 0},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int nums[10];
    int n;
    int expect;
} Case;

int main(void)
{
    printf("Q11. Missing Number\n");

    const Case cases[] = {
        {{3, 0, 1}, 3, 2},
        {{0, 1}, 2, 2},                         /* missing value is n itself */
        {{9, 6, 4, 2, 3, 5, 7, 0, 1}, 9, 8},
        {{0}, 0, 0},                            /* empty array: range is {0} */
        {{0}, 1, 1},
        {{1}, 1, 0},
        {{1, 2, 3, 4}, 4, 0},                   /* missing value is 0 */
        {{4, 2, 0, 3}, 4, 1},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->nums, c->n);
            snprintf(label, sizeof label, "test %d, %-14s -> %d", t + 1, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Large case: n = 70,000 makes 0 + 1 + ... + n = 2,450,035,000, which is
     * above INT_MAX, so a sum kept in int would overflow here. The values
     * are stored in descending order so sorting has real work to do. The
     * O(n^2) linear search would need about 2.5 billion steps, so it is
     * skipped to keep the run under a second. */
    {
        const int n = 70000, missing = 12345;
        int *nums = malloc((size_t)n * sizeof *nums);
        if (nums == NULL) {
            check(0, "large case: allocation failed");
        } else {
            int w = 0;
            for (int v = n; v >= 0; v--)
                if (v != missing)
                    nums[w++] = v;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (APPROACHES[a].quadratic)
                    continue;
                int got = APPROACHES[a].fn(nums, n);
                snprintf(label, sizeof label, "large n = %d, %-14s -> %d", n, APPROACHES[a].name, got);
                check(got == missing, label);
            }
            free(nums);
        }
    }

    /* Random cross-check: shuffle 0..n, drop the last element, and make sure
     * every approach finds the dropped value and agrees with linear search. */
    {
        srand(1111);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int full[21];
            int n = rand() % 21; /* 0..20, so full holds n + 1 <= 21 values */
            for (int k = 0; k <= n; k++)
                full[k] = k;
            for (int k = n; k > 0; k--) { /* Fisher-Yates shuffle */
                int j = rand() % (k + 1);
                int tmp = full[k];
                full[k] = full[j];
                full[j] = tmp;
            }
            int dropped = full[n]; /* nums is full[0..n-1] */

            int expect = missing_linear(full, n);
            if (expect != dropped)
                bad++;
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(full, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
