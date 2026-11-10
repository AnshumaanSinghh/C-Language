/*
 * Q50. Count Inversions
 * Difficulty : Hard
 * Topics     : Divide and Conquer, Merge Sort, Fenwick Tree, Sorting
 *
 * Problem
 *   Given an array a of n integers, count the inversions: pairs (i, j) with
 *   i < j and a[i] > a[j]. Equal values are NOT an inversion. The count can
 *   reach n(n-1)/2, far beyond INT_MAX, so it is returned as long long.
 *
 * Example
 *   a = [2, 4, 1, 3, 5]   ->   3   the pairs (2,1), (4,1), (4,3)
 *
 * Approaches
 *   1. Brute force                 check every pair                    Time O(n^2)          Space O(1)
 *   2. Insertion sort              count the shifts                    Time O(n + inv)      Space O(1)
 *   3. Merge sort      [optimal]   count while merging the halves      Time O(n log n)      Space O(n)
 *   4. Fenwick tree (BIT)          count larger values seen so far     Time O(n log n)      Space O(n)
 *   (inv = the number of inversions, which can itself be as large as n^2 / 2)
 *
 * Key insight
 *   Split the array into halves. Every inversion is inside the left half,
 *   inside the right half, or "crossing" (big element on the left, small on
 *   the right). The first two are counted recursively. Crossing ones are
 *   counted while merging the sorted halves: when the merge takes R from the
 *   right half, every element still waiting in the left half is larger than
 *   R and sits before it, so that is (elements left in the left half) new
 *   inversions at once. Counting in batches is what beats checking pairs.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 50_count_inversions.c -o count_inversions && ./count_inversions
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Every approach takes int *a so they share one function-pointer type.
 * Insertion sort and merge sort sort a in place while counting; brute force
 * and the Fenwick tree only read it. The harness gives each call its own
 * copy, so no approach sees another one's output.
 */

/* -------------------------------------------------------------------------
 * Approach 1: Brute force
 *   Literally the definition: test every pair i < j.
 *   Pitfall: the counter must be long long. An int overflows once there are
 *   more than about 2.1 billion inversions, which a 66000-element reversed
 *   array already has.
 *   Time O(n^2)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long inversions_brute(int *a, int n)
{
    long long count = 0;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] > a[j])
                count++;
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: Insertion sort, counting shifts
 *   Insertion sort moves x = a[i] left past every larger element before it.
 *   Each single shift moves x past exactly one element y with y > x that
 *   came earlier, i.e. it removes exactly one inversion (y, x), and sorting
 *   ends with zero inversions. So the number of shifts IS the inversion
 *   count. Using a strict > means equal values are never shifted past each
 *   other, matching the definition.
 *   Fast on nearly sorted input, quadratic on reversed input.
 *   Time O(n + inversions)   Space O(1)
 * ------------------------------------------------------------------------- */
static long long inversions_insertion(int *a, int n)
{
    long long count = 0;
    for (int i = 1; i < n; i++) {
        int x = a[i], j = i - 1;
        while (j >= 0 && a[j] > x) {
            a[j + 1] = a[j];
            j--;
            count++;
        }
        a[j + 1] = x;
    }
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: Merge sort counting (optimal)
 *   See Key insight. While merging sorted a[lo..mid) and a[mid..hi):
 *     a[i] <= a[j]: take a[i]; it is not larger than anything taken from
 *                   the right later, so it adds nothing.
 *     a[i] >  a[j]: take a[j]; a[i..mid) are all > a[j] (the left half is
 *                   sorted) and all come before it: add mid - i.
 *   Pitfall: on a tie the merge must take from the LEFT (<=). Taking a[j]
 *   first would count equal pairs as inversions.
 *   One shared temp buffer is allocated once instead of once per merge.
 *   Time O(n log n)   Space O(n) buffer + O(log n) recursion
 * ------------------------------------------------------------------------- */
static long long merge_count(int *a, int *tmp, int lo, int hi) /* a[lo..hi) */
{
    if (hi - lo < 2)
        return 0;
    int mid = lo + (hi - lo) / 2;
    long long count = merge_count(a, tmp, lo, mid) + merge_count(a, tmp, mid, hi);

    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
            count += mid - i;
        }
    }
    while (i < mid)
        tmp[k++] = a[i++];
    while (j < hi)
        tmp[k++] = a[j++];
    memcpy(a + lo, tmp + lo, (size_t)(hi - lo) * sizeof *a);
    return count;
}

