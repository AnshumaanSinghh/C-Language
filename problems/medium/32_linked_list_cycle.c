/*
 * Q32. Linked List Cycle
 * Difficulty : Medium
 * Topics     : Linked List, Two Pointers, Hashing
 *
 * Problem
 *   Given the head of a singly linked list, decide whether it contains a
 *   cycle (following next pointers forever never reaches NULL). If it does,
 *   return the node where the cycle begins and the cycle's length;
 *   otherwise return NULL. The list must not be modified.
 *
 * Example
 *   3 -> 2 -> 0 -> -4
 *        ^          |
 *        +----------+        ->   cycle starts at the node holding 2, length 3
 *
 * Approaches
 *   1. Visited array          remember every node, linear lookup  Time O(n^2)  Space O(n)
 *   2. Hash set of pointers   remember every node, O(1) lookup    Time O(n)    Space O(n)
 *   3. Floyd      [optimal]   slow/fast pointers, then reset one  Time O(n)    Space O(1)
 *   4. Brent                  teleporting tortoise, length first  Time O(n)    Space O(1)
 *
 * Key insight
 *   Inside a cycle, a pointer moving 2 steps per turn gains exactly 1 step
 *   per turn on a pointer moving 1 step, so it must land on it. Simple
 *   arithmetic on where they meet then locates the start of the cycle with
 *   no extra memory.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 32_linked_list_cycle.c -o linked_list_cycle && ./linked_list_cycle
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

/* Every approach returns the first node of the cycle (NULL if there is no
 * cycle) and writes the cycle length to *cycle_len (0 if there is none).
 * Nodes are compared by ADDRESS, never by value: two different nodes may
 * hold the same value. */

/* -------------------------------------------------------------------------
 * Approach 1: Visited array (brute force)
 *   Walk the list and append every node pointer to an array. Before
 *   appending, search the array for the current node. The first node that
 *   is already there is where the cycle begins, because it is the first
 *   node we reach a second time. If it was stored at index j and we have
 *   taken `count` steps, the cycle length is count - j.
 *   We do not know the length of the list up front (counting nodes would
 *   loop forever on a cycle), so the array grows by doubling.
 *   Time O(n^2) for the linear lookups   Space O(n)
 * ------------------------------------------------------------------------- */
