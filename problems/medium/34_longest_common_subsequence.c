/*
 * Q34. Longest Common Subsequence
 * Difficulty : Medium
 * Topics     : Strings, Dynamic Programming
 *
 * Problem
 *   Given two strings a and b, return the length of their longest common
 *   subsequence (LCS), and build one such subsequence. A subsequence keeps
 *   characters in their original order but may skip any of them: "ace" is
 *   a subsequence of "abcde", "aec" is not.
 *
 * Example
 *   a = "abcde", b = "ace"   ->   3   ("ace")
 *   a = "abc",   b = "def"   ->   0   ("")
 *
 * Approaches
 *   1. Plain recursion          match or skip one char each call   Time O(2^(m+n))  Space O(m+n)
 *   2. Memoization              cache each (i, j) suffix pair      Time O(m*n)      Space O(m*n)
 *   3. Tabulation + rebuild     full table, walk back for a string Time O(m*n)      Space O(m*n)
 *   4. Two rows     [optimal]   keep only the previous table row   Time O(m*n)      Space O(min(m,n))
 *
 *   Approach 3 is the one to use when you need the subsequence itself;
 *   approach 4 is optimal when only the length is needed.
 *
 * Key insight
 *   Compare the first characters. If they match, they can both be used:
 *   LCS = 1 + LCS(rest, rest). If not, at least one of them is not in the
 *   LCS, so LCS = max(LCS(skip a's), LCS(skip b's)). There are only
 *   (m+1)(n+1) different (suffix of a, suffix of b) pairs.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 34_longest_common_subsequence.c -o lcs && ./lcs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Plain recursion (brute force)
 *   go(i, j) = LCS of the suffixes a[i..] and b[j..]:
 *     either suffix empty      -> 0
 *     a[i] == b[j]             -> 1 + go(i+1, j+1)
 *     otherwise                -> max(go(i+1, j), go(i, j+1))
 *   Why taking a match is always safe: if a[i] == b[j], any common
 *   subsequence that does not use both can be changed to use them without
 *   getting shorter, so there is an optimal answer that matches them.
 *   Every mismatch branches in two, so the call tree has up to 2^(m+n)
 *   leaves. Only short strings are used with it in the tests (<= 10).
 *   Time O(2^(m+n))   Space O(m+n) recursion depth
 * ------------------------------------------------------------------------- */
static int lcs_rec_go(const char *a, const char *b)
{
    if (*a == '\0' || *b == '\0')
        return 0;
    if (*a == *b)
        return 1 + lcs_rec_go(a + 1, b + 1);
    int skip_a = lcs_rec_go(a + 1, b);
    int skip_b = lcs_rec_go(a, b + 1);
    return skip_a > skip_b ? skip_a : skip_b;
}

static int lcs_recursive(const char *a, const char *b)
{
    return lcs_rec_go(a, b);
}

/* -------------------------------------------------------------------------
 * Approach 2: Memoization (top-down DP)
 *   The recursion only ever asks about (i, j) pairs, and there are only
 *   (m+1)(n+1) of them. Store each answer the first time it is computed
 *   in memo[i*(n+1) + j] (-1 = not computed yet) and reuse it afterwards.
 *   Pitfall: recursion depth is up to m + n, which is fine for strings of
 *   a few thousand characters but not for millions.
 *   Time O(m*n)   Space O(m*n) memo + O(m+n) stack
 * ------------------------------------------------------------------------- */
static int lcs_memo_go(const char *a, int m, const char *b, int n, int i, int j, int *memo)
{
    if (i == m || j == n)
        return 0;
    int *cell = &memo[(size_t)i * ((size_t)n + 1) + (size_t)j];
    if (*cell >= 0)
        return *cell;
    if (a[i] == b[j]) {
        *cell = 1 + lcs_memo_go(a, m, b, n, i + 1, j + 1, memo);
    } else {
        int skip_a = lcs_memo_go(a, m, b, n, i + 1, j, memo);
        int skip_b = lcs_memo_go(a, m, b, n, i, j + 1, memo);
        *cell = skip_a > skip_b ? skip_a : skip_b;
    }
    return *cell;
}

