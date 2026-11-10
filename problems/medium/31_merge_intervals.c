/*
 * Q31. Merge Intervals
 * Difficulty : Medium
 * Topics     : Arrays, Sorting, Intervals
 *
 * Problem
 *   Given n closed intervals [start, end] (start <= end), merge every group
 *   of overlapping intervals into one and return the result sorted by
 *   start. Intervals that only touch, like [1, 4] and [4, 5], count as
 *   overlapping and merge into [1, 5].
 *
 * Example
 *   [[1,3], [2,6], [8,10], [15,18]]   ->   [[1,6], [8,10], [15,18]]
 *
 * Approaches
 *   1. Brute force             merge any overlapping pair, repeat  Time O(n^3)      Space O(1)
 *   2. Sort + sweep [optimal]  sort by start, extend or append     Time O(n log n)  Space O(1)
 *
 *   Both write into the caller's output buffer (n slots), which is not
 *   counted as extra space.
 *
 * Key insight
 *   Once intervals are sorted by start, anything that overlaps the current
 *   merged block must come right after it. So a single left-to-right pass
 *   only ever has to compare against the last block it produced.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 31_merge_intervals.c -o merge_intervals && ./merge_intervals
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int start;
    int end;
} Interval;

/* Closed intervals overlap (or touch) when neither lies strictly before the
 * other: a ends at or after b starts AND b ends at or after a starts. */
static int overlaps(Interval a, Interval b)
{
    return a.start <= b.end && b.start <= a.end;
}

/* Sort by start, then by end. Compares instead of subtracting, so values
 * like INT_MIN and INT_MAX cannot overflow. */
static int cmp_interval(const void *pa, const void *pb)
{
    const Interval *a = pa;
    const Interval *b = pb;
    if (a->start != b->start)
        return (a->start > b->start) - (a->start < b->start);
    return (a->end > b->end) - (a->end < b->end);
}

/* -------------------------------------------------------------------------
 * Approach 1: Brute force (repeat until stable)
 *   Copy the intervals into out. Look for ANY pair (i, j) that overlaps;
 *   replace out[i] with their union and delete out[j] by moving the last
 *   interval into its slot. Start the search over, because the bigger
 *   out[i] may now overlap intervals that were already checked. Stop when
 *   a full search finds no overlapping pair.
 *   Why it is correct: each merge keeps the union of all intervals the same
 *   and only joins intervals that really overlap, and at the end no two
 *   remaining intervals overlap, which is exactly the definition of the
 *   answer. Finally sort, so the output order is well defined.
 *   Time: one search is O(n^2), and there are at most n - 1 merges, so
 *   O(n^3) worst case (plus the final sort).   Space O(1) extra
 * ------------------------------------------------------------------------- */
