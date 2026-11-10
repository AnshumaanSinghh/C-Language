/*
 * Q08. Check Prime
 * Difficulty : Easy
 * Topics     : Math, Number Theory, Sieve
 *
 * Problem
 *   Given an int n, return 1 if n is prime and 0 otherwise. A prime is an
 *   integer greater than 1 whose only positive divisors are 1 and itself,
 *   so every n <= 1 (including all negative numbers) is not prime.
 *
 * Example
 *   n = 97   ->   1
 *   n = 91   ->   0        because 91 = 7 * 13
 *
 * Approaches
 *   1. Trial division to n-1        try every d in 2..n-1           Time O(n)           Space O(1)
 *   2. Trial division to sqrt(n)    stop once d * d > n             Time O(sqrt n)      Space O(1)
 *   3. 6k +/- 1 wheel  [optimal]    skip multiples of 2 and 3       Time O(sqrt n)      Space O(1)
 *   4. Sieve of Eratosthenes        precompute, then look up        Time O(N log log N) once, O(1) per query
 *                                                                   Space O(N)
 *   Approach 3 is the best for a single query. For many queries with
 *   n <= N, build the sieve once and every query becomes an array lookup.
 *
 * Key insight
 *   Divisors come in pairs d * (n / d) = n, and the smaller one of each pair
 *   is at most sqrt(n). So if nothing in 2..sqrt(n) divides n, nothing
 *   larger can either.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 08_check_prime.c -o check_prime && ./check_prime
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Instrumentation: approaches 1-3 do their trial divisions through
 * divides(), which counts them, so the tests can show how many each one
 * needs for the same n. */
static long long trial_divisions = 0;

static int divides(int d, int n)
{
    trial_divisions++;
    return n % d == 0;
}

/* -------------------------------------------------------------------------
 * Approach 1: Trial division by every number 2..n-1
 *   The definition, literally: if any d with 1 < d < n divides n, it is
 *   composite. A prime near INT_MAX would take about 2 billion divisions,
 *   so the tests only run this on n <= 100000.
 *   Handle n < 2 first: it also keeps "i < n" from ever needing n - 1,
 *   which would overflow for n = INT_MIN.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int is_prime_naive(int n)
{
    if (n < 2)
        return 0;
    for (int i = 2; i < n; i++)
        if (divides(i, n))
            return 0;
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 2: Trial division up to sqrt(n)
 *   If n = a * b with 1 < a <= b, then a * a <= a * b = n, so a <= sqrt(n).
 *   Every composite number therefore has a divisor no larger than its
 *   square root, and we can stop there.
 *   Without math.h (and without floating-point rounding worries) the bound
 *   is written as i <= n / i, which for positive ints is the same test as
 *   i * i <= n. Pitfall: i * i itself overflows. For the prime n = INT_MAX
 *   the loop reaches i = 46341, and 46341 * 46341 = 2147488281 > INT_MAX,
 *   which is undefined behaviour for a signed int.
 *   Time O(sqrt n), at most about 46340 divisions for any int   Space O(1)
 * ------------------------------------------------------------------------- */