static int lcs_memo(const char *a, const char *b)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    size_t cells = ((size_t)m + 1) * ((size_t)n + 1);
    int *memo = malloc(cells * sizeof *memo);
    if (memo == NULL)
        return -1;
    for (size_t k = 0; k < cells; k++)
        memo[k] = -1;
    int result = lcs_memo_go(a, m, b, n, 0, 0, memo);
    free(memo);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Tabulation with reconstruction
 *   dp[i][j] = LCS length of the PREFIXES a[0..i) and b[0..j). Row 0 and
 *   column 0 are 0 (an empty prefix has nothing in common). Then
 *     a[i-1] == b[j-1]  ->  dp[i][j] = dp[i-1][j-1] + 1
 *     otherwise         ->  dp[i][j] = max(dp[i-1][j], dp[i][j-1])
 *   Filling row by row means the three cells each entry needs are done.
 *
 *   Rebuilding one LCS: start at (m, n) and retrace which rule produced
 *   each cell. If the characters match, that character is in the LCS: step
 *   diagonally. Otherwise step towards the larger neighbour (either one
 *   when they tie; ties are why several different LCS strings can exist).
 *   Characters are found back to front, so they are written into out from
 *   the end. out must have room for min(m, n) + 1 chars; may be NULL.
 *   Time O(m*n) + O(m+n) to rebuild   Space O(m*n)
 * ------------------------------------------------------------------------- */
static int lcs_table_build(const char *a, const char *b, char *out)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    size_t w = (size_t)n + 1;
    int *dp = calloc(((size_t)m + 1) * w, sizeof *dp);
    if (dp == NULL)
        return -1;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (a[i - 1] == b[j - 1]) {
                dp[i * w + j] = dp[(i - 1) * w + (j - 1)] + 1;
            } else {
                int up = dp[(i - 1) * w + j], left = dp[i * w + (j - 1)];
                dp[i * w + j] = up > left ? up : left;
            }
        }
    }
    int len = dp[(size_t)m * w + (size_t)n];

    if (out != NULL) {
        out[len] = '\0';
        int i = m, j = n, k = len;
        while (i > 0 && j > 0) {
            if (a[i - 1] == b[j - 1]) {
                out[--k] = a[i - 1];
                i--;
                j--;
            } else if (dp[(i - 1) * w + j] >= dp[i * w + (j - 1)]) {
                i--;
            } else {
                j--;
            }
        }
    }
    free(dp);
    return len;
}

static int lcs_table(const char *a, const char *b)
{
    return lcs_table_build(a, b, NULL);
}

/* -------------------------------------------------------------------------
 * Approach 4: Two rows (optimal space for the length)
 *   Row i of the table only reads row i - 1, so keep just two rows and swap
 *   them after each row. Making the SHORTER string the columns gives
 *   O(min(m, n)) space.
 *   Trade-off: the full table is gone, so this cannot walk back to rebuild
 *   the string. (Hirschberg's algorithm combines this trick with divide
 *   and conquer to rebuild the string in linear space too.)
 *   Variant: one row plus a variable holding the old diagonal value.
 *   Time O(m*n)   Space O(min(m, n))
 * ------------------------------------------------------------------------- */
static int lcs_two_rows(const char *a, const char *b)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    if (n > m) { /* make b the shorter one */
        const char *ts = a;
        a = b;
        b = ts;
        int tn = m;
        m = n;
        n = tn;
    }
    int *prev = calloc((size_t)n + 1, sizeof *prev);
    int *cur = calloc((size_t)n + 1, sizeof *cur);
    int result = -1;
    if (prev != NULL && cur != NULL) {
        for (int i = 1; i <= m; i++) {
            for (int j = 1; j <= n; j++) {
                if (a[i - 1] == b[j - 1])
                    cur[j] = prev[j - 1] + 1;
                else
                    cur[j] = prev[j] > cur[j - 1] ? prev[j] : cur[j - 1];
            }
            int *t = prev; /* cur becomes the previous row; cur[0] stays 0 */
            prev = cur;
            cur = t;
        }
        result = prev[n];
    }
    free(prev);
    free(cur);
    return result;
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

typedef int (*LcsFn)(const char *a, const char *b);

/* max_len: longest input string an approach is run on in these tests, so
 * the exponential recursion stays fast. */
