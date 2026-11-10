/*
 * Q06. Fibonacci Number
 * Difficulty : Easy
 * Topics     : Recursion, Dynamic Programming, Memoization, Math
 *
 * Problem
 *   Return the n-th Fibonacci number, where F(0) = 0, F(1) = 1 and
 *   F(n) = F(n - 1) + F(n - 2) for n >= 2.
 *
 *   Limit: F(93) = 12200160415121876738 is the largest Fibonacci number that
 *   fits in a 64-bit unsigned long long (max 18446744073709551615);
 *   F(94) = 19740274219868223167 does not. So every function here expects
 *   0 <= n <= 93. Beyond that you need big-integer arithmetic.
 *
 * Example
 *   n = 10   ->   55        (0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55)
 *
 * Approaches
 *   1. Plain recursion             F(n-1) + F(n-2), recomputed     Time O(2^n)      Space O(n) stack
 *   2. Memoization                 recursion + cache of answers    Time O(n)        Space O(n)
 *   3. Tabulation                  fill table[0..n] bottom-up      Time O(n)        Space O(n)
 *   4. Two variables               keep only the last two values   Time O(n)        Space O(1)
 *   5. Fast doubling  [optimal]    jump from k to 2k by identity   Time O(log n)    Space O(1)
 *
 * Key insight
 *   Plain recursion is slow only because it solves the same subproblems
 *   over and over. Solve each F(i) once (approaches 2-4) and it becomes
 *   linear. Going further, the doubling identities jump from k to 2k, so
 *   only about log2(n) steps are needed.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 06_fibonacci.c -o fibonacci && ./fibonacci
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define FIB_MAX_N 93 /* largest n with F(n) <= ULLONG_MAX */

/* Instrumentation: approaches 1 and 2 count their calls so the test output
 * can show the difference between exponential and linear work. */
static long long calls = 0;

/* -------------------------------------------------------------------------
 * Approach 1: Plain recursion
 *   Transcribe the definition directly. Correct, but F(n - 2) is computed
 *   once inside F(n - 1) and again on its own, and so on all the way down.
 *   The number of calls is exactly 2F(n + 1) - 1 (it obeys the same
 *   recurrence, plus one), which grows like 1.618^n: n = 30 already makes
 *   2,692,537 calls, n = 50 would make about 40 billion. That is why this
 *   approach is only ever run with n <= 30 below.
 *   Time O(2^n) (more precisely O(1.618^n))   Space O(n) recursion depth
 * ------------------------------------------------------------------------- */
static unsigned long long fib_recursive(int n)
{
    calls++;
    if (n < 2)
        return (unsigned long long)n;
    return fib_recursive(n - 1) + fib_recursive(n - 2);
}

/* -------------------------------------------------------------------------
 * Approach 2: Memoization (top-down dynamic programming)
 *   Same recursion, but remember every answer the first time it is found
 *   and return the stored value on later calls. Each F(i) is now computed
 *   once, so there are only 2n - 1 calls (n >= 1).
 *   ULLONG_MAX marks "not computed yet": no F(i) with i <= 93 equals it,
 *   whereas 0 would not work as a marker because F(0) = 0.
 *   The two recursive calls are made in separate statements on purpose: in
 *   "f(n - 1) + f(n - 2)" C does not specify which operand runs first.
 *   Time O(n)   Space O(n) for the cache plus O(n) recursion depth
 * ------------------------------------------------------------------------- */
#define NOT_COMPUTED ULLONG_MAX

static unsigned long long fib_memo_rec(int n, unsigned long long *memo)
{
    calls++;
    if (memo[n] != NOT_COMPUTED)
        return memo[n];
    unsigned long long value;
    if (n < 2) {
        value = (unsigned long long)n;
    } else {
        unsigned long long f1 = fib_memo_rec(n - 1, memo);
        unsigned long long f2 = fib_memo_rec(n - 2, memo);
        value = f1 + f2;
    }
    memo[n] = value;
    return value;
}

static unsigned long long fib_memo(int n)
{
    unsigned long long memo[FIB_MAX_N + 1];
    for (int i = 0; i <= n; i++)
        memo[i] = NOT_COMPUTED;
    return fib_memo_rec(n, memo);
}

/* -------------------------------------------------------------------------
 * Approach 3: Tabulation (bottom-up dynamic programming)
 *   Fill table[i] for i = 0, 1, 2, ..., n in order. When table[i] is
 *   computed, table[i - 1] and table[i - 2] are already there. No recursion,
 *   so no risk of running out of stack.
 *   Time O(n)   Space O(n) for the table
 * ------------------------------------------------------------------------- */
static unsigned long long fib_table(int n)
{
    unsigned long long table[FIB_MAX_N + 1];
    table[0] = 0;
    table[1] = 1; /* the array always has room for index 1, even if n == 0 */
    for (int i = 2; i <= n; i++)
        table[i] = table[i - 1] + table[i - 2];
    return table[n];
}

/* -------------------------------------------------------------------------
 * Approach 4: Two variables
 *   The table in approach 3 only ever looks back two entries, so keep just
 *   those two values and slide them forward.
 *   The loop stops after computing F(n), never computing F(n + 1), so even
 *   n = 93 never overflows.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static unsigned long long fib_iterative(int n)
{
    if (n == 0)
        return 0;
    unsigned long long prev = 0, cur = 1; /* F(i - 1), F(i) with i = 1 */
    for (int i = 2; i <= n; i++) {
        unsigned long long next = prev + cur;
        prev = cur;
        cur = next;
    }
    return cur;
}

