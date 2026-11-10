/*
 * Q19. Reverse Linked List
 * Difficulty : Easy
 * Topics     : Linked List, Recursion, Stack
 *
 * Problem
 *   Given the head of a singly linked list, reverse the list and return
 *   the new head. An empty list is just a NULL head.
 *
 * Example
 *   1 -> 2 -> 3 -> 4 -> 5 -> NULL   ->   5 -> 4 -> 3 -> 2 -> 1 -> NULL
 *
 * Approaches
 *   1. Reverse values             copy values out, write back reversed  Time O(n)  Space O(n)
 *   2. Stack of nodes             push node pointers, pop to relink     Time O(n)  Space O(n)
 *   3. Three pointers [optimal]   flip each next pointer while walking  Time O(n)  Space O(1)
 *   4. Recursion                  reverse the rest, then hook head on   Time O(n)  Space O(n) call stack
 *
 * Key insight
 *   Reversing a list only means turning every next arrow around. Walking
 *   forward with prev / cur / next, you can flip cur->next to prev as long
 *   as you saved the old cur->next first; no extra memory is needed.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 19_reverse_linked_list.c -o reverse_linked_list && ./reverse_linked_list
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int val;
    struct Node *next;
} Node;

/* ------------------------------------------------------------------------- */
/* List helpers                                                              */
/* ------------------------------------------------------------------------- */
static void list_free(Node *head)
{
    while (head != NULL) {
        Node *next = head->next; /* read next before the node is gone */
        free(head);
        head = next;
    }
}

/* Builds a list holding a[0..n-1] in order. Returns NULL for n == 0, and
 * also NULL (after freeing the partial list) if an allocation fails.
 * tail points at the link to fill next: first &head, then &last->next,
 * which saves the usual "is this the first node?" special case. */
static Node *list_from_array(const int *a, int n)
{
    Node *head = NULL;
    Node **tail = &head;
    for (int i = 0; i < n; i++) {
        Node *node = malloc(sizeof *node);
        if (node == NULL) {
            list_free(head);
            return NULL;
        }
        node->val = a[i];
        node->next = NULL;
        *tail = node;
        tail = &node->next;
    }
    return head;
}

/* Copies the list's values into out[] and returns the node count. Returns
 * -1 if the list has more than cap nodes, which also stops a cycle left by
 * a buggy reversal from looping forever. */
static int list_to_array(const Node *head, int *out, int cap)
{
    int n = 0;
    for (const Node *p = head; p != NULL; p = p->next) {
        if (n == cap)
            return -1;
        out[n++] = p->val;
    }
    return n;
}

static int list_length(const Node *head)
{
    int n = 0;
    for (const Node *p = head; p != NULL; p = p->next)
        n++;
    return n;
}

/* -------------------------------------------------------------------------
 * Approach 1: Reverse the values, keep the nodes
 *   Copy every value into an array, then walk the list again writing the
 *   values back from the end of the array. The links never change.
 *   Pitfall: this is only "reversing" if nobody cares which node holds
 *   which value. If nodes carry big payloads, or other code keeps pointers
 *   to particular nodes, moving values around is the wrong tool.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static Node *reverse_values(Node *head)
{
    int n = list_length(head);
    if (n < 2)
        return head;
    int *vals = malloc((size_t)n * sizeof *vals);
    if (vals == NULL)
        return head; /* out of memory: list left as it was */

    int i = 0;
    for (Node *p = head; p != NULL; p = p->next)
        vals[i++] = p->val;
    for (Node *p = head; p != NULL; p = p->next)
        p->val = vals[--i];
    free(vals);
    return head;
}

