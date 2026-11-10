/*
 * Q35. Set Matrix Zeroes
 * Difficulty : Medium
 * Topics     : Arrays, Matrix, In-place
 *
 * Problem
 *   Given a rows x cols integer matrix, if a cell is 0, set its entire row
 *   and its entire column to 0. Do it in place. Only zeros that were in
 *   the ORIGINAL matrix count: a cell that becomes 0 does not spread.
 *   The matrix is stored row-major in a flat array: cell (r, c) is
 *   mat[r * cols + c].
 *
 * Example
 *   1 1 1        1 0 1
 *   1 0 1   ->   0 0 0
 *   1 1 1        1 0 1
 *
 * Approaches
 *   1. Copy the matrix           find zeros in an untouched copy   Time O(mn(m+n))  Space O(mn)
 *   2. Row + column flags        mark rows/cols, then clear them   Time O(mn)       Space O(m+n)
 *   3. First row/col  [optimal]  keep those flags in the matrix    Time O(mn)       Space O(1)
 *
 * Key insight
 *   The danger is reading a zero you wrote yourself and spreading it
 *   further. Separate "find the zeros" from "write the zeros": first record
 *   which rows and columns must be cleared, then clear them. The record
 *   fits in the matrix's own first row and first column.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 35_set_matrix_zeroes.c -o set_matrix_zeroes && ./set_matrix_zeroes
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Approach 1: Copy the matrix (brute force)
 *   Keep an untouched copy and look for zeros there, never in mat. For each
 *   original zero at (r, c), clear row r and column c of mat. Writing into
 *   mat while scanning the copy is safe because the copy never changes.
 *   Time: each of up to m*n zeros clears m + n cells, so O(mn(m+n)) in the
 *   worst case (an all-zero matrix).   Space O(mn) for the copy
 * ------------------------------------------------------------------------- */
static void zeroes_copy(int *mat, int rows, int cols)
{
    if (rows == 0 || cols == 0)
        return;
    size_t cells = (size_t)rows * (size_t)cols;
    int *orig = malloc(cells * sizeof *orig);
    if (orig == NULL)
        return;
    memcpy(orig, mat, cells * sizeof *orig);

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (orig[r * cols + c] != 0)
                continue;
            for (int k = 0; k < cols; k++)
                mat[r * cols + k] = 0;
            for (int k = 0; k < rows; k++)
                mat[k * cols + c] = 0;
        }
    }
    free(orig);
}

/* -------------------------------------------------------------------------
 * Approach 2: Row and column flag arrays
 *   Pass 1 only reads: row_zero[r] = 1 if row r has a zero, col_zero[c] = 1
 *   if column c has a zero. Pass 2 only writes: cell (r, c) becomes 0 when
 *   row_zero[r] or col_zero[c] is set. Since pass 1 finishes before pass 2
 *   writes anything, no written zero is ever mistaken for an original one.
 *   Every cell is visited twice.
 *   Time O(mn)   Space O(m+n)
 * ------------------------------------------------------------------------- */
static void zeroes_flags(int *mat, int rows, int cols)
{
    if (rows == 0 || cols == 0)
        return;
    unsigned char *row_zero = calloc((size_t)rows, 1);
    unsigned char *col_zero = calloc((size_t)cols, 1);
    if (row_zero != NULL && col_zero != NULL) {
        for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols; c++)
                if (mat[r * cols + c] == 0)
                    row_zero[r] = col_zero[c] = 1;
        for (int r = 0; r < rows; r++)
            for (int c = 0; c < cols; c++)
                if (row_zero[r] || col_zero[c])
                    mat[r * cols + c] = 0;
    }
    free(row_zero);
    free(col_zero);
}

/* -------------------------------------------------------------------------
 * Approach 3: First row and first column as the flag arrays (optimal)
 *   Approach 2's col_zero array is stored in row 0 (mat[0][c] = 0 means
 *   "clear column c") and row_zero in column 0 (mat[r][0] = 0 means "clear
 *   row r"). A cell that gets marked will be zeroed anyway, so overwriting
 *   it with the marker loses nothing.
 *
 *   Why row 0 and column 0 need their OWN two flags:
 *     - Once markers are written into row 0, a 0 there could be an original
 *       zero (clear all of row 0) or just a marker for its column (clear
 *       only that column). They look the same, so record "row 0 had a zero"
 *       BEFORE writing any marker. Same for column 0.
 *     - mat[0][0] sits in both row 0 and column 0, so it cannot be the
 *       marker for both. If it were, one zero in column 0 (say at (1, 0))
 *       would make mat[0][0] = 0 and wrongly clear all of row 0.
 *   (The common one-variable version uses mat[0][0] for row 0 and one extra
 *   variable for column 0; two named flags are easier to get right.)
 *
 *   Order matters: mark using cells (r >= 1, c >= 1), clear those same
 *   cells from the markers, and only then clear row 0 and column 0 from the
 *   two flags. Clearing row 0 earlier would wipe out the column markers.
 *   Time O(mn)   Space O(1)
 * ------------------------------------------------------------------------- */