static long long inversions_merge(int *a, int n)
{
    if (n < 2)
        return 0;
    int *tmp = malloc((size_t)n * sizeof *tmp);
    if (tmp == NULL)
        return -1;
    long long count = merge_count(a, tmp, 0, n);
    free(tmp);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 4: Fenwick tree (binary indexed tree) + coordinate compression
 *   Walk left to right. For a[i], the inversions it closes are the earlier
 *   elements that are larger: (elements seen so far) - (seen ones <= a[i]).
 *   A Fenwick tree over value ranks answers "how many seen values have rank
 *   <= r" and records a new value, both in O(log n).
 *   Compression: values can be anything from INT_MIN to INT_MAX, far too
 *   many for a tree indexed by value. Only their relative order matters, so
 *   replace each value by its rank (1..m) among the distinct sorted values,
 *   found by binary search. Equal values get equal ranks, and the "<= r"
 *   query counts them as not larger, so ties are not inversions.
 *   Fenwick tree recap: tree[i] stores the sum over a block of indices that
 *   ends at i and has length i & -i (the lowest set bit). Prefix sums climb
 *   down by removing that bit; updates climb up by adding it.
 *   Time O(n log n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *x, const void *y)
{
    int p = *(const int *)x, q = *(const int *)y;
    return (p > q) - (p < q);
}

static void fenwick_add(int *tree, int m, int i)
{
    for (; i <= m; i += i & -i)
        tree[i]++;
}

static int fenwick_prefix(const int *tree, int i)
{
    int sum = 0;
    for (; i > 0; i -= i & -i)
        sum += tree[i];
    return sum;
}

static long long inversions_fenwick(int *a, int n)
{
    if (n < 2)
        return 0;
    int *sorted = malloc((size_t)n * sizeof *sorted);
    int *tree = calloc((size_t)n + 1, sizeof *tree);
    if (sorted == NULL || tree == NULL) {
        free(sorted);
        free(tree);
        return -1;
    }

    memcpy(sorted, a, (size_t)n * sizeof *sorted);
    qsort(sorted, (size_t)n, sizeof *sorted, cmp_int);
    int m = 0; /* number of distinct values */
    for (int i = 0; i < n; i++)
        if (m == 0 || sorted[i] != sorted[m - 1])
            sorted[m++] = sorted[i];

    long long count = 0;
    for (int i = 0; i < n; i++) {
        int lo = 0, hi = m - 1; /* a[i] is guaranteed to be in sorted[0..m) */
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (sorted[mid] < a[i])
                lo = mid + 1;
            else
                hi = mid;
        }
        int rank = lo + 1;
        count += i - fenwick_prefix(tree, rank);
        fenwick_add(tree, m, rank);
    }
    free(sorted);
    free(tree);
    return count;
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

typedef long long (*InversionsFn)(int *a, int n);

/* How the running time grows decides which large tests an approach joins. */
enum { QUADRATIC, ADAPTIVE, N_LOG_N };

static const struct {
    const char *name;
    InversionsFn fn;
    int growth;
} APPROACHES[] = {
    {"brute force", inversions_brute, QUADRATIC},
    {"insertion sort", inversions_insertion, ADAPTIVE},
    {"merge sort", inversions_merge, N_LOG_N},
    {"Fenwick tree", inversions_fenwick, N_LOG_N},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    int a[10];
    int n;
    long long expect;
} Case;

/* Copies a so approaches that sort in place cannot affect each other. */
static long long run_on_copy(InversionsFn fn, const int *a, int n)
{
    int *copy = malloc((size_t)(n > 0 ? n : 1) * sizeof *copy);
    if (copy == NULL)
        return -1;
    if (n > 0)
        memcpy(copy, a, (size_t)n * sizeof *copy);
    long long result = fn(copy, n);
    free(copy);
    return result;
}

/* Runs every approach whose growth is at least min_growth on a large array. */
static void check_large(const char *title, const int *a, int n, int min_growth, long long expect)
{
    char label[160];
    for (int k = 0; k < NUM_APPROACHES; k++) {
        if (APPROACHES[k].growth < min_growth)
            continue;
        long long got = run_on_copy(APPROACHES[k].fn, a, n);
        snprintf(label, sizeof label, "%s, %-14s -> %lld", title, APPROACHES[k].name, got);
        check(got == expect, label);
    }
}

int main(void)
{
    printf("Q50. Count Inversions\n");

    /* Expected counts worked out by hand by listing the pairs. */
    const Case cases[] = {
        {{2, 4, 1, 3, 5}, 5, 3},              /* (2,1) (4,1) (4,3)            */
        {{0}, 0, 0},                          /* empty                        */
        {{7}, 1, 0},                          /* single element               */
        {{1, 2, 3, 4, 5}, 5, 0},              /* sorted                       */
        {{5, 4, 3, 2, 1}, 5, 10},             /* reversed: 5*4/2              */
        {{3, 3, 3, 3}, 4, 0},                 /* all equal: ties don't count  */
        {{2, 1, 2, 1}, 4, 3},                 /* (2,1) (2,1) (2,1) at 0-1, 0-3, 2-3 */
        {{INT_MAX, INT_MIN, 0, -1}, 4, 4},    /* MAX>MIN, MAX>0, MAX>-1, 0>-1 */
        {{1, 20, 6, 4, 5}, 5, 5},             /* 20>6,4,5  6>4,5              */
        {{8, 4, 2, 1}, 4, 6},
        {{-1, -2, -3}, 3, 3},                 /* negatives                    */
        {{1, 1, 2, 2, 1}, 5, 2},              /* the two 2s before the last 1 */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int k = 0; k < NUM_APPROACHES; k++) {
            long long got = run_on_copy(APPROACHES[k].fn, c->a, c->n);
            snprintf(label, sizeof label, "test %2d, %-14s -> %lld", t + 1, APPROACHES[k].name, got);
            check(got == c->expect, label);
        }
    }

    {
        const int n = 100000;
        int *a = malloc((size_t)n * sizeof *a);
        if (a != NULL) {
            /* Nearly sorted: 0..n-1 with each pair (10q, 10q+1) swapped.
             * Each swap creates exactly one inversion: n/10 = 10000. Insertion
             * sort shines here (O(n + 10000)); brute force would need ~5e9 steps. */
            for (int i = 0; i < n; i++)
                a[i] = i;
            for (int i = 0; i + 1 < n; i += 10) {
                int tmp = a[i];
                a[i] = a[i + 1];
                a[i + 1] = tmp;
            }
            check_large("nearly sorted n = 100000", a, n, ADAPTIVE, 10000LL);

            /* Strictly decreasing: every pair is an inversion, so the count is
             * n(n-1)/2 = 100000 * 99999 / 2 = 4999950000 > INT_MAX. Only the
             * O(n log n) approaches run here. */
            for (int i = 0; i < n; i++)
                a[i] = n - i;
            check_large("decreasing n = 100000", a, n, N_LOG_N, 4999950000LL);
        } else {
            check(0, "large tests: out of memory");
        }
        free(a);
    }

    /* Random cross-check against brute force; values -10..10 on up to 30
     * elements produce plenty of duplicates. */
    {
        srand(5050);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int a[30] = {0};
            int n = rand() % 31;
            for (int k = 0; k < n; k++)
                a[k] = rand() % 21 - 10;

            long long expect = run_on_copy(inversions_brute, a, n);
            for (int k = 0; k < NUM_APPROACHES; k++)
                if (run_on_copy(APPROACHES[k].fn, a, n) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d arrays, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
