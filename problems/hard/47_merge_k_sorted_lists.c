/*
 * Q47. Merge k Sorted Lists
 * Difficulty : Hard
 * Topics     : Linked List, Heap, Divide and Conquer, Sorting
 *
 * Problem
 *   You are given k singly linked lists, each sorted in non-decreasing
 *   order. Merge them into one sorted linked list and return its head.
 *   N is the total number of nodes across all lists.
 *
 * Example
 *   [[1, 4, 5], [1, 3, 4], [2, 6]]   ->   1 1 2 3 4 4 5 6
 *
 * Approaches
 *   1. Collect + qsort               copy values, sort, rebuild        Time O(N log N)  Space O(N)
 *   2. Merge one by one              fold each list into the result    Time O(kN)       Space O(1)
 *   3. Scan k heads                  take the smallest head each step  Time O(kN)       Space O(1)
 *   4. Min-heap of heads [optimal]   heap hands out the smallest head  Time O(N log k)  Space O(k)
 *   5. Divide and conquer [optimal]  merge the lists in pairs          Time O(N log k)  Space O(log k)
 *
 * Key insight
 *   At every step the next output node is the smallest of the k current
 *   heads. Scanning all k heads costs O(k) per node; a min-heap answers
 *   "smallest head" in O(log k), giving O(N log k). Divide and conquer gets
 *   the same bound differently: merging lists in pairs means each node takes
 *   part in only log2(k) two-way merges, while merging one by one drags the
 *   growing result through up to k merges.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 47_merge_k_sorted_lists.c -o merge_k_sorted_lists && ./merge_k_sorted_lists
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

/*
 * Contract shared by every approach: merge(lists, k) takes ownership of all
 * nodes in lists[0..k-1] and returns one sorted list containing exactly
 * those values. The entries of lists[] may be overwritten. Approaches 2-5
 * relink the existing nodes; approach 1 frees them and builds new ones.
 */