static int is_prime_sqrt(int n)
{
    if (n < 2)
        return 0;
    for (int i = 2; i <= n / i; i++)
        if (divides(i, n))
            return 0;
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 3: The 6k +/- 1 wheel (optimal for a single query)
 *   Every integer is 6k, 6k+1, 6k+2, 6k+3, 6k+4 or 6k+5. Of these, 6k,
 *   6k+2 and 6k+4 are even and 6k+3 is a multiple of 3. So once n has been
 *   tested against 2 and 3, any further prime divisor must be of the form
 *   6k - 1 or 6k + 1: 5, 7, 11, 13, 17, 19, ... We test i and i + 2 for
 *   i = 5, 11, 17, ... which is 2 candidates out of every 6 numbers, about
 *   3x fewer divisions than approach 2.
 *   Testing i + 2 slightly past sqrt(n) is harmless: any divisor strictly
 *   between 1 and n proves n composite.
 *   i stays below 46347, so i + 2 and i += 6 cannot overflow.
 *   Time O(sqrt n), about sqrt(n) / 3 divisions   Space O(1)
 * ------------------------------------------------------------------------- */
static int is_prime_wheel(int n)
{
    if (n <= 3)
        return n >= 2; /* 2 and 3 are prime; everything below 2 is not */
    if (divides(2, n) || divides(3, n))
        return 0;
    for (int i = 5; i <= n / i; i += 6)
        if (divides(i, n) || divides(i + 2, n))
            return 0;
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 4: Sieve of Eratosthenes (best for many queries)
 *   Start with every number 2..N marked prime. Take the smallest number p
 *   still marked: it is prime, so cross out its multiples. Repeat.
 *   Crossing out can start at p * p, because a smaller multiple k * p with
 *   k < p has a prime factor below p and was already crossed out. For the
 *   same reason the outer loop can stop once p * p > N.
 *   Total work is N/2 + N/3 + N/5 + ... over primes p, which is
 *   O(N log log N). After that each query is a single array lookup.
 *   j is a long long so j += p cannot overflow if N is near INT_MAX.
 *   Time O(N log log N) to build, O(1) per query   Space O(N) bytes
 * ------------------------------------------------------------------------- */
typedef struct {
    unsigned char *is_prime; /* is_prime[k] for 0 <= k <= limit */
    int limit;
} Sieve;

/* Returns 1 on success, 0 if out of memory. Requires limit >= 1. */
static int sieve_build(Sieve *s, int limit)
{
    s->limit = limit;
    s->is_prime = malloc((size_t)limit + 1);
    if (s->is_prime == NULL)
        return 0;
    memset(s->is_prime, 1, (size_t)limit + 1);
    s->is_prime[0] = 0;
    s->is_prime[1] = 0;
    for (int p = 2; p <= limit / p; p++)
        if (s->is_prime[p])
            for (long long j = (long long)p * p; j <= limit; j += p)
                s->is_prime[j] = 0;
    return 1;
}

/* 1 if prime, 0 if not, -1 if n is above the sieve's limit (unknown). */
static int sieve_is_prime(const Sieve *s, int n)
{
    if (n < 2)
        return 0;
    if (n > s->limit)
        return -1;
    return s->is_prime[n];
}

static void sieve_free(Sieve *s)
{
    free(s->is_prime);
    s->is_prime = NULL;
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

#define SIEVE_LIMIT 1000000

/* The sieve needs a prebuilt table, so the harness wraps one table in a
 * plain is_prime(int) function to fit the same APPROACHES[] list. */
static Sieve test_sieve;

static int is_prime_sieve(int n)
{
    return sieve_is_prime(&test_sieve, n);
}

typedef int (*PrimeFn)(int n);

static const struct {
    const char *name;
    PrimeFn fn;
    int max_n; /* largest n this approach is run on in the tests */
} APPROACHES[] = {
    {"trial to n-1", is_prime_naive, 100000}, /* O(n): keep n small */
    {"trial to sqrt", is_prime_sqrt, INT_MAX},
    {"6k+/-1 wheel", is_prime_wheel, INT_MAX},
    {"sieve lookup", is_prime_sieve, SIEVE_LIMIT},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int n;
    int expect;
    const char *why;
} Case;

int main(void)
{
    printf("Q08. Check Prime\n");

    if (!sieve_build(&test_sieve, SIEVE_LIMIT)) {
        check(0, "sieve allocation");
        printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
        return 1;
    }

    const Case cases[] = {
        {INT_MIN, 0, "negative"},
        {-7, 0, "negative"},
        {-1, 0, "negative"},
        {0, 0, "not > 1"},
        {1, 0, "not > 1"},
        {2, 1, "smallest prime, the only even one"},
        {3, 1, "prime"},
        {4, 0, "2 * 2"},
        {5, 1, "prime"},
        {9, 0, "3 * 3"},
        {25, 0, "5 * 5, wheel candidate i"},
        {35, 0, "5 * 7"},
        {49, 0, "7 * 7, wheel candidate i + 2"},
        {91, 0, "7 * 13"},
        {97, 1, "prime"},
        {121, 0, "11 * 11"},
        {561, 0, "3 * 11 * 17"},
        {7919, 1, "the 1000th prime"},
        {65537, 1, "Fermat prime 2^16 + 1"},
        {1000000007, 1, "well-known prime 10^9 + 7"},
        {2146654199, 0, "46327 * 46337, two large prime factors"},
        {2147117569, 0, "46337 * 46337, divisor exactly at sqrt(n)"},
        {2147483629, 1, "the second largest int prime"},
        {2147483646, 0, "INT_MAX - 1, even"},
        {INT_MAX, 1, "2^31 - 1, a Mersenne prime"},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[192];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (c->n > APPROACHES[a].max_n)
                continue;
            int got = APPROACHES[a].fn(c->n);
            snprintf(label, sizeof label, "n = %11d, %-13s -> %d  (%s)", c->n, APPROACHES[a].name,
                     got, c->why);
            check(got == c->expect, label);
        }
    }

    /* Trial divisions needed to confirm a prime, worked out by hand:
     *   n = 65537:   trial to n-1   tries 2..65536          = 65535
     *                trial to sqrt  tries 2..256            =   255  (256 * 256 = 65536 <= n)
     *                wheel          2 and 3, then i = 5, 11, ..., 251 (42 values,
     *                               2 divisions each)       = 2 + 84 = 86
     *   n = INT_MAX: trial to sqrt  tries 2..46340          = 46339
     *                wheel          i = 5, 11, ..., 46337 (7723 values) = 2 + 15446 = 15448 */
    {
        const struct {
            int n;
            int approach;
            long long expect;
        } work[] = {
            {65537, 0, 65535}, {65537, 1, 255},   {65537, 2, 86},
            {INT_MAX, 1, 46339}, {INT_MAX, 2, 15448},
        };
        const int num_work = (int)(sizeof work / sizeof work[0]);
        for (int w = 0; w < num_work; w++) {
            trial_divisions = 0;
            int got = APPROACHES[work[w].approach].fn(work[w].n);
            snprintf(label, sizeof label, "n = %10d, %-13s -> %lld trial divisions (expected %lld)",
                     work[w].n, APPROACHES[work[w].approach].name, trial_divisions,
                     work[w].expect);
            check(got == 1 && trial_divisions == work[w].expect, label);
        }
    }

    /* The sieve, checked by counting primes: pi(N) for powers of ten are
     * classic published values. */
    {
        const int limits[] = {10, 100, 1000, 10000, 100000, 1000000};
        const int expect[] = {4, 25, 168, 1229, 9592, 78498};
        const int num_limits = (int)(sizeof limits / sizeof limits[0]);
        for (int k = 0; k < num_limits; k++) {
            int count = 0;
            for (int n = 0; n <= limits[k]; n++)
                count += sieve_is_prime(&test_sieve, n);
            snprintf(label, sizeof label, "sieve: %d primes up to %d (expected %d)", count,
                     limits[k], expect[k]);
            check(count == expect[k], label);
        }
        check(sieve_is_prime(&test_sieve, SIEVE_LIMIT + 1) == -1,
              "sieve: n above its limit is reported as unknown (-1)");
    }

    /* Exhaustive: for every n in 0..100000 the sqrt, wheel and sieve
     * approaches agree. */
    {
        int bad = 0;
        for (int n = 0; n <= 100000; n++) {
            int expect = is_prime_sieve(n);
            if (is_prime_sqrt(n) != expect || is_prime_wheel(n) != expect)
                bad++;
        }
        check(bad == 0, "every n in 0..100000: trial to sqrt, wheel and sieve agree");
    }

    /* Random cross-check: every approach against trial division to n-1,
     * on small n (including negatives) where the O(n) method is fast. */
    {
        srand(1234);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int n = rand() % 5101 - 100; /* -100..5000 */
            int expect = is_prime_naive(n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d small n, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    /* Random large n near INT_MAX: the two O(sqrt n) methods must agree. */
    {
        srand(4321);
        int bad = 0, primes = 0;
        const int rounds = 200;
        for (int r = 0; r < rounds; r++) {
            int n = INT_MAX - rand() % 1000000;
            int expect = is_prime_sqrt(n);
            primes += expect;
            if (is_prime_wheel(n) != expect)
                bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d n near INT_MAX (%d prime), sqrt and wheel agree", rounds,
                 primes);
        check(bad == 0, label);
    }

    sieve_free(&test_sieve);
    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
