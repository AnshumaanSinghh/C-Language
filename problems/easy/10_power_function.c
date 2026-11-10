/*
 * Q10. Pow(x, n)
 * Difficulty : Easy
 * Topics     : Math, Recursion, Binary Exponentiation, Bit Manipulation
 *
 * Problem
 *   Implement pow(x, n): return x raised to the integer power n, where x
 *   is a double and n is any int, including negative values and INT_MIN.
 *   x^0 = 1 for every x (including 0^0, as in C's pow), and x^-n = 1 / x^n.
 *   When n < 0, x must not be 0. (Same task as LeetCode 50.)
 *
 *   Bonus: the same squaring idea computes a^b mod m for huge b without
 *   overflow, the workhorse of number theory and cryptography.
 *
 * Example
 *   x = 2.0, n = 10    ->   1024.0
 *   x = 2.0, n = -2    ->   0.25
 *
 * Approaches
 *   1. Naive loop                         multiply x, |n| times      Time O(|n|)      Space O(1)
 *   2. Recursive fast power               x^n = (x^(n/2))^2 [* x]    Time O(log |n|)  Space O(log |n|) stack
 *   3. Binary exponentiation  [optimal]   square x, read bits of n   Time O(log |n|)  Space O(1)
 *
 *   Also: modular exponentiation a^b mod m with 64-bit integers, same
 *   technique, Time O(log b).
 *
 * Key insight
 *   x^13 = x^8 * x^4 * x^1 because 13 = 1101 in binary. Squaring x over and
 *   over gives x, x^2, x^4, x^8, ...; multiply in the ones whose bit is set.
 *   That is about 2 log2(n) multiplications instead of n.
 *
 * Pitfall: negating INT_MIN
 *   -INT_MIN does not fit in an int (it is INT_MAX + 1), so "n = -n" is
 *   undefined behaviour for n = INT_MIN. Copy n into a long long first,
 *   then negate.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 10_power_function.c -o power_function && ./power_function
 */

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Approach 1: Naive loop
 *   Multiply 1 by x, |n| times. For a negative n, use 1/x as the base
 *   instead. Simple and correct, but n = INT_MIN means more than two
 *   billion multiplications, so the tests run it only for |n| <= 10000.
 *   Time O(|n|)   Space O(1)
 * ------------------------------------------------------------------------- */
