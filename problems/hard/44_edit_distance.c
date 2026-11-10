/*
 * Q44. Edit Distance
 * Difficulty : Hard
 * Topics     : Strings, Dynamic Programming, Recursion
 *
 * Problem
 *   Given two strings a and b, return the minimum number of operations that
 *   turn a into b, where one operation is inserting a character, deleting a
 *   character, or replacing a character (each costs 1). This is the
 *   Levenshtein distance. Also recover one optimal sequence of edits.
 *
 * Example
 *   a = "horse", b = "ros"   ->   3
 *       horse -> rorse   (replace 'h' with 'r')
 *       rorse -> rose    (delete 'r')
 *       rose  -> ros     (delete 'e')
 *
 * Approaches
 *   1. Plain recursion            try all three edits at each mismatch    Time O(3^(m+n))  Space O(m+n)
 *   2. Memoization                cache the answer for each (i, j)        Time O(mn)       Space O(mn)
 *   3. Tabulation                 fill an (m+1) x (n+1) table bottom-up   Time O(mn)       Space O(mn)
 *   4. One row  [optimal space]   keep only the previous row of the table Time O(mn)       Space O(min(m,n))
 *
 * Key insight
 *   Look at the last characters. If a[i-1] == b[j-1] they can be matched for
 *   free. Otherwise the last operation is one of three, and each leaves a
 *   smaller version of the same problem:
 *       replace a[i-1] by b[j-1]  ->  dist(i-1, j-1) + 1
 *       delete  a[i-1]            ->  dist(i-1, j)   + 1
 *       insert  b[j-1]            ->  dist(i,   j-1) + 1
 *   There are only (m+1)(n+1) distinct (i, j) pairs, so caching or a table
 *   turns the exponential recursion into O(mn), and walking the table back
 *   from (m, n) recovers which edit was chosen at each step.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 44_edit_distance.c -o edit_distance && ./edit_distance
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int min3(int x, int y, int z)
{
    int m = x < y ? x : y;
    return m < z ? m : z;
}

/* -------------------------------------------------------------------------
 * Approach 1: Plain recursion
 *   Work on suffixes: dist(a, b) where a and b point into the strings.
 *     - one string empty: insert (or delete) everything left in the other;
 *     - first characters equal: match them and move on. This greedy step is
 *       always safe: removing one character from either string changes the
 *       distance by at most 1, so matching never costs more than editing;
 *     - otherwise try all three edits and keep the cheapest.
 *   The same (suffix, suffix) pairs are solved again and again, so the call
 *   tree grows like 3^(m+n). Only short strings (m + n <= 18) are run.
 *   Time O(3^(m+n))   Space O(m+n) recursion depth
 * ------------------------------------------------------------------------- */
static int ed_recursive(const char *a, const char *b)
{
    if (*a == '\0')
        return (int)strlen(b);
    if (*b == '\0')
        return (int)strlen(a);
    if (*a == *b)
        return ed_recursive(a + 1, b + 1);
    return 1 + min3(ed_recursive(a + 1, b + 1),  /* replace a[0] by b[0] */
                    ed_recursive(a + 1, b),      /* delete a[0] */
                    ed_recursive(a, b + 1));     /* insert b[0] in front */
}

/* -------------------------------------------------------------------------
 * Approach 2: Memoization (top-down DP)
 *   The recursion only ever asks about (i, j) = "suffix a[i..] vs suffix
 *   b[j..]", and there are m * n such pairs with both suffixes non-empty.
 *   Store each answer the first time it is computed (-1 = not yet) and
 *   return the stored value on every later call.
 *   Pitfall: recursion depth is up to m + n, which is fine here but can
 *   overflow the stack for very long strings; approach 3 has no recursion.
 *   Returns -1 if memory runs out.
 *   Time O(mn)   Space O(mn)
 * ------------------------------------------------------------------------- */
typedef struct {
    const char *a, *b;
    int m, n;
    int *memo; /* m x n, memo[i * n + j] */
} MemoCtx;

static int memo_dist(MemoCtx *s, int i, int j)
{
    if (i == s->m)
        return s->n - j;
    if (j == s->n)
        return s->m - i;

    int *slot = &s->memo[(size_t)i * (size_t)s->n + (size_t)j];
    if (*slot < 0) {
        if (s->a[i] == s->b[j])
            *slot = memo_dist(s, i + 1, j + 1);
        else
            *slot = 1 + min3(memo_dist(s, i + 1, j + 1), memo_dist(s, i + 1, j),
                             memo_dist(s, i, j + 1));
    }
    return *slot;
}

