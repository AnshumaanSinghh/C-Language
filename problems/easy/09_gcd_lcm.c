/*
 * Q09. GCD and LCM
 * Difficulty : Easy
 * Topics     : Math, Number Theory, Recursion, Bit Manipulation
 *
 * Problem
 *   Given two non-negative ints a and b, return their greatest common
 *   divisor gcd(a, b) and their least common multiple lcm(a, b).
 *   Conventions: gcd(a, 0) = a (every number divides 0), so gcd(0, 0) = 0;
 *   lcm(a, 0) = 0.
 *
 * Example
 *   a = 12, b = 18   ->   gcd = 6, lcm = 36
 *
 * Approaches (GCD)
 *   1. Brute force                  try d = min(a,b) down to 1    Time O(min(a,b))        Space O(1)
 *   2. Euclid by subtraction        gcd(a,b) = gcd(a-b, b)        Time O(max(a,b))        Space O(1)
 *   3. Euclid by modulo  [optimal]  gcd(a,b) = gcd(b, a mod b)    Time O(log min(a,b))    Space O(1) loop,
 *      (iterative and recursive)                                                          O(log) recursion
 *   4. Binary GCD (Stein)           halve evens, subtract odds    Time O(log a + log b)   Space O(1)
 *
 *   LCM uses any GCD: lcm(a, b) = a / gcd(a, b) * b, computed in long long.
 *
 * Key insight
 *   Any number that divides both a and b also divides a - b, and therefore
 *   a mod b (which is a minus some multiple of b). So the pair (a, b) can be
 *   replaced by the much smaller pair (b, a mod b) without changing the gcd.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 09_gcd_lcm.c -o gcd_lcm && ./gcd_lcm
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* All GCD functions below require a >= 0 and b >= 0. */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   The gcd cannot exceed the smaller number, so try every candidate from
 *   min(a, b) downwards and return the first one that divides both.
 *   Zero must be handled first: every d divides 0, so gcd(a, 0) = a, and
 *   min(a, 0) = 0 would leave nothing to try.
 *   Time O(min(a, b)), e.g. a million steps for two coprime numbers near
 *   a million   Space O(1)
 * ------------------------------------------------------------------------- */
