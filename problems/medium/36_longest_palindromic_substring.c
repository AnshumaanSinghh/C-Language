/*
 * Q36. Longest Palindromic Substring
 * Difficulty : Medium
 * Topics     : Strings, Dynamic Programming, Two Pointers, Manacher
 *
 * Problem
 *   Given a string s of length n, find a longest contiguous substring that
 *   reads the same forwards and backwards. Return its start index and its
 *   length. If several substrings tie, any one of them is accepted. The
 *   empty string has answer (start 0, length 0).
 *
 * Example
 *   s = "babad"   ->   start 0, length 3 ("bab"; "aba" is also correct)
 *   s = "cbbd"    ->   start 1, length 2 ("bb")
 *
 * Approaches
 *   1. Brute force             test every substring             Time O(n^3)  Space O(1)
 *   2. DP table                pal[i][j] from pal[i+1][j-1]     Time O(n^2)  Space O(n^2)
 *   3. Expand around centre    grow from each of 2n-1 centres   Time O(n^2)  Space O(1)   [best practical]
 *   4. Manacher    [optimal]   reuse mirror radii               Time O(n)    Space O(n)
 *
 * Key insight
 *   A palindrome is symmetric around its centre, and it stays a palindrome
 *   when you strip one character from each end. So palindromes can be
 *   grown outward from a centre one layer at a time, and Manacher's trick
 *   is that a palindrome's left half already tells you a lot about the
 *   palindromes centred in its right half.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 36_longest_palindromic_substring.c -o lps && ./lps
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_palindrome(const char *s, int lo, int hi) /* s[lo..hi] inclusive */
{
    while (lo < hi)
        if (s[lo++] != s[hi--])
            return 0;
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Try every substring s[i..j], test it with two pointers, and keep the
 *   longest. Only test substrings longer than the best found so far; that
 *   skips work but does not change the worst case.
 *   Time O(n^3): O(n^2) substrings, O(n) per test.   Space O(1)
 * ------------------------------------------------------------------------- */
static int lps_brute(const char *s, int *start)
{
    int n = (int)strlen(s);
    int best_len = 0;
    *start = 0;
    for (int i = 0; i < n; i++) {
        for (int j = i + best_len; j < n; j++) {
            if (is_palindrome(s, i, j)) {
                best_len = j - i + 1;
                *start = i;
            }
        }
    }
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 2: DP table
 *   pal[i][j] = 1 if s[i..j] is a palindrome:
 *     length 1:   always
 *     length 2:   s[i] == s[i+1]
 *     length 3+:  s[i] == s[j] and pal[i+1][j-1]   (strip both ends)
 *   pal[i][j] needs the shorter s[i+1..j-1], so fill by increasing length.
 *   Each cell is O(1) work, which removes the O(n) test of approach 1.
 *   Pitfall: n*n bytes. For n = 10^5 that is 10 GB, so this is a teaching
 *   step, not something to ship.
 *   Time O(n^2)   Space O(n^2)
 * ------------------------------------------------------------------------- */
static int lps_dp(const char *s, int *start)
{
    int n = (int)strlen(s);
    *start = 0;
    if (n == 0)
        return 0;
    size_t un = (size_t)n;
    unsigned char *pal = calloc(un * un, 1);
    if (pal == NULL)
        return 0;

    int best_len = 1;
    for (int i = 0; i < n; i++)
        pal[(size_t)i * un + (size_t)i] = 1;
    for (int len = 2; len <= n; len++) {
        for (int i = 0; i + len - 1 < n; i++) {
            int j = i + len - 1;
            if (s[i] == s[j] && (len == 2 || pal[(size_t)(i + 1) * un + (size_t)(j - 1)])) {
                pal[(size_t)i * un + (size_t)j] = 1;
                if (len > best_len) {
                    best_len = len;
                    *start = i;
                }
            }
        }
    }
    free(pal);
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 3: Expand around the centre (best practical)
 *   Every palindrome has a centre: a character (odd length, n choices) or
 *   the gap between two characters (even length, n - 1 choices). From each
 *   centre push lo left and hi right while s[lo] == s[hi]. The last good
 *   window is the longest palindrome with that centre.
 *   Why it is correct: the longest palindrome has some centre, and its
 *   expansion reaches it because every inner layer is also a palindrome.
 *   Pitfall: forgetting even-length centres misses "abba".
 *   Time O(n^2) worst case (e.g. "aaaa...a"), but usually far less, and
 *   no memory at all, which is why it is the usual interview answer.
 *   Space O(1)
 * ------------------------------------------------------------------------- */
static int expand(const char *s, int n, int lo, int hi)
{
    while (lo >= 0 && hi < n && s[lo] == s[hi]) {
        lo--;
        hi++;
    }
    return hi - lo - 1; /* window that last matched is lo+1 .. hi-1 */
}

static int lps_expand(const char *s, int *start)
{
    int n = (int)strlen(s);
    int best_len = 0;
    *start = 0;
    for (int c = 0; c < n; c++) {
        int odd = expand(s, n, c, c);
        int even = expand(s, n, c, c + 1);
        int len = odd > even ? odd : even;
        if (len > best_len) {
            best_len = len;
            /* both kinds are centred so that start = c - (len - 1) / 2 */
            *start = c - (len - 1) / 2;
        }
    }
    return best_len;
}

/* -------------------------------------------------------------------------
 * Approach 4: Manacher's algorithm (optimal)
 *   Step 1: remove the odd/even split by putting a separator between all
 *   characters and at both ends: "abba" -> "|a|b|b|a|" (length 2n + 1).
 *   Now every palindrome has a single centre position in t. Here t holds
 *   ints: -1 for the separator and 0..255 for characters. The separator's
 *   value does not actually matter: the two positions compared, i - k and
 *   i + k, always have the same parity, so a separator is only ever
 *   compared with another separator. (It does matter in the common variant
 *   that puts '^' and '$' sentinels at the ends instead of checking
 *   bounds: those must differ from each other and from every character.)
 *
 *   Step 2: p[i] = radius of the longest palindrome in t centred at i. A
 *   radius of p[i] in t corresponds to a palindrome of length p[i] in s,
 *   starting at s index (i - p[i]) / 2.
 *
 *   Step 3, the trick: remember the palindrome found so far that reaches
 *   furthest right, by its centre c and right edge r = c + p[c]. For a new
 *   i < r, its mirror is m = 2c - i. Inside that big palindrome the
 *   surroundings of i and m are mirror images, so p[i] >= min(p[m], r - i)
 *   without comparing anything. Only then expand further by comparison.
 *   (The min matters: beyond r we know nothing, so the mirror's radius can
 *   only be trusted up to the edge.)
 *
 *   Why O(n): every successful comparison in the expand loop pushes r one
 *   step to the right, and r never moves left and is at most 2n + 1. So
 *   the total number of successful comparisons over the whole run is O(n),
 *   plus at most one failed comparison per i.
 *   Time O(n)   Space O(n) for t and p
 * ------------------------------------------------------------------------- */
static int lps_manacher(const char *s, int *start)
{
    int n = (int)strlen(s);
    *start = 0;
    if (n == 0)
        return 0;
    int m = 2 * n + 1;
    int *t = malloc((size_t)m * sizeof *t);
    int *p = malloc((size_t)m * sizeof *p);
    int best_len = 0;
    if (t != NULL && p != NULL) {
        for (int i = 0; i < m; i++)
            t[i] = (i % 2 == 0) ? -1 : (unsigned char)s[i / 2];

        int c = 0, r = 0; /* centre and right edge of the furthest-right palindrome */
        for (int i = 0; i < m; i++) {
            int rad = 0;
            if (i < r) {
                int mirror = 2 * c - i;
                rad = p[mirror] < r - i ? p[mirror] : r - i;
            }
            while (i - rad - 1 >= 0 && i + rad + 1 < m && t[i - rad - 1] == t[i + rad + 1])
                rad++;
            p[i] = rad;
            if (i + rad > r) {
                c = i;
                r = i + rad;
            }
            if (rad > best_len) {
                best_len = rad;
                *start = (i - rad) / 2;
            }
        }
    }
    free(t);
    free(p);
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

typedef int (*LpsFn)(const char *s, int *start);

/* max_len: longest string an approach is run on in these tests, so the
 * O(n^3) brute force stays fast. */
static const struct {
    const char *name;
    LpsFn fn;
    int max_len;
} APPROACHES[] = {
    {"brute force", lps_brute, 200},
    {"DP table", lps_dp, 1 << 30},
    {"expand centre", lps_expand, 1 << 30},
    {"Manacher", lps_manacher, 1 << 30},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

/* An answer is correct if it has the expected length and the substring it
 * names lies inside s and really is a palindrome. */
static int answer_ok(const char *s, int start, int len, int expect_len)
{
    int n = (int)strlen(s);
    if (len != expect_len || start < 0 || start + len > n)
        return 0;
    return len == 0 || is_palindrome(s, start, start + len - 1);
}

typedef struct {
    const char *s;
    int expect_len;
} Case;

int main(void)
{
    printf("Q36. Longest Palindromic Substring\n");

    /* Expected lengths worked out by hand. */
    const Case cases[] = {
        {"babad", 3},             /* bab or aba */
        {"cbbd", 2},              /* bb */
        {"a", 1},
        {"", 0},
        {"ac", 1},                /* any single char */
        {"racecar", 7},           /* whole string */
        {"aaaa", 4},              /* all equal, even length */
        {"aaaaa", 5},             /* all equal, odd length */
        {"forgeeksskeegfor", 10}, /* geeksskeeg */
        {"abacdfgdcaba", 3},      /* aba; "cdfgdc" is NOT a palindrome */
        {"abcdef", 1},            /* no repeats */
        {"aab", 2},               /* aa at the start */
        {"baa", 2},               /* aa at the end */
        {"abb", 2},
        {"xabbay", 4},            /* abba in the middle */
        {"#a#", 3},               /* the usual separator character in the input */
        {"\xff\xfe\xff", 3},      /* bytes > 127 (negative if char is signed) */
        {"a\xff\xff" "b", 2},      /* \xff\xff; split literal so "b" is not hex */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int start = -1;
            int len = APPROACHES[a].fn(c->s, &start);
            snprintf(label, sizeof label, "test %2d (len %2d), %-14s -> start %d, len %d", t + 1,
                     (int)strlen(c->s), APPROACHES[a].name, start, len);
            check(answer_ok(c->s, start, len, c->expect_len), label);
        }
    }

    /* Long strings: 3000 'a's then 'b' (answer 3000) and "ab" repeated 1500
     * times (an odd-length alternating palindrome of length 2999). Brute
     * force is skipped (O(n^3) = 2.7 * 10^10). */
    {
        enum { N = 3000 };
        char *x = malloc(N + 2), *y = malloc(N + 1);
        if (x != NULL && y != NULL) {
            memset(x, 'a', N);
            x[N] = 'b';
            x[N + 1] = '\0';
            for (int i = 0; i < N; i++)
                y[i] = (i % 2 == 0) ? 'a' : 'b';
            y[N] = '\0';
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (N > APPROACHES[a].max_len)
                    continue;
                int s1, s2;
                int l1 = APPROACHES[a].fn(x, &s1);
                int l2 = APPROACHES[a].fn(y, &s2);
                snprintf(label, sizeof label, "n=%d long strings, %-14s -> %d and %d", N,
                         APPROACHES[a].name, l1, l2);
                check(answer_ok(x, s1, l1, N) && answer_ok(y, s2, l2, N - 1), label);
            }
        } else {
            check(0, "allocation for long strings");
        }
        free(x);
        free(y);
    }

    /* Random cross-check: lengths must match brute force, and the returned
     * substring must be a real palindrome. Small alphabets make long
     * palindromes (and ties) common. */
    {
        srand(36);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            char s[31];
            int n = rand() % 31;
            int alpha = 1 + rand() % 3;
            for (int i = 0; i < n; i++)
                s[i] = (char)('a' + rand() % alpha);
            s[n] = '\0';
            int bs;
            int expect = lps_brute(s, &bs);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int start = -1;
                int len = APPROACHES[a].fn(s, &start);
                if (!answer_ok(s, start, len, expect))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d strings, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