static const struct {
    const char *name;
    LcsFn fn;
    size_t max_len;
} APPROACHES[] = {
    {"plain recursion", lcs_recursive, 10},
    {"memoization", lcs_memo, (size_t)-1},
    {"tabulation", lcs_table, (size_t)-1},
    {"two rows", lcs_two_rows, (size_t)-1},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

static int is_subsequence(const char *sub, const char *s)
{
    for (; *s != '\0' && *sub != '\0'; s++)
        if (*s == *sub)
            sub++;
    return *sub == '\0';
}

/* A rebuilt LCS is correct if it has the expected length and really is a
 * subsequence of both strings. (Several different strings can be correct.) */
static int rebuild_ok(const char *a, const char *b, int expect, char *buf)
{
    int len = lcs_table_build(a, b, buf);
    return len == expect && (int)strlen(buf) == expect && is_subsequence(buf, a) &&
           is_subsequence(buf, b);
}

typedef struct {
    const char *a;
    const char *b;
    int expect;
} Case;

int main(void)
{
    printf("Q34. Longest Common Subsequence\n");

    /* Expected lengths worked out by hand (one LCS shown in each comment). */
    const Case cases[] = {
        {"abcde", "ace", 3},          /* ace */
        {"abc", "abc", 3},            /* abc */
        {"abc", "def", 0},            /* nothing in common */
        {"", "abc", 0},               /* one empty string */
        {"", "", 0},                  /* both empty */
        {"a", "a", 1},
        {"a", "b", 0},
        {"AGGTAB", "GXTXAYB", 4},     /* GTAB: b has one each of G,T,A,B */
        {"aaaa", "aa", 2},            /* duplicates */
        {"bl", "yby", 1},             /* b */
        {"abcba", "abcbcba", 5},      /* the whole first string */
        {"XMJYAUZ", "MZJAWXU", 4},    /* MJAU */
        {"abc", "cba", 1},            /* reversed: any single letter */
        {"Abc", "abc", 2},            /* case matters: bc */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    char buf[64];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->a, c->b);
            snprintf(label, sizeof label, "test %2d (\"%s\", \"%s\"), %-15s -> %d", t + 1, c->a,
                     c->b, APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
        int ok = rebuild_ok(c->a, c->b, c->expect, buf);
        snprintf(label, sizeof label, "test %2d rebuilt LCS \"%s\" is common, length %d", t + 1,
                 buf, c->expect);
        check(ok, label);
    }

    /* Longer strings: 300 'a's followed by "b", against "b" followed by 300
     * 'a's. Matching the b's would allow only one character, so the LCS is
     * the 300 a's. Plain recursion is skipped (far too slow). */
    {
        enum { K = 300 };
        char *x = malloc(K + 2), *y = malloc(K + 2), *out = malloc(K + 2);
        if (x != NULL && y != NULL && out != NULL) {
            memset(x, 'a', K);
            x[K] = 'b';
            x[K + 1] = '\0';
            y[0] = 'b';
            memset(y + 1, 'a', K);
            y[K + 1] = '\0';
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (strlen(x) > APPROACHES[a].max_len)
                    continue;
                int got = APPROACHES[a].fn(x, y);
                snprintf(label, sizeof label, "%d a's + b vs b + %d a's, %-15s -> %d", K, K,
                         APPROACHES[a].name, got);
                check(got == K, label);
            }
            check(rebuild_ok(x, y, K, out), "rebuilt LCS of the long strings is 300 a's");
        } else {
            check(0, "allocation for long strings");
        }
        free(x);
        free(y);
        free(out);
    }

    /* Random cross-check 1: short strings over a 3-letter alphabet, every
     * approach against plain recursion, plus a validity check of the
     * rebuilt string. */
    {
        srand(34);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            char x[11], y[11];
            int m = rand() % 11, n = rand() % 11;
            for (int i = 0; i < m; i++)
                x[i] = (char)('a' + rand() % 3);
            for (int i = 0; i < n; i++)
                y[i] = (char)('a' + rand() % 3);
            x[m] = y[n] = '\0';
            int expect = lcs_recursive(x, y);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(x, y) != expect)
                    bad++;
            if (!rebuild_ok(x, y, expect, buf))
                bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d short pairs, all agree + rebuilt LCS valid", rounds);
        check(bad == 0, label);
    }

    /* Random cross-check 2: longer strings (up to 60 chars), the polynomial
     * approaches against tabulation. */
    {
        int bad = 0;
        const int rounds = 200;
        for (int r = 0; r < rounds; r++) {
            char x[61], y[61];
            int m = rand() % 61, n = rand() % 61;
            for (int i = 0; i < m; i++)
                x[i] = (char)('a' + rand() % 4);
            for (int i = 0; i < n; i++)
                y[i] = (char)('a' + rand() % 4);
            x[m] = y[n] = '\0';
            int expect = lcs_table(x, y);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if ((size_t)(m > n ? m : n) <= APPROACHES[a].max_len &&
                    APPROACHES[a].fn(x, y) != expect)
                    bad++;
            if (!rebuild_ok(x, y, expect, buf))
                bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d longer pairs, polynomial ones agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
