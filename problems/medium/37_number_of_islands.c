/*
 * Q37. Number of Islands
 * Difficulty : Medium
 * Topics     : Graphs, DFS, BFS, Union-Find, Matrix
 *
 * Problem
 *   Given a rows x cols grid of '1' (land) and '0' (water), count the
 *   islands. An island is a group of land cells connected up, down, left
 *   or right (not diagonally). The grid is stored row-major in a flat char
 *   array: cell (r, c) is grid[r * cols + c]. The grid must not be changed.
 *
 * Example
 *   1 1 0 0 0
 *   1 1 0 0 0
 *   0 0 1 0 0       ->   3
 *   0 0 0 1 1
 *
 * Approaches
 *   1. Recursive DFS             sink each island on a copy        Time O(mn)     Space O(mn)
 *   2. BFS with a queue          flood each island level by level  Time O(mn)     Space O(mn)
 *   3. Iterative DFS  [optimal]  explicit stack, no deep recursion Time O(mn)     Space O(mn)
 *   4. Union-Find                merge neighbouring land cells     Time O(mn*a)   Space O(mn)
 *
 *   All four are linear; 3 is marked optimal because it is as fast as 1
 *   without 1's risk of overflowing the call stack. a = inverse Ackermann,
 *   at most 4 for any grid that fits in memory.
 *
 * Key insight
 *   Scan the grid. Every time you meet land that no earlier flood reached,
 *   you have found a new island: count it and flood all of it, so none of
 *   its other cells is counted again. Islands = connected components.
 *
 * Build & run
 *   cc -std=c11 -Wall -Wextra -O2 37_number_of_islands.c -o number_of_islands && ./number_of_islands
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int DR[4] = {-1, 1, 0, 0};
static const int DC[4] = {0, 0, -1, 1};

static char *copy_grid(const char *grid, int rows, int cols)
{
    size_t cells = (size_t)rows * (size_t)cols;
    char *g = malloc(cells > 0 ? cells : 1);
    if (g != NULL && cells > 0)
        memcpy(g, grid, cells);
    return g;
}

/* -------------------------------------------------------------------------
 * Approach 1: Recursive DFS, sinking a copy
 *   Copy the grid so the caller's is untouched. Scan it; on each '1',
 *   count an island and "sink" it: turn the cell to '0' and recurse into
 *   the four neighbours. Sinking doubles as the visited mark, so no cell
 *   is processed twice.
 *   Pitfall: recursion depth can reach rows * cols (one long snake, or a
 *   grid that is all land). A 1000 x 1000 land grid means up to a million
 *   nested calls, which overflows a typical 8 MB stack. Approaches 2 and 3
 *   fix that by keeping the pending cells on the heap.
 *   Time O(mn)   Space O(mn) for the copy + O(mn) recursion depth worst case
 * ------------------------------------------------------------------------- */
static void sink(char *g, int rows, int cols, int r, int c)
{
    if (r < 0 || r >= rows || c < 0 || c >= cols || g[r * cols + c] != '1')
        return;
    g[r * cols + c] = '0';
    for (int d = 0; d < 4; d++)
        sink(g, rows, cols, r + DR[d], c + DC[d]);
}

static int islands_dfs_recursive(const char *grid, int rows, int cols)
{
    char *g = copy_grid(grid, rows, cols);
    if (g == NULL)
        return -1;
    int count = 0;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (g[r * cols + c] == '1') {
                count++;
                sink(g, rows, cols, r, c);
            }
        }
    }
    free(g);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 2: BFS with an explicit queue
 *   Same scan, but each island is flooded with a queue of cell indices
 *   (r * cols + c): pop a cell, push its unvisited land neighbours.
 *   Pitfall: mark a cell visited when it is PUSHED, not when it is popped.
 *   Otherwise the same cell can be pushed by several neighbours before it
 *   is popped, which wastes work and can overflow a queue sized m*n.
 *   Marked on push, each cell enters the queue at most once, so an m*n
 *   queue is always enough.
 *   Time O(mn)   Space O(mn) for the visited flags and the queue
 * ------------------------------------------------------------------------- */