static int ed_memo(const char *a, const char *b)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    if (m == 0 || n == 0)
        return m + n;

    MemoCtx s = {a, b, m, n, malloc((size_t)m * (size_t)n * sizeof(int))};
    if (s.memo == NULL)
        return -1;
    for (size_t k = 0; k < (size_t)m * (size_t)n; k++)
        s.memo[k] = -1;

    int d = memo_dist(&s, 0, 0);
    free(s.memo);
    return d;
}

/* -------------------------------------------------------------------------
 * Approach 3: Tabulation (bottom-up DP)
 *   dp[i][j] = distance between the first i chars of a and the first j of b.
 *       dp[i][0] = i                 (delete all i characters)
 *       dp[0][j] = j                 (insert all j characters)
 *       dp[i][j] = dp[i-1][j-1]                         if a[i-1] == b[j-1]
 *                = 1 + min(dp[i-1][j-1],  replace
 *                          dp[i-1][j],    delete a[i-1]
 *                          dp[i][j-1])    insert b[j-1]  otherwise
 *   Each cell depends only on its upper-left, upper and left neighbours, so
 *   filling row by row, left to right, always has them ready. The full
 *   table is kept because the edit script is read back out of it.
 *   Stored flat: dp[i][j] lives at dp[i * (n + 1) + j].
 *   Returns -1 if memory runs out.
 *   Time O(mn)   Space O(mn)
 * ------------------------------------------------------------------------- */
static int *build_table(const char *a, int m, const char *b, int n)
{
    size_t w = (size_t)n + 1;
    int *dp = malloc(((size_t)m + 1) * w * sizeof *dp);
    if (dp == NULL)
        return NULL;

    for (int i = 0; i <= m; i++)
        dp[(size_t)i * w] = i;
    for (int j = 0; j <= n; j++)
        dp[j] = j;
    for (int i = 1; i <= m; i++) {
        int *row = dp + (size_t)i * w;
        const int *up = row - w;
        for (int j = 1; j <= n; j++) {
            if (a[i - 1] == b[j - 1])
                row[j] = up[j - 1];
            else
                row[j] = 1 + min3(up[j - 1], up[j], row[j - 1]);
        }
    }
    return dp;
}

static int ed_table(const char *a, const char *b)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    int *dp = build_table(a, m, b, n);
    if (dp == NULL)
        return -1;
    int d = dp[(size_t)m * ((size_t)n + 1) + (size_t)n];
    free(dp);
    return d;
}

/* -------------------------------------------------------------------------
 * Approach 4: One row (optimal space)
 *   Row i of the table only reads row i-1, so keep a single row and update
 *   it in place from left to right. Before row[j] is overwritten it still
 *   holds the "up" value dp[i-1][j]; the "upper-left" value dp[i-1][j-1]
 *   was overwritten one step earlier, so save it in `diag` first.
 *   The distance is symmetric (an insert in one direction is a delete in
 *   the other), so swap the strings to make the row as short as possible.
 *   Returns -1 if memory runs out.
 *   Time O(mn)   Space O(min(m, n))
 * ------------------------------------------------------------------------- */
static int ed_one_row(const char *a, const char *b)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    if (n > m) {
        const char *ts = a;
        a = b;
        b = ts;
        int t = m;
        m = n;
        n = t;
    }

    int *row = malloc(((size_t)n + 1) * sizeof *row);
    if (row == NULL)
        return -1;
    for (int j = 0; j <= n; j++)
        row[j] = j;

    for (int i = 1; i <= m; i++) {
        int diag = row[0]; /* dp[i-1][0] */
        row[0] = i;
        for (int j = 1; j <= n; j++) {
            int up = row[j]; /* dp[i-1][j], about to be overwritten */
            if (a[i - 1] == b[j - 1])
                row[j] = diag;
            else
                row[j] = 1 + min3(diag, up, row[j - 1]);
            diag = up;
        }
    }

    int d = row[n];
    free(row);
    return d;
}

/* -------------------------------------------------------------------------
 * Reconstructing the edits
 *   Start at dp[m][n] and walk back to dp[0][0]. At each cell, find a
 *   neighbour the value could have come from:
 *       a[i-1] == b[j-1] and dp[i][j] == dp[i-1][j-1]      -> keep
 *       dp[i][j] == dp[i-1][j-1] + 1                       -> replace
 *       dp[i][j] == dp[i-1][j]   + 1                       -> delete a[i-1]
 *       otherwise (it must be dp[i][j-1] + 1)              -> insert b[j-1]
 *   The recurrence guarantees one of them holds, and the last case never
 *   happens with j == 0 (there dp[i][0] = dp[i-1][0] + 1, a delete).
 *   The steps come out last-to-first, so reverse them at the end.
 *   Each non-keep step costs exactly 1, so the script has dp[m][n] edits.
 * ------------------------------------------------------------------------- */