/* -------------------------------------------------------------------------
 * Approach 5: Fast doubling (optimal)
 *   From the pair (F(k), F(k+1)) you can jump straight to k' = 2k:
 *       F(2k)     = F(k) * (2 F(k+1) - F(k))
 *       F(2k + 1) = F(k)^2 + F(k+1)^2
 *   and from there to 2k + 1 with one addition. These come from squaring
 *   the matrix [[1,1],[1,0]]^k = [[F(k+1), F(k)], [F(k), F(k-1)]], i.e.
 *   this is matrix exponentiation with the redundant work removed.
 *   Read the bits of n from the most significant one down, starting at
 *   k = 0: each bit doubles k, and a 1 bit adds one more. After the last
 *   bit, k = n. Example n = 11 = 1011b: k goes 1, 2, 5, 11.
 *   Overflow note: on the final step b becomes F(n + 1), which for n = 93
 *   is F(94) and wraps around. Unsigned arithmetic is defined to wrap
 *   modulo 2^64 (it is not undefined behaviour), and b is never returned,
 *   so a = F(n) is still exact.
 *   Time O(log n)   Space O(1)
 * ------------------------------------------------------------------------- */
static unsigned long long fib_fast_doubling(int n)
{
    unsigned long long a = 0, b = 1; /* (F(k), F(k+1)) with k = 0 */
    int top = 0;
    while ((n >> top) > 1) /* index of the highest 1 bit of n */
        top++;
    for (int bit = top; bit >= 0; bit--) {
        unsigned long long c = a * (2 * b - a); /* F(2k) */
        unsigned long long d = a * a + b * b;   /* F(2k + 1) */
        if ((n >> bit) & 1) {
            a = d; /* k becomes 2k + 1 */
            b = c + d;
        } else {
            a = c; /* k becomes 2k */
            b = d;
        }
    }
    return a;
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

typedef unsigned long long (*FibFn)(int n);

static const struct {
    const char *name;
    FibFn fn;
    int max_n; /* largest n this approach is run on in the tests */
} APPROACHES[] = {
    {"plain recursion", fib_recursive, 30}, /* exponential: keep n small */
    {"memoization", fib_memo, FIB_MAX_N},
    {"tabulation", fib_table, FIB_MAX_N},
    {"two variables", fib_iterative, FIB_MAX_N},
    {"fast doubling", fib_fast_doubling, FIB_MAX_N},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int n;
    unsigned long long expect;
} Case;

int main(void)
{
    printf("Q06. Fibonacci Number\n");

    /* Small values by counting along the sequence; large ones are standard
     * published values, cross-checked by F(91) + F(92) = F(93):
     * 4660046610375530309 + 7540113804746346429 = 12200160415121876738. */
    const Case cases[] = {
        {0, 0ULL},
        {1, 1ULL},
        {2, 1ULL},
        {3, 2ULL},
        {5, 5ULL},
        {10, 55ULL},
        {20, 6765ULL},
        {30, 832040ULL},
        {50, 12586269025ULL},
        {64, 10610209857723ULL},
        {90, 2880067194370816120ULL},
        {92, 7540113804746346429ULL},
        {93, 12200160415121876738ULL}, /* the largest that fits */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (c->n > APPROACHES[a].max_n)
                continue;
            unsigned long long got = APPROACHES[a].fn(c->n);
            snprintf(label, sizeof label, "F(%2d), %-15s -> %llu", c->n, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* How much work the two recursive versions do for n = 20:
     * plain recursion makes 2F(21) - 1 = 2 * 10946 - 1 = 21891 calls,
     * memoization makes 2n - 1 = 39. */
    {
        calls = 0;
        fib_recursive(20);
        snprintf(label, sizeof label, "n = 20: plain recursion makes %lld calls (2F(21) - 1 = 21891)",
                 calls);
        check(calls == 21891, label);

        calls = 0;
        fib_memo(20);
        snprintf(label, sizeof label, "n = 20: memoization makes %lld calls (2n - 1 = 39)", calls);
        check(calls == 39, label);
    }

    /* The documented limit: F(94) = F(92) + F(93) does not fit. */
    check(fib_iterative(93) > ULLONG_MAX - fib_iterative(92),
          "F(92) + F(93) exceeds ULLONG_MAX, so F(93) is the last that fits");

    /* Every n from 0 to 93: the fast approaches agree with each other and
     * satisfy the recurrence F(n) = F(n - 1) + F(n - 2). */
    {
        int bad = 0;
        for (int n = 0; n <= FIB_MAX_N; n++) {
            unsigned long long expect = fib_table(n);
            for (int a = 1; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(n) != expect)
                    bad++;
            if (n >= 2 && expect != fib_table(n - 1) + fib_table(n - 2))
                bad++;
        }
        check(bad == 0, "all n in 0..93: approaches 2-5 agree and follow the recurrence");
    }

    /* Random cross-check: every approach against plain recursion. n is kept
     * at 22 or below so the exponential version stays fast. */
    {
        srand(31337);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int n = rand() % 23;
            unsigned long long expect = fib_recursive(n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d values of n, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
