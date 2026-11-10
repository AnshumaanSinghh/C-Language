/*
 * Q26. Longest Substring Without Repeating Characters
 * Difficulty : Medium
 * Topics     : Strings, Sliding Window, Hashing
 *
 * Problem
 *   Given a string s of n bytes, return the length of the longest substring
 *   (contiguous run) in which no byte appears twice. Also report where one
 *   such longest substring starts.
 *
 * Example
 *   s = "abcabcbb"   ->   3   ("abc", starting at index 0)
 *   s = "pwwkew"     ->   3   ("wke", starting at index 2; "pwke" is not
 *                              contiguous)
 *
 * Approaches
 *   1. Every substring           check each one for repeats       Time O(n^3)  Space O(256)
 *   2. Extend from each start    grow until the first repeat      Time O(n^2)  Space O(256)
 *   3. Sliding window + set      shrink one step at a time        Time O(n)    Space O(256)
 *   4. Last-seen jump [optimal]  jump left past the old copy      Time O(n)    Space O(256)
 *
 * Key insight
 *   If s[l..r] has no repeats, so does every substring inside it. So a
 *   window [left, right] can slide forward: grow it on the right, and when
 *   the new byte is already inside, move left just past the earlier copy.
 *   Neither end ever moves backwards, so the whole scan is linear.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 26_longest_substring_no_repeat.c -o longest_substring_no_repeat && ./longest_substring_no_repeat
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Every approach returns the best length and writes the start index of one
 * longest substring to *out_start (0 for the empty string). Several
 * substrings can tie, so approaches may report different starts.
 *
 * Pitfall: plain char may be signed, so a byte like 0xFF read as char is
 * -1 and would index seen[-1]. Always convert to unsigned char before using
 * a byte as a table index.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Every substring
 *   For every start i and end j, test whether s[i..j] has a repeated byte
 *   using a fresh 256-entry table. Keep the longest that passes.
 *   O(n^2) substrings, each checked in O(length), so O(n^3) overall.
 *   Time O(n^3)   Space O(256)
 * ------------------------------------------------------------------------- */
static int all_unique(const char *s, int i, int j)
{
    unsigned char seen[256] = {0};
    for (int k = i; k <= j; k++) {
        unsigned char c = (unsigned char)s[k];
        if (seen[c])
            return 0;
        seen[c] = 1;
    }
    return 1;
}