/* -------------------------------------------------------------------------
 * Approach 2: Explicit stack of node pointers
 *   Push every node, so stack[n-1] is the old tail and becomes the new
 *   head. Then link each node to the one pushed before it, and terminate
 *   the old head (now the tail) with NULL. Forgetting that last NULL leaves
 *   a two-node cycle at the end of the list.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static Node *reverse_stack(Node *head)
{
    int n = list_length(head);
    if (n < 2)
        return head;
    Node **stack = malloc((size_t)n * sizeof *stack);
    if (stack == NULL)
        return head;

    int top = 0;
    for (Node *p = head; p != NULL; p = p->next)
        stack[top++] = p;

    Node *new_head = stack[n - 1];
    for (int i = n - 1; i > 0; i--)
        stack[i]->next = stack[i - 1];
    stack[0]->next = NULL;
    free(stack);
    return new_head;
}

/* -------------------------------------------------------------------------
 * Approach 3: Iterative three-pointer reversal (optimal)
 *   Invariant: prev is the head of the already reversed part, cur is the
 *   head of the part still to do. Each step:
 *     next = cur->next    save the rest before we lose it
 *     cur->next = prev    flip one arrow
 *     prev = cur, cur = next
 *   When cur is NULL, prev is the new head. Empty and one-node lists need
 *   no special case.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static Node *reverse_iterative(Node *head)
{
    Node *prev = NULL;
    Node *cur = head;
    while (cur != NULL) {
        Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    return prev;
}

/* -------------------------------------------------------------------------
 * Approach 4: Recursion
 *   Reverse everything after head; that returns the new head (the old
 *   tail). head->next still points at what is now the LAST node of the
 *   reversed rest, so hook head after it and cut head->next.
 *   Each node costs one stack frame, so a list of a million nodes can
 *   overflow the call stack. Fine for learning, risky in production.
 *   Time O(n)   Space O(n) call stack
 * ------------------------------------------------------------------------- */
static Node *reverse_recursive(Node *head)
{
    if (head == NULL || head->next == NULL)
        return head;
    Node *new_head = reverse_recursive(head->next);
    head->next->next = head;
    head->next = NULL;
    return new_head;
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

typedef Node *(*ReverseFn)(Node *head);

static const struct {
    const char *name;
    ReverseFn fn;
    int relinks; /* 1 = moves nodes; 0 = moves values and keeps the links */
} APPROACHES[] = {
    {"reverse values", reverse_values, 0},
    {"stack of nodes", reverse_stack, 1},
    {"three pointers", reverse_iterative, 1},
    {"recursion", reverse_recursive, 1},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

#define MAXN 20

typedef struct {
    int in[8];
    int n;
    int expect[8];
} Case;

/* Builds a fresh list from in[], reverses it with fn, and checks the result
 * equals expect[0..n-1]. Always frees the list it built. */
static int reverses_to(ReverseFn fn, const int *in, int n, const int *expect)
{
    Node *head = list_from_array(in, n);
    if (n > 0 && head == NULL)
        return 0; /* allocation failed */
    head = fn(head);

    int got[MAXN];
    int len = list_to_array(head, got, MAXN);
    list_free(head);
    if (len != n)
        return 0;
    for (int i = 0; i < n; i++)
        if (got[i] != expect[i])
            return 0;
    return 1;
}

int main(void)
{
    printf("Q19. Reverse Linked List\n");

    const Case cases[] = {
        {{1, 2, 3, 4, 5}, 5, {5, 4, 3, 2, 1}},
        {{0}, 0, {0}},                                        /* empty list */
        {{7}, 1, {7}},                                        /* single node */
        {{1, 2}, 2, {2, 1}},                                  /* two nodes */
        {{3, 3, 1, 3}, 4, {3, 1, 3, 3}},                      /* duplicates */
        {{9, 9, 9}, 3, {9, 9, 9}},                            /* all equal */
        {{-1, 0, INT_MIN, INT_MAX}, 4, {INT_MAX, INT_MIN, 0, -1}},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            snprintf(label, sizeof label, "test %d, %-14s (n = %d)", t + 1, APPROACHES[a].name, c->n);
            check(reverses_to(APPROACHES[a].fn, c->in, c->n, c->expect), label);
        }
    }

    /* The pointer-based approaches must reuse the original nodes: the new
     * head is the node that used to be the tail. */
    for (int a = 0; a < NUM_APPROACHES; a++) {
        if (!APPROACHES[a].relinks)
            continue;
        const int in[] = {10, 20, 30};
        Node *head = list_from_array(in, 3);
        Node *old_tail = head != NULL ? head->next->next : NULL;
        Node *new_head = APPROACHES[a].fn(head);
        snprintf(label, sizeof label, "%-14s relinks nodes: new head is the old tail", APPROACHES[a].name);
        check(old_tail != NULL && new_head == old_tail, label);
        list_free(new_head);
    }

    /* Random cross-check: every approach must produce the input read
     * backwards, on its own freshly built list. */
    {
        srand(1919);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            int in[MAXN], expect[MAXN];
            int n = rand() % (MAXN + 1);
            for (int k = 0; k < n; k++)
                in[k] = rand() % 21 - 10;
            for (int k = 0; k < n; k++)
                expect[k] = in[n - 1 - k];

            for (int a = 0; a < NUM_APPROACHES; a++)
                if (!reverses_to(APPROACHES[a].fn, in, n, expect))
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d lists, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