static void free_list(Node *head)
{
    while (head != NULL) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

/* Standard two-way merge with a dummy head node, so the first append needs
 * no special case. Taking from a on ties (<=) keeps the merge stable.
 * Time O(|a| + |b|)   Space O(1) */
static Node *merge_two(Node *a, Node *b)
{
    Node dummy = {0, NULL};
    Node *tail = &dummy;
    while (a != NULL && b != NULL) {
        if (a->val <= b->val) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a != NULL ? a : b;
    return dummy.next;
}

/* -------------------------------------------------------------------------
 * Approach 1: Collect all values, qsort, build a new list
 *   Ignores the fact that the lists are already sorted: copy every value into
 *   an array, sort it, and build a fresh list. Simple and hard to get wrong,
 *   but it pays for a full sort and allocates N new nodes.
 *   Pitfall: the qsort comparator must not return x - y, which overflows for
 *   values like INT_MAX and INT_MIN.
 *   Time O(N log N)   Space O(N)
 * ------------------------------------------------------------------------- */
static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

static Node *merge_collect_sort(Node **lists, int k)
{
    size_t total = 0;
    for (int i = 0; i < k; i++)
        for (Node *p = lists[i]; p != NULL; p = p->next)
            total++;

    int *vals = NULL;
    if (total > 0) {
        vals = malloc(total * sizeof *vals);
        if (vals == NULL) {
            for (int i = 0; i < k; i++)
                free_list(lists[i]);
            return NULL;
        }
    }

    /* Copy the values out and free the old nodes as we go. */
    size_t idx = 0;
    for (int i = 0; i < k; i++) {
        Node *p = lists[i];
        while (p != NULL) {
            Node *next = p->next;
            vals[idx++] = p->val;
            free(p);
            p = next;
        }
        lists[i] = NULL;
    }
    if (total > 0)
        qsort(vals, total, sizeof *vals, cmp_int);

    Node dummy = {0, NULL};
    Node *tail = &dummy;
    for (size_t i = 0; i < total; i++) {
        Node *nd = malloc(sizeof *nd);
        if (nd == NULL) {
            free_list(dummy.next);
            dummy.next = NULL;
            break;
        }
        nd->val = vals[i];
        nd->next = NULL;
        tail->next = nd;
        tail = nd;
    }
    free(vals);
    return dummy.next;
}

/* -------------------------------------------------------------------------
 * Approach 2: Merge the lists one by one
 *   result = merge(result, lists[i]) for i = 0..k-1. Correct because merging
 *   two sorted lists gives a sorted list. Slow because the result keeps
 *   growing: with k lists of N/k nodes, merge i walks about i*N/k nodes, and
 *   1 + 2 + ... + k of those adds up to about kN/2.
 *   Time O(kN)   Space O(1)
 * ------------------------------------------------------------------------- */
static Node *merge_one_by_one(Node **lists, int k)
{
    Node *result = NULL;
    for (int i = 0; i < k; i++) {
        result = merge_two(result, lists[i]);
        lists[i] = NULL;
    }
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Scan all k heads for the minimum each step
 *   The next node of the output is always the smallest current head. Find it
 *   by checking every head, append it, and advance that list. Every one of
 *   the N steps scans k heads.
 *   Time O(kN)   Space O(1)
 * ------------------------------------------------------------------------- */
static Node *merge_scan_heads(Node **lists, int k)
{
    Node dummy = {0, NULL};
    Node *tail = &dummy;
    for (;;) {
        int best = -1;
        for (int i = 0; i < k; i++)
            if (lists[i] != NULL && (best < 0 || lists[i]->val < lists[best]->val))
                best = i;
        if (best < 0)
            break; /* every list is empty */
        tail->next = lists[best];
        tail = tail->next;
        lists[best] = lists[best]->next;
    }
    tail->next = NULL;
    return dummy.next;
}

/* -------------------------------------------------------------------------
 * Approach 4: Min-heap of the k heads (optimal)
 *   Same idea as approach 3, but the heads live in a binary min-heap ordered
 *   by value, so the smallest one is always heap[0]. After appending heap[0],
 *   replace it by its successor (or by the last heap element if that list is
 *   finished) and sift it down: one O(log k) repair instead of pop + push.
 *   Building the heap bottom-up from the non-empty heads costs O(k).
 *   Time O(N log k)   Space O(k) for the heap
 * ------------------------------------------------------------------------- */
static void sift_down(Node **heap, int size, int i)
{
    for (;;) {
        int smallest = i, l = 2 * i + 1, r = 2 * i + 2;
        if (l < size && heap[l]->val < heap[smallest]->val)
            smallest = l;
        if (r < size && heap[r]->val < heap[smallest]->val)
            smallest = r;
        if (smallest == i)
            return;
        Node *tmp = heap[i];
        heap[i] = heap[smallest];
        heap[smallest] = tmp;
        i = smallest;
    }
}

static Node *merge_heap(Node **lists, int k)
{
    if (k == 0)
        return NULL;
    Node **heap = malloc((size_t)k * sizeof *heap);
    if (heap == NULL)
        return merge_one_by_one(lists, k); /* still correct, just slower */

    int size = 0;
    for (int i = 0; i < k; i++) {
        if (lists[i] != NULL)
            heap[size++] = lists[i];
        lists[i] = NULL;
    }
    for (int i = size / 2 - 1; i >= 0; i--)
        sift_down(heap, size, i);

    Node dummy = {0, NULL};
    Node *tail = &dummy;
    while (size > 0) {
        Node *top = heap[0];
        tail->next = top;
        tail = top;
        heap[0] = top->next != NULL ? top->next : heap[--size];
        sift_down(heap, size, 0);
    }
    tail->next = NULL;
    free(heap);
    return dummy.next;
}

/* -------------------------------------------------------------------------
 * Approach 5: Divide and conquer (optimal)
 *   Merge the left half of the lists, merge the right half, then merge the
 *   two results. The recursion tree has log2(k) levels and each level does
 *   two-way merges over all N nodes once, so the total is O(N log k).
 *   This is merge sort, except the "elements" being merged are whole lists.
 *   Time O(N log k)   Space O(log k) recursion depth
 * ------------------------------------------------------------------------- */
static Node *merge_range(Node **lists, int lo, int hi) /* lists[lo..hi) */
{
    if (lo >= hi)
        return NULL;
    if (hi - lo == 1) {
        Node *only = lists[lo];
        lists[lo] = NULL;
        return only;
    }
    int mid = lo + (hi - lo) / 2;
    Node *left = merge_range(lists, lo, mid);
    Node *right = merge_range(lists, mid, hi);
    return merge_two(left, right);
}

static Node *merge_divide_conquer(Node **lists, int k)
{
    return merge_range(lists, 0, k);
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

typedef Node *(*MergeFn)(Node **lists, int k);

static const struct {
    const char *name;
    MergeFn fn;
} APPROACHES[] = {
    {"collect + qsort", merge_collect_sort},
    {"merge one by one", merge_one_by_one},
    {"scan k heads", merge_scan_heads},
    {"min-heap", merge_heap},
    {"divide and conquer", merge_divide_conquer},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAX_K 128

/* Builds a list from vals[0..n-1]; returns NULL and frees the partial list
 * if malloc fails (only meaningful when n > 0). */
static Node *build_list(const int *vals, int n, int *failed)
{
    Node dummy = {0, NULL};
    Node *tail = &dummy;
    for (int i = 0; i < n; i++) {
        Node *nd = malloc(sizeof *nd);
        if (nd == NULL) {
            free_list(dummy.next);
            *failed = 1;
            return NULL;
        }
        nd->val = vals[i];
        nd->next = NULL;
        tail->next = nd;
        tail = nd;
    }
    return dummy.next;
}

/* Builds k fresh lists (list i has len[i] values taken in order from flat),
 * runs one approach, and compares the result with expect[0..expect_n-1].
 * The result is freed here, so every node is released no matter what. */
static int run_merge(MergeFn fn, int k, const int *len, const int *flat, const int *expect,
                     int expect_n)
{
    Node *lists[MAX_K];
    int failed = 0, offset = 0;
    for (int i = 0; i < k; i++) {
        lists[i] = build_list(flat + offset, len[i], &failed);
        offset += len[i];
    }
    if (failed) {
        for (int i = 0; i < k; i++)
            free_list(lists[i]);
        return 0;
    }

    Node *result = fn(lists, k);
    int ok = 1, count = 0;
    for (Node *p = result; p != NULL; p = p->next, count++)
        if (count >= expect_n || p->val != expect[count])
            ok = 0;
    free_list(result);
    return ok && count == expect_n;
}

typedef struct {
    int k;
    int len[8];
    int vals[8][8];
    int expect_n;
    int expect[40];
} Case;

int main(void)
{
    printf("Q47. Merge k Sorted Lists\n");

    /* Expected outputs merged by hand. */
    const Case cases[] = {
        {3, {3, 3, 2}, {{1, 4, 5}, {1, 3, 4}, {2, 6}}, 8, {1, 1, 2, 3, 4, 4, 5, 6}},
        {0, {0}, {{0}}, 0, {0}},                               /* k = 0             */
        {1, {0}, {{0}}, 0, {0}},                               /* one empty list    */
        {3, {0, 0, 0}, {{0}}, 0, {0}},                         /* only empty lists  */
        {3, {0, 1, 0}, {{0}, {1}, {0}}, 1, {1}},               /* empties around one */
        {1, {3}, {{1, 2, 3}}, 3, {1, 2, 3}},                   /* single list       */
        {3, {3, 2, 1}, {{5, 5, 5}, {5, 5}, {5}}, 6, {5, 5, 5, 5, 5, 5}}, /* all equal */
        {4, {2, 0, 3, 2}, {{1, 3}, {0}, {1, 2, 3}, {3, 4}}, 7, {1, 1, 2, 3, 3, 3, 4}}, /* duplicates */
        {3, {3, 2, 2}, {{-10, -3, 0}, {-5, 7}, {INT_MIN, INT_MAX}}, 7,
         {INT_MIN, -10, -5, -3, 0, 7, INT_MAX}},               /* negatives, extremes */
        {6, {1, 1, 1, 1, 1, 1}, {{6}, {5}, {4}, {3}, {2}, {1}}, 6, {1, 2, 3, 4, 5, 6}},
        {3, {6, 1, 2}, {{1, 2, 3, 4, 5, 6}, {0}, {3, 3}}, 9, {0, 1, 2, 3, 3, 3, 4, 5, 6}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[160];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        int flat[64] = {0}, n = 0;
        for (int i = 0; i < c->k; i++)
            for (int j = 0; j < c->len[i]; j++)
                flat[n++] = c->vals[i][j];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            snprintf(label, sizeof label, "test %2d (k = %d, N = %d), %s", t + 1, c->k, n,
                     APPROACHES[a].name);
            check(run_merge(APPROACHES[a].fn, c->k, c->len, flat, c->expect, c->expect_n), label);
        }
    }

    /* Interleaved: k = 100 lists, list i = i, i+100, i+200, ..., i+1900.
     * Together they hold every value 0..1999 exactly once, so the merge must
     * be exactly 0, 1, 2, ..., 1999. */
    {
        enum { K = 100, M = 20 };
        int len[K] = {0};
        int *flat = malloc((size_t)K * M * sizeof *flat);
        int *expect = malloc((size_t)K * M * sizeof *expect);
        if (flat != NULL && expect != NULL) {
            for (int i = 0; i < K; i++) {
                len[i] = M;
                for (int j = 0; j < M; j++)
                    flat[i * M + j] = i + j * K;
            }
            for (int v = 0; v < K * M; v++)
                expect[v] = v;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                snprintf(label, sizeof label, "interleaved (k = %d, N = %d), %s", K, K * M,
                         APPROACHES[a].name);
                check(run_merge(APPROACHES[a].fn, K, len, flat, expect, K * M), label);
            }
        } else {
            check(0, "interleaved: out of memory");
        }
        free(flat);
        free(expect);
    }

    /* Random cross-check: up to 6 lists of up to 6 sorted values each. The
     * reference answer is simply all values sorted, computed independently
     * of the approaches. Every approach gets its own freshly built lists. */
    {
        srand(4747);
        int bad = 0;
        const int rounds = 400;
        for (int r = 0; r < rounds; r++) {
            int k = rand() % 7, len[6] = {0}, flat[36] = {0}, expect[36] = {0}, n = 0;
            for (int i = 0; i < k; i++) {
                len[i] = rand() % 7;
                for (int j = 0; j < len[i]; j++)
                    flat[n + j] = rand() % 41 - 20;
                if (len[i] > 0)
                    qsort(flat + n, (size_t)len[i], sizeof *flat, cmp_int);
                n += len[i];
            }
            for (int v = 0; v < n; v++)
                expect[v] = flat[v];
            if (n > 0)
                qsort(expect, (size_t)n, sizeof *expect, cmp_int);

            for (int a = 0; a < NUM_APPROACHES; a++)
                if (!run_merge(APPROACHES[a].fn, k, len, flat, expect, n))
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d list sets, all approaches agree",
                 rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