static int gcd_brute(int a, int b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;
    for (int d = a < b ? a : b; d > 1; d--)
        if (a % d == 0 && b % d == 0)
            return d;
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Euclid's algorithm by repeated subtraction
 *   Euclid's original form. If d divides a and b it divides a - b, and if
 *   d divides a - b and b it divides a. So gcd(a, b) = gcd(a - b, b) for
 *   a > b. Keep subtracting the smaller from the larger until they are
 *   equal; that common value is the gcd.
 *   Pitfall: with a zero argument the loop never ends (a - 0 = a), so
 *   zeros are handled up front. Also slow when the numbers are lopsided:
 *   gcd(1000000, 1) takes 999999 subtractions.
 *   Time O(max(a, b)) in the worst case   Space O(1)
 * ------------------------------------------------------------------------- */
static int gcd_subtract(int a, int b)
{
    if (a == 0)
        return b;
    if (b == 0)
        return a;
    while (a != b) {
        if (a > b)
            a -= b;
        else
            b -= a;
    }
    return a;
}

/* -------------------------------------------------------------------------
 * Approach 3: Euclid's algorithm by modulo (optimal)
 *   Subtracting b from a until a < b is exactly what a % b computes, in one
 *   operation. So gcd(a, b) = gcd(b, a % b), and gcd(a, 0) = a ends it.
 *   If a < b, the first step just swaps them (a % b == a).
 *   Speed: after two steps the larger number has at least halved, so the
 *   number of steps is O(log min(a, b)). The worst inputs are consecutive
 *   Fibonacci numbers (Lame's theorem); even F(46) and F(45), the largest
 *   pair in an int, take only 44 steps.
 *   Written both ways: the loop uses O(1) space, the recursion is the
 *   formula word for word but uses one stack frame per step.
 *   Time O(log min(a, b))   Space O(1) iterative, O(log min(a, b)) recursive
 * ------------------------------------------------------------------------- */
static int gcd_euclid_iter(int a, int b)
{
    while (b != 0) {
        int r = a % b;
        a = b;
        b = r;
    }
    return a;
}

static int gcd_euclid_rec(int a, int b)
{
    return b == 0 ? a : gcd_euclid_rec(b, a % b);
}

/* -------------------------------------------------------------------------
 * Approach 4: Binary GCD (Stein's algorithm)
 *   Uses only shifts, subtraction and comparison, no division (which was
 *   slow on older hardware). Three facts drive it:
 *     gcd(2u, 2v) = 2 * gcd(u, v)      both even: pull out a common 2
 *     gcd(2u, v)  = gcd(u, v), v odd   2 is not a common factor, drop it
 *     gcd(u, v)   = gcd(u, v - u)      Euclid's subtraction; for odd u, v
 *                                      the difference is even, so the next
 *                                      step can halve it again
 *   Every round removes at least one bit from one of the numbers.
 *   Unsigned types make the shifts well defined.
 *   Time O(log a + log b) rounds   Space O(1)
 * ------------------------------------------------------------------------- */
static int gcd_binary(int a, int b)
{
    unsigned int u = (unsigned int)a, v = (unsigned int)b;
    if (u == 0)
        return b;
    if (v == 0)
        return a;

    int shift = 0;
    while (((u | v) & 1u) == 0) { /* both even */
        u >>= 1;
        v >>= 1;
        shift++;
    }
    while ((u & 1u) == 0) /* from here on u is odd */
        u >>= 1;
    do {
        while ((v & 1u) == 0)
            v >>= 1;
        if (u > v) { /* keep u <= v so that v - u does not wrap */
            unsigned int t = u;
            u = v;
            v = t;
        }
        v -= u; /* odd - odd = even */
    } while (v != 0);
    return (int)(u << shift); /* restore the common factors of 2 */
}

/* -------------------------------------------------------------------------
 * LCM from GCD
 *   a * b = gcd(a, b) * lcm(a, b), so lcm = a * b / gcd. Two pitfalls:
 *   - a * b in int overflows long before the lcm does. Divide first
 *     (a / gcd is exact) and do the multiplication in long long. The
 *     largest possible result, about (2^31)^2 = 2^62, fits.
 *   - With a zero argument gcd may be 0, so return 0 before dividing.
 *   Time O(log min(a, b))   Space O(1)
 * ------------------------------------------------------------------------- */
static long long lcm(int a, int b)
{
    if (a == 0 || b == 0)
        return 0;
    return (long long)(a / gcd_euclid_iter(a, b)) * b;
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

typedef int (*GcdFn)(int a, int b);

static const struct {
    const char *name;
    GcdFn fn;
    int max_input; /* largest a or b this approach is run on in the tests */
} APPROACHES[] = {
    {"brute force", gcd_brute, 1000000},     /* linear in min(a, b) */
    {"subtraction", gcd_subtract, 1000000},  /* linear in max(a, b) */
    {"Euclid iterative", gcd_euclid_iter, INT_MAX},
    {"Euclid recursive", gcd_euclid_rec, INT_MAX},
    {"binary (Stein)", gcd_binary, INT_MAX},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int a, b;
    int expect;
} GcdCase;

typedef struct {
    int a, b;
    long long expect;
} LcmCase;

/* Smallest positive multiple of a that b divides: the definition of lcm. */
static long long lcm_brute(int a, int b)
{
    if (a == 0 || b == 0)
        return 0;
    long long m = a;
    while (m % b != 0)
        m += a;
    return m;
}

/* A random int in 0..INT_MAX that does not depend on RAND_MAX being large. */
static int rand_int31(void)
{
    unsigned int x = ((unsigned int)rand() << 16) ^ (unsigned int)rand();
    return (int)(x & (unsigned int)INT_MAX);
}

int main(void)
{
    printf("Q09. GCD and LCM\n");

    const GcdCase gcases[] = {
        {0, 0, 0},                   /* convention */
        {0, 7, 7},                   /* gcd(0, b) = b */
        {7, 0, 7},
        {1, 1, 1},
        {13, 13, 13},                /* equal */
        {12, 18, 6},
        {18, 12, 6},                 /* order does not matter */
        {17, 5, 1},                  /* coprime */
        {100, 10, 10},               /* one divides the other */
        {48, 180, 12},               /* 48 = 2^4 * 3, 180 = 2^2 * 3^2 * 5 */
        {1071, 462, 21},             /* 1071 = 2*462 + 147, 462 = 3*147 + 21, 147 = 7*21 */
        {1024, 96, 32},              /* 2^10 and 2^5 * 3 */
        {1, 1000000, 1},             /* subtraction needs 999999 steps */
        {1000000, 999999, 1},        /* consecutive numbers are coprime */
        {1836311903, 1134903170, 1}, /* F(46), F(45): Euclid's worst case */
        {INT_MAX, INT_MAX, INT_MAX},
        {INT_MAX, INT_MAX - 1, 1},   /* consecutive */
        {2147483646, 1073741823, 1073741823}, /* 2147483646 = 2 * 1073741823 */
        {2147483646, 6, 6},          /* 2147483646 = 6 * 357913941 */
        {INT_MAX, 0, INT_MAX},
        {1073741824, 1048576, 1048576}, /* 2^30 and 2^20 */
    };
    const int num_gcases = (int)(sizeof gcases / sizeof gcases[0]);

    char label[160];
    for (int t = 0; t < num_gcases; t++) {
        const GcdCase *c = &gcases[t];
        int biggest = c->a > c->b ? c->a : c->b;
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (biggest > APPROACHES[a].max_input)
                continue;
            int got = APPROACHES[a].fn(c->a, c->b);
            snprintf(label, sizeof label, "gcd(%d, %d), %-16s -> %d", c->a, c->b,
                     APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* LCM. 65536 * 65537 = 2^32 + 2^16 and (2^31 - 1)(2^31 - 2) = 2^62 - 3 * 2^31 + 2
     * do not fit in int, which is why lcm() returns long long. */
    const LcmCase lcases[] = {
        {0, 0, 0},
        {0, 5, 0},
        {5, 0, 0},
        {1, 1, 1},
        {7, 7, 7},
        {4, 6, 12},
        {21, 6, 42},
        {12, 18, 36},
        {17, 5, 85},
        {INT_MAX, INT_MAX, INT_MAX},
        {2147483646, 1073741823, 2147483646},
        {65536, 65537, 4295032832LL},
        {INT_MAX, INT_MAX - 1, 4611686011984936962LL},
    };
    const int num_lcases = (int)(sizeof lcases / sizeof lcases[0]);
    for (int t = 0; t < num_lcases; t++) {
        const LcmCase *c = &lcases[t];
        long long got = lcm(c->a, c->b);
        snprintf(label, sizeof label, "lcm(%d, %d) -> %lld", c->a, c->b, got);
        check(got == c->expect, label);
    }

    /* Random cross-check: every GCD approach against brute force on small
     * values, with zeros mixed in. */
    {
        srand(2718);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int a = rand() % 10 == 0 ? 0 : rand() % 2001;
            int b = rand() % 10 == 0 ? 0 : rand() % 2001;
            int expect = gcd_brute(a, b);
            for (int k = 0; k < NUM_APPROACHES; k++)
                if (APPROACHES[k].fn(a, b) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d small pairs, all GCD approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    /* Random cross-check of lcm() against its definition. */
    {
        srand(3141);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int a = rand() % 301, b = rand() % 301;
            if (lcm(a, b) != lcm_brute(a, b))
                bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d pairs, lcm matches its definition",
                 rounds);
        check(bad == 0, label);
    }

    /* Large random pairs, where only the fast approaches can run. Half the
     * pairs share a random factor g so the answers are not always 1. Each
     * result must divide both numbers and the three fast versions must
     * agree. */
    {
        srand(1618);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int a = rand_int31(), b = rand_int31();
            if (rand() % 2) {
                int g = 1 + rand() % 1000;
                a = g * (a % (INT_MAX / g));
                b = g * (b % (INT_MAX / g));
            }
            int g1 = gcd_euclid_iter(a, b);
            if (gcd_euclid_rec(a, b) != g1 || gcd_binary(a, b) != g1)
                bad++;
            if (g1 != 0 && (a % g1 != 0 || b % g1 != 0))
                bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d large pairs, fast GCDs agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
