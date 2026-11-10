/*
 * Q04. Valid Palindrome
 * Difficulty : Easy
 * Topics     : Strings, Two Pointers, Recursion
 *
 * Problem
 *   Given a string s, return 1 if it reads the same forwards and backwards
 *   after removing every character that is not a letter or a digit and
 *   ignoring upper/lower case; return 0 otherwise. The empty string (or a
 *   string with no letters or digits) counts as a palindrome.
 *   (Same rules as LeetCode 125.)
 *
 * Example
 *   "A man, a plan, a canal: Panama"   ->   1   ("amanaplanacanalpanama")
 *   "race a car"                       ->   0   ("raceacar")
 *
 * Approaches
 *   1. Clean + reverse + compare    build cleaned and reversed copies   Time O(n)   Space O(n)
 *   2. Clean + two pointers         build cleaned copy, scan from ends  Time O(n)   Space O(n)
 *   3. Two pointers  [optimal]      skip non-alphanumerics on the fly   Time O(n)   Space O(1)
 *   4. Recursion                    compare ends, recurse on middle     Time O(n)   Space O(n) stack
 *
 * Key insight
 *   You never need the cleaned string itself. Two indices, one from each
 *   end, can step over punctuation as they go and compare the letters they
 *   land on, so no copy is ever made.
 *
 * Using <ctype.h> correctly
 *   isalnum() and tolower() take an int that must be EOF or a value of
 *   unsigned char. On most platforms plain char is signed, so a byte such as
 *   0xFF arrives as -1 and passing it directly is undefined behaviour.
 *   Always cast: isalnum((unsigned char)c). This program never calls
 *   setlocale(), so it runs in the "C" locale, where only ASCII letters and
 *   digits are alphanumeric.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 04_palindrome_string.c -o palindrome_string && ./palindrome_string
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Lower-cased copy of s with every non-alphanumeric character removed.
 * Writes its length to *out_len. The caller must free the result.
 * Returns NULL if out of memory. */
static char *cleaned_copy(const char *s, size_t *out_len)
{
    size_t n = strlen(s);
    char *c = malloc(n + 1);
    if (c == NULL)
        return NULL;
    size_t k = 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char ch = (unsigned char)s[i];
        if (isalnum(ch))
            c[k++] = (char)tolower(ch);
    }
    c[k] = '\0';
    *out_len = k;
    return c;
}

/* -------------------------------------------------------------------------
 * Approach 1: Clean, reverse, compare
 *   Straight from the definition: build the cleaned string, build its
 *   reverse, and check that the two are equal. Two extra buffers.
 *   Returns -1 if out of memory (both buffers are freed on every path).
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int is_pal_reverse(const char *s)
{
    size_t n = 0;
    char *clean = cleaned_copy(s, &n);
    if (clean == NULL)
        return -1;
    char *rev = malloc(n + 1);
    if (rev == NULL) {
        free(clean);
        return -1;
    }
    for (size_t i = 0; i < n; i++)
        rev[i] = clean[n - 1 - i];
    rev[n] = '\0';

    int result = strcmp(clean, rev) == 0;
    free(clean);
    free(rev);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 2: Clean, then two pointers
 *   The reversed copy is unnecessary: compare clean[lo] with clean[hi] and
 *   move both inward. Stops at the first mismatch, so a non-palindrome is
 *   often rejected early. Still needs the cleaned copy.
 *   Pitfall: with size_t indices, hi = n - 1 wraps around to a huge number
 *   when n == 0. Guard the empty case first.
 *   Time O(n)   Space O(n)
 * ------------------------------------------------------------------------- */
static int is_pal_clean_two_pointer(const char *s)
{
    size_t n = 0;
    char *clean = cleaned_copy(s, &n);
    if (clean == NULL)
        return -1;

    int result = 1;
    if (n > 0) {
        size_t lo = 0, hi = n - 1;
        while (lo < hi) {
            if (clean[lo] != clean[hi]) {
                result = 0;
                break;
            }
            lo++;
            hi--;
        }
    }
    free(clean);
    return result;
}

/* -------------------------------------------------------------------------
 * Approach 3: Two pointers, skipping on the fly (optimal)
 *   Work on s directly. lo walks right, hi walks left (hi is kept ONE PAST
 *   the character it looks at, so it never underflows). If either side is
 *   not alphanumeric, step over it. Otherwise compare the two characters
 *   case-insensitively; a mismatch means "not a palindrome".
 *   Each character is visited at most once by one of the pointers.
 *   Time O(n)   Space O(1)
 * ------------------------------------------------------------------------- */
