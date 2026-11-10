/*
 * Q46. Minimum Window Substring
 * Difficulty : Hard
 * Topics     : Strings, Sliding Window, Hashing
 *
 * Problem
 *   Given strings s and t, return the shortest substring of s that contains
 *   every character of t, counting multiplicity (if t has two 'a's, the
 *   window must have at least two 'a's). Return the empty string if there is
 *   no such window. If several windows share the minimum length, return the
 *   leftmost one. Characters are bytes and are case-sensitive.
 *   The answer is reported as (start, length); length 0 means "empty".
 *
 * Example
 *   s = "ADOBECODEBANC", t = "ABC"   ->   (9, 4)   i.e. "BANC"
 *
 * Approaches   (n = |s|, m = |t|)
 *   1. Brute force                 every substring, compare 256 counts    Time O(n^2 * 256)  Space O(1)
 *   2. Extend from each start      stop at the first valid end             Time O(n^2)        Space O(1)
 *   3. Sliding window  [optimal]   need[] counts + "missing" counter       Time O(n + m)      Space O(1)
 *   4. Filtered sliding window     same window, only over t's characters   Time O(n + m)      Space O(n)
 *   The count tables always have 256 entries (one per byte), so they are O(1) space.
 *
 * Key insight
 *   If a window is valid, every window that contains it is valid too. So for
 *   each right end, the best window ending there is found by pushing the left
 *   end forward as far as it can go while the window stays valid. Both ends
 *   only ever move forward, so the whole scan is O(n).
 *   Track need[c] = how many more copies of c the window still needs
 *   (negative means a surplus) and missing = the sum of the positive needs.
 *   Then "is the window valid?" is just missing == 0, instead of comparing
 *   256 counts.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 46_minimum_window_substring.c -o minimum_window_substring && ./minimum_window_substring
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Every approach returns the window length (0 = no window) and writes its
 * start to *out_start (0 when there is no window).
 * Pitfall shared by all of them: char may be signed, so a byte like 0xFF
 * would be a negative array index. Always index count tables with
 * (unsigned char)c.
 * An empty t is satisfied by the empty window, so every approach returns
 * (0, 0) for it.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Look at every substring s[i..j]. For a fixed start i, the counts of
 *   s[i..j] are built up one character at a time as j grows, and each
 *   substring is checked by comparing all 256 counts against t's counts.
 *   Starts are tried left to right and a window only replaces the best one
 *   when it is strictly shorter, so ties keep the leftmost window.
 *   Time O(n^2 * 256)   Space O(1) (two 256-entry tables)
 * ------------------------------------------------------------------------- */