typedef enum { OP_KEEP, OP_REPLACE, OP_DELETE, OP_INSERT } OpKind;

typedef struct {
    OpKind kind;
    char from; /* character of a that is kept / replaced / deleted */
    char to;   /* character of b that is kept / written / inserted */
} EditOp;

/* Writes the steps (keeps included) to ops, which must have room for
 * strlen(a) + strlen(b) entries. Returns the number of steps, or -1. */
static int edit_script(const char *a, const char *b, EditOp *ops)
{
    int m = (int)strlen(a), n = (int)strlen(b);
    int *dp = build_table(a, m, b, n);
    if (dp == NULL)
        return -1;

    size_t w = (size_t)n + 1;
#define DP(i, j) dp[(size_t)(i) * w + (size_t)(j)]
    int i = m, j = n, k = 0;
    while (i > 0 || j > 0) {
        int cur = DP(i, j);
        if (i > 0 && j > 0 && a[i - 1] == b[j - 1] && cur == DP(i - 1, j - 1)) {
            ops[k++] = (EditOp){OP_KEEP, a[i - 1], b[j - 1]};
            i--;
            j--;
        } else if (i > 0 && j > 0 && cur == DP(i - 1, j - 1) + 1) {
            ops[k++] = (EditOp){OP_REPLACE, a[i - 1], b[j - 1]};
            i--;
            j--;
        } else if (i > 0 && cur == DP(i - 1, j) + 1) {
            ops[k++] = (EditOp){OP_DELETE, a[i - 1], '\0'};
            i--;
        } else {
            ops[k++] = (EditOp){OP_INSERT, '\0', b[j - 1]};
            j--;
        }
    }
#undef DP
    free(dp);

    for (int lo = 0, hi = k - 1; lo < hi; lo++, hi--) {
        EditOp t = ops[lo];
        ops[lo] = ops[hi];
        ops[hi] = t;
    }
    return k;
}

/* Applies the script to src in buf (capacity cap) from left to right, with
 * pos = the next character of the working string to process. Returns 1 if
 * every step fits the string it is applied to, 0 otherwise. With verbose
 * set, prints the string after every real edit. */
static int apply_script(const char *src, const EditOp *ops, int num_ops, char *buf, size_t cap,
                        int verbose)
{
    size_t len = strlen(src);
    if (len + 1 > cap)
        return 0;
    memcpy(buf, src, len + 1);

    size_t pos = 0;
    for (int k = 0; k < num_ops; k++) {
        const EditOp *op = &ops[k];
        char what[32];
        switch (op->kind) {
        case OP_KEEP:
            if (pos >= len || buf[pos] != op->from || op->from != op->to)
                return 0;
            pos++;
            continue;
        case OP_REPLACE:
            if (pos >= len || buf[pos] != op->from)
                return 0;
            buf[pos++] = op->to;
            snprintf(what, sizeof what, "replace '%c' with '%c'", op->from, op->to);
            break;
        case OP_DELETE:
            if (pos >= len || buf[pos] != op->from)
                return 0;
            memmove(buf + pos, buf + pos + 1, len - pos); /* moves the '\0' too */
            len--;
            snprintf(what, sizeof what, "delete '%c'", op->from);
            break;
        case OP_INSERT:
            if (len + 2 > cap)
                return 0;
            memmove(buf + pos + 1, buf + pos, len - pos + 1);
            buf[pos++] = op->to;
            len++;
            snprintf(what, sizeof what, "insert '%c'", op->to);
            break;
        default:
            return 0;
        }
        if (verbose)
            printf("      %-22s -> %s\n", what, buf);
    }
    return pos == len; /* every character of the working string was visited */
}