static double pow_naive(double x, int n)
{
    long long e = n; /* widen BEFORE negating: -INT_MIN overflows an int */
    if (e < 0) {
        x = 1.0 / x;
        e = -e;
    }
    double result = 1.0;
    for (long long i = 0; i < e; i++)
        result *= x;
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 2: Recursive fast power
 *   x^e = (x^(e/2))^2 when e is even, and (x^(e/2))^2 * x when e is odd
 *   (integer division). Each call halves e, so there are about log2(e)
 *   calls, i.e. at most 32 for any int exponent.
 *   Pitfall: compute x^(e/2) ONCE and square the stored value. Writing
 *   pow(x, e/2) * pow(x, e/2) makes two calls per level and brings the cost
 *   right back to O(e).
 *   Time O(log |n|)   Space O(log |n|) recursion depth
 * ------------------------------------------------------------------------- */
static double pow_rec_nonneg(double x, long long e) /* requires e >= 0 */
{
    if (e == 0)
        return 1.0;
    double half = pow_rec_nonneg(x, e / 2);
    return e % 2 ? half * half * x : half * half;
}

static double pow_recursive(double x, int n)
{
    long long e = n;
    if (e < 0) {
        x = 1.0 / x;
        e = -e;
    }
    return pow_rec_nonneg(x, e);
}

/* -------------------------------------------------------------------------
 * Approach 3: Iterative binary exponentiation (optimal)
 *   Walk the bits of e from least to most significant. base holds
 *   x^1, x^2, x^4, x^8, ... (square it every step); whenever the current
 *   bit of e is 1, multiply that power into the result.
 *   Example e = 13 = 1101b: result = x^1 * x^4 * x^8 = x^13.
 *   Same number of multiplications as approach 2 but no recursion, so
 *   O(1) space. base is squared only while bits remain, so it does not
 *   needlessly overflow to infinity after the last bit.
 *   Time O(log |n|)   Space O(1)
 * ------------------------------------------------------------------------- */
static double pow_iterative(double x, int n)
{
    long long e = n;
    if (e < 0) {
        x = 1.0 / x;
        e = -e;
    }
    double result = 1.0, base = x;
    while (e > 0) {
        if (e & 1)
            result *= base;
        e >>= 1;
        if (e > 0)
            base *= base;
    }
    return result;
}

/* -------------------------------------------------------------------------
 * Bonus: modular exponentiation  a^b mod m   (b >= 0, m >= 1)
 *   Exactly approach 3, with every multiplication done modulo m. Because
 *   (x * y) mod m = ((x mod m) * (y mod m)) mod m, numbers never grow past
 *   m, and even b = 10^18 needs only about 60 squarings.
 *   The remaining danger is the product itself: two values just below m
 *   multiply to almost m^2, which overflows a long long once m > ~3 * 10^9.
 *   mul_mod() checks for that and, when needed, multiplies by the very same
 *   doubling idea with + in place of *: a * b = sum of a * 2^k over the set
 *   bits k of b, adding modulo m at each step so nothing ever exceeds m.
 *   (C11 has no portable 128-bit integer, so this is the standard fix.)
 *   Time O(log b) multiplications, each O(1) or O(log m)   Space O(1)
 * ------------------------------------------------------------------------- */

/* (x + y) mod m for 0 <= x, y < m, without ever forming x + y >= m. */
static long long add_mod(long long x, long long y, long long m)
{
    return x >= m - y ? x - (m - y) : x + y;
}

/* (a * b) mod m for 0 <= a, b < m, without overflow for any m >= 1. */
static long long mul_mod(long long a, long long b, long long m)
{
    if (a == 0 || b <= LLONG_MAX / a)
        return a * b % m; /* the product fits: do it directly */
    long long result = 0;
    while (b > 0) {
        if (b & 1)
            result = add_mod(result, a, m);
        a = add_mod(a, a, m); /* a * 2^k mod m */
        b >>= 1;
    }
    return result;
}

static long long mod_pow(long long a, long long b, long long m)
{
    a %= m; /* in C, % keeps the sign of a, so fix up negatives */
    if (a < 0)
        a += m;
    long long result = 1 % m; /* 1 % m makes m == 1 give 0 */
    while (b > 0) {
        if (b & 1)
            result = mul_mod(result, a, m);
        a = mul_mod(a, a, m);
        b >>= 1;
    }
    return result;
}

/* Reference for the tests: multiply b times. Same rules as mod_pow. */
static long long mod_pow_naive(long long a, long long b, long long m)
{
    a %= m;
    if (a < 0)
        a += m;
    long long result = 1 % m;
    for (long long i = 0; i < b; i++)
        result = mul_mod(result, a, m);
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

static double my_fabs(double v)
{
    return v < 0 ? -v : v;
}

/* Floating-point results are compared with a RELATIVE tolerance: an error
 * of 1e-12 matters for 0.001 but not for 1e90. Exact equality is accepted
 * first so that 0 == 0 passes. */
static int close_enough(double got, double expect)
{
    if (got == expect)
        return 1;
    double scale = my_fabs(expect) > my_fabs(got) ? my_fabs(expect) : my_fabs(got);
    return my_fabs(got - expect) <= 1e-12 * scale;
}

typedef double (*PowFn)(double x, int n);

static const struct {
    const char *name;
    PowFn fn;
    long long max_abs_n; /* largest |n| this approach is run on in the tests */
} APPROACHES[] = {
    {"naive loop", pow_naive, 10000}, /* O(|n|): keep |n| small */
    {"recursive", pow_recursive, 2147483648LL},
    {"binary exponentiation", pow_iterative, 2147483648LL},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    double x;
    int n;
    int inf_sign; /* +1: result must be +infinity, -1: -infinity, 0: compare with expect */
    double expect;
} PowCase;

typedef struct {
    long long a, b, m;
    long long expect;
} ModCase;

int main(void)
{
    printf("Q10. Pow(x, n)\n");

    /* Expected values by hand: 1.5^5 = 243/32 = 7.59375, 3^19 = 1162261467,
     * 2^-1074 is the smallest positive double (DBL_TRUE_MIN, a subnormal).
     * (+/-1)^n only depends on whether n is even; INT_MIN is even. 2^INT_MIN
     * is far below the smallest double, so it underflows to 0, while
     * 0.5^INT_MIN = 2^(2^31) overflows to +infinity. */
    const PowCase cases[] = {
        {2.0, 10, 0, 1024.0},
        {2.1, 3, 0, 9.261},
        {2.0, -2, 0, 0.25},
        {5.0, 0, 0, 1.0},
        {0.0, 0, 0, 1.0},          /* 0^0 = 1 by convention */
        {0.0, 5, 0, 0.0},
        {-2.0, 3, 0, -8.0},        /* odd power keeps the sign */
        {-2.0, 2, 0, 4.0},
        {-2.0, -3, 0, -0.125},
        {1.5, 5, 0, 7.59375},
        {10.0, -3, 0, 0.001},
        {0.1, 2, 0, 0.01},
        {3.0, 19, 0, 1162261467.0},
        {2.0, 62, 0, 4611686018427387904.0},
        {2.0, -1074, 0, DBL_TRUE_MIN},
        {1.0, INT_MAX, 0, 1.0},
        {1.0, INT_MIN, 0, 1.0},
        {-1.0, INT_MAX, 0, -1.0},  /* INT_MAX is odd */
        {-1.0, INT_MIN, 0, 1.0},   /* INT_MIN is even */
        {2.0, INT_MIN, 0, 0.0},    /* underflow */
        {2.0, INT_MIN + 1, 0, 0.0},
        {0.5, INT_MAX, 0, 0.0},
        {0.5, INT_MIN, +1, 0.0},   /* overflow to +infinity */
        {-0.5, INT_MIN + 1, -1, 0.0}, /* (-2)^(2^31 - 1): odd, so -infinity */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const PowCase *c = &cases[t];
        long long abs_n = c->n < 0 ? -(long long)c->n : c->n;
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (abs_n > APPROACHES[a].max_abs_n)
                continue;
            double got = APPROACHES[a].fn(c->x, c->n);
            int ok;
            if (c->inf_sign > 0)
                ok = got > DBL_MAX;
            else if (c->inf_sign < 0)
                ok = got < -DBL_MAX;
            else
                ok = close_enough(got, c->expect);
            snprintf(label, sizeof label, "pow(%g, %d), %-21s -> %.10g", c->x, c->n,
                     APPROACHES[a].name, got);
            check(ok, label);
        }
    }

    /* Random cross-check against the naive loop: x in [-2, 2], n in
     * [-30, 30] (x = 0 is replaced when n < 0). */
    {
        srand(8675309);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            double x = (rand() % 4001 - 2000) / 1000.0;
            int n = rand() % 61 - 30;
            if (x == 0.0 && n < 0)
                x = 0.5;
            double expect = pow_naive(x, n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (!close_enough(APPROACHES[a].fn(x, n), expect))
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d (x, n) pairs, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    /* Modular exponentiation. Hand derivations:
     *   7^222 mod 10: powers of 7 end in 7, 9, 3, 1 and repeat; 222 = 4*55 + 2 -> 9
     *   Fermat's little theorem: a^(p-1) mod p = 1 for prime p not dividing a
     *     (10^9 + 7, 998244353 and 2^61 - 1 are prime)
     *   2^61 = 1 mod (2^61 - 1), so 2^100 = 2^39 = 549755813888 mod (2^61 - 1)
     *   2^63 = 1 mod (2^63 - 1), so 2^100 = 2^37 = 137438953472 mod LLONG_MAX
     *   10^19 - (2^63 - 1) = 10000000000000000000 - 9223372036854775807
     *                      = 776627963145224193
     *   m - 1 = -1 (mod m), so (m-1)^2 = 1 and (m-1)^3 = m - 1
     * The cases with m near 2^61 or 2^63 would overflow a plain a * a % m. */
    const long long M61 = 2305843009213693951LL; /* 2^61 - 1 */
    const ModCase mcases[] = {
        {2, 10, 1000, 24},
        {3, 0, 7, 1},
        {0, 0, 7, 1},              /* 0^0 = 1 */
        {0, 5, 7, 0},
        {5, 3, 1, 0},              /* everything is 0 mod 1 */
        {0, 0, 1, 0},
        {2, 5, 13, 6},             /* 32 = 2*13 + 6 */
        {-2, 3, 5, 2},             /* -8 = -2*5 + 2 */
        {7, 222, 10, 9},
        {3, 1000000006, 1000000007, 1},
        {2, 998244352, 998244353, 1},
        {2, 61, M61, 1},
        {2, 100, M61, 549755813888LL},
        {3, M61 - 1, M61, 1},
        {2, 100, LLONG_MAX, 137438953472LL},
        {10, 18, LLONG_MAX, 1000000000000000000LL},
        {10, 19, LLONG_MAX, 776627963145224193LL},
        {LLONG_MAX - 1, 2, LLONG_MAX, 1},
        {LLONG_MAX - 1, 3, LLONG_MAX, LLONG_MAX - 1},
    };
    const int num_mcases = (int)(sizeof mcases / sizeof mcases[0]);
    for (int t = 0; t < num_mcases; t++) {
        const ModCase *c = &mcases[t];
        long long got = mod_pow(c->a, c->b, c->m);
        snprintf(label, sizeof label,
                 c->a < 0 ? "(%lld)^%lld mod %lld -> %lld" : "%lld^%lld mod %lld -> %lld", c->a,
                 c->b, c->m, got);
        check(got == c->expect, label);
    }

    /* Random cross-check of mod_pow against multiplying b times. */
    {
        srand(2357);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            long long a = rand() % 2001 - 1000;
            long long b = rand() % 61;
            long long m = 1 + rand() % 1000000;
            if (mod_pow(a, b, m) != mod_pow_naive(a, b, m))
                bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d (a, b, m), mod_pow matches naive",
                 rounds);
        check(bad == 0, label);
    }

    /* Random Fermat checks with the 61-bit prime, which exercise the
     * overflow-safe multiplication: a^(p-1) = 1 and a^p = a (mod p). */
    {
        srand(1009);
        int bad = 0;
        const int rounds = 200;
        for (int r = 0; r < rounds; r++) {
            unsigned long long x = 0;
            for (int k = 0; k < 4; k++)
                x = (x << 16) ^ (unsigned long long)(rand() & 0xFFFF);
            long long a = 1 + (long long)(x % (unsigned long long)(M61 - 1)); /* 1..p-1 */
            if (mod_pow(a, M61 - 1, M61) != 1 || mod_pow(a, M61, M61) != a)
                bad++;
        }
        snprintf(label, sizeof label,
                 "random Fermat check: %d bases, a^(p-1) = 1 mod 2^61 - 1", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
