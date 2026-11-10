/*
 * Q18. Valid Parentheses
 * Difficulty : Easy
 * Topics     : Strings, Stack
 *
 * Problem
 *   Given a string s made of the characters ( ) [ ] { }, decide whether it
 *   is valid: every opening bracket is closed by a bracket of the same
 *   type, and brackets close in the correct order (the most recently
 *   opened one first). The empty string is valid. (LeetCode 20)
 *   Here every approach returns 1 for valid, 0 for invalid, and -1 if it
 *   could not allocate memory. Any other character makes s invalid.
 *
 * Example
 *   "()[]{}"   ->   valid
 *   "([)]"     ->   invalid   ( is closed by ] before [ is closed
 *   "{[]}"     ->   valid
 *
 * Approaches
 *   1. Remove pairs            delete "()" "[]" "{}" until stuck  Time O(n^2)  Space O(n)
 *   2. Stack      [optimal]    push openers, match each closer    Time O(n)    Space O(n)
 *   Special case: depth counter for "(" and ")" only               Time O(n)    Space O(1)
 *
 * Key insight
 *   A closing bracket must match the most recent unmatched opener: last in,
 *   first out. That is exactly what a stack gives you. A plain counter
 *   remembers how many brackets are open but not which ones, which is
 *   enough for a single bracket type and wrong for mixed types.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 18_valid_parentheses.c -o valid_parentheses && ./valid_parentheses
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_opener(char c)
{
    return c == '(' || c == '[' || c == '{';
}

static int is_pair(char open, char close)
{
    return (open == '(' && close == ')') || (open == '[' && close == ']') ||
           (open == '{' && close == '}');
}

/* -------------------------------------------------------------------------
 * Approach 1: Repeatedly remove adjacent matching pairs (brute force)
 *   Any valid string that is not empty contains an adjacent pair like "()"
 *   (its innermost pair). Deleting a matched pair never changes whether a
 *   string is valid, in either direction. So keep deleting adjacent pairs:
 *   the string is valid exactly when this reaches the empty string.
 *   Each pass is O(n) and removes at least one pair, so there are at most
 *   n/2 + 1 passes. "((((...))))" removes only one pair per pass.
 *   Time O(n^2)   Space O(n) for a writable copy of s
 * ------------------------------------------------------------------------- */
static int valid_remove_pairs(const char *s)
{
    size_t len = strlen(s);
    char *buf = malloc(len + 1);
    if (buf == NULL)
        return -1;
    memcpy(buf, s, len + 1);

    int changed = 1;
    while (changed) {
        changed = 0;
        size_t w = 0;
        for (size_t i = 0; i < len; i++) {
            if (i + 1 < len && is_pair(buf[i], buf[i + 1])) {
                i++; /* drop buf[i] and buf[i + 1] */
                changed = 1;
            } else {
                buf[w++] = buf[i]; /* w <= i, so unread characters are safe */
            }
        }
        len = w;
    }
    free(buf);
    return len == 0;
}

/* -------------------------------------------------------------------------
 * Approach 2: Stack (optimal)
 *   Scan left to right. Push every opener. For a closer, the top of the
 *   stack must be the matching opener: pop it. Otherwise (empty stack or
 *   wrong type) the string is invalid. At the end the stack must be empty,
 *   or some opener was never closed.
 *   An odd length can never balance, so reject it before allocating.
 *   The stack is a plain char array of size len: at most len pushes.
 *   Pitfall: check top > 0 before reading stack[top - 1].
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int valid_stack(const char *s)
{
    size_t len = strlen(s);
    if (len % 2 != 0)
        return 0;
    if (len == 0)
        return 1;

    char *stack = malloc(len);
    if (stack == NULL)
        return -1;
    size_t top = 0;
    int ok = 1;
    for (size_t i = 0; i < len && ok; i++) {
        char c = s[i];
        if (is_opener(c))
            stack[top++] = c;
        else if (top > 0 && is_pair(stack[top - 1], c))
            top--;
        else
            ok = 0; /* closer with nothing open, wrong type, or not a bracket */
    }
    free(stack);
    return ok && top == 0;
}

/* -------------------------------------------------------------------------
 * Special case: only "(" and ")" (a depth counter is enough)
 *   With a single bracket type, every opener is interchangeable, so the
 *   stack would only ever hold copies of '('. All that matters is its size.
 *   depth = number of currently open brackets. It must never go below zero
 *   (a ")" with nothing to close) and must end at zero.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int valid_counter_round(const char *s)
{
    size_t depth = 0;
    for (; *s != '\0'; s++) {
        if (*s == '(') {
            depth++;
        } else if (*s == ')') {
            if (depth == 0)
                return 0;
            depth--;
        } else {
            return 0;
        }
    }
    return depth == 0;
}

/* Why counters fail for mixed types: one counter per bracket type checks
 * that each type balances on its own, but not the ORDER in which different
 * types close. For "([)]" every counter rises to 1 and falls back to 0,
 * so this function wrongly calls it valid. Only used for the demo below. */