static int count_edits(const EditOp *ops, int num_ops)
{
    int edits = 0;
    for (int k = 0; k < num_ops; k++)
        if (ops[k].kind != OP_KEEP)
            edits++;
    return edits;
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

typedef int (*EditDistanceFn)(const char *a, const char *b);

/* max_total limits strlen(a) + strlen(b) for the exponential approach. */
static const struct {
    const char *name;
    EditDistanceFn fn;
    size_t max_total;
} APPROACHES[] = {
    {"plain recursion", ed_recursive, 18},
    {"memoization", ed_memo, SIZE_MAX},
    {"tabulation", ed_table, SIZE_MAX},
    {"one row", ed_one_row, SIZE_MAX},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *a;
    const char *b;
    int expect;
} Case;

/* Builds the script for a -> b, applies it, and checks the result. */
static int script_ok(const char *a, const char *b, int verbose)
{
    EditOp ops[64];
    char buf[64];
    if (strlen(a) + strlen(b) > 32)
        return 0;
    int steps = edit_script(a, b, ops);
    if (steps < 0)
        return 0;
    if (verbose)
        printf("    edit script for \"%s\" -> \"%s\":\n", a, b);
    return apply_script(a, ops, steps, buf, sizeof buf, verbose) && strcmp(buf, b) == 0 &&
           count_edits(ops, steps) == ed_table(a, b);
}

int main(void)
{
    printf("Q44. Edit Distance\n");

    /* Expected distances worked out by hand (or classic published examples):
     * each is reachable with that many edits, and fewer is impossible. */
    const Case cases[] = {
        {"horse", "ros", 3},              /* LeetCode example */
        {"intention", "execution", 5},    /* LeetCode example */
        {"", "", 0},                      /* both empty */
        {"", "abc", 3},                   /* insert everything */
        {"abc", "", 3},                   /* delete everything */
        {"a", "a", 0},
        {"a", "b", 1},
        {"abc", "abc", 0},                /* identical */
        {"kitten", "sitting", 3},         /* k->s, e->i, insert g */
        {"sunday", "saturday", 3},        /* insert a, insert t, n->r */
        {"aaaa", "aa", 2},                /* all-equal characters: 2 deletes */
        {"abcdef", "azced", 3},           /* b->z, delete d, f->d */
        {"flaw", "lawn", 2},              /* delete f, insert n */
        {"ab", "ba", 2},                  /* every position differs, same length */
        {"abc", "yabd", 2},               /* insert y, c->d */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            if (strlen(c->a) + strlen(c->b) > APPROACHES[a].max_total)
                continue;
            int got = APPROACHES[a].fn(c->a, c->b);
            snprintf(label, sizeof label, "test %2d, %-15s \"%s\" -> \"%s\" = %d", t + 1,
                     APPROACHES[a].name, c->a, c->b, got);
            check(got == c->expect, label);
        }
    }

    /* Longer strings for the polynomial approaches (too long for approach 1):
     *   300 x 'a'  vs 300 x 'b'   -> 300 (replace every character)
     *   (ab)^150   vs (ba)^150    -> 2   (delete the first 'a', append 'a';
     *                                     one edit cannot fix 300 mismatches)
     *   ""         vs 500 x 'x'   -> 500 */
    {
        static char as[301], bs[301], ab[301], ba[301], xs[501];
        memset(as, 'a', 300);
        memset(bs, 'b', 300);
        for (int k = 0; k < 300; k++) {
            ab[k] = k % 2 ? 'b' : 'a';
            ba[k] = k % 2 ? 'a' : 'b';
        }
        memset(xs, 'x', 500);
        const Case longs[] = {{as, bs, 300}, {ab, ba, 2}, {"", xs, 500}};
        const char *names[] = {"300 a's vs 300 b's", "(ab)^150 vs (ba)^150", "\"\" vs 500 x's"};
        for (int t = 0; t < 3; t++) {
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (strlen(longs[t].a) + strlen(longs[t].b) > APPROACHES[a].max_total)
                    continue;
                int got = APPROACHES[a].fn(longs[t].a, longs[t].b);
                snprintf(label, sizeof label, "long %d, %-15s %s = %d", t + 1,
                         APPROACHES[a].name, names[t], got);
                check(got == longs[t].expect, label);
            }
        }
    }

    /* Edit scripts: rebuild the edits from the table, print them, apply them
     * to the source string, and check that the result is the target and the
     * number of edits equals the distance. */
    check(script_ok("horse", "ros", 1), "script for horse -> ros: 3 edits, applying it gives ros");
    check(script_ok("intention", "execution", 1),
          "script for intention -> execution: 5 edits, applying it gives execution");

    /* Random cross-check: strings of length 0..7 over {a, b, c} (many equal
     * characters); every approach must agree with plain recursion. */
    {
        srand(44);
        int bad = 0, bad_script = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            char a[8], b[8];
            int m = rand() % 8, n = rand() % 8;
            for (int k = 0; k < m; k++)
                a[k] = (char)('a' + rand() % 3);
            for (int k = 0; k < n; k++)
                b[k] = (char)('a' + rand() % 3);
            a[m] = '\0';
            b[n] = '\0';

            int expect = ed_recursive(a, b);
            for (int k = 1; k < NUM_APPROACHES; k++)
                if (APPROACHES[k].fn(a, b) != expect)
                    bad++;
            if (!script_ok(a, b, 0))
                bad_script++;
        }
        snprintf(label, sizeof label, "random cross-check: %d string pairs, all approaches agree", rounds);
        check(bad == 0, label);
        snprintf(label, sizeof label,
                 "random edit scripts: %d pairs, each applies cleanly with distance edits", rounds);
        check(bad_script == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
