/*
 * Q05. Valid Anagram
 * Difficulty : Easy
 * Topics     : Strings, Hashing, Sorting, Counting
 *
 * Problem
 *   Given two strings s and t, return 1 if t is an anagram of s, that is,
 *   t uses exactly the same characters as s, each exactly as many times,
 *   possibly in a different order. Return 0 otherwise.
 *   Comparison is byte by byte and case-sensitive ('a' and 'A' differ),
 *   and every byte counts, including spaces.
 *
 * Example
 *   s = "anagram", t = "nagaram"   ->   1
 *   s = "rat",     t = "car"       ->   0
 *
 * Approaches
 *   1. Brute force          match each char of s to an unused one in t   Time O(n^2)      Space O(n)
 *   2. Sort both            sorted copies must be identical              Time O(n log n)  Space O(n)
 *   3. Count  [optimal]     one counter per byte value, +1 / -1          Time O(n)        Space O(1)
 *
 * Key insight
 *   Two strings are anagrams exactly when every character occurs the same
 *   number of times in both. A char has only 256 possible values, so an
 *   array of 256 counters holds the whole "histogram" in constant space.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 05_valid_anagram.c -o valid_anagram && ./valid_anagram
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Brute force matching
 *   Different lengths can never be anagrams, so check that first. Then for
 *   each character of s, scan t for an equal character that has not been
 *   used yet and mark it used. If some character finds no partner, the
 *   answer is no. If all n characters are matched, every character of t was
 *   used exactly once, so the multisets are equal.
 *   The "used" marks matter: without them "aab" would wrongly match "abb"
 *   (both a's would pair with t's single 'a').
 *   Returns -1 if out of memory.
 *   Time O(n^2)   Space O(n) for the marks
 * ------------------------------------------------------------------------- */
