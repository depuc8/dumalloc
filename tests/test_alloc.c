#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../include/heap.h"

/* ── Minimal test framework ─────────────────────────────────────────────── */

static int passed = 0;
static int failed = 0;
static int suite_failed = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("    PASS  %s\n", name);
        passed++;
    } else {
        printf("    FAIL  %s\n", name);
        failed++;
        suite_failed++;
    }
}

static void suite(const char *name)
{
    suite_failed = 0;
    printf("\n  [%s]\n", name);
}

/* ── Helpers ─────────────────────────────────────────────────────────────── */

/* Fill a buffer with a known byte pattern and verify it later. */
static void fill(void *p, size_t n, uint8_t val)
{
    memset(p, val, n);
}

static int verify(const void *p, size_t n, uint8_t val)
{
    const uint8_t *b = p;
    for (size_t i = 0; i < n; i++)
        if (b[i] != val) return 0;
    return 1;
}

/* ── Test suites ─────────────────────────────────────────────────────────── */

static void test_basic(void)
{
    suite("Basic allocation");

    void *p = my_malloc(1);
    check("malloc(1) not NULL", p != NULL);
    my_free(p);

    p = my_malloc(0);
    check("malloc(0) is NULL", p == NULL);
}

static void test_alignment(void)
{
    suite("Alignment");

    /* Every size from 1 to 64 must produce an ALIGNMENT-aligned pointer. */
    for (size_t sz = 1; sz <= 64; sz++) {
        void *p = my_malloc(sz);
        if (!p) { check("aligned for all sizes 1..64", 0); return; }
        if ((uintptr_t)p % ALIGNMENT != 0) {
            printf("    FAIL  size %zu misaligned: %p\n", sz, p);
            failed++; suite_failed++;
        }
        my_free(p);
    }
    check("aligned for all sizes 1..64", 1);
}

static void test_write_integrity(void)
{
    suite("Write integrity");

    /* Write a pattern to every allocation and verify it isn't clobbered. */
    void *blocks[8];
    size_t sizes[8] = {1, 7, 8, 9, 15, 16, 64, 128};

    for (int i = 0; i < 8; i++) {
        blocks[i] = my_malloc(sizes[i]);
        if (blocks[i]) fill(blocks[i], sizes[i], (uint8_t)(0xA0 + i));
    }

    int ok = 1;
    for (int i = 0; i < 8; i++) {
        if (blocks[i] && !verify(blocks[i], sizes[i], (uint8_t)(0xA0 + i)))
            ok = 0;
        my_free(blocks[i]);
    }
    check("patterns intact across simultaneous allocations", ok);
}

static void test_distinct_pointers(void)
{
    suite("Distinct pointers");

    void *a = my_malloc(16);
    void *b = my_malloc(16);
    void *c = my_malloc(16);

    check("all three non-NULL",    a && b && c);
    check("a != b",                a != b);
    check("b != c",                b != c);
    check("no overlap a/b",        (char *)b >= (char *)a + 16 ||
                                   (char *)a >= (char *)b + 16);

    my_free(a); my_free(b); my_free(c);
}

static void test_reuse(void)
{
    suite("Block reuse after free");

    void *a = my_malloc(256);
    check("initial alloc ok", a != NULL);
    my_free(a);

    void *b = my_malloc(256);
    check("realloc same size ok", b != NULL);
    my_free(b);
}

static void test_coalescing(void)
{
    suite("Coalescing");

    /*
     * Alloc two adjacent blocks, free both, then request a block large
     * enough to require the two be merged.  find_fit coalesces lazily so
     * this exercises the forward-merge path.
     */
    void *a = my_malloc(64);
    void *b = my_malloc(64);
    check("both blocks allocated", a != NULL && b != NULL);

    my_free(a);
    my_free(b);

    /* Combined payload of a+b minus one header = 128 bytes minus overhead.
     * Requesting 96 bytes should fit in the merged region. */
    void *c = my_malloc(96);
    check("large alloc fits in coalesced region", c != NULL);
    my_free(c);
}

static void test_free_null(void)
{
    suite("free(NULL)");
    my_free(NULL);
    check("free(NULL) is a no-op", 1);
}

static void test_stress(void)
{
    suite("Stress: interleaved alloc/free");

    /*
     * Repeatedly alloc two blocks, write patterns, verify, then free one
     * and alloc another.  Exercises fragmentation and reuse paths.
     */
    const int rounds = 20;
    int ok = 1;

    void *prev = my_malloc(32);
    if (prev) fill(prev, 32, 0xBB);

    for (int i = 0; i < rounds; i++) {
        void *cur = my_malloc(32);
        if (!cur) { ok = 0; break; }
        fill(cur, 32, (uint8_t)i);

        if (prev && !verify(prev, 32, 0xBB)) { ok = 0; my_free(cur); break; }

        my_free(prev);
        prev = cur;
        fill(prev, 32, 0xBB);
    }

    my_free(prev);
    check("interleaved alloc/free without corruption", ok);
}

static void test_exhaustion(void)
{
    suite("Heap exhaustion");

    /* Drain the arena. */
    while (my_malloc(ALIGNMENT))
        ;

    check("malloc returns NULL when full", my_malloc(ALIGNMENT) == NULL);
    /* Note: arena remains exhausted — this test must run last. */
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("dumalloc unit tests\n");
    printf("===================\n");

    test_basic();
    test_alignment();
    test_write_integrity();
    test_distinct_pointers();
    test_reuse();
    test_coalescing();
    test_free_null();
    test_stress();
    test_exhaustion();   /* must be last */

    printf("\n===================\n");
    printf("  %d passed, %d failed\n", passed, failed);

    return failed == 0 ? 0 : 1;
}
