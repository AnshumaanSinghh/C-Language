/*
 * Q33. Coin Change
 * Difficulty : Medium
 * Topics     : Dynamic Programming, BFS, Greedy (counterexample)
 *
 * Problem
 *   Given c coin denominations (positive integers, unlimited supply of
 *   each) and a non-negative amount, return the fewest coins whose values
 *   add up to amount, or -1 if it cannot be done. amount = 0 needs 0 coins.
 *
 * Example
 *   coins = [1, 2, 5], amount = 11   ->   3   (5 + 5 + 1)
 *   coins = [2],       amount = 3    ->  -1
 *
 * Approaches
 *   1. Plain recursion          try every coin as the last one     Time O(c^amount)    Space O(amount)
 *   2. Memoized recursion       same, but cache each sub-amount    Time O(amount*c)    Space O(amount)
 *   3. Bottom-up DP [optimal]   fill dp[0..amount] in order        Time O(amount*c)    Space O(amount)
 *   4. BFS over amounts         level = number of coins used       Time O(amount*c)    Space O(amount)
 *   x. Greedy (WRONG)           always take the biggest coin       shown failing, not a real answer
 *
 * Key insight
 *   The best way to pay a, if it uses coin x last, is 1 + (best way to pay
 *   a - x). So  best(a) = 1 + min over coins x <= a of best(a - x),  and
 *   there are only amount + 1 different sub-problems.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 33_coin_change.c -o coin_change && ./coin_change
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Plain recursion (brute force)
 *   best(0) = 0. For a > 0 try every coin x <= a as the last coin and keep
 *   the smallest 1 + best(a - x); if no choice works, a is impossible.
 *   The same sub-amount is solved again and again (best(5) is reached via
 *   1+1+3, 1+3+1, 3+1+1, ...), so the call tree grows exponentially.
 *   Only run it on small amounts; the tests keep amount <= 15.
 *   Time O(c^(amount / min coin))   Space O(amount) recursion depth
 * ------------------------------------------------------------------------- */
static int coin_rec_go(const int *coins, int c, int amount)
{
    if (amount == 0)
        return 0;
    int best = -1;
    for (int i = 0; i < c; i++) {
        if (coins[i] <= amount) {
            int sub = coin_rec_go(coins, c, amount - coins[i]);
            if (sub >= 0 && (best < 0 || sub + 1 < best))
                best = sub + 1;
        }
    }
    return best;
}

static int coin_recursive(const int *coins, int c, int amount)
{
    return coin_rec_go(coins, c, amount);
}

/* -------------------------------------------------------------------------
 * Approach 2: Memoized recursion (top-down DP)
 *   Same recursion, but the answer for each sub-amount is stored the first
 *   time it is computed. memo[a] == UNKNOWN means "not solved yet"; -1 is
 *   a real answer ("impossible") and must be cached too, or impossible
 *   sub-amounts get re-explored every time.
 *   Each of the amount + 1 sub-amounts is solved once, looping over c coins.
 *   Pitfall: recursion depth can reach amount (all 1-coins), so a huge
 *   amount can overflow the call stack. Bottom-up avoids that.
 *   Time O(amount*c)   Space O(amount) memo + O(amount) stack
 * ------------------------------------------------------------------------- */
enum { UNKNOWN = -2 };

static int coin_memo_go(const int *coins, int c, int amount, int *memo)
{
    if (amount == 0)
        return 0;
    if (memo[amount] != UNKNOWN)
        return memo[amount];
    int best = -1;
    for (int i = 0; i < c; i++) {
        if (coins[i] <= amount) {
            int sub = coin_memo_go(coins, c, amount - coins[i], memo);
            if (sub >= 0 && (best < 0 || sub + 1 < best))
                best = sub + 1;
        }
    }
    memo[amount] = best;
    return best;
}

