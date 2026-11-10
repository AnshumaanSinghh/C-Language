/*
 * Q07. Factorial Trailing Zeros
 * Difficulty : Easy
 * Topics     : Math, Number Theory
 *
 * Problem
 *   Given an integer n >= 0, return the number of trailing zeros in the
 *   decimal representation of n! = 1 * 2 * ... * n (with 0! = 1).
 *
 * Example
 *   n = 10   ->   2        because 10! = 3628800
 *   n = 25   ->   6        because 25! = 15511210043330985984000000
 *
 * Approaches
 *   1. Compute n!                  multiply, then strip zeros      Time O(n)        Space O(1)   only n <= 20
 *   2. Count fives in 1..n         factors of 5 in each number     Time O(n)        Space O(1)
 *   3. Legendre  [optimal]         n/5 + n/25 + n/125 + ...        Time O(log n)    Space O(1)
 *
 * Key insight
 *   A trailing zero is a factor 10 = 2 * 5. Among 1..n there are always
 *   more factors of 2 than of 5, so the number of 10s is simply the number
 *   of 5s in the prime factorisation of n!. Multiples of 5 give one each,
 *   multiples of 25 one more, multiples of 125 another, and so on.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 07_factorial_trailing_zeros.c -o factorial_trailing_zeros && ./factorial_trailing_zeros
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define FACT_MAX_N 20 /* 20! = 2432902008176640000 fits in 64 bits, 21! does not */

/* -------------------------------------------------------------------------
 * Approach 1: Compute n! and count its zeros
 *   Multiply 1 * 2 * ... * n, then divide by 10 while the last digit is 0.
 *   Overflow: n! grows so fast that 21! = 51090942171709440000 already
 *   exceeds the 64-bit maximum 18446744073709551615. An unsigned value
 *   would silently wrap around and give a wrong count, so this function
 *   refuses n > 20 and returns -1. It is the "obvious" solution, and it is
 *   useless for any interesting n.
 *   Time O(n)   Space O(1)   valid only for 0 <= n <= 20
 * ------------------------------------------------------------------------- */
static unsigned long long factorial_ull(int n) /* requires 0 <= n <= 20 */
{
    unsigned long long f = 1;
    for (int i = 2; i <= n; i++)
        f *= (unsigned long long)i;
    return f;
}

