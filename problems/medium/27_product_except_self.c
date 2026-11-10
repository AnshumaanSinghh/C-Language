/*
 * Q27. Product of Array Except Self
 * Difficulty : Medium
 * Topics     : Arrays, Prefix Products
 *
 * Problem
 *   Given an array nums of n integers, fill out[i] with the product of
 *   every element except nums[i]. Results are long long. For n == 1 the
 *   product of "no other elements" is the empty product, 1.
 *   The classic version (LeetCode 238) forbids division and asks for O(n)
 *   time and O(1) extra space (the output array does not count).
 *
 * Example
 *   nums = [1, 2, 3, 4]   ->   [24, 12, 8, 6]
 *
 * Approaches
 *   1. Brute force               multiply all j != i               Time O(n^2)  Space O(1)
 *   2. Division                  total / nums[i], handle zeros     Time O(n)    Space O(1)
 *   3. Prefix + suffix arrays    left[i] * right[i]                Time O(n)    Space O(n)
 *   4. Running suffix [optimal]  out = prefixes, times a suffix    Time O(n)    Space O(1) extra
 *
 * Key insight
 *   out[i] = (product of everything left of i) * (product of everything
 *   right of i). Both halves can be built incrementally, and the output
 *   array itself can hold the left halves while a single variable carries
 *   the right half back across the array.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 27_product_except_self.c -o product_except_self && ./product_except_self
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Every approach writes n results to out and returns 1, or returns 0 if it
 * ran out of memory. Products must fit in long long; the tests keep values
 * small enough for that.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   For each i, multiply every nums[j] with j != i.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int product_brute(const int *nums, int n, long long *out)
{
    for (int i = 0; i < n; i++) {
        long long p = 1;
        for (int j = 0; j < n; j++)
            if (j != i)
                p *= nums[j];
        out[i] = p;
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Division, with zero handling
 *   Multiply all NON-zero elements into nz and count the zeros:
 *     0 zeros  -> out[i] = nz / nums[i]  (exact: nums[i] divides nz)
 *     1 zero   -> only the zero's slot is non-zero: it gets nz; all others
 *                 include that zero, so they are 0
 *     2+ zeros -> every product includes at least one zero: all 0
 *   Why interviewers usually forbid it:
 *     - the problem statement says so; the point is to find the
 *       prefix/suffix idea, not to undo a multiplication;
 *     - the total can overflow even when every answer fits, e.g. three
 *       copies of INT_MIN: each answer is 2^62 but the total is -2^93;
 *     - zeros need special cases (dividing by zero is undefined);
 *     - with floating-point values, division adds rounding error.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int product_division(const int *nums, int n, long long *out)
{
    long long nz = 1;
    int zeros = 0;
    for (int i = 0; i < n; i++) {
        if (nums[i] == 0)
            zeros++;
        else
            nz *= nums[i];
    }
    for (int i = 0; i < n; i++) {
        if (zeros == 0)
            out[i] = nz / nums[i];
        else if (zeros == 1)
            out[i] = nums[i] == 0 ? nz : 0;
        else
            out[i] = 0;
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 3: Prefix and suffix product arrays
 *   left[i]  = nums[0] * ... * nums[i-1]      (left[0] = 1, empty product)
 *   right[i] = nums[i+1] * ... * nums[n-1]    (right[n-1] = 1)
 *   Each is built in one pass from the previous entry, and
 *   out[i] = left[i] * right[i]. No division, so zeros need no special case.
 *   Time O(n)   Space O(n) for the two arrays
 * ------------------------------------------------------------------------- */