static int islands_bfs(const char *grid, int rows, int cols)
{
    size_t cells = (size_t)rows * (size_t)cols;
    unsigned char *seen = calloc(cells > 0 ? cells : 1, 1);
    int *queue = malloc((cells > 0 ? cells : 1) * sizeof *queue);
    int count = -1;
    if (seen != NULL && queue != NULL) {
        count = 0;
        for (int start = 0; start < rows * cols; start++) {
            if (grid[start] != '1' || seen[start])
                continue;
            count++;
            int head = 0, tail = 0;
            queue[tail++] = start;
            seen[start] = 1;
            while (head < tail) {
                int cell = queue[head++];
                int r = cell / cols, c = cell % cols;
                for (int d = 0; d < 4; d++) {
                    int nr = r + DR[d], nc = c + DC[d];
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols)
                        continue;
                    int next = nr * cols + nc;
                    if (grid[next] == '1' && !seen[next]) {
                        seen[next] = 1;
                        queue[tail++] = next;
                    }
                }
            }
        }
    }
    free(seen);
    free(queue);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 3: Iterative DFS with an explicit stack
 *   Exactly approach 2 with the queue replaced by a stack (push and pop at
 *   the same end), which visits cells in depth-first order like approach 1
 *   but without recursion. The "call stack" is now a heap array of m*n
 *   ints, so the only limit is memory, not the few MB the OS gives the
 *   call stack. Same rule: mark on push, so each cell is pushed once.
 *   For counting components the visit order does not matter; DFS and BFS
 *   give the same count.
 *   Time O(mn)   Space O(mn)
 * ------------------------------------------------------------------------- */
static int islands_dfs_iterative(const char *grid, int rows, int cols)
{
    size_t cells = (size_t)rows * (size_t)cols;
    unsigned char *seen = calloc(cells > 0 ? cells : 1, 1);
    int *stack = malloc((cells > 0 ? cells : 1) * sizeof *stack);
    int count = -1;
    if (seen != NULL && stack != NULL) {
        count = 0;
        for (int start = 0; start < rows * cols; start++) {
            if (grid[start] != '1' || seen[start])
                continue;
            count++;
            int top = 0;
            stack[top++] = start;
            seen[start] = 1;
            while (top > 0) {
                int cell = stack[--top];
                int r = cell / cols, c = cell % cols;
                for (int d = 0; d < 4; d++) {
                    int nr = r + DR[d], nc = c + DC[d];
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols)
                        continue;
                    int next = nr * cols + nc;
                    if (grid[next] == '1' && !seen[next]) {
                        seen[next] = 1;
                        stack[top++] = next;
                    }
                }
            }
        }
    }
    free(seen);
    free(stack);
    return count;
}

/* -------------------------------------------------------------------------
 * Approach 4: Union-Find (disjoint set union)
 *   Start with every land cell as its own set, so count = number of land
 *   cells. For each land cell, union it with its land neighbour to the
 *   right and below (left and up were already handled from the other
 *   side). Each union that joins two DIFFERENT sets lowers count by one.
 *   What remains is the number of sets, i.e. islands.
 *   Path compression: find() makes every node on the path point straight
 *   at the root, so later finds are short. Union by rank: hang the
 *   shallower tree under the deeper one, so trees stay flat. Together they
 *   make each operation O(a(mn)), effectively constant.
 *   Why bother: no traversal at all, and it works ONLINE. If land cells
 *   are added one at a time (LeetCode 305), each addition is just a few
 *   unions, while DFS/BFS would have to rescan.
 *   Time O(mn * a(mn))   Space O(mn)
 * ------------------------------------------------------------------------- */
static int uf_find(int *parent, int x)
{
    int root = x;
    while (parent[root] != root)
        root = parent[root];
    while (parent[x] != root) { /* second pass: point the path at the root */
        int next = parent[x];
        parent[x] = root;
        x = next;
    }
    return root;
}

/* Returns 1 if x and y were in different sets (and are now joined). */
static int uf_union(int *parent, unsigned char *rank, int x, int y)
{
    int rx = uf_find(parent, x), ry = uf_find(parent, y);
    if (rx == ry)
        return 0;
    if (rank[rx] < rank[ry]) {
        int t = rx;
        rx = ry;
        ry = t;
    }
    parent[ry] = rx;
    if (rank[rx] == rank[ry])
        rank[rx]++;
    return 1;
}

static int islands_union_find(const char *grid, int rows, int cols)
{
    size_t cells = (size_t)rows * (size_t)cols;
    int *parent = malloc((cells > 0 ? cells : 1) * sizeof *parent);
    unsigned char *rank = calloc(cells > 0 ? cells : 1, 1);
    int count = -1;
    if (parent != NULL && rank != NULL) {
        count = 0;
        for (int i = 0; i < rows * cols; i++) {
            parent[i] = i;
            if (grid[i] == '1')
                count++;
        }
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                int i = r * cols + c;
                if (grid[i] != '1')
                    continue;
                if (c + 1 < cols && grid[i + 1] == '1')
                    count -= uf_union(parent, rank, i, i + 1);
                if (r + 1 < rows && grid[i + cols] == '1')
                    count -= uf_union(parent, rank, i, i + cols);
            }
        }
    }
    free(parent);
    free(rank);
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

typedef int (*IslandsFn)(const char *grid, int rows, int cols);

/* max_cells: largest grid an approach is run on here. The recursive DFS is
 * kept off the big grids on purpose (see its pitfall). */