static int zeros_compute_factorial(int n)
{
    if (n > FACT_MAX_N)
        return -1; /* n! does not fit in unsigned long long */
    unsigned long long f = factorial_ull(n);
    int count = 0;
    while (f % 10 == 0) { /* f >= 1, so this loop always ends */
        count++;
        f /= 10;
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Count the factors of 5 in every number 1..n
 *   By the key insight we only need the number of 5s in n!, which is the
 *   sum over i = 1..n of "how many times 5 divides i". No big numbers are
 *   ever formed, so this works for any n.
 *   Cost: n outer steps, and the inner while loop runs once per factor of
 *   5, which over the whole run is the answer itself (< n / 4) times. So
 *   the total is O(n); the naive bound "log5(n) inner steps per number",
 *   O(n log n), is loose.
 *   i is a long long so that i++ cannot overflow when n == INT_MAX.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int zeros_count_fives(int n)
{
    int count = 0;
    for (long long i = 1; i <= n; i++) {
        long long x = i;
        while (x % 5 == 0) {
            count++;
            x /= 5;
        }
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: Legendre's formula (optimal)
 *   Count the 5s group by group instead of number by number:
 *     floor(n / 5)   numbers in 1..n are multiples of 5    (one 5 each)
 *     floor(n / 25)  of those are multiples of 25          (one more each)
 *     floor(n / 125) are multiples of 125                  (one more each) ...
 *   Summing these counts every factor of 5 exactly once.
 *   Pitfall: computing the powers 5, 25, 125, ... in an int overflows
 *   (5^14 > INT_MAX) when n is near INT_MAX. Dividing n by 5 repeatedly
 *   gives the same terms, because floor(floor(n / 5) / 5) = floor(n / 25),
 *   and never overflows.
 *   Time O(log n), about 13 iterations for n = 2^31 - 1   Space O(1)
 * ------------------------------------------------------------------------- */
static int zeros_legendre(int n)
{
    int count = 0;
    while (n > 0) {
        n /= 5;
        count += n;
    }
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

typedef int (*ZerosFn)(int n);

static const struct {
    const char *name;
    ZerosFn fn;
    int max_n; /* largest n this approach is run on in the tests */
} APPROACHES[] = {
    {"compute n!", zeros_compute_factorial, FACT_MAX_N}, /* overflows beyond 20 */
    {"count fives in 1..n", zeros_count_fives, 1000000},  /* linear: keep n moderate */
    {"Legendre", zeros_legendre, INT_MAX},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int n;
    int expect;
} Case;

int main(void)
{
    printf("Q07. Factorial Trailing Zeros\n");

    /* Small n: from the factorial itself (e.g. 15! = 1307674368000 -> 3).
     * Large n: Legendre's sum by hand, e.g. n = 1000000000:
     *   200000000 + 40000000 + 8000000 + 1600000 + 320000 + 64000 + 12800
     *   + 2560 + 512 + 102 + 20 + 4 = 249999998
     * and n = INT_MAX = 2147483647:
     *   429496729 + 85899345 + 17179869 + 3435973 + 687194 + 137438 + 27487
     *   + 5497 + 1099 + 219 + 43 + 8 + 1 = 536870902 */
    const Case cases[] = {
        {0, 0},                 /* 0! = 1 */
        {1, 0},
        {4, 0},                 /* 24 */
        {5, 1},                 /* 120 */
        {6, 1},                 /* 720 */
        {9, 1},                 /* 362880 */
        {10, 2},                /* 3628800 */
        {14, 2},                /* 87178291200 */
        {15, 3},                /* 1307674368000 */
        {20, 4},                /* 2432902008176640000, the largest computable */
        {24, 4},                /* 24/5 = 4 */
        {25, 6},                /* 5 + 1: 25 = 5 * 5 contributes two fives */
        {30, 7},                /* 6 + 1 */
        {100, 24},              /* 20 + 4 */
        {125, 31},              /* 25 + 5 + 1 */
        {1000, 249},            /* 200 + 40 + 8 + 1 */
        {10000, 2499},          /* 2000 + 400 + 80 + 16 + 3 */
        {1000000, 249998},      /* 200000 + 40000 + 8000 + 1600 + 320 + 64 + 12 + 2 */
        {1000000000, 249999998},
        {INT_MAX, 536870902},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (c->n > APPROACHES[a].max_n)
                continue;
            int got = APPROACHES[a].fn(c->n);
            snprintf(label, sizeof label, "n = %10d, %-19s -> %d", c->n, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* The documented overflow limit of approach 1. */
    check(factorial_ull(20) == 2432902008176640000ULL, "20! = 2432902008176640000 computed exactly");
    check(factorial_ull(20) > ULLONG_MAX / 21, "21! would overflow unsigned long long");
    check(zeros_compute_factorial(21) == -1, "compute n! refuses n = 21 instead of returning garbage");

    /* Random cross-check: every approach against computing n! directly,
     * which limits n to 0..20. */
    {
        srand(555);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int n = rand() % (FACT_MAX_N + 1);
            int expect = zeros_compute_factorial(n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d values of n <= 20, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    /* Wider random cross-check: Legendre against counting fives one number
     * at a time, for n up to 20000 where n! itself cannot be formed. */
    {
        srand(556);
        int bad = 0;
        const int rounds = 200;
        for (int r = 0; r < rounds; r++) {
            int n = rand() % 20001;
            if (zeros_legendre(n) != zeros_count_fives(n))
                bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d values of n <= 20000, Legendre agrees",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