static int product_prefix_suffix(const int *nums, int n, long long *out)
{
    if (n <= 0)
        return 1;
    long long *left = malloc((size_t)n * sizeof *left);
    long long *right = malloc((size_t)n * sizeof *right);
    if (left == NULL || right == NULL) {
        free(left);
        free(right);
        return 0;
    }
    left[0] = 1;
    for (int i = 1; i < n; i++)
        left[i] = left[i - 1] * nums[i - 1];
    right[n - 1] = 1;
    for (int i = n - 2; i >= 0; i--)
        right[i] = right[i + 1] * nums[i + 1];
    for (int i = 0; i < n; i++)
        out[i] = left[i] * right[i];
    free(left);
    free(right);
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 4: Output array holds the prefixes, running suffix (optimal)
 *   Same formula as approach 3 without the two extra arrays:
 *     pass 1 (left to right):  out[i] = out[i-1] * nums[i-1], out[0] = 1,
 *                              so out[i] = product of nums[0..i-1]
 *     pass 2 (right to left):  suffix = product of nums[i+1..n-1], kept in
 *                              one variable; out[i] *= suffix
 *   Pitfall: the common "prefix *= nums[i]" loop also multiplies in the
 *   last element, computing the product of ALL n values, which nobody
 *   needs and which can overflow (see the INT_MIN test) even though every
 *   answer fits. Both passes here stop one element short.
 *   Time O(n)   Space O(1) extra (the output array is required anyway)
 * ------------------------------------------------------------------------- */
static int product_running(const int *nums, int n, long long *out)
{
    if (n <= 0)
        return 1;
    out[0] = 1;
    for (int i = 1; i < n; i++)
        out[i] = out[i - 1] * nums[i - 1];
    long long suffix = 1;
    for (int i = n - 2; i >= 0; i--) {
        suffix *= nums[i + 1];
        out[i] *= suffix;
    }
    return 1;
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

typedef int (*ProductFn)(const int *nums, int n, long long *out);

static const struct {
    const char *name;
    ProductFn fn;
} APPROACHES[] = {
    {"brute force", product_brute},
    {"division", product_division},
    {"prefix + suffix", product_prefix_suffix},
    {"running suffix", product_running},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 10

typedef struct {
    int nums[MAXN];
    int n;
    long long expect[MAXN];
} Case;

static int same(const long long *a, const long long *b, int n)
{
    for (int i = 0; i < n; i++)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int main(void)
{
    printf("Q27. Product of Array Except Self\n");

    /* Expected outputs multiplied out by hand. */
    const Case cases[] = {
        {{1, 2, 3, 4}, 4, {24, 12, 8, 6}},
        {{-1, 1, 0, -3, 3}, 5, {0, 0, 9, 0, 0}},            /* one zero */
        {{0, 4, 0}, 3, {0, 0, 0}},                          /* two zeros */
        {{0, 0}, 2, {0, 0}},
        {{2, 3}, 2, {3, 2}},
        {{5}, 1, {1}},                                      /* single: empty product */
        {{0}, 1, {1}},                                      /* single zero */
        {{0}, 0, {0}},                                      /* empty */
        {{-2, -3, -4}, 3, {12, 8, 6}},                      /* negatives */
        {{2, 2, 2}, 3, {4, 4, 4}},                          /* all equal */
        {{100000, 100000, 100000}, 3,                       /* 10^10 overflows int */
         {10000000000LL, 10000000000LL, 10000000000LL}},
        {{INT_MIN, 2, -1}, 3, {-2, 2147483648LL, -4294967296LL}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            long long out[MAXN] = {0};
            int ok = APPROACHES[a].fn(c->nums, c->n, out);
            snprintf(label, sizeof label, "test %2d, %-15s n=%d", t + 1, APPROACHES[a].name, c->n);
            check(ok && same(out, c->expect, c->n), label);
        }
    }

    /* Each answer fits in long long but the product of ALL elements does
     * not: (-2^31)^2 = 2^62 fits, (-2^31)^3 = -2^93 does not. Division needs
     * that total, so it is skipped here; the other approaches never form it. */
    {
        const int nums[] = {INT_MIN, INT_MIN, INT_MIN};
        const long long expect[] = {4611686018427387904LL, 4611686018427387904LL,
                                    4611686018427387904LL}; /* 2^62 */
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (APPROACHES[a].fn == product_division)
                continue;
            long long out[3] = {0};
            int ok = APPROACHES[a].fn(nums, 3, out);
            snprintf(label, sizeof label, "total overflows, %-15s -> 2^62 each", APPROACHES[a].name);
            check(ok && same(out, expect, 3), label);
        }
    }

    /* Random cross-check against brute force. Values in [-5, 5] and n <= 10
     * keep every product (and the division approach's total) far inside
     * long long: 5^10 is about 10^7. Zero appears often, so the 0 / 1 / 2+
     * zero branches all get exercised. */
    {
        srand(2727);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int nums[MAXN];
            int n = rand() % (MAXN + 1);
            for (int i = 0; i < n; i++)
                nums[i] = rand() % 11 - 5;
            long long expect[MAXN] = {0};
            product_brute(nums, n, expect);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                long long out[MAXN] = {0};
                if (!APPROACHES[a].fn(nums, n, out) || !same(out, expect, n))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