static int is_pal_two_pointer(const char *s)
{
    size_t lo = 0, hi = strlen(s);
    while (lo < hi) {
        unsigned char left = (unsigned char)s[lo];
        unsigned char right = (unsigned char)s[hi - 1];
        if (!isalnum(left)) {
            lo++;
        } else if (!isalnum(right)) {
            hi--;
        } else {
            if (tolower(left) != tolower(right))
                return 0;
            lo++;
            hi--;
        }
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Approach 4: Recursion
 *   A string is a palindrome if, after skipping junk at both ends, its
 *   first and last characters match AND the part between them is a
 *   palindrome. Base case: 0 or 1 characters left.
 *   Same comparisons as approach 3, but each matched pair adds a stack
 *   frame, so a long string uses O(n) stack (and could overflow it).
 *   Time O(n)   Space O(n) call stack
 * ------------------------------------------------------------------------- */
static int is_pal_range(const char *s, size_t lo, size_t hi) /* checks s[lo .. hi-1] */
{
    while (lo < hi && !isalnum((unsigned char)s[lo]))
        lo++;
    while (lo < hi && !isalnum((unsigned char)s[hi - 1]))
        hi--;
    if (hi - lo <= 1)
        return 1;
    if (tolower((unsigned char)s[lo]) != tolower((unsigned char)s[hi - 1]))
        return 0;
    return is_pal_range(s, lo + 1, hi - 1);
}

static int is_pal_recursive(const char *s)
{
    return is_pal_range(s, 0, strlen(s));
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

typedef int (*PalFn)(const char *s);

static const struct {
    const char *name;
    PalFn fn;
} APPROACHES[] = {
    {"clean + reverse", is_pal_reverse},
    {"clean + two pointers", is_pal_clean_two_pointer},
    {"two pointers", is_pal_two_pointer},
    {"recursion", is_pal_recursive},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *s;
    const char *shown; /* printable form for the label */
    int expect;
} Case;

int main(void)
{
    printf("Q04. Valid Palindrome\n");

    /* Expected answers come from cleaning each string by hand, e.g.
     * "No 'x' in Nixon" -> "noxinnixon", which reads the same both ways. */
    const Case cases[] = {
        {"A man, a plan, a canal: Panama", "A man, a plan, a canal: Panama", 1},
        {"race a car", "race a car", 0},
        {"", "(empty)", 1},
        {" ", "(one space)", 1},
        {".,!?", ".,!?  (no alphanumerics)", 1},
        {"a", "a", 1},
        {"Aa", "Aa  (case ignored)", 1},
        {"ab", "ab", 0},
        {"0P", "0P  (digits count: '0' != 'p')", 0},
        {"12321", "12321", 1},
        {"1a2", "1a2", 0},
        {"ab_a", "ab_a  ('_' is not alphanumeric)", 1},
        {"a.b,B;A", "a.b,B;A", 1},
        {"No 'x' in Nixon", "No 'x' in Nixon", 1},
        {"Was it a car or a cat I saw?", "Was it a car or a cat I saw?", 1},
        {"abcd", "abcd", 0},
        /* Byte 0xFF is negative as a signed char; without the unsigned char
         * cast, isalnum() would be undefined behaviour here. The literal is
         * split because "\xffba" would read as the single escape \xffba. */
        {"ab\xff" "ba", "ab\\xffba  (high byte skipped)", 1},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[192];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->s);
            snprintf(label, sizeof label, "test %2d, %-20s \"%s\" -> %d", t + 1,
                     APPROACHES[a].name, c->shown, got);
            check(got == c->expect, label);
        }
    }

    /* Random cross-check against approach 1. Half the strings are built to
     * be palindromes (mirror the first half, then flip the case of random
     * letters), so both answers occur often. */
    {
        static const char alphabet[] = "aAbB1 ,.";
        const int alpha_len = (int)(sizeof alphabet - 1);
        srand(4242);
        int bad = 0, yes = 0, no = 0;
        const int rounds = 600;
        for (int r = 0; r < rounds; r++) {
            char s[16];
            int n = rand() % 15; /* 0..14 characters */
            for (int k = 0; k < n; k++)
                s[k] = alphabet[rand() % alpha_len];
            if (rand() % 2) {
                for (int k = 0; k < n / 2; k++)
                    s[n - 1 - k] = s[k];
                for (int k = 0; k < n; k++)
                    if (isalpha((unsigned char)s[k]) && rand() % 2)
                        s[k] = (char)(isupper((unsigned char)s[k]) ? tolower((unsigned char)s[k])
                                                                   : toupper((unsigned char)s[k]));
            }
            s[n] = '\0';

            int expect = is_pal_reverse(s);
            if (expect == 1)
                yes++;
            else if (expect == 0)
                no++;
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(s) != expect)
                    bad++;
        }
        snprintf(label, sizeof label,
                 "random cross-check: %d strings (%d palindromes, %d not), all approaches agree",
                 rounds, yes, no);
        check(bad == 0 && yes > 0 && no > 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
