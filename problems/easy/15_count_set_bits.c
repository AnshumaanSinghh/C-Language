/*
 * Q15. Count Set Bits
 * Difficulty : Easy
 * Topics     : Bit Manipulation, Lookup Table, Dynamic Programming
 *
 * Problem
 *   Given an unsigned 32-bit integer x, return how many of its bits are 1
 *   (its "population count" or Hamming weight).
 *   Extension (LeetCode 338): for every i in 0..n, return the count for i.
 *
 * Example
 *   x = 11 = 0b1011           ->   3
 *   x = 0xFFFFFFFF            ->   32
 *   n = 5 (extension)         ->   [0, 1, 1, 2, 1, 2]
 *
 * Approaches
 *   1. Test every bit          shift and test all 32 bits       Time O(32)       Space O(1)
 *   2. Brian Kernighan         x &= x - 1 clears the lowest 1   Time O(set bits) Space O(1)
 *   3. Lookup table            256-entry table, 4 byte lookups  Time O(1)        Space O(256)
 *   4. SWAR       [optimal]    add bit counts in parallel       Time O(1)        Space O(1)
 *   Extension: naive per-number O(n * set bits)  vs  DP bits[i] = bits[i >> 1] + (i & 1) O(n)
 *
 * Key insight
 *   A register can be treated as many small counters side by side. SWAR
 *   ("SIMD within a register") adds neighbouring 1-bit counts into 2-bit
 *   counts, then 4-bit, then 8-bit, using a handful of shifts, masks and
 *   adds: no loop, no branch, no table.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 15_count_set_bits.c -o count_set_bits && ./count_set_bits
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------
 * Approach 1: Test every bit (brute force)
 *   (x >> i) & 1 is bit i. Add them up for i = 0..31. Always 32 steps, even
 *   for x = 0. Shift the value, not a mask: 1 << 31 on a signed int is
 *   undefined behaviour, which is why everything here is unsigned.
 *   Time O(32)   Space O(1)
 * ------------------------------------------------------------------------- */