static const Node *cycle_visited_array(const Node *head, int *cycle_len)
{
    *cycle_len = 0;
    size_t cap = 16, count = 0;
    const Node **seen = malloc(cap * sizeof *seen);
    if (seen == NULL)
        return NULL;

    const Node *result = NULL;
    for (const Node *p = head; p != NULL && result == NULL; p = p->next) {
        for (size_t j = 0; j < count; j++) {
            if (seen[j] == p) {
                result = p;
                *cycle_len = (int)(count - j);
                break;
            }
        }
        if (result == NULL) {
            if (count == cap) {
                const Node **bigger = realloc(seen, 2 * cap * sizeof *seen);
                if (bigger == NULL)
                    break; /* out of memory: give up, still free below */
                seen = bigger;
                cap *= 2;
            }
            seen[count++] = p;
        }
    }
    free(seen);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 2: Hash set of node pointers
 *   Same idea as approach 1, but the "have I seen this node?" question is
 *   answered by a hash table keyed on the node's address. The table also
 *   stores the step at which each node was first seen, which gives the
 *   cycle length.
 *   An address is just a number once converted to uintptr_t. Nodes are
 *   aligned, so the low bits are always zero; mixing the bits (a multiply
 *   and xor-shifts) spreads them over the whole table.
 *   The table uses open addressing and doubles when it is half full.
 *   Time O(n) expected   Space O(n)
 * ------------------------------------------------------------------------- */
typedef struct {
    const Node **keys; /* NULL marks an empty slot */
    size_t *steps;
    size_t mask;       /* capacity - 1, capacity is a power of two */
    size_t size;
} PtrSet;

static size_t hash_ptr(const Node *p)
{
    uint64_t x = (uint64_t)(uintptr_t)p;
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return (size_t)x;
}

static int set_init(PtrSet *s, size_t cap)
{
    s->keys = calloc(cap, sizeof *s->keys);
    s->steps = malloc(cap * sizeof *s->steps);
    s->mask = cap - 1;
    s->size = 0;
    return s->keys != NULL && s->steps != NULL;
}

static void set_free(PtrSet *s)
{
    free(s->keys);
    free(s->steps);
}

/* Returns the slot holding p, or the empty slot where p would go. */
static size_t set_slot(const PtrSet *s, const Node *p)
{
    size_t h = hash_ptr(p) & s->mask;
    while (s->keys[h] != NULL && s->keys[h] != p)
        h = (h + 1) & s->mask;
    return h;
}

static int set_grow(PtrSet *s)
{
    PtrSet bigger;
    if (!set_init(&bigger, 2 * (s->mask + 1))) {
        set_free(&bigger);
        return 0;
    }
    for (size_t i = 0; i <= s->mask; i++) {
        if (s->keys[i] != NULL) {
            size_t h = set_slot(&bigger, s->keys[i]);
            bigger.keys[h] = s->keys[i];
            bigger.steps[h] = s->steps[i];
        }
    }
    bigger.size = s->size;
    set_free(s);
    *s = bigger;
    return 1;
}

static const Node *cycle_hash_set(const Node *head, int *cycle_len)
{
    *cycle_len = 0;
    PtrSet s;
    if (!set_init(&s, 16)) {
        set_free(&s);
        return NULL;
    }

    const Node *result = NULL;
    size_t step = 0;
    for (const Node *p = head; p != NULL; p = p->next, step++) {
        size_t h = set_slot(&s, p);
        if (s.keys[h] == p) {
            result = p;
            *cycle_len = (int)(step - s.steps[h]);
            break;
        }
        s.keys[h] = p;
        s.steps[h] = step;
        if (++s.size * 2 > s.mask + 1 && !set_grow(&s))
            break; /* out of memory */
    }
    set_free(&s);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Floyd's tortoise and hare (optimal)
 *   Phase 1, detect: slow moves 1 step, fast moves 2 steps. If fast hits
 *   NULL there is no cycle. Otherwise they meet inside the cycle: once
 *   both are in the cycle, the gap between them shrinks by exactly 1 per
 *   turn, so fast cannot jump over slow.
 *
 *   Phase 2, find the start. Let
 *     mu  = number of steps from head to the cycle start,
 *     lam = cycle length,
 *     a   = steps slow has taken inside the cycle when they meet, so the
 *           meeting point is a steps (mod lam) past the cycle start.
 *   At the meeting point slow has walked mu + a steps and fast twice that.
 *   Fast's extra mu + a steps were whole laps of the cycle, so
 *       mu + a = t * lam   for some integer t >= 1,
 *   which means  mu = t * lam - a.
 *   Now put one pointer back at head and leave the other at the meeting
 *   point, and move both 1 step at a time. After mu steps the first is at
 *   the cycle start. The second has moved from position a to position
 *   a + mu = t * lam, i.e. a whole number of laps: also the cycle start.
 *   So the first node where they coincide is the start.
 *
 *   Phase 3, length: walk once around the cycle from the start, counting.
 *
 *   Slow takes at most mu + lam steps before the meeting, phase 2 takes
 *   mu steps and phase 3 takes lam, so the whole thing is linear and uses
 *   two pointers.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static const Node *cycle_floyd(const Node *head, int *cycle_len)
{
    *cycle_len = 0;
    const Node *slow = head, *fast = head;
    int met = 0;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) {
            met = 1;
            break;
        }
    }
    if (!met)
        return NULL;

    const Node *p = head;
    while (p != slow) {
        p = p->next;
        slow = slow->next;
    }

    int len = 1;
    for (const Node *q = p->next; q != p; q = q->next)
        len++;
    *cycle_len = len;
    return p;
}

/* -------------------------------------------------------------------------
 * Approach 4: Brent's algorithm
 *   The tortoise stays put while the hare runs ahead one step at a time.
 *   Whenever the hare has run `power` steps (power = 1, 2, 4, 8, ...), the
 *   tortoise teleports to the hare and power doubles. Once power >= lam
 *   and the tortoise is inside the cycle, the hare comes back round to the
 *   tortoise within lam steps, and the number of steps it ran since the
 *   last teleport is exactly lam. So Brent finds the LENGTH first.
 *   Then the start: put one pointer lam steps ahead of the other, both
 *   starting from head, and advance both together. The leading pointer is
 *   always exactly one lap ahead once both are in the cycle, so they first
 *   coincide at the cycle start.
 *   Compared with Floyd it follows fewer next pointers on average (only
 *   one pointer moves in the search phase); Brent reported it about 36%
 *   faster. Same O(n) time and O(1) space.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static const Node *cycle_brent(const Node *head, int *cycle_len)
{
    *cycle_len = 0;
    if (head == NULL)
        return NULL;

    long power = 1, lam = 1;
    const Node *tortoise = head, *hare = head->next;
    while (hare != tortoise) {
        if (hare == NULL)
            return NULL;
        if (power == lam) {
            tortoise = hare;
            power *= 2;
            lam = 0;
        }
        hare = hare->next;
        lam++;
    }

    tortoise = hare = head;
    for (long i = 0; i < lam; i++)
        hare = hare->next;
    while (tortoise != hare) {
        tortoise = tortoise->next;
        hare = hare->next;
    }
    *cycle_len = (int)lam;
    return tortoise;
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

typedef const Node *(*CycleFn)(const Node *head, int *cycle_len);

static const struct {
    const char *name;
    CycleFn fn;
} APPROACHES[] = {
    {"visited array", cycle_visited_array},
    {"hash set", cycle_hash_set},
    {"Floyd", cycle_floyd},
    {"Brent", cycle_brent},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

/* Builds a list from vals[0..n-1]. If pos >= 0 the tail links back to node
 * pos. All nodes live in ONE malloc block, so a single free() releases the
 * list even when it is cyclic (walking a cyclic list to free it node by
 * node would never end). Returns the block, which is also the head. */