static int longest_every_substring(const char *s, int n, int *out_start)
{
    int best = 0, best_start = 0;
    for (int i = 0; i < n; i++)
        for (int j = i; j < n; j++)
            if (j - i + 1 > best && all_unique(s, i, j)) {
                best = j - i + 1;
                best_start = i;
            }
    *out_start = best_start;
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 2: Extend from each start
 *   Approach 1 re-checks s[i..j] from scratch even though it just checked
 *   s[i..j-1]. Instead, for each start i walk j forward while remembering
 *   which bytes are in s[i..j-1]; the first repeat ends this start (any
 *   longer substring from i would contain the same repeat).
 *   Time O(n^2)   Space O(256)
 * ------------------------------------------------------------------------- */
static int longest_extend(const char *s, int n, int *out_start)
{
    int best = 0, best_start = 0;
    for (int i = 0; i < n; i++) {
        unsigned char seen[256] = {0};
        int j = i;
        while (j < n && !seen[(unsigned char)s[j]]) {
            seen[(unsigned char)s[j]] = 1;
            j++;
        }
        if (j - i > best) {
            best = j - i;
            best_start = i;
        }
    }
    *out_start = best_start;
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 3: Sliding window with a set, shrinking one step at a time
 *   Keep the window s[left..right-1] repeat-free and a table in[] of the
 *   bytes it contains. To add s[right]: while it is already in the window,
 *   remove s[left] and left++. Then add s[right].
 *   Each index enters the window once and leaves at most once, so the inner
 *   while loop does O(n) work in total across the whole run, even though
 *   it is nested.
 *   Time O(n)   Space O(256)
 * ------------------------------------------------------------------------- */
static int longest_window_set(const char *s, int n, int *out_start)
{
    unsigned char in[256] = {0};
    int best = 0, best_start = 0, left = 0;
    for (int right = 0; right < n; right++) {
        unsigned char c = (unsigned char)s[right];
        while (in[c]) {
            in[(unsigned char)s[left]] = 0;
            left++;
        }
        in[c] = 1;
        if (right - left + 1 > best) {
            best = right - left + 1;
            best_start = left;
        }
    }
    *out_start = best_start;
    return best;
}

/* -------------------------------------------------------------------------
 * Approach 4: Sliding window with last-seen index (optimal)
 *   Instead of shrinking one byte at a time, remember last[c] = the most
 *   recent index of byte c. When s[right] == c was last seen at an index
 *   >= left, it is inside the window, so jump left straight to last[c] + 1.
 *   Pitfall: the "last[c] >= left" test matters. In "abba", when the final
 *   'a' arrives, last['a'] = 0 is already outside the window [2, 2]; jumping
 *   to 1 would move left BACKWARDS and wrongly report 3.
 *   Time O(n)   Space O(256)
 * ------------------------------------------------------------------------- */
static int longest_last_seen(const char *s, int n, int *out_start)
{
    int last[256];
    for (int c = 0; c < 256; c++)
        last[c] = -1;
    int best = 0, best_start = 0, left = 0;
    for (int right = 0; right < n; right++) {
        unsigned char c = (unsigned char)s[right];
        if (last[c] >= left)
            left = last[c] + 1;
        last[c] = right;
        if (right - left + 1 > best) {
            best = right - left + 1;
            best_start = left;
        }
    }
    *out_start = best_start;
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

typedef int (*LongestFn)(const char *s, int n, int *out_start);

static const struct {
    const char *name;
    LongestFn fn;
} APPROACHES[] = {
    {"every substring", longest_every_substring},
    {"extend each start", longest_extend},
    {"window + set", longest_window_set},
    {"last-seen jump", longest_last_seen},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *s;
    int expect;
} Case;

/* A reported start is valid if s[start .. start+len-1] lies inside the
 * string and has no repeated byte. */
static int window_valid(const char *s, int n, int start, int len)
{
    if (len == 0)
        return start == 0;
    return start >= 0 && start + len <= n && all_unique(s, start, start + len - 1);
}

int main(void)
{
    printf("Q26. Longest Substring Without Repeating Characters\n");

    /* Expected lengths found by hand. */
    const Case cases[] = {
        {"abcabcbb", 3},                    /* "abc" */
        {"bbbbb", 1},                       /* all equal */
        {"pwwkew", 3},                      /* "wke" */
        {"", 0},                            /* empty */
        {" ", 1},                           /* a space is a byte like any other */
        {"au", 2},
        {"dvdf", 3},                        /* "vdf": restart must keep the 'v' */
        {"abba", 2},                        /* last-seen jump must not go back */
        {"tmmzuxt", 5},                     /* "mzuxt" */
        {"aab", 2},
        {"abcdefghijklmnopqrstuvwxyz", 26}, /* no repeats at all */
        {"\xff\xfe\xff\x01", 3},            /* high bytes: signed char trap */
        {"a1!a1!", 3},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        int n = (int)strlen(c->s);
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int start = -1;
            int len = APPROACHES[a].fn(c->s, n, &start);
            snprintf(label, sizeof label, "test %2d, %-17s -> %d (start %d)", t + 1,
                     APPROACHES[a].name, len, start);
            check(len == c->expect && window_valid(c->s, n, start, len), label);
        }
    }

    /* Random cross-check: random strings over small alphabets (1 to 6
     * letters, so repeats are frequent), length up to 30. Every approach
     * must match the brute-force length and report a valid window. */
    {
        srand(2626);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            char s[31];
            int n = rand() % 31;
            int alphabet = 1 + rand() % 6;
            for (int i = 0; i < n; i++)
                s[i] = (char)('a' + rand() % alphabet);
            s[n] = '\0';

            int bstart;
            int expect = longest_every_substring(s, n, &bstart);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int start = -1;
                int len = APPROACHES[a].fn(s, n, &start);
                if (len != expect || !window_valid(s, n, start, len))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d strings, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