static int anagram_brute(const char *s, const char *t)
{
    size_t n = strlen(s);
    if (strlen(t) != n)
        return 0;
    if (n == 0)
        return 1; /* two empty strings; also avoids calloc(0) */

    unsigned char *used = calloc(n, 1);
    if (used == NULL)
        return -1;

    int result = 1;
    for (size_t i = 0; i < n && result; i++) {
        size_t j = 0;
        while (j < n && (used[j] || t[j] != s[i]))
            j++;
        if (j == n)
            result = 0; /* s[i] has no unused partner in t */
        else
            used[j] = 1;
    }
    free(used);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort both and compare
 *   Sorting puts equal characters next to each other, so two anagrams turn
 *   into the very same sorted string. Sort copies (the inputs are const),
 *   then compare them byte by byte.
 *   The comparator compares as unsigned char so that bytes >= 0x80 sort
 *   consistently whether plain char is signed or not.
 *   Returns -1 if out of memory; both copies are freed on every path.
 *   Time O(n log n)   Space O(n) for the two copies
 * ------------------------------------------------------------------------- */
static int cmp_byte(const void *pa, const void *pb)
{
    unsigned char x = *(const unsigned char *)pa;
    unsigned char y = *(const unsigned char *)pb;
    return (x > y) - (x < y);
}

static int anagram_sort(const char *s, const char *t)
{
    size_t n = strlen(s);
    if (strlen(t) != n)
        return 0;
    if (n == 0)
        return 1;

    char *a = malloc(n);
    char *b = malloc(n);
    if (a == NULL || b == NULL) {
        free(a); /* free(NULL) is a no-op, so this is safe for either one */
        free(b);
        return -1;
    }
    memcpy(a, s, n);
    memcpy(b, t, n);
    qsort(a, n, 1, cmp_byte);
    qsort(b, n, 1, cmp_byte);
    int result = memcmp(a, b, n) == 0;
    free(a);
    free(b);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Frequency count (optimal)
 *   One counter per possible byte value. Add 1 for every byte of s and
 *   subtract 1 for every byte of t. The strings are anagrams exactly when
 *   every counter ends at 0.
 *   Index with (unsigned char): a plain char can be negative, and a
 *   negative array index is out of bounds.
 *   The table is 256 ints no matter how long the strings are, so the extra
 *   space is O(1). (LeetCode's lowercase-only version needs just 26.)
 *   Time O(n + 256) = O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int anagram_count(const char *s, const char *t)
{
    size_t n = strlen(s);
    if (strlen(t) != n)
        return 0;

    int count[256] = {0};
    for (size_t i = 0; i < n; i++) {
        count[(unsigned char)s[i]]++;
        count[(unsigned char)t[i]]--;
    }
    for (int c = 0; c < 256; c++)
        if (count[c] != 0)
            return 0;
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

typedef int (*AnagramFn)(const char *s, const char *t);

static const struct {
    const char *name;
    AnagramFn fn;
} APPROACHES[] = {
    {"brute force", anagram_brute},
    {"sort both", anagram_sort},
    {"count array", anagram_count},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *s, *t;
    const char *shown; /* printable description for the label */
    int expect;
} Case;

int main(void)
{
    printf("Q05. Valid Anagram\n");

    const Case cases[] = {
        {"anagram", "nagaram", "anagram / nagaram", 1},
        {"rat", "car", "rat / car", 0},
        {"", "", "(empty) / (empty)", 1},
        {"a", "", "a / (empty)  (lengths differ)", 0},
        {"a", "a", "a / a", 1},
        {"ab", "a", "ab / a  (lengths differ)", 0},
        {"aab", "abb", "aab / abb  (same letters, different counts)", 0},
        {"aacc", "ccac", "aacc / ccac", 0},
        {"aaa", "aaa", "aaa / aaa  (all equal)", 1},
        {"abc", "abd", "abc / abd", 0},
        {"listen", "silent", "listen / silent", 1},
        {"Listen", "Silent", "Listen / Silent  (case-sensitive)", 0},
        {"a b", "b a", "\"a b\" / \"b a\"  (spaces count)", 1},
        {"a b", "ab ", "\"a b\" / \"ab \"", 1},
        {"\xff\x01" "z", "z\x01\xff", "\\xff\\x01z / z\\x01\\xff  (high bytes)", 1},
        {"\xff\xff", "\xfe\xff", "\\xff\\xff / \\xfe\\xff", 0},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[192];
    for (int k = 0; k < num_cases; k++) {
        const Case *c = &cases[k];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->s, c->t);
            snprintf(label, sizeof label, "test %2d, %-11s %s -> %d", k + 1, APPROACHES[a].name,
                     c->shown, got);
            check(got == c->expect, label);
        }
    }

    /* Random cross-check against brute force. A tiny alphabet makes
     * accidental anagrams common; on top of that, half the time t is built
     * as a shuffle of s, so it is an anagram for sure. */
    {
        srand(9001);
        int bad = 0, yes = 0, no = 0;
        const int rounds = 600;
        for (int r = 0; r < rounds; r++) {
            char s[16], t[16];
            int n = rand() % 13; /* 0..12 */
            for (int k = 0; k < n; k++)
                s[k] = (char)('a' + rand() % 3);
            s[n] = '\0';

            if (rand() % 2) {
                memcpy(t, s, (size_t)n + 1);
                for (int k = n - 1; k > 0; k--) { /* Fisher-Yates shuffle */
                    int j = rand() % (k + 1);
                    char tmp = t[k];
                    t[k] = t[j];
                    t[j] = tmp;
                }
            } else {
                int m = n + rand() % 3 - 1; /* length n-1, n or n+1 */
                if (m < 0)
                    m = 0;
                for (int k = 0; k < m; k++)
                    t[k] = (char)('a' + rand() % 3);
                t[m] = '\0';
            }

            int expect = anagram_brute(s, t);
            if (expect == 1)
                yes++;
            else if (expect == 0)
                no++;
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(s, t) != expect)
                    bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d pairs (%d anagrams, %d not), all approaches agree",
                 rounds, yes, no);
        check(bad == 0 && yes > 0 && no > 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