static Node *build_list(const int *vals, int n, int pos)
{
    if (n == 0)
        return NULL;
    Node *nodes = malloc((size_t)n * sizeof *nodes);
    if (nodes == NULL)
        return NULL;
    for (int i = 0; i < n; i++) {
        nodes[i].val = vals[i];
        nodes[i].next = i + 1 < n ? &nodes[i + 1] : NULL;
    }
    if (pos >= 0)
        nodes[n - 1].next = &nodes[pos];
    return nodes;
}

/* Expected answer straight from the construction: the cycle starts at
 * node pos and contains nodes pos..n-1. */
static int answer_ok(const Node *nodes, int n, int pos, const Node *got, int got_len)
{
    if (pos < 0)
        return got == NULL && got_len == 0;
    return got == &nodes[pos] && got_len == n - pos;
}

typedef struct {
    const char *what;
    int vals[10];
    int n;
    int pos;
} Case;

int main(void)
{
    printf("Q32. Linked List Cycle\n");

    /* The expected start is node `pos` and the expected length is n - pos,
     * both read straight off the picture of each list. */
    const Case cases[] = {
        {"example", {3, 2, 0, -4}, 4, 1},
        {"two nodes, back to head", {1, 2}, 2, 0},
        {"single node, no cycle", {1}, 1, -1},
        {"single node, self loop", {1}, 1, 0},
        {"empty list", {0}, 0, -1},
        {"no cycle", {1, 2, 3, 4, 5}, 5, -1},
        {"tail points to itself", {1, 2, 3, 4, 5}, 5, 4},
        {"whole list is a cycle", {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 10, 0},
        {"equal values, compare ptrs", {7, 7, 7, 7}, 4, 2},
        {"negative values", {-1, -2, -3, -4, -5, -6}, 6, 3},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        Node *list = build_list(c->vals, c->n, c->pos);
        if (c->n > 0 && list == NULL) {
            check(0, "allocation");
            continue;
        }
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int len = -1;
            const Node *start = APPROACHES[a].fn(list, &len);
            int idx = start ? (int)(start - list) : -1;
            snprintf(label, sizeof label, "test %2d %-27s, %-13s -> start %d, len %d", t + 1,
                     c->what, APPROACHES[a].name, idx, len);
            check(answer_ok(list, c->n, c->pos, start, len), label);
        }
        free(list);
    }

    /* Long tail and a small cycle, then a long cycle: n = 20000. The O(n^2)
     * visited array is skipped here (2 * 10^8 comparisons). */
    {
        enum { N = 20000 };
        int *vals = calloc(N, sizeof *vals);
        const int positions[] = {N - 3, 1};
        for (int pi = 0; pi < 2; pi++) {
            Node *list = vals ? build_list(vals, N, positions[pi]) : NULL;
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (APPROACHES[a].fn == cycle_visited_array)
                    continue;
                int len = -1;
                const Node *start = list ? APPROACHES[a].fn(list, &len) : NULL;
                snprintf(label, sizeof label, "n=%d, pos=%d, %-13s -> len %d", N,
                         positions[pi], APPROACHES[a].name, len);
                check(list != NULL && answer_ok(list, N, positions[pi], start, len), label);
            }
            free(list);
        }
        free(vals);
    }

    /* Random cross-check: every approach must match both the construction
     * and the brute force. */
    {
        srand(32);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            int vals[40];
            int n = rand() % 41;
            for (int i = 0; i < n; i++)
                vals[i] = rand() % 5; /* lots of repeated values on purpose */
            int pos = n == 0 ? -1 : rand() % (n + 1) - 1;
            Node *list = build_list(vals, n, pos);
            if (n > 0 && list == NULL) {
                bad++;
                continue;
            }
            int brute_len;
            const Node *brute = cycle_visited_array(list, &brute_len);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                int len;
                const Node *start = APPROACHES[a].fn(list, &len);
                if (start != brute || len != brute_len || !answer_ok(list, n, pos, start, len))
                    bad++;
            }
            free(list);
        }
        snprintf(label, sizeof label, "random cross-check: %d lists, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