static const struct {
    const char *name;
    IslandsFn fn;
    int max_cells;
} APPROACHES[] = {
    {"recursive DFS", islands_dfs_recursive, 400},
    {"BFS", islands_bfs, 1 << 30},
    {"iterative DFS", islands_dfs_iterative, 1 << 30},
    {"union-find", islands_union_find, 1 << 30},
};
#define NUM_APPROACHES ((int)(sizeof APPROACHES / sizeof APPROACHES[0]))

typedef struct {
    const char *what;
    int rows, cols;
    const char *grid; /* rows * cols characters, row by row */
    int expect;
} Case;

int main(void)
{
    printf("Q37. Number of Islands\n");

    /* Expected counts worked out by hand from each picture. */
    const Case cases[] = {
        {"one big island", 4, 5,
         "11110"
         "11010"
         "11000"
         "00000", 1},
        {"three islands", 4, 5,
         "11000"
         "11000"
         "00100"
         "00011", 3},
        {"empty grid", 0, 0, "", 0},
        {"single land", 1, 1, "1", 1},
        {"single water", 1, 1, "0", 0},
        {"all water", 3, 3, "000000000", 0},
        {"all land", 3, 4, "111111111111", 1},
        {"checkerboard (no diagonals)", 3, 3,
         "101"
         "010"
         "101", 5},
        {"S-shaped snake", 5, 3,
         "111"
         "001"
         "111"
         "100"
         "111", 1},
        {"U shape joins at bottom", 3, 3,
         "101"
         "101"
         "111", 1},
        {"single row", 1, 5, "10101", 3},
        {"single column", 4, 1, "1101", 2},
        {"ring with a lake", 3, 3,
         "111"
         "101"
         "111", 1},
        {"island inside a lake", 5, 5,
         "11111"
         "10001"
         "10101"
         "10001"
         "11111", 2},
    };
    const int num_cases = (int)(sizeof cases / sizeof cases[0]);

    char label[128];
    for (int t = 0; t < num_cases; t++) {
        const Case *c = &cases[t];
        for (int a = 0; a < NUM_APPROACHES; a++) {
            int got = APPROACHES[a].fn(c->grid, c->rows, c->cols);
            snprintf(label, sizeof label, "test %2d %-28s, %-14s -> %d", t + 1, c->what,
                     APPROACHES[a].name, got);
            check(got == c->expect, label);
        }
    }

    /* Big grids: 300 x 300 all land (1 island; a recursive DFS would nest
     * 90000 calls deep) and 300 x 300 with land on even rows only (rows 0,
     * 2, ..., 298 are separate strips: 150 islands). */
    {
        enum { R = 300, C = 300 };
        char *land = malloc(R * C), *strips = malloc(R * C);
        if (land != NULL && strips != NULL) {
            memset(land, '1', R * C);
            for (int r = 0; r < R; r++)
                memset(strips + r * C, r % 2 == 0 ? '1' : '0', C);
            for (int a = 0; a < NUM_APPROACHES; a++) {
                if (R * C > APPROACHES[a].max_cells)
                    continue;
                int g1 = APPROACHES[a].fn(land, R, C);
                int g2 = APPROACHES[a].fn(strips, R, C);
                snprintf(label, sizeof label, "%dx%d all land / strips, %-14s -> %d / %d", R, C,
                         APPROACHES[a].name, g1, g2);
                check(g1 == 1 && g2 == R / 2, label);
            }
        } else {
            check(0, "allocation for big grids");
        }
        free(land);
        free(strips);
    }

    /* The grid must be left exactly as it was. */
    {
        const char orig[] = "110011001";
        char grid[10];
        memcpy(grid, orig, sizeof grid);
        for (int a = 0; a < NUM_APPROACHES; a++)
            (void)APPROACHES[a].fn(grid, 3, 3);
        check(memcmp(grid, orig, sizeof grid) == 0, "grid unchanged by every approach");
    }

    /* Random cross-check on grids up to 12 x 12 with a random land density
     * per grid (20% to 80%), every approach against the recursive DFS. */
    {
        srand(37);
        int bad = 0;
        const int rounds = 500;
        for (int r = 0; r < rounds; r++) {
            char grid[144];
            int rows = 1 + rand() % 12, cols = 1 + rand() % 12;
            int density = 20 + rand() % 61;
            for (int i = 0; i < rows * cols; i++)
                grid[i] = rand() % 100 < density ? '1' : '0';
            int expect = islands_dfs_recursive(grid, rows, cols);
            for (int a = 0; a < NUM_APPROACHES; a++)
                if (APPROACHES[a].fn(grid, rows, cols) != expect)
                    bad++;
        }
        snprintf(label, sizeof label, "random cross-check: %d grids, all approaches agree", rounds);
        check(bad == 0, label);
    }

    printf("%d/%d checks passed\n", tests_run - tests_failed, tests_run);
    return tests_failed ? 1 : 0;
}
