/*
 * Q43. N-Queens
 * Difficulty : Hard
 * Topics     : Backtracking, Recursion, Bit Manipulation
 *
 * Problem
 *   Place n queens on an n x n chessboard so that no two attack each other
 *   (no two share a row, a column or a diagonal). Count the number of
 *   distinct solutions for a given n, and print one solution board for
 *   n = 4 and n = 8.
 *
 * Example
 *   n = 4   ->   2 solutions. The first one in lexicographic order:
 *       . Q . .
 *       . . . Q
 *       Q . . .
 *       . . Q .
 *
 * Approaches
 *   1. Brute force           try all n^n "one queen per row" boards   Time O(n^n * n^2)  Space O(n)
 *   2. Backtracking + scan   place row by row, check earlier rows     Time O(n! * n)     Space O(n)
 *   3. Backtracking + flags  O(1) lookups for columns and diagonals   Time O(n!)         Space O(n)
 *   4. Bitmask  [optimal]    attacked squares of a row as bit masks   Time O(n!)         Space O(n)
 *
 * Key insight
 *   Every row holds exactly one queen, so a board is just col[row]. Two
 *   queens (r1, c1) and (r2, c2) share a diagonal exactly when r1 - c1 ==
 *   r2 - c2 or r1 + c1 == r2 + c2. Placing queens row by row and refusing a
 *   square as soon as it is attacked prunes almost the whole n^n search
 *   space. With bit masks, the squares attacked in the next row are just
 *   (cols | diag_left << 1 | diag_right >> 1): each diagonal mask moves one
 *   column per row.
 *
 * Testing note
 *   There is no meaningful "random input" for this problem: the input is
 *   just n. So the cross-check is a sweep over n instead: every approach is
 *   run for every n it can afford and must give the same count.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 43_n_queens.c -o n_queens && ./n_queens
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 16 /* larger boards take far longer than this program allows */

/* Independent validity check used by approach 1 and by the tests:
 * col[r] is the column of the queen in row r. */
