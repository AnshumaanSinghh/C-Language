/*
 * Q24. Majority Element
 * Difficulty : Medium
 * Topics     : Arrays, Hashing, Sorting, Bit Manipulation, Voting
 *
 * Problem
 *   Given an array nums of n >= 1 integers, return the majority element:
 *   the value that appears MORE than n / 2 times. You may assume such a
 *   value always exists. (A note at the end handles the case where it might
 *   not.)
 *
 * Example
 *   nums = [2, 2, 1, 1, 1, 2, 2]   ->   2   (4 of 7 elements)
 *
 * Approaches
 *   1. Brute count                 count each value from scratch     Time O(n^2)      Space O(1)
 *   2. Sort, take middle           majority covers index n/2         Time O(n log n)  Space O(n)
 *   3. Hash map counts             count in one pass                 Time O(n)        Space O(n)
 *   4. Bit-by-bit voting           majority of each bit              Time O(32n)      Space O(1)
 *   5. Boyer-Moore     [optimal]   pair off different values         Time O(n)        Space O(1)
 *
 * Key insight
 *   If you repeatedly cancel one majority element against one element that
 *   differs from it, the majority still has copies left over, because it
 *   outnumbers everything else combined. Boyer-Moore does that cancelling
 *   with a single candidate and a counter.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 24_majority_element.c -o majority_element && ./majority_element
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Brute count
 *   For each position, count how many times its value appears in the whole
 *   array. The first value whose count exceeds n / 2 is the answer.
 *   "More than n / 2" with integer division is count > n / 2: for n = 7
 *   that is count >= 4, for n = 4 it is count >= 3.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int majority_brute(const int *nums, int n)
{
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++)
            if (nums[j] == nums[i])
                count++;
        if (count > n / 2)
            return nums[i];
    }
    return nums[0]; /* unreachable when a majority exists */
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort a copy, take the middle
 *   In sorted order the majority's copies form one contiguous block of
 *   length > n / 2. Any block that long must cover index n / 2, whether it
 *   starts at the very left or ends at the very right. So sorted[n / 2] is
 *   the answer. We sort a copy so the caller's array is left untouched.
 *   If malloc fails we fall back to the brute count, which needs no memory,
 *   so the answer is still correct.
 *   Time O(n log n)   Space O(n) for the copy
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y); /* x - y could overflow for INT_MIN / INT_MAX */
}

static int majority_sort(const int *nums, int n)
{
    int *copy = malloc((size_t)n * sizeof *copy);
    if (copy == NULL)
        return majority_brute(nums, n);
    memcpy(copy, nums, (size_t)n * sizeof *copy);
    qsort(copy, (size_t)n, sizeof *copy, cmp_int);
    int answer = copy[n / 2];
    free(copy);
    return answer;
}

/* -------------------------------------------------------------------------
 * Approach 3: Hash map of counts
 *   Count each value in a map value -> count and stop as soon as a count
 *   passes n / 2. C has no built-in map, so this is a small open-addressing
 *   table with linear probing, sized to at least 2n slots so it never fills
 *   and probe chains stay short.
 *   Time O(n) expected   Space O(n)
 * ------------------------------------------------------------------------- */