static int popcount_each_bit(uint32_t x)
{
    int count = 0;
    for (int i = 0; i < 32; i++)
        count += (int)((x >> i) & 1u);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Brian Kernighan's trick
 *   Subtracting 1 flips the lowest set bit to 0 and every 0 below it to 1,
 *   e.g. 0b10100 - 1 = 0b10011. ANDing with the original therefore clears
 *   exactly that lowest set bit: 0b10100 & 0b10011 = 0b10000. Repeat until
 *   x is 0; the number of rounds is the number of set bits.
 *   Fast for sparse values, still 32 rounds for 0xFFFFFFFF.
 *   Time O(number of set bits)   Space O(1)
 * ------------------------------------------------------------------------- */
static int popcount_kernighan(uint32_t x)
{
    int count = 0;
    while (x != 0) {
        x &= x - 1;
        count++;
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: 256-entry lookup table
 *   Precompute the count for every byte 0..255, then split x into 4 bytes
 *   and add 4 table entries. The table is built with the same recurrence as
 *   the extension below: count(i) = count(i / 2) + (i & 1).
 *   It is built on the first call here; real code would usually fill it at
 *   start-up or write it out as a constant array.
 *   Time O(1) per call (4 lookups)   Space O(256)
 * ------------------------------------------------------------------------- */
static int popcount_table(uint32_t x)
{
    static unsigned char table[256]; /* static, so table[0] starts as 0 */
    static int ready = 0;
    if (!ready) {
        for (int i = 1; i < 256; i++)
            table[i] = (unsigned char)((i & 1) + table[i >> 1]);
        ready = 1;
    }
    return table[x & 0xFFu] + table[(x >> 8) & 0xFFu] + table[(x >> 16) & 0xFFu] + table[x >> 24];
}

/* -------------------------------------------------------------------------
 * Approach 4: SWAR parallel bit count (optimal)
 *   Step 1: view x as 16 two-bit fields "ab". The field's value is 2a + b,
 *           its bit count is a + b = (2a + b) - a. Subtracting
 *           (x >> 1) & 0x5555... does that for all 16 fields at once, and
 *           no field can borrow from its neighbour because 2a + b >= a.
 *   Step 2: add neighbouring 2-bit counts into 8 four-bit fields
 *           (mask 0x3333..., each sum is at most 4).
 *   Step 3: add neighbouring 4-bit counts into 4 bytes. Each sum is at most
 *           8, which fits in 4 bits, so we can add first and mask after.
 *   Step 4: multiplying by 0x01010101 adds the 4 bytes into the top byte
 *           (b3 + b2 + b1 + b0 lands in bits 24..31; at most 32, no carry
 *           out). Shift it down.
 *   Constant number of operations and no branches.
 *   Time O(1)   Space O(1)
 * ------------------------------------------------------------------------- */
static int popcount_swar(uint32_t x)
{
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    return (int)((uint32_t)(x * 0x01010101u) >> 24);
}

/* -------------------------------------------------------------------------
 * Extension: count bits for every i in 0..n (LeetCode 338)
 *   Naive: run Kernighan on each number. Time O(n * bits per number).
 *   DP:    i >> 1 is i with its lowest bit dropped, and it is smaller than i,
 *          so its count is already known. Add back the dropped bit (i & 1).
 *          bits[i] = bits[i >> 1] + (i & 1). Time O(n), one add per number.
 *          (Another valid recurrence: bits[i] = bits[i & (i - 1)] + 1.)
 *   Both fill out[0..n], so out must have room for n + 1 ints.
 * ------------------------------------------------------------------------- */
static void count_bits_naive(int n, int *out)
{
    for (int i = 0; i <= n; i++)
        out[i] = popcount_kernighan((uint32_t)i);
}

static void count_bits_dp(int n, int *out)
{
    out[0] = 0;
    for (int i = 1; i <= n; i++)
        out[i] = out[i >> 1] + (i & 1);
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

typedef int (*PopcountFn)(uint32_t x);

static const struct {
    const char *name;
    PopcountFn fn;
} APPROACHES[] = {
    {"test every bit", popcount_each_bit},
    {"kernighan", popcount_kernighan},
    {"lookup table", popcount_table},
    {"swar", popcount_swar},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef void (*CountBitsFn)(int n, int *out);

static const struct {
    const char *name;
    CountBitsFn fn;
} RANGE_APPROACHES[] = {
    {"naive", count_bits_naive},
    {"dp", count_bits_dp},
};
#define NUM_RANGE_APPROACHES ((int)(sizeof RANGE_APPROACHES / sizeof RANGE_APPROACHES[0]))

typedef struct {
    uint32_t x;
    int expect;
} Case;

typedef struct {
    int n;
    int expect[17];
} RangeCase;

/* rand() is only guaranteed to give 15 random bits, so stitch three calls
 * together (11 + 11 + 10 bits) to cover all 32. */
static uint32_t rand32(void)
{
    uint32_t hi = (uint32_t)(rand() & 0x7FF);
    uint32_t mid = (uint32_t)(rand() & 0x7FF);
    uint32_t lo = (uint32_t)(rand() & 0x3FF);
    return (hi << 21) | (mid << 10) | lo;
}

int main(void)
{
    printf("Q15. Count Set Bits\n");

    const Case cases[] = {
        {0x00000000u, 0},
        {0x00000001u, 1},
        {0x80000000u, 1},  /* top bit only */
        {0xFFFFFFFFu, 32}, /* every bit */
        {0xFFFFFFFEu, 31},
        {0x7FFFFFFFu, 31},
        {0xAAAAAAAAu, 16}, /* 1010... alternating */
        {0x55555555u, 16}, /* 0101... alternating */
        {0x0F0F0F0Fu, 16},
        {0xF0F0F0F0u, 16},
        {0x0000FFFFu, 16},
        {0x00000100u, 1},  /* first bit of the second byte */
        {0x01010101u, 4},  /* one bit per byte */
        {11u, 3},          /* 0b1011 */
        {0x12345678u, 13}, /* nibbles 1+1+2+1+2+2+3+1 */
        {0xDEADBEEFu, 24}, /* nibbles 3+3+2+3+3+3+3+4 */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(cases[t].x);
            snprintf(label, sizeof label, "0x%08lX, %-14s -> %d", (unsigned long)cases[t].x,
                     APPROACHES[a].name, got);
            check(got == cases[t].expect, label);
        }
    }

    /* Every power of two has exactly one set bit, and 2^k - 1 has exactly k. */
    for (int a = 0; a < NUM_APPROACHES; a++) {
        int ok = 1;
        for (int k = 0; k < 32; k++) {
            uint32_t p = (uint32_t)1 << k;
            if (APPROACHES[a].fn(p) != 1 || APPROACHES[a].fn(p - 1) != k)
                ok = 0;
        }
        snprintf(label, sizeof label, "2^k and 2^k - 1 for k = 0..31, %s", APPROACHES[a].name);
        check(ok, label);
    }

    /* Random cross-check: mix plain, sparse (a & b) and dense (a | b) values
     * so the bit counts are spread out, and compare against approach 1. */
    {
        srand(1515);
        int bad = 0;
        const int rounds = 2000;
        for (int r = 0; r < rounds; r++) {
            uint32_t x = rand32();
            if (r % 3 == 1)
                x &= rand32();
            else if (r % 3 == 2)
                x |= rand32();

            int expect = popcount_each_bit(x);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(x) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d values, all approaches agree", rounds);
        check(bad == 0, label);
    }

    /* Extension: counts for 0..n. */
    {
        const RangeCase rcases[] = {
            {0, {0}},
            {2, {0, 1, 1}},
            {5, {0, 1, 1, 2, 1, 2}},
            {16, {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1}},
        };
        const int num_rcases = (int)(sizeof rcases / sizeof rcases[0]);
        for (int t = 0; t < num_rcases; t++) {
            for (int a = 0; a < NUM_RANGE_APPROACHES; a++) {
                int out[17];
                RANGE_APPROACHES[a].fn(rcases[t].n, out);
                int ok = 1;
                for (int i = 0; i <= rcases[t].n; i++)
                    if (out[i] != rcases[t].expect[i])
                        ok = 0;
                snprintf(label, sizeof label, "counts for 0..%d, %s", rcases[t].n, RANGE_APPROACHES[a].name);
                check(ok, label);
            }
        }

        /* One large range: DP must match naive everywhere. */
        const int n = 100000;
        int *naive = malloc(((size_t)n + 1) * sizeof *naive);
        int *dp = malloc(((size_t)n + 1) * sizeof *dp);
        if (naive == NULL || dp == NULL) {
            check(0, "counts for 0..100000: allocation failed");
        } else {
            count_bits_naive(n, naive);
            count_bits_dp(n, dp);
            int ok = 1;
            for (int i = 0; i <= n; i++)
                if (naive[i] != dp[i] || dp[i] != popcount_swar((uint32_t)i))
                    ok = 0;
            snprintf(label, sizeof label, "counts for 0..%d, dp matches naive and swar", n);
            check(ok, label);
        }
        free(naive);
        free(dp);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