static int window_brute(const char *s, const char *t, int *out_start)
{
    int n = (int)strlen(s), m = (int)strlen(t);
    *out_start = 0;
    if (m == 0)
        return 0;

    int need[256] = {0};
    for (int k = 0; k < m; k++)
        need[(unsigned char)t[k]]++;

    int best_len = 0, best_start = 0;
    for (int i = 0; i < n; i++) {
        int have[256] = {0};
        for (int j = i; j < n; j++) {
            have[(unsigned char)s[j]]++;
            int ok = 1;
            for (int c = 0; c < 256 && ok; c++)
                if (have[c] < need[c])
                    ok = 0;
            int len = j - i + 1;
            if (ok && (best_len == 0 || len < best_len)) {
                best_len = len;
                best_start = i;
            }
        }
    }
    *out_start = best_start;
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 2: For each start, extend until valid
 *   Two improvements over brute force:
 *     - For a fixed start, the FIRST valid end gives the shortest window from
 *       that start; anything longer can be skipped, so stop there.
 *     - Keep a "missing" counter: when a character arrives that the window
 *       still needs (left[c] > 0), missing drops by one. The window is valid
 *       exactly when missing == 0, an O(1) test instead of 256 comparisons.
 *   Resetting the 256-entry table per start adds O(256 n), which is small
 *   next to the O(n^2) scanning.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static int window_extend(const char *s, const char *t, int *out_start)
{
    int n = (int)strlen(s), m = (int)strlen(t);
    *out_start = 0;
    if (m == 0)
        return 0;

    int need[256] = {0};
    for (int k = 0; k < m; k++)
        need[(unsigned char)t[k]]++;

    int best_len = 0, best_start = 0;
    for (int i = 0; i < n; i++) {
        int left[256];
        memcpy(left, need, sizeof left);
        int missing = m;
        for (int j = i; j < n; j++) {
            if (left[(unsigned char)s[j]]-- > 0)
                missing--;
            if (missing == 0) {
                int len = j - i + 1;
                if (best_len == 0 || len < best_len) {
                    best_len = len;
                    best_start = i;
                }
                break;
            }
        }
    }
    *out_start = best_start;
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 3: Sliding window with need counts and a missing counter (optimal)
 *   need[c] starts as the count of c in t; missing starts as m.
 *   Grow: add s[right]. If need[c] was positive the window was short of c,
 *         so missing--. Decrement need[c] either way (it may go negative,
 *         meaning the window has spare copies of c).
 *   Shrink: while missing == 0 the window [left, right] is valid. Record it,
 *         then drop s[left]: need[c]++, and if need[c] becomes positive the
 *         window is now short of c again, so missing++.
 *   Why this finds the answer: let [L, R] be a shortest valid window. Left
 *   only steps past L if [L, right] is valid, and for right < R that would be
 *   a valid window shorter than [L, R], so before right reaches R, left <= L.
 *   At right = R every [left, R] with left <= L is valid, so the loop shrinks
 *   left to exactly L and records [L, R]. Windows are recorded in order of
 *   their start and only a strictly shorter one replaces the best, so among
 *   equal-length windows the leftmost wins.
 *   Each index enters and leaves the window once.
 *   Time O(n + m)   Space O(1)
 * ------------------------------------------------------------------------- */
static int window_sliding(const char *s, const char *t, int *out_start)
{
    int n = (int)strlen(s), m = (int)strlen(t);
    *out_start = 0;
    if (m == 0)
        return 0;

    int need[256] = {0};
    for (int k = 0; k < m; k++)
        need[(unsigned char)t[k]]++;

    int missing = m, left = 0, best_len = 0, best_start = 0;
    for (int right = 0; right < n; right++) {
        unsigned char c = (unsigned char)s[right];
        if (need[c] > 0)
            missing--;
        need[c]--;

        while (missing == 0) {
            int len = right - left + 1;
            if (best_len == 0 || len < best_len) {
                best_len = len;
                best_start = left;
            }
            unsigned char d = (unsigned char)s[left++];
            need[d]++;
            if (need[d] > 0)
                missing++;
        }
    }
    *out_start = best_start;
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 4: Sliding window over the filtered positions
 *   A shortest window always starts and ends on a character of t (otherwise
 *   trimming that end gives a shorter valid window). So first collect the
 *   positions of s whose character appears in t, then run the same sliding
 *   window over that list. The window's real extent is pos[left]..pos[right].
 *   When s is long and only a few of its characters are in t, the window
 *   steps over far fewer positions. Same big-O, smaller constant.
 *   Pitfall: decide "is this character in t?" before the window starts
 *   changing need[], because need[] is decremented as characters arrive.
 *   Time O(n + m)   Space O(n) for the position list
 * ------------------------------------------------------------------------- */
static int window_filtered(const char *s, const char *t, int *out_start)
{
    int n = (int)strlen(s), m = (int)strlen(t);
    *out_start = 0;
    if (m == 0 || n == 0)
        return 0;

    int need[256] = {0};
    for (int k = 0; k < m; k++)
        need[(unsigned char)t[k]]++;

    int *pos = malloc((size_t)n * sizeof *pos);
    if (pos == NULL)
        return 0;
    int cnt = 0;
    for (int i = 0; i < n; i++)
        if (need[(unsigned char)s[i]] > 0)
            pos[cnt++] = i;

    int missing = m, left = 0, best_len = 0, best_start = 0;
    for (int right = 0; right < cnt; right++) {
        unsigned char c = (unsigned char)s[pos[right]];
        if (need[c] > 0)
            missing--;
        need[c]--;

        while (missing == 0) {
            int len = pos[right] - pos[left] + 1;
            if (best_len == 0 || len < best_len) {
                best_len = len;
                best_start = pos[left];
            }
            unsigned char d = (unsigned char)s[pos[left++]];
            need[d]++;
            if (need[d] > 0)
                missing++;
        }
    }
    free(pos);
    *out_start = best_start;
    return best_len;
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

typedef int (*WindowFn)(const char *s, const char *t, int *out_start);

static const struct {
    const char *name;
    WindowFn fn;
} APPROACHES[] = {
    {"brute force", window_brute},
    {"extend from each start", window_extend},
    {"sliding window", window_sliding},
    {"filtered sliding window", window_filtered},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *s;
    const char *t;
    int expect_start, expect_len;
} Case;

int main(void)
{
    printf("Q46. Minimum Window Substring\n");

    /* Expected (start, length) worked out by hand. (0, 0) means "". */
    const Case cases[] = {
        {"ADOBECODEBANC", "ABC", 9, 4},       /* "BANC"                                  */
        {"a", "a", 0, 1},                     /* whole string                            */
        {"a", "aa", 0, 0},                    /* needs two a's, only one exists          */
        {"aa", "aa", 0, 2},                   /* multiplicity satisfied exactly          */
        {"", "a", 0, 0},                      /* empty s                                 */
        {"abc", "", 0, 0},                    /* empty t: empty window                   */
        {"abc", "d", 0, 0},                   /* character of t never appears            */
        {"ab", "abc", 0, 0},                  /* t longer than s                         */
        {"abcba", "ab", 0, 2},                /* tie "ab"(0) vs "ba"(3): leftmost        */
        {"baabab", "aab", 0, 3},              /* "baa"(0) and "aab"(1) tie: leftmost     */
        {"aAbB", "AB", 1, 3},                 /* case-sensitive: "AbB"                   */
        {"aaaa", "aaa", 0, 3},                /* all equal                               */
        {"cabwefgewcwaefgcf", "cae", 9, 4},   /* "cwae"                                  */
        {"x\xffy\xff", "\xff\xff", 1, 3},     /* high bytes: needs unsigned char indexes */
        {"bdab", "ab", 2, 2},                 /* best window is at the very end          */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int c = 0; c < num_cases; c++) {
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int start = -1;
            int len = APPROACHES[a].fn(cases[c].s, cases[c].t, &start);
            snprintf(label, sizeof label, "test %2d, %-24s -> (%d, %d)", c + 1,
                     APPROACHES[a].name, start, len);
            check(start == cases[c].expect_start && len == cases[c].expect_len, label);
        }
    }

    /* Random cross-check. The leftmost-shortest tie rule makes the answer
     * unique, so every approach must return exactly brute force's
     * (start, length). Short strings over a tiny alphabet produce plenty of
     * ties, repeats and "no window" cases. */
    {
        srand(4646);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            char s[16] = {0}, t[6] = {0};
            int n = rand() % 15, m = rand() % 5;
            for (int k = 0; k < n; k++)
                s[k] = (char)('a' + rand() % 3);
            s[n] = '\0';
            for (int k = 0; k < m; k++)
                t[k] = (char)('a' + rand() % (rand() % 4 == 0 ? 4 : 3)); /* 'd' sometimes */
            t[m] = '\0';

            int bstart;
            int blen = window_brute(s, t, &bstart);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int start = -1;
                int len = APPROACHES[a].fn(s, t, &start);
                if (start != bstart || len != blen)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d string pairs, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