static int merge_brute(const Interval *in, int n, Interval *out)
{
    if (n > 0)
        memcpy(out, in, (size_t)n * sizeof *out);
    int count = n;

    int merged = 1;
    while (merged) {
        merged = 0;
        for (int i = 0; i < count && !merged; i++) {
            for (int j = i + 1; j < count && !merged; j++) {
                if (overlaps(out[i], out[j])) {
                    if (out[j].start < out[i].start)
                        out[i].start = out[j].start;
                    if (out[j].end > out[i].end)
                        out[i].end = out[j].end;
                    out[j] = out[--count];
                    merged = 1;
                }
            }
        }
    }
    qsort(out, (size_t)count, sizeof *out, cmp_interval);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Sort by start + one sweep (optimal)
 *   Sort a copy by start. Keep the last merged block out[w]. For the next
 *   interval cur:
 *     cur.start <= out[w].end : it overlaps (or touches), so extend
 *                               out[w].end to max(out[w].end, cur.end);
 *     otherwise               : there is a gap, start a new block.
 *   Why it is correct: sorted by start, every later interval starts at or
 *   after cur.start. If cur starts after the block's end, so does every
 *   later interval, so the block is final and can never grow again.
 *   Pitfall: use max for the end. A nested interval like [2, 3] inside
 *   [1, 10] must not shrink the block to end at 3.
 *   Pitfall: write "<=" not "<", or touching intervals will not merge.
 *   Time O(n log n) for the sort + O(n) sweep   Space O(1) extra
 * ------------------------------------------------------------------------- */
static int merge_sorted(const Interval *in, int n, Interval *out)
{
    if (n == 0)
        return 0;
    memcpy(out, in, (size_t)n * sizeof *out);
    qsort(out, (size_t)n, sizeof *out, cmp_interval);

    int w = 0;
    for (int i = 1; i < n; i++) {
        if (out[i].start <= out[w].end) {
            if (out[i].end > out[w].end)
                out[w].end = out[i].end;
        } else {
            out[++w] = out[i];
        }
    }
    return w + 1;
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

typedef int (*MergeFn)(const Interval *in, int n, Interval *out);

static const struct {
    const char *name;
    MergeFn fn;
} APPROACHES[] = {
    {"brute force", merge_brute},
    {"sort + sweep", merge_sorted},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 8

typedef struct {
    const char *what;
    Interval in[MAXN];
    int n;
    Interval expect[MAXN];
    int expect_n;
} Case;

static int same_intervals(const Interval *a, int na, const Interval *b, int nb)
{
    if (na != nb)
        return 0;
    for (int i = 0; i < na; i++)
        if (a[i].start != b[i].start || a[i].end != b[i].end)
            return 0;
    return 1;
}

/* Independent sanity check of a result: sorted with real gaps between
 * blocks, every input lies inside some block, and every block starts and
 * ends at an endpoint of some input. */
static int is_valid_merge(const Interval *in, int n, const Interval *out, int m)
{
    for (int i = 0; i + 1 < m; i++)
        if (out[i].end >= out[i + 1].start)
            return 0;
    for (int i = 0; i < n; i++) {
        int covered = 0;
        for (int j = 0; j < m; j++)
            if (out[j].start <= in[i].start && in[i].end <= out[j].end)
                covered = 1;
        if (!covered)
            return 0;
    }
    for (int j = 0; j < m; j++) {
        int s_ok = 0, e_ok = 0;
        for (int i = 0; i < n; i++) {
            s_ok |= in[i].start == out[j].start;
            e_ok |= in[i].end == out[j].end;
        }
        if (!s_ok || !e_ok)
            return 0;
    }
    return 1;
}

int main(void)
{
    printf("Q31. Merge Intervals\n");

    /* Expected outputs merged by hand. */
    const Case cases[] = {
        {"basic", {{1, 3}, {2, 6}, {8, 10}, {15, 18}}, 4, {{1, 6}, {8, 10}, {15, 18}}, 3},
        {"touching", {{1, 4}, {4, 5}}, 2, {{1, 5}}, 1},
        {"unsorted", {{8, 10}, {1, 3}, {15, 18}, {2, 6}}, 4, {{1, 6}, {8, 10}, {15, 18}}, 3},
        {"fully nested", {{1, 10}, {2, 3}, {4, 5}, {6, 9}}, 4, {{1, 10}}, 1},
        {"single", {{5, 7}}, 1, {{5, 7}}, 1},
        {"all disjoint", {{7, 8}, {1, 2}, {4, 5}}, 3, {{1, 2}, {4, 5}, {7, 8}}, 3},
        {"empty", {{0, 0}}, 0, {{0, 0}}, 0},
        {"chain via middle", {{1, 2}, {3, 4}, {2, 3}}, 3, {{1, 4}}, 1},
        {"duplicates+negatives", {{-5, -1}, {-3, 0}, {-5, -1}, {2, 2}}, 4, {{-5, 0}, {2, 2}}, 2},
        {"points", {{1, 1}, {1, 1}, {0, 0}}, 3, {{0, 0}, {1, 1}}, 2},
        {"gap of one", {{1, 4}, {0, 0}}, 2, {{0, 0}, {1, 4}}, 2},
        {"same end", {{1, 4}, {0, 4}}, 2, {{0, 4}}, 1},
        {"extreme values", {{INT_MIN, 0}, {INT_MAX, INT_MAX}, {0, 5}}, 3,
         {{INT_MIN, 5}, {INT_MAX, INT_MAX}}, 2},
        {"one swallows all", {{3, 4}, {6, 7}, {9, 9}, {0, 100}, {1, 2}}, 5, {{0, 100}}, 1},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            Interval out[MAXN];
            int got = APPROACHES[a].fn(c->in, c->n, out);
            snprintf(label, sizeof label, "test %2d %-20s, %-13s -> %d interval(s)", t + 1,
                     c->what, APPROACHES[a].name, got);
            check(same_intervals(out, got, c->expect, c->expect_n), label);
        }
    }

    /* The input must be left exactly as it was. */
    {
        const Interval orig[] = {{9, 12}, {1, 3}, {2, 4}};
        Interval in[3], out[3];
        memcpy(in, orig, sizeof in);
        for (int a = 0; a < NUM_APPROACHES; a++)
            (void)APPROACHES[a].fn(in, 3, out);
        check(memcmp(in, orig, sizeof in) == 0, "input unchanged by every approach");
    }

    /* Random cross-check: both approaches must produce exactly the same
     * list, since the output is sorted by start and therefore unique. The
     * brute-force answer is also checked against the definition directly. */
    {
        srand(31);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            Interval in[12], expect[12], got[12];
            int n = rand() % 13;
            for (int i = 0; i < n; i++) {
                in[i].start = rand() % 41 - 20;
                in[i].end = in[i].start + rand() % 9;
            }
            int expect_n = merge_brute(in, n, expect);
            if (!is_valid_merge(in, n, expect, expect_n))
                bad++;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int got_n = APPROACHES[a].fn(in, n, got);
                if (!same_intervals(got, got_n, expect, expect_n))
                    bad++;
            }
        }
        snprintf(label, sizeof label, "random cross-check: %d inputs, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
