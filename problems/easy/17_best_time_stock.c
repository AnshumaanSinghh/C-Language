/*
 * Q17. Best Time to Buy and Sell Stock
 * Difficulty : Easy
 * Topics     : Arrays, Greedy, Dynamic Programming
 *
 * Problem
 *   prices[i] is a stock's price on day i (prices are never negative). Buy
 *   on one day and sell on a strictly later day, at most once. Return the
 *   largest possible profit, or 0 if no trade makes money. (LeetCode 121)
 *
 * Example
 *   prices = [7, 1, 5, 3, 6, 4]   ->   5   buy on day 1 at 1, sell on day 4 at 6
 *   prices = [7, 6, 4, 3, 1]      ->   0   the price only falls
 *
 * Approaches
 *   1. Brute force               try every buy/sell pair          Time O(n^2)  Space O(1)
 *   2. Suffix max                best future price for each day   Time O(n)    Space O(n)
 *   3. Min so far    [optimal]   track the cheapest day so far    Time O(n)    Space O(1)
 *   4. Kadane on differences     max subarray of daily changes    Time O(n)    Space O(1)
 *
 * Key insight
 *   Fix the sell day j. The best buy day is simply the cheapest day before
 *   j, and that minimum can be carried along in one variable. So a single
 *   left-to-right pass tries every sell day against its best buy day.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 17_best_time_stock.c -o best_time_stock && ./best_time_stock
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* Overflow note: prices are in 0..INT_MAX, so any difference
 * prices[j] - prices[i] lies in -INT_MAX..INT_MAX and fits in an int.
 * That is the only arithmetic any approach below does. */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Try every buy day i and every later sell day j; keep the best profit.
 *   Starting best at 0 handles "no profitable trade" for free.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int stock_brute(const int *prices, int n)
{
    int best = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (prices[j] - prices[i] > best)
                best = prices[j] - prices[i];
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 2: Suffix maximum array
 *   suffix_max[i] = the highest price on day i or later. If you buy on day
 *   i, the best you can do is sell at suffix_max[i]. Fill the array from
 *   the right, then take the best suffix_max[i] - prices[i].
 *   Including day i itself in its suffix only adds a "buy and sell the same
 *   day" option worth 0, which never beats the true answer (which is >= 0).
 *   Returns -1 if the array cannot be allocated.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int stock_suffix_max(const int *prices, int n)
{
    if (n < 2)
        return 0;
    int *suffix_max = malloc((size_t)n * sizeof *suffix_max);
    if (suffix_max == NULL)
        return -1;

    suffix_max[n - 1] = prices[n - 1];
    for (int i = n - 2; i >= 0; i--)
        suffix_max[i] = prices[i] > suffix_max[i + 1] ? prices[i] : suffix_max[i + 1];

    int best = 0;
    for (int i = 0; i < n; i++)
        if (suffix_max[i] - prices[i] > best)
            best = suffix_max[i] - prices[i];
    free(suffix_max);
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 3: Single pass with the minimum so far (optimal)
 *   Walk the days once. For sell day j, the best buy is the cheapest day
 *   before j, which we track as min_day. Check the profit first, then
 *   update min_day, so we never "buy" on the day we sell.
 *   This version also reports the days: *buy and *sell are set to the
 *   chosen days, or both to -1 when the best profit is 0 (do not trade).
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int stock_min_so_far_days(const int *prices, int n, int *buy, int *sell)
{
    int best = 0, min_day = 0;
    *buy = -1;
    *sell = -1;
    for (int j = 1; j < n; j++) {
        if (prices[j] - prices[min_day] > best) {
            best = prices[j] - prices[min_day];
            *buy = min_day;
            *sell = j;
        }
        if (prices[j] < prices[min_day])
            min_day = j;
    }
    return best;
}

static int stock_min_so_far(const int *prices, int n)
{
    int buy, sell;
    return stock_min_so_far_days(prices, n, &buy, &sell);
}

/* -------------------------------------------------------------------------
 * Approach 4: Kadane's algorithm on day-to-day differences
 *   Let d[j] = prices[j] - prices[j-1]. Buying on day i and selling on day
 *   j earns d[i+1] + ... + d[j] (the sum telescopes to prices[j] -
 *   prices[i]). So the answer is the maximum subarray sum of d, or 0 if all
 *   sums are negative: exactly Kadane's algorithm. cur is the best sum of
 *   a run ending today; once it drops below 0, starting afresh is better.
 *   Overflow: cur always equals prices[j] - prices[s] for some s, so it
 *   stays within -INT_MAX..INT_MAX like every other difference.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int stock_kadane(const int *prices, int n)
{
    int cur = 0, best = 0;
    for (int j = 1; j < n; j++) {
        cur += prices[j] - prices[j - 1];
        if (cur < 0)
            cur = 0;
        if (cur > best)
            best = cur;
    }
    return best;
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

typedef int (*StockFn)(const int *prices, int n);

static const struct {
    const char *name;
    StockFn fn;
} APPROACHES[] = {
    {"brute force", stock_brute},
    {"suffix max", stock_suffix_max},
    {"min so far", stock_min_so_far},
    {"kadane", stock_kadane},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int prices[8];
    int n;
    int expect;
} Case;

/* The reported days must be a real trade that earns the reported profit,
 * or (-1, -1) when the profit is 0. */
static int valid_days(const int *prices, int n, int profit, int buy, int sell)
{
    if (profit == 0)
        return buy == -1 && sell == -1;
    return 0 <= buy && buy < sell && sell < n && prices[sell] - prices[buy] == profit;
}

int main(void)
{
    printf("Q17. Best Time to Buy and Sell Stock\n");

    const Case cases[] = {
        {{7, 1, 5, 3, 6, 4}, 6, 5},
        {{7, 6, 4, 3, 1}, 5, 0},                /* only falls */
        {{0}, 0, 0},                            /* empty */
        {{5}, 1, 0},                            /* one day: cannot sell later */
        {{3, 3, 3}, 3, 0},                      /* all equal */
        {{1, 2}, 2, 1},
        {{2, 4, 1}, 3, 2},                      /* the later minimum is useless */
        {{3, 8, 1, 6}, 4, 5},                   /* two different trades both earn 5 */
        {{2, 1, 2, 1, 0, 1, 2}, 7, 2},
        {{0, INT_MAX}, 2, INT_MAX},             /* the largest possible profit */
        {{INT_MAX, 0, INT_MAX}, 3, INT_MAX},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->prices, c->n);
            snprintf(label, sizeof label, "test %2d, %-11s -> %d", t + 1, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
        int buy, sell;
        int profit = stock_min_so_far_days(c->prices, c->n, &buy, &sell);
        snprintf(label, sizeof label, "test %2d, days        -> buy %d, sell %d", t + 1, buy, sell);
        check(valid_days(c->prices, c->n, profit, buy, sell), label);
    }

    /* The example has exactly one best trade, so the days are fixed. */
    {
        const int prices[] = {7, 1, 5, 3, 6, 4};
        int buy, sell;
        stock_min_so_far_days(prices, 6, &buy, &sell);
        check(buy == 1 && sell == 4, "example: buy on day 1, sell on day 4");
    }

    /* Random cross-check against brute force; the days reported by
     * approach 3 must also be a valid trade worth the profit. */
    {
        srand(1717);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int prices[16];
            int n = rand() % 17;
            for (int k = 0; k < n; k++)
                prices[k] = rand() % 21;

            int expect = stock_brute(prices, n);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(prices, n) != expect)
                    bad++;
            int buy, sell;
            int profit = stock_min_so_far_days(prices, n, &buy, &sell);
            if (!valid_days(prices, n, profit, buy, sell))
                bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d price lists, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