static size_t hash_int(int key)
{
    unsigned int x = (unsigned int)key;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

static int majority_hash(const int *nums, int n)
{
    size_t cap = 16;
    while (cap < (size_t)n * 2)
        cap <<= 1;
    size_t mask = cap - 1;
    int *keys = malloc(cap * sizeof *keys);
    int *counts = malloc(cap * sizeof *counts);
    unsigned char *used = calloc(cap, 1);
    if (keys == NULL || counts == NULL || used == NULL) {
        free(keys);
        free(counts);
        free(used);
        return majority_brute(nums, n);
    }

    int answer = nums[0];
    for (int i = 0; i < n; i++) {
        size_t h = hash_int(nums[i]) & mask;
        while (used[h] && keys[h] != nums[i])
            h = (h + 1) & mask;
        if (!used[h]) {
            used[h] = 1;
            keys[h] = nums[i];
            counts[h] = 0;
        }
        if (++counts[h] > n / 2) {
            answer = nums[i];
            break;
        }
    }
    free(keys);
    free(counts);
    free(used);
    return answer;
}

/* -------------------------------------------------------------------------
 * Approach 4: Bit-by-bit voting
 *   Look at one bit position at a time. The majority element has the same
 *   value of that bit in > n / 2 positions, so whichever bit value (0 or 1)
 *   wins the vote at that position is the majority element's bit. Build
 *   the answer from the 32 winners.
 *   Pitfall: do the bit work on unsigned int. Shifting a negative int, or
 *   setting the sign bit with 1 << 31, is undefined or implementation-
 *   defined; with unsigned the sign bit is just bit 31 and negatives work.
 *   Converting back is done without relying on implementation-defined
 *   unsigned -> int conversion (assumes two's complement, like every
 *   mainstream platform).
 *   Time O(32 n)   Space O(1)
 * ------------------------------------------------------------------------- */
#define INT_BITS ((int)(sizeof(unsigned int) * CHAR_BIT))

static int unsigned_to_int(unsigned int u)
{
    if (u <= (unsigned int)INT_MAX)
        return (int)u;
    /* u represents u - 2^32; write that as -(UINT_MAX - u) - 1 so every
     * intermediate value fits in int. */
    return -(int)(UINT_MAX - u) - 1;
}

static int majority_bits(const int *nums, int n)
{
    unsigned int result = 0;
    for (int b = 0; b < INT_BITS; b++) {
        unsigned int bit = 1u << b;
        int ones = 0;
        for (int i = 0; i < n; i++)
            if ((unsigned int)nums[i] & bit)
                ones++;
        if (ones > n / 2)
            result |= bit;
    }
    return unsigned_to_int(result);
}

/* -------------------------------------------------------------------------
 * Approach 5: Boyer-Moore majority vote (optimal)
 *   Keep a candidate and a counter. For each x:
 *     counter == 0  -> adopt x as the new candidate, counter = 1
 *     x == candidate -> counter++
 *     otherwise      -> counter-- (x cancels one copy of the candidate)
 *   Why it works: every decrement pairs two DIFFERENT values and discards
 *   both. The majority has more copies than all other values together, so
 *   even if each of its copies were cancelled by a non-majority value, some
 *   would remain. Whatever survives the cancelling must be the majority.
 *   The candidate is only guaranteed correct WHEN a majority exists; see
 *   has_majority() below.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int majority_boyer_moore(const int *nums, int n)
{
    int candidate = nums[0], count = 0;
    for (int i = 0; i < n; i++) {
        if (count == 0) {
            candidate = nums[i];
            count = 1;
        } else if (nums[i] == candidate) {
            count++;
        } else {
            count--;
        }
    }
    return candidate;
}

/* -------------------------------------------------------------------------
 * Note: when a majority is NOT guaranteed
 *   Boyer-Moore always returns some candidate, even for [1, 2, 3] (it
 *   returns 3). If the input might have no majority, do a second pass that
 *   counts the candidate and accept it only if count > n / 2. Still O(n)
 *   time and O(1) space. Returns 1 and writes *out if a majority exists.
 * ------------------------------------------------------------------------- */
static int has_majority(const int *nums, int n, int *out)
{
    if (n <= 0)
        return 0;
    int candidate = majority_boyer_moore(nums, n);
    int count = 0;
    for (int i = 0; i < n; i++)
        if (nums[i] == candidate)
            count++;
    if (count > n / 2) {
        *out = candidate;
        return 1;
    }
    return 0;
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

typedef int (*MajorityFn)(const int *nums, int n);

static const struct {
    const char *name;
    MajorityFn fn;
} APPROACHES[] = {
    {"brute count", majority_brute},
    {"sort + middle", majority_sort},
    {"hash map", majority_hash},
    {"bit voting", majority_bits},
    {"boyer-moore", majority_boyer_moore},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 16

typedef struct {
    int nums[MAXN];
    int n;
    int expect;
} Case;

typedef struct {
    int nums[MAXN];
    int n;
    int expect_found;
    int expect_value; /* only meaningful when expect_found */
} VerifyCase;

/* Reference for the "maybe no majority" check: count every value. */
static int brute_has_majority(const int *nums, int n, int *out)
{
    for (int i = 0; i < n; i++) {
        int count = 0;
        for (int j = 0; j < n; j++)
            if (nums[j] == nums[i])
                count++;
        if (count > n / 2) {
            *out = nums[i];
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    printf("Q24. Majority Element\n");

    /* Every case has a majority; expected values counted by hand. */
    const Case cases[] = {
        {{3, 2, 3}, 3, 3},
        {{2, 2, 1, 1, 1, 2, 2}, 7, 2},
        {{7}, 1, 7},                                  /* single element */
        {{5, 5, 5, 5}, 4, 5},                         /* all equal */
        {{-1, -1, 2}, 3, -1},                         /* negative majority */
        {{-5, 4, -5, 4, -5}, 5, -5},
        {{1, 2, 1, 3, 1}, 5, 1},                      /* never adjacent */
        {{1, 2, 3, 3, 3}, 5, 3},                      /* majority only at the end */
        {{4, 4, 4, 1, 2}, 5, 4},                      /* majority only at the start */
        {{INT_MIN, INT_MIN, INT_MAX}, 3, INT_MIN},    /* sign bit set */
        {{INT_MAX, INT_MIN, INT_MAX}, 3, INT_MAX},
        {{0, -1, 0, -1, -1, 0, -1}, 7, -1},           /* -1 is all ones in binary */
        {{6, 6, 9, 9, 6}, 5, 6},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->nums, c->n);
            snprintf(label, sizeof label, "test %2d, %-13s -> %d", t + 1, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Majority not guaranteed: the verification pass must reject candidates
     * that do not actually exceed n / 2. */
    {
        const VerifyCase vcases[] = {
            {{1, 2, 3}, 3, 0, 0},       /* Boyer-Moore alone would say 3 */
            {{3, 3, 4, 4}, 4, 0, 0},    /* exactly n/2 is not a majority */
            {{1, 2}, 2, 0, 0},
            {{0}, 0, 0, 0},             /* empty */
            {{1, 1, 2}, 3, 1, 1},
            {{9}, 1, 1, 9},
            {{2, 1, 2, 1, 2}, 5, 1, 2},
        };
        const int nv = (int)(sizeof vcases / sizeof vcases[0]);
        for (int t = 0; t < nv; t++) {
            const VerifyCase *v = &vcases[t];
            int value = 0;
            int found = has_majority(v->nums, v->n, &value);
            snprintf(label, sizeof label, "verify %d, n=%d -> %s", t + 1, v->n,
                     found ? "majority" : "no majority");
            check(found == v->expect_found && (!found || value == v->expect_value), label);
        }
        snprintf(label, sizeof label, "boyer-moore on [1, 2, 3] returns %d without verification",
                 majority_boyer_moore(vcases[0].nums, 3));
        check(majority_boyer_moore(vcases[0].nums, 3) == 3, label);
    }

    /* Random cross-check. Build arrays that DO have a majority: place m > n/2
     * copies of a chosen value, fill the rest randomly, then shuffle. The
     * value pool includes INT_MIN and INT_MAX so the bit approach sees the
     * sign bit. */
    {
        srand(2424);
        const int pool[] = {INT_MIN, -7, -3, -1, 0, 1, 2, 5, 8, INT_MAX};
        const int pool_n = (int)(sizeof pool / sizeof pool[0]);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int nums[MAXN];
            int n = 1 + rand() % MAXN;
            int maj = pool[rand() % pool_n];
            int m = n / 2 + 1 + rand() % (n - n / 2); /* n/2 + 1 .. n copies */
            for (int i = 0; i < n; i++)
                nums[i] = i < m ? maj : pool[rand() % pool_n];
            for (int i = n - 1; i > 0; i--) { /* Fisher-Yates shuffle */
                int j = rand() % (i + 1);
                int t = nums[i];
                nums[i] = nums[j];
                nums[j] = t;
            }
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(nums, n) != maj)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    /* Random check of the verification helper on arrays that may or may not
     * have a majority (tiny value range so both outcomes are common). */
    {
        srand(4242);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int nums[MAXN];
            int n = rand() % (MAXN + 1);
            for (int i = 0; i < n; i++)
                nums[i] = rand() % 3 - 1;
            int want = 0, got = 0;
            int want_found = brute_has_majority(nums, n, &want);
            int got_found = has_majority(nums, n, &got);
            if (want_found != got_found || (want_found && want != got))
                bad++;
        }
        snprintf(label, sizeof label, "random verify check: %d arrays, matches brute force", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