static int valid_counters_mixed_wrong(const char *s)
{
    static const char opens[] = "([{";
    static const char closes[] = ")]}";
    int depth[3] = {0, 0, 0};
    for (; *s != '\0'; s++) {
        int known = 0;
        for (int t = 0; t < 3; t++) {
            if (*s == opens[t]) {
                depth[t]++;
                known = 1;
            } else if (*s == closes[t]) {
                if (--depth[t] < 0)
                    return 0;
                known = 1;
            }
        }
        if (!known)
            return 0;
    }
    return depth[0] == 0 && depth[1] == 0 && depth[2] == 0;
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

typedef int (*ValidFn)(const char *s);

static const struct {
    const char *name;
    ValidFn fn;
} APPROACHES[] = {
    {"remove pairs", valid_remove_pairs},
    {"stack", valid_stack},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *s;
    int expect;
} Case;

/* Writes a random string of length len (even) that is valid by
 * construction: must open when nothing is open, must close when every
 * remaining slot is needed for closers, otherwise pick at random. depth and
 * remaining always have the same parity, so an open never leaves more open
 * brackets than slots to close them. */
static void random_valid(char *out, int len)
{
    static const char opens[] = "([{";
    static const char closes[] = ")]}";
    int types[64];
    int depth = 0;
    for (int pos = 0; pos < len; pos++) {
        int remaining = len - pos;
        int do_open = depth == 0 ? 1 : depth == remaining ? 0 : rand() % 2;
        if (do_open) {
            types[depth] = rand() % 3;
            out[pos] = opens[types[depth]];
            depth++;
        } else {
            depth--;
            out[pos] = closes[types[depth]];
        }
    }
    out[len] = '\0';
}

int main(void)
{
    printf("Q18. Valid Parentheses\n");

    const Case cases[] = {
        {"()", 1},
        {"()[]{}", 1},
        {"{[]}", 1},
        {"{[()()]}", 1},
        {"(((())))", 1},
        {"", 1},       /* empty string is valid */
        {"(]", 0},     /* wrong type */
        {"([)]", 0},   /* right counts, wrong order */
        {"(", 0},      /* never closed */
        {")", 0},      /* nothing to close */
        {"((", 0},     /* even length, still unbalanced */
        {"))((", 0},   /* balanced counts, closes too early */
        {"([]", 0},
        {"[{]}", 0},
        {"(a)", 0},    /* not a bracket */
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(cases[t].s);
            snprintf(label, sizeof label, "\"%s\", %-12s -> %s", cases[t].s, APPROACHES[a].name,
                     got == 1 ? "valid" : "invalid");
            check(got == cases[t].expect, label);
        }
    }

    /* The depth counter on strings with only round brackets. */
    {
        const Case round_cases[] = {
            {"", 1}, {"()", 1}, {"(())()", 1}, {")(", 0}, {"(()", 0}, {"())(", 0},
        };
        const int num_round = (int)(sizeof round_cases / sizeof round_cases[0]);
        for (int t = 0; t < num_round; t++) {
            int got = valid_counter_round(round_cases[t].s);
            snprintf(label, sizeof label, "\"%s\", depth counter -> %s", round_cases[t].s,
                     got ? "valid" : "invalid");
            check(got == round_cases[t].expect, label);
        }
    }

    /* Demo, not part of the cross-check: counters cannot see ordering. */
    check(valid_counters_mixed_wrong("([)]") == 1 && valid_stack("([)]") == 0,
          "demo: per-type counters wrongly accept \"([)]\", the stack rejects it");

    /* Random cross-check. Purely random strings are almost always invalid,
     * so a third are valid by construction, a third are valid strings with
     * one character changed (usually, not always, invalid), and a third are
     * random. The stack must agree with the remove-pairs brute force. */
    {
        static const char alphabet[] = "()[]{}";
        srand(1818);
        int bad = 0;
        const int rounds = 600;
        for (int r = 0; r < rounds; r++) {
            char s[21];
            int len;
            if (r % 3 == 2) {
                len = rand() % 21;
                for (int k = 0; k < len; k++)
                    s[k] = alphabet[rand() % 6];
                s[len] = '\0';
            } else {
                len = 2 * (rand() % 11); /* 0..20, even */
                random_valid(s, len);
                if (r % 3 == 1 && len > 0)
                    s[rand() % len] = alphabet[rand() % 6];
            }

            int expect = valid_remove_pairs(s);
            if (r % 3 == 0 && expect != 1)
                bad++; /* constructed strings must really be valid */
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(s) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d strings, all approaches agree", rounds);
        check(bad == 0, label);
    }

    /* Random cross-check for the special case: on "(" / ")" strings the
     * depth counter must agree with the stack. */
    {
        srand(1819);
        int bad = 0;
        const int rounds = 300;
        for (int r = 0; r < rounds; r++) {
            char s[17];
            int len = rand() % 17;
            for (int k = 0; k < len; k++)
                s[k] = rand() % 2 ? '(' : ')';
            s[len] = '\0';
            if (valid_counter_round(s) != valid_stack(s))
                bad++;
        }
        snprintf(label, sizeof label, "depth counter cross-check: %d round-only strings, agrees with stack", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