static int board_is_valid(const int *col, int n)
{
    for (int r1 = 0; r1 < n; r1++) {
        for (int r2 = r1 + 1; r2 < n; r2++) {
            if (col[r1] == col[r2])
                return 0;
            if (abs(col[r1] - col[r2]) == r2 - r1)
                return 0;
        }
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Put one queen in each row (that already rules out row attacks) and try
 *   every way to choose the columns: n^n boards, enumerated like an
 *   odometer (increment the last row; on overflow reset it and carry into
 *   the row above). Count the boards that pass the full pairwise check.
 *   n^n grows explosively (6^6 = 46,656 but 10^10 = ten billion), so this
 *   is only run for n <= 6.
 *   Time O(n^n * n^2)   Space O(n)
 * ------------------------------------------------------------------------- */
static long long queens_brute(int n)
{
    int col[MAXN] = {0};
    long long count = 0;
    for (;;) {
        if (board_is_valid(col, n))
            count++;

        int r = n - 1;
        while (r >= 0 && col[r] == n - 1) {
            col[r] = 0;
            r--;
        }
        if (r < 0)
            break;
        col[r]++;
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Backtracking with an is_safe scan
 *   Fill rows top to bottom. For row `row`, try each column c and keep it
 *   only if no queen in rows 0..row-1 attacks (row, c); then recurse into
 *   the next row and afterwards try the next c ("backtrack"). A full board
 *   (row == n) is one solution.
 *   The first row has n choices, the second at most n - 1 safe ones, and so
 *   on, so the tree has at most about n! leaves; each candidate square costs
 *   an O(n) scan of earlier rows.
 *   Time O(n! * n)   Space O(n) (the col array plus recursion depth n)
 * ------------------------------------------------------------------------- */
static int is_safe(const int *col, int row, int c)
{
    for (int r = 0; r < row; r++)
        if (col[r] == c || abs(col[r] - c) == row - r)
            return 0;
    return 1;
}

static long long scan_place(int *col, int row, int n)
{
    if (row == n)
        return 1;
    long long count = 0;
    for (int c = 0; c < n; c++) {
        if (is_safe(col, row, c)) {
            col[row] = c;
            count += scan_place(col, row + 1, n);
        }
    }
    return count;
}

static long long queens_scan(int n)
{
    int col[MAXN];
    return scan_place(col, 0, n);
}

/* -------------------------------------------------------------------------
 * Approach 3: Backtracking with boolean arrays
 *   Instead of rescanning earlier rows, remember which lines are taken:
 *       used_col[c]                   column c
 *       used_diag[r + c]              "/" diagonal: r + c is constant on it
 *       used_anti[r - c + n - 1]      "\" diagonal: r - c is constant on it
 *                                     (+ n - 1 shifts it into 0..2n-2)
 *   A square is safe iff all three flags are clear: O(1). Set them before
 *   recursing and clear them afterwards (that undo step is the backtrack).
 *   Time O(n!)   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    int n;
    unsigned char used_col[MAXN];
    unsigned char used_diag[2 * MAXN - 1];
    unsigned char used_anti[2 * MAXN - 1];
} Flags;

static long long flags_place(Flags *f, int row)
{
    if (row == f->n)
        return 1;
    long long count = 0;
    for (int c = 0; c < f->n; c++) {
        int d = row + c, a = row - c + f->n - 1;
        if (f->used_col[c] || f->used_diag[d] || f->used_anti[a])
            continue;
        f->used_col[c] = f->used_diag[d] = f->used_anti[a] = 1;
        count += flags_place(f, row + 1);
        f->used_col[c] = f->used_diag[d] = f->used_anti[a] = 0;
    }
    return count;
}

static long long queens_flags(int n)
{
    Flags f;
    memset(&f, 0, sizeof f);
    f.n = n;
    return flags_place(&f, 0);
}

/* -------------------------------------------------------------------------
 * Approach 4: Bitmask backtracking (optimal)
 *   Same search as approach 3, but each row's attacked squares are three
 *   n-bit masks (bit c = column c):
 *       cols        columns already holding a queen
 *       diag_left   squares in THIS row hit by a "\" diagonal from above
 *       diag_right  squares in THIS row hit by a "/" diagonal from above
 *   A queen's "\" diagonal hits column c+1 one row lower and its "/"
 *   diagonal hits c-1, so going down one row is just
 *       diag_left  = (diag_left  | bit) << 1
 *       diag_right = (diag_right | bit) >> 1
 *   The free squares are full & ~(cols | diag_left | diag_right), and
 *   avail & -avail isolates the lowest free square, so the loop only visits
 *   squares that are actually free. The asymptotic bound is the same as
 *   approach 3, but the work per node is a handful of register operations.
 *   Unsigned masks are used so that shifts are always well defined.
 *   Time O(n!)   Space O(n) recursion depth
 * ------------------------------------------------------------------------- */
static long long bits_place(unsigned full, unsigned cols, unsigned diag_left, unsigned diag_right)
{
    if (cols == full)
        return 1;
    long long count = 0;
    unsigned avail = full & ~(cols | diag_left | diag_right);
    while (avail) {
        unsigned bit = avail & (0u - avail); /* lowest set bit */
        avail ^= bit;
        count += bits_place(full, cols | bit, ((diag_left | bit) << 1) & full,
                            (diag_right | bit) >> 1);
    }
    return count;
}

static long long queens_bits(int n)
{
    unsigned full = (1u << n) - 1u; /* n <= MAXN < 32, so the shift is safe */
    return bits_place(full, 0, 0, 0);
}

/* -------------------------------------------------------------------------
 * Finding and printing one solution
 *   Backtracking with an early exit: return as soon as row n is reached.
 *   Columns are tried in increasing order, so the first solution found is
 *   the lexicographically smallest col[] array.
 * ------------------------------------------------------------------------- */
static int first_solution(int *col, int row, int n)
{
    if (row == n)
        return 1;
    for (int c = 0; c < n; c++) {
        if (is_safe(col, row, c)) {
            col[row] = c;
            if (first_solution(col, row + 1, n))
                return 1;
        }
    }
    return 0;
}

static void print_board(const int *col, int n)
{
    for (int r = 0; r < n; r++) {
        printf("      ");
        for (int c = 0; c < n; c++)
            printf(c ? " %c" : "%c", col[r] == c ? 'Q' : '.');
        printf("\n");
    }
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

typedef long long (*QueensFn)(int n);

/* max_n keeps every approach well under a second, even with sanitizers. */
static const struct {
    const char *name;
    QueensFn fn;
    int max_n;
} APPROACHES[] = {
    {"brute force", queens_brute, 6},
    {"backtrack + scan", queens_scan, 10},
    {"backtrack + flags", queens_flags, 10},
    {"bitmask", queens_bits, 12},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

/* Well-known solution counts (OEIS A000170), index = n. */
static const long long EXPECTED[] = {0, 1, 0, 0, 2, 10, 4, 40, 92, 352, 724, 2680, 14200};
#define MAX_TESTED_N 12

int main(void)
{
    printf("Q43. N-Queens\n");

    char label[160];

    /* Fixed cases: published counts for n = 1..max_n of each approach. */
    long long got[NUM_APPROACHES][MAX_TESTED_N + 1];
    for (int a = 0; a < NUM_APPROACHES; a++) {
        for (int n = 1; n <= APPROACHES[a].max_n; n++) {
            got[a][n] = APPROACHES[a].fn(n);
            snprintf(label, sizeof label, "n = %2d, %-17s -> %lld", n, APPROACHES[a].name,
                     got[a][n]);
            check(got[a][n] == EXPECTED[n], label);
        }
    }

    /* Cross-check (a sweep over n, see the testing note in the header):
     * for each n, every approach that ran on it must give the same count. */
    {
        int bad = 0, compared = 0;
        for (int n = 1; n <= MAX_TESTED_N; n++) {
            long long first = -1;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (n > APPROACHES[a].max_n)
                    continue;
                if (first < 0)
                    first = got[a][n];
                else if (got[a][n] != first)
                    bad++;
                compared++;
            }
        }
        snprintf(label, sizeof label,
                 "sweep cross-check: n = 1..%d, %d counts, all approaches agree", MAX_TESTED_N,
                 compared);
        check(bad == 0, label);
    }

    /* One board each for n = 4 and n = 8. The expected boards are the
     * lexicographically first solutions: for n = 4 the only two solutions
     * are [1,3,0,2] and [2,0,3,1]; for n = 8 it is the classic 1 5 8 6 3 7 2 4
     * (1-based). The board is also re-checked by the independent validator. */
    {
        static const int expect4[] = {1, 3, 0, 2};
        static const int expect8[] = {0, 4, 7, 5, 2, 6, 1, 3};
        const struct {
            int n;
            const int *expect;
        } boards[] = {{4, expect4}, {8, expect8}};

        for (int b = 0; b < 2; b++) {
            int n = boards[b].n;
            int col[MAXN] = {0};
            int found = first_solution(col, 0, n);
            printf("    one solution for n = %d:\n", n);
            if (found)
                print_board(col, n);
            snprintf(label, sizeof label, "n = %d board is valid and lexicographically first", n);
            check(found && board_is_valid(col, n) &&
                      memcmp(col, boards[b].expect, (size_t)n * sizeof col[0]) == 0,
                  label);
        }
    }

    /* n = 2 and n = 3 have no solution at all. */
    {
        int col[MAXN] = {0};
        check(!first_solution(col, 0, 2) && !first_solution(col, 0, 3),
              "n = 2 and n = 3 have no board to print");
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
