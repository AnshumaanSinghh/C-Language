# C Language Practice

[![build-and-test](https://github.com/AnshumaanSinghh/C-Language/actions/workflows/ci.yml/badge.svg)](https://github.com/AnshumaanSinghh/C-Language/actions/workflows/ci.yml)

My C practice repo. It started with sorting algorithms and basic data structures. It now has **50 interview-style problems** (20 Easy, 18 Medium, 12 Hard). Each one is solved several ways, from brute force up to the optimal solution.

Each problem file has:

- **The problem statement**, an example, and a table of every approach with its time and space complexity.
- **Every approach, from brute force to optimal**, each with comments on the idea, why it works, and where people usually go wrong.
- **Built-in tests.** These include hand-checked cases, edge cases, and a random cross-check that compares every approach against the brute force on hundreds of inputs.

Every file is plain C11 with only the standard library. All of them compile with no warnings under `-Wall -Wextra -Wpedantic` and run clean under AddressSanitizer and UndefinedBehaviorSanitizer. CI checks this on every push with GCC and Clang on Linux and macOS.

## Quick start

```sh
make test          # build and run all 50 problems, print a pass/fail summary
make run Q=21      # build and run one problem by its number
make sanitize      # run all 50 under AddressSanitizer + UBSan
make list          # list every problem file
make clean
```

You can also build one file directly, with no Makefile:

```sh
cc -std=c11 -Wall -Wextra -O2 problems/easy/01_two_sum.c -o two_sum && ./two_sum
```

Running a problem prints one line per check:

```
Q01. Two Sum
  [PASS] test 1, brute force          -> (0, 1)
  [PASS] test 1, sort + two pointers  -> (0, 1)
  [PASS] test 1, hash map             -> (0, 1)
  ...
  [PASS] random cross-check: 500 arrays, all approaches agree
22/22 checks passed
```

On Windows, use WSL or MinGW-w64 `gcc`. The `make` targets need a POSIX shell.

## How to practise with this repo

1. Open a problem and read only the header: the problem, the example and the approach table. Then close the file and try it yourself.
2. Write the brute force first. It is almost always easy, and it gives you something to test the fast version against.
3. Look for the repeated work in the brute force. That is where the better approach comes from: hashing, sorting, two pointers, a stack, or a DP table.
4. Compare your code with the solutions in the file, then check the complexity table.
5. To test your own version, add your function to that file's `APPROACHES[]` table. The fixed tests and the random cross-check will run on it too.

## Problems

Complexity is the best approach's time / extra space. n is the input size.

### Easy (20)

| # | Problem | Topics | Approaches | Best time / space |
|---|---|---|---|---|
| 01 | [Two Sum](problems/easy/01_two_sum.c) | Arrays, Hashing, Two Pointers | Brute force · Sort + two pointers · Hash map | O(n) / O(n) |
| 02 | [Reverse an Array](problems/easy/02_reverse_array.c) | Arrays, Two Pointers | Temp array · Two-pointer swap · Recursion | O(n) / O(1) |
| 03 | [Find Minimum and Maximum](problems/easy/03_find_min_max.c) | Arrays, Divide and Conquer, Sorting | Sort a copy · Single pass · Pairwise comparison · Divide and conquer | O(n) / O(1) |
| 04 | [Valid Palindrome](problems/easy/04_palindrome_string.c) | Strings, Two Pointers, Recursion | Clean + reverse · Clean + two pointers · Two pointers in place · Recursion | O(n) / O(1) |
| 05 | [Valid Anagram](problems/easy/05_valid_anagram.c) | Strings, Hashing, Sorting, Counting | Brute-force matching · Sort both · Frequency count | O(n) / O(1) |
| 06 | [Fibonacci Number](problems/easy/06_fibonacci.c) | Recursion, Dynamic Programming, Math | Plain recursion · Memoization · Tabulation · Two variables · Fast doubling | O(log n) / O(1) |
| 07 | [Factorial Trailing Zeros](problems/easy/07_factorial_trailing_zeros.c) | Math, Number Theory | Compute n! · Count fives in 1..n · Legendre's formula | O(log n) / O(1) |
| 08 | [Check Prime](problems/easy/08_check_prime.c) | Math, Number Theory, Sieve | Trial division to n-1 · Trial division to √n · 6k ± 1 wheel · Sieve of Eratosthenes | O(√n) / O(1) |
| 09 | [GCD and LCM](problems/easy/09_gcd_lcm.c) | Math, Number Theory, Recursion, Bit Manipulation | Brute force · Euclid by subtraction · Euclid by modulo (iterative, recursive) · Binary GCD | O(log min(a, b)) / O(1) |
| 10 | [Pow(x, n)](problems/easy/10_power_function.c) | Math, Recursion, Binary Exponentiation | Naive loop · Recursive fast power · Binary exponentiation · Modular exponentiation | O(log n) / O(1) |
| 11 | [Missing Number](problems/easy/11_missing_number.c) | Arrays, Math, Bit Manipulation | Linear search · Sort · Seen array · Sum formula · XOR | O(n) / O(1) |
| 12 | [Move Zeroes](problems/easy/12_move_zeroes.c) | Arrays, Two Pointers | Extra array · Bubble zeros right · Compact + fill · Snowball swap | O(n) / O(1) |
| 13 | [Remove Duplicates from Sorted Array](problems/easy/13_remove_duplicates_sorted.c) | Arrays, Two Pointers | Extra array · Shift left · Two pointers · At most m copies (LC 80) | O(n) / O(1) |
| 14 | [Second Largest Distinct Element](problems/easy/14_second_largest.c) | Arrays, Sorting | Sort descending · Two passes · One pass | O(n) / O(1) |
| 15 | [Count Set Bits](problems/easy/15_count_set_bits.c) | Bit Manipulation, Lookup Table, DP | Test every bit · Brian Kernighan · Lookup table · SWAR · Counting bits DP (LC 338) | O(1) / O(1) |
| 16 | [Single Number](problems/easy/16_single_number.c) | Arrays, Hashing, Sorting, Bit Manipulation | Brute force · Sort · Hash map · XOR | O(n) / O(1) |
| 17 | [Best Time to Buy and Sell Stock](problems/easy/17_best_time_stock.c) | Arrays, Greedy, DP | Brute force · Suffix max · Min so far · Kadane on differences | O(n) / O(1) |
| 18 | [Valid Parentheses](problems/easy/18_valid_parentheses.c) | Strings, Stack | Remove pairs · Stack · Depth counter ("()" only) | O(n) / O(n) |
| 19 | [Reverse Linked List](problems/easy/19_reverse_linked_list.c) | Linked List, Recursion, Stack | Reverse values · Stack of nodes · Three pointers · Recursion | O(n) / O(1) |
| 20 | [Merge Sorted Array](problems/easy/20_merge_sorted_arrays.c) | Arrays, Two Pointers, Sorting | Insert + shift · Append + qsort · Buffer merge · Back merge | O(m+n) / O(1) |

### Medium (18)

| # | Problem | Topics | Approaches | Best time / space |
|---|---|---|---|---|
| 21 | [Maximum Subarray Sum](problems/medium/21_max_subarray_sum.c) | Arrays, DP, Divide and Conquer | Triple loop · Running sum · Divide and conquer · Kadane | O(n) / O(1) |
| 22 | [Rotate Array](problems/medium/22_rotate_array.c) | Arrays, Two Pointers, Math | Rotate by one k times · Extra array · Reversal · Juggling (gcd cycles) | O(n) / O(1) |
| 23 | [Sort Colors](problems/medium/23_sort_colors.c) | Arrays, Two Pointers, Sorting, Counting | qsort · Counting two-pass · Dutch National Flag | O(n) / O(1) |
| 24 | [Majority Element](problems/medium/24_majority_element.c) | Arrays, Hashing, Bit Manipulation, Voting | Brute count · Sort + middle · Hash map · Bit voting · Boyer–Moore | O(n) / O(1) |
| 25 | [3Sum](problems/medium/25_three_sum.c) | Arrays, Two Pointers, Sorting, Hashing | Brute force + dedupe · Sort + hash set · Sort + two pointers | O(n²) / O(1) |
| 26 | [Longest Substring Without Repeating Characters](problems/medium/26_longest_substring_no_repeat.c) | Strings, Sliding Window, Hashing | Every substring · Extend each start · Window + set · Last-seen jump | O(n) / O(1) |
| 27 | [Product of Array Except Self](problems/medium/27_product_except_self.c) | Arrays, Prefix Products | Brute force · Division · Prefix + suffix arrays · Running suffix | O(n) / O(1) |
| 28 | [Subarray Sum Equals K](problems/medium/28_subarray_sum_k.c) | Arrays, Prefix Sums, Hashing, Sliding Window | Triple loop · Running sum · Prefix sum + hash map · Sliding window (non-negative only) | O(n) / O(n) |
| 29 | [Search in Rotated Sorted Array](problems/medium/29_search_rotated_array.c) | Arrays, Binary Search | Linear scan · Pivot + binary search · One-pass binary search | O(log n) / O(1) |
| 30 | [Kth Largest Element](problems/medium/30_kth_largest.c) | Sorting, Heap, Quickselect | Sort · Partial selection · Min-heap of size k · Quickselect | O(n) avg / O(1) |
| 31 | [Merge Intervals](problems/medium/31_merge_intervals.c) | Arrays, Sorting, Intervals | Repeated pairwise merge · Sort + sweep | O(n log n) / O(1) |
| 32 | [Linked List Cycle](problems/medium/32_linked_list_cycle.c) | Linked List, Two Pointers, Hashing | Visited array · Hash set of pointers · Floyd's tortoise and hare · Brent | O(n) / O(1) |
| 33 | [Coin Change](problems/medium/33_coin_change.c) | DP, BFS, Greedy (counterexample) | Plain recursion · Memoization · Bottom-up DP · BFS over amounts | O(amount·c) / O(amount) |
| 34 | [Longest Common Subsequence](problems/medium/34_longest_common_subsequence.c) | Strings, DP | Plain recursion · Memoization · Tabulation + rebuild · Two rows | O(mn) / O(min(m,n)) |
| 35 | [Set Matrix Zeroes](problems/medium/35_set_matrix_zeroes.c) | Arrays, Matrix, In-place | Copy the matrix · Row/column flags · First row/col as markers | O(mn) / O(1) |
| 36 | [Longest Palindromic Substring](problems/medium/36_longest_palindromic_substring.c) | Strings, DP, Two Pointers | Brute force · DP table · Expand around centre · Manacher | O(n) / O(n) |
| 37 | [Number of Islands](problems/medium/37_number_of_islands.c) | Graphs, DFS, BFS, Union-Find, Matrix | Recursive DFS · BFS · Iterative DFS · Union-Find | O(mn) / O(mn) |
| 38 | [Find the Duplicate Number](problems/medium/38_find_duplicate_number.c) | Arrays, Binary Search, Cycle Detection | Brute pairs · Sort a copy · Seen array · Negative marking · Binary search on values · Floyd cycle detection | O(n) / O(1) |

### Hard (12)

| # | Problem | Topics | Approaches | Best time / space |
|---|---|---|---|---|
| 39 | [Trapping Rain Water](problems/hard/39_trapping_rain_water.c) | Arrays, Two Pointers, Stack | Per-bar scan · Prefix/suffix max · Monotonic stack · Two pointers | O(n) / O(1) |
| 40 | [Median of Two Sorted Arrays](problems/hard/40_median_two_sorted_arrays.c) | Arrays, Binary Search, Divide and Conquer | Merge · Walk to the middle · K-th smallest · Partition search | O(log min(m,n)) / O(1) |
| 41 | [Largest Rectangle in Histogram](problems/hard/41_largest_rectangle_histogram.c) | Arrays, Monotonic Stack, Divide and Conquer | Every pair · Divide and conquer · Nearest smaller arrays · One-pass stack | O(n) / O(n) |
| 42 | [Sliding Window Maximum](problems/hard/42_sliding_window_maximum.c) | Arrays, Sliding Window, Heap, Monotonic Queue | Brute force · Max-heap with lazy deletion · Block decomposition · Monotonic deque | O(n) / O(k) |
| 43 | [N-Queens](problems/hard/43_n_queens.c) | Backtracking, Recursion, Bit Manipulation | Brute force · Backtracking + scan · Backtracking + flags · Bitmask | O(n!) / O(n) |
| 44 | [Edit Distance](problems/hard/44_edit_distance.c) | Strings, DP, Recursion | Plain recursion · Memoization · Tabulation · One row | O(mn) / O(min(m,n)) |
| 45 | [Longest Increasing Subsequence](problems/hard/45_longest_increasing_subsequence.c) | DP, Binary Search | Recursion · Memoization · O(n²) DP · Patience sorting | O(n log n) / O(n) |
| 46 | [Minimum Window Substring](problems/hard/46_minimum_window_substring.c) | Strings, Sliding Window, Hashing | Brute force · Extend from each start · Sliding window · Filtered sliding window | O(\|s\| + \|t\|) / O(1) |
| 47 | [Merge k Sorted Lists](problems/hard/47_merge_k_sorted_lists.c) | Linked List, Heap, Divide and Conquer | Collect + qsort · Merge one by one · Scan k heads · Min-heap · Divide and conquer | O(N log k) / O(log k) |
| 48 | [First Missing Positive](problems/hard/48_first_missing_positive.c) | Arrays, Hashing, Cyclic Sort | Brute force · Sort a copy · Boolean array · Cyclic sort · Sign marking | O(n) / O(1) |
| 49 | [LRU Cache](problems/hard/49_lru_cache.c) | Design, Hashing, Linked List | Timestamps · Recency-ordered array · Hash map + doubly linked list | O(1) per op / O(capacity) |
| 50 | [Count Inversions](problems/hard/50_count_inversions.c) | Divide and Conquer, Merge Sort, Fenwick Tree | Brute force · Insertion sort · Merge sort · Fenwick tree | O(n log n) / O(n) |

## Problems by pattern

| Pattern | Problems |
|---|---|
| Two pointers | 01, 02, 04, 12, 13, 20, 22, 23, 25, 39 |
| Hashing | 01, 05, 16, 24, 25, 28, 32, 49 |
| Sliding window | 26, 42, 46 |
| Prefix sums / products | 17, 27, 28, 39 |
| Binary search | 29, 38, 40, 45 |
| Sorting and selection | 14, 23, 30, 31 |
| Stack, monotonic stack and deque | 18, 39, 41, 42 |
| Heap | 30, 42, 47 |
| Linked lists | 19, 32, 47, 49 |
| Cycle detection (Floyd) | 32, 38 |
| Dynamic programming | 06, 17, 21, 33, 34, 36, 44, 45 |
| Recursion and backtracking | 06, 43, 44 |
| Divide and conquer | 03, 21, 40, 41, 47, 50 |
| Graphs (DFS, BFS, Union-Find) | 37 |
| Bit manipulation | 11, 15, 16, 24, 43 |
| Math and number theory | 07, 08, 09, 10 |
| Design | 49 |

## Original practice files

These are the files the repo started with, kept as they were.

| File | Topic |
|---|---|
| [bubblesort.c](bubblesort.c) | Bubble sort |
| [selectionsort.c](selectionsort.c) | Selection sort |
| [insertionsort.c](insertionsort.c) | Insertion sort |
| [mergesort.c](mergesort.c) | Merge sort |
| [quicksort.c](quicksort.c) | Quick sort |
| [implementationINArray.C](implementationINArray.C) | Array insertion and deletion |
| [linkedlist.c](linkedlist.c) | Linked list traversal |
| [stack1.c](stack1.c) | Stack using an array |
| [queueusingarray.c](queueusingarray.c) | Queue using an array |
| [circularqueue.c](circularqueue.c) | Circular queue (in progress) |

## Repository layout

```
problems/
  easy/      01–20
  medium/    21–38
  hard/      39–50
scripts/run_all.sh    runs every built problem and prints the summary
Makefile              build, test, sanitize, run one problem
.github/workflows/    CI: GCC and Clang on Linux, Clang on macOS
```