static void zeroes_in_place(int *mat, int rows, int cols)
{
    if (rows == 0 || cols == 0)
        return;
    int first_row_zero = 0, first_col_zero = 0;
    for (int c = 0; c < cols; c++)
        if (mat[c] == 0)
            first_row_zero = 1;
    for (int r = 0; r < rows; r++)
        if (mat[r * cols] == 0)
            first_col_zero = 1;

    for (int r = 1; r < rows; r++) {
        for (int c = 1; c < cols; c++) {
            if (mat[r * cols + c] == 0) {
                mat[r * cols] = 0; /* clear row r later */
                mat[c] = 0;        /* clear column c later */
            }
        }
    }
    for (int r = 1; r < rows; r++)
        for (int c = 1; c < cols; c++)
            if (mat[r * cols] == 0 || mat[c] == 0)
                mat[r * cols + c] = 0;

    if (first_row_zero)
        for (int c = 0; c < cols; c++)
            mat[c] = 0;
    if (first_col_zero)
        for (int r = 0; r < rows; r++)
            mat[r * cols] = 0;
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

typedef void (*ZeroFn)(int *mat, int rows, int cols);

static const struct {
    const char *name;
    ZeroFn fn;
} APPROACHES[] = {
    {"copy matrix", zeroes_copy},
    {"row/col flags", zeroes_flags},
    {"first row/col", zeroes_in_place},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXCELLS 16

typedef struct {
    const char *what;
    int rows, cols;
    int in[MAXCELLS];
    int expect[MAXCELLS];
} Case;

int main(void)
{
    printf("Q35. Set Matrix Zeroes\n");

    /* Expected matrices worked out by hand. Every approach gets its own
     * fresh copy of the input, since they all modify it. */
    const Case cases[] = {
        {"example", 3, 3, {1, 1, 1, 1, 0, 1, 1, 1, 1}, {1, 0, 1, 0, 0, 0, 1, 0, 1}},
        {"zeros in row 0", 3, 4,
         {0, 1, 2, 0,
          3, 4, 5, 2,
          1, 3, 1, 5},
         {0, 0, 0, 0,
          0, 4, 5, 0,
          0, 3, 1, 0}},
        {"no zeros", 2, 2, {1, 2, 3, 4}, {1, 2, 3, 4}},
        {"all zeros", 2, 2, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {"1x1 zero", 1, 1, {0}, {0}},
        {"1x1 non-zero", 1, 1, {5}, {5}},
        {"single row", 1, 3, {1, 0, 3}, {0, 0, 0}},
        {"single column", 3, 1, {1, 0, 3}, {0, 0, 0}},
        {"only row 0 has a zero", 3, 3, {1, 0, 3, 4, 5, 6, 7, 8, 9}, {0, 0, 0, 4, 0, 6, 7, 0, 9}},
        {"only col 0 has a zero", 3, 3, {1, 2, 3, 0, 5, 6, 7, 8, 9}, {0, 2, 3, 0, 0, 0, 0, 8, 9}},
        /* the shared mat[0][0] trap: a zero at (1,0) must NOT clear row 0 */
        {"mat[0][0] trap", 2, 2, {1, 1, 0, 1}, {0, 1, 0, 0}},
        {"corner zero", 2, 3, {0, 1, 2, 3, 4, 5}, {0, 0, 0, 0, 4, 5}},
        {"extremes + negatives", 2, 2, {INT_MIN, -1, INT_MAX, 0}, {INT_MIN, 0, 0, 0}},
        /* a written zero must not spread: only (1,1) was zero originally */
        {"no chain reaction", 4, 4,
         {1, 1, 1, 1,
          1, 0, 1, 1,
          1, 1, 1, 1,
          1, 1, 1, 7},
         {1, 0, 1, 1,
          0, 0, 0, 0,
          1, 0, 1, 1,
          1, 0, 1, 7}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        size_t bytes = (size_t)c->rows * (size_t)c->cols * sizeof(int);
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int mat[MAXCELLS];
            memcpy(mat, c->in, bytes);
            APPROACHES[a].fn(mat, c->rows, c->cols);
            snprintf(label, sizeof label, "test %2d %-22s (%dx%d), %-14s", t + 1, c->what,
                     c->rows, c->cols, APPROACHES[a].name);
            check(memcmp(mat, c->expect, bytes) == 0, label);
        }
    }

    /* Empty matrices: nothing to do, and nothing may be touched. */
    {
        int sentinel[1] = {0};
        for (int a = 0; a < NUM_APPROACHES; a++) {
            APPROACHES[a].fn(sentinel, 0, 0);
            APPROACHES[a].fn(sentinel, 0, 3);
            APPROACHES[a].fn(sentinel, 3, 0);
        }
        check(sentinel[0] == 0, "empty matrices (0x0, 0x3, 3x0) are left alone");
    }

    /* Random cross-check: random matrices up to 8x8 with values -3..3, so
     * about one cell in seven is zero (and some matrices have none). */
    {
        srand(35);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int in[64], expect[64], got[64];
            int rows = 1 + rand() % 8, cols = 1 + rand() % 8;
            size_t bytes = (size_t)rows * (size_t)cols * sizeof(int);
            for (int i = 0; i < rows * cols; i++)
                in[i] = rand() % 7 - 3;
            memcpy(expect, in, bytes);
            zeroes_copy(expect, rows, cols);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                memcpy(got, in, bytes);
                APPROACHES[a].fn(got, rows, cols);
                if (memcmp(got, expect, bytes) != 0)
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d matrices, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