static int coin_memo(const int *coins, int c, int amount)
{
    int *memo = malloc(((size_t)amount + 1) * sizeof *memo);
    if (memo == NULL)
        return -1;
    for (int a = 0; a <= amount; a++)
        memo[a] = UNKNOWN;
    int result = coin_memo_go(coins, c, amount, memo);
    free(memo);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Bottom-up tabulation (optimal)
 *   dp[a] = fewest coins for amount a. dp[0] = 0, and every dp[a - x] that
 *   dp[a] depends on has a smaller index, so filling a = 1, 2, ... in order
 *   always reads finished values.
 *   "Infinity" is amount + 1: every real answer uses at most amount coins
 *   (each coin is >= 1), so amount + 1 can never be a real answer.
 *   Pitfall: do NOT use INT_MAX as infinity; INT_MAX + 1 overflows, which
 *   is undefined behaviour in C.
 *   Time O(amount*c)   Space O(amount), no recursion
 * ------------------------------------------------------------------------- */
static int coin_dp(const int *coins, int c, int amount)
{
    int *dp = malloc(((size_t)amount + 1) * sizeof *dp);
    if (dp == NULL)
        return -1;
    const int INF = amount + 1;
    dp[0] = 0;
    for (int a = 1; a <= amount; a++) {
        dp[a] = INF;
        for (int i = 0; i < c; i++)
            if (coins[i] <= a && dp[a - coins[i]] + 1 < dp[a])
                dp[a] = dp[a - coins[i]] + 1;
    }
    int result = dp[amount] == INF ? -1 : dp[amount];
    free(dp);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 4: BFS over amounts
 *   Treat each amount 0..amount as a graph node, with an edge a -> a + x
 *   for every coin x. Every edge costs one coin, so the fewest coins is the
 *   shortest path from 0 to amount, and BFS finds shortest paths in
 *   unweighted graphs level by level: level d holds every amount payable
 *   with exactly d coins and not fewer.
 *   It can stop as soon as it reaches amount, and amounts above the target
 *   are never enqueued, so it often touches fewer states than approach 3.
 *   Pitfall: test x <= amount - cur rather than cur + x <= amount; with a
 *   coin like INT_MAX, cur + x would overflow.
 *   Time O(amount*c)   Space O(amount) for the queue and visited flags
 * ------------------------------------------------------------------------- */
static int coin_bfs(const int *coins, int c, int amount)
{
    if (amount == 0)
        return 0;
    int *queue = malloc(((size_t)amount + 1) * sizeof *queue);
    unsigned char *seen = calloc((size_t)amount + 1, 1);
    int result = -1;
    if (queue != NULL && seen != NULL) {
        int head = 0, tail = 0;
        queue[tail++] = 0;
        seen[0] = 1;
        for (int level = 1; head < tail && result < 0; level++) {
            int level_end = tail;
            while (head < level_end && result < 0) {
                int cur = queue[head++];
                for (int i = 0; i < c; i++) {
                    if (coins[i] > amount - cur)
                        continue;
                    int next = cur + coins[i];
                    if (next == amount) {
                        result = level;
                        break;
                    }
                    if (!seen[next]) {
                        seen[next] = 1;
                        queue[tail++] = next;
                    }
                }
            }
        }
    }
    free(queue);
    free(seen);
    return result;
}

/* -------------------------------------------------------------------------
 * Wrong approach: Greedy (for demonstration only)
 *   Take as many of the largest coin as fit, then the next largest, ...
 *   This happens to be right for "canonical" systems such as 1, 5, 10, 25,
 *   which is why it feels natural. It is wrong in general:
 *     coins {1, 3, 4}, amount 6: greedy 4 + 1 + 1 = 3 coins, best 3 + 3 = 2.
 *     coins {2, 5},    amount 6: greedy takes 5, is stuck at 1 and reports
 *                                -1, but 2 + 2 + 2 works.
 *   Taking the big coin now can force bad choices later, and greedy never
 *   reconsiders. That is exactly what the DP's "try every last coin" fixes.
 *   It is not in APPROACHES and not in the cross-check.
 * ------------------------------------------------------------------------- */
static int cmp_desc(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x < y) - (x > y);
}

static int coin_greedy(const int *coins, int c, int amount)
{
    int *sorted = malloc((size_t)(c > 0 ? c : 1) * sizeof *sorted);
    if (sorted == NULL)
        return -1;
    memcpy(sorted, coins, (size_t)c * sizeof *sorted);
    qsort(sorted, (size_t)c, sizeof *sorted, cmp_desc);
    int count = 0;
    for (int i = 0; i < c; i++) {
        count += amount / sorted[i];
        amount %= sorted[i];
    }
    free(sorted);
    return amount == 0 ? count : -1;
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

typedef int (*CoinFn)(const int *coins, int c, int amount);

/* max_amount: largest amount an approach is run on in these tests, so the
 * exponential recursion stays fast. */
static const struct {
    const char *name;
    CoinFn fn;
    int max_amount;
} APPROACHES[] = {
    {"plain recursion", coin_recursive, 15},
    {"memoized", coin_memo, INT_MAX},
    {"bottom-up DP", coin_dp, INT_MAX},
    {"BFS", coin_bfs, INT_MAX},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *what;
    int coins[6];
    int c;
    int amount;
    int expect;
} Case;

int main(void)
{
    printf("Q33. Coin Change\n");

    /* Expected answers worked out by hand. */
    const Case cases[] = {
        {"example", {1, 2, 5}, 3, 11, 3},                 /* 5+5+1 */
        {"impossible", {2}, 1, 3, -1},
        {"amount 0", {1}, 1, 0, 0},
        {"amount 0, no 1-coin", {2}, 1, 0, 0},
        {"greedy trap", {1, 3, 4}, 3, 6, 2},              /* 3+3 */
        {"greedy gets stuck", {2, 5}, 2, 6, 3},           /* 2+2+2 */
        {"all coins too big", {5, 10}, 2, 3, -1},
        {"duplicate coins", {3, 3, 3}, 3, 9, 3},
        {"even coins, odd amount", {2, 4}, 2, 7, -1},
        {"coin = INT_MAX", {INT_MAX, 2}, 2, 4, 2},        /* cur + x would overflow */
        {"exact single coin", {7, 3}, 2, 7, 1},
        {"unordered coins", {5, 1, 2}, 3, 13, 4},         /* 5+5+2+1 */
        {"canonical US coins", {1, 5, 10, 25}, 4, 63, 6}, /* 25+25+10+1+1+1 */
        /* 7a + 3b = 100: b = (100 - 7a) / 3 needs a = 1 (mod 3), and a + b
         * shrinks as a grows, so a = 13, b = 3, total 16. */
        {"bigger amount", {7, 3}, 2, 100, 16},
        {"large amount", {1, 2, 5}, 3, 1000, 200},        /* 200 fives */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (c->amount > APPROACHES[a].max_amount)
                continue; /* exponential approach: skipped on big amounts */
            int got = APPROACHES[a].fn(c->coins, c->c, c->amount);
            snprintf(label, sizeof label, "test %2d %-22s, %-15s -> %d", t + 1, c->what,
                     APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Greedy is wrong: these checks PASS when greedy gives the wrong answer
     * that the comment above predicts. */
    {
        const int c1[] = {1, 3, 4};
        int g = coin_greedy(c1, 3, 6), d = coin_dp(c1, 3, 6);
        snprintf(label, sizeof label, "greedy {1,3,4}, 6 -> %d coins, but best is %d", g, d);
        check(g == 3 && d == 2, label);

        const int c2[] = {2, 5};
        g = coin_greedy(c2, 2, 6);
        d = coin_dp(c2, 2, 6);
        snprintf(label, sizeof label, "greedy {2,5}, 6 -> %d (gives up), but best is %d", g, d);
        check(g == -1 && d == 3, label);

        const int c3[] = {25, 10, 5, 1};
        g = coin_greedy(c3, 4, 63);
        snprintf(label, sizeof label, "greedy {25,10,5,1}, 63 -> %d (canonical: correct)", g);
        check(g == 6, label);
    }

    /* Random cross-check 1: small amounts so plain recursion can run. Every
     * approach must match the brute force. */
    {
        srand(33);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int coins[4];
            int c = 1 + rand() % 4;
            for (int i = 0; i < c; i++)
                coins[i] = 1 + rand() % 12;
            int amount = rand() % 16;
            int expect = coin_recursive(coins, c, amount);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(coins, c, amount) != expect)
                    bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d small cases (amount <= 15), all agree", rounds);
        check(bad == 0, label);
    }

    /* Random cross-check 2: larger amounts, the polynomial approaches must
     * match bottom-up DP (itself validated above). */
    {
        int bad = 0;
        const int rounds = 200;
        for (int r = 0; r < rounds; r++) {
            int coins[5];
            int c = 1 + rand() % 5;
            for (int i = 0; i < c; i++)
                coins[i] = 2 + rand() % 40; /* no 1-coin, so -1 answers appear */
            int amount = rand() % 401;
            int expect = coin_dp(coins, c, amount);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (amount <= APPROACHES[a].max_amount &&
                    APPROACHES[a].fn(coins, c, amount) != expect)
                    bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d cases (amount <= 400), polynomial ones agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
