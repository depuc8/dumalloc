/*
 * main.c — test driver for dumalloc.
 *
 * Runs a series of focused tests against my_malloc and my_free, printing
 * PASS / FAIL for each case and a summary at the end.
 */

#include <stdio.h>
#include <string.h>
#include "heap.h"

/* ── Test helpers ─────────────────────────────────────────────────────────── */

static int passed = 0;
static int failed = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("  PASS  %s\n", name);
        passed++;
    } else {
        printf("  FAIL  %s\n", name);
        failed++;
    }
}

/* ── Individual tests ─────────────────────────────────────────────────────── */

/* my_malloc with size 0 must return NULL. */
static void test_malloc_zero(void)
{
    void *p = my_malloc(0);
    check("malloc(0) returns NULL", p == NULL);
}

/* A normal allocation must return a non-NULL pointer. */
static void test_malloc_basic(void)
{
    void *p = my_malloc(32);
    check("malloc(32) returns non-NULL", p != NULL);
    my_free(p);
}

/* The returned pointer must be ALIGNMENT-byte aligned. */
static void test_alignment(void)
{
    void *p = my_malloc(1);
    check("payload is aligned", p != NULL && ((size_t)p % ALIGNMENT) == 0);
    my_free(p);
}

/* Writing to the allocation and reading it back must work without corruption. */
static void test_write_read(void)
{
    const char msg[] = "dumalloc works";
    char *p = my_malloc(sizeof(msg));
    if (!p) { check("write/read roundtrip", 0); return; }
    memcpy(p, msg, sizeof(msg));
    check("write/read roundtrip", memcmp(p, msg, sizeof(msg)) == 0);
    my_free(p);
}

/* Multiple independent allocations must return distinct, non-overlapping
 * pointers. */
static void test_multiple_allocs(void)
{
    void *a = my_malloc(16);
    void *b = my_malloc(16);
    void *c = my_malloc(16);
    check("multiple allocs return distinct pointers",
          a != NULL && b != NULL && c != NULL && a != b && b != c);
    my_free(a);
    my_free(b);
    my_free(c);
}

/* After freeing a block, re-allocating the same size should succeed (the
 * freed block must be reused rather than the heap being exhausted). */
static void test_reuse_after_free(void)
{
    void *a = my_malloc(128);
    my_free(a);
    void *b = my_malloc(128);
    check("freed block is reused", b != NULL);
    my_free(b);
}

/* Freeing NULL must be a safe no-op. */
static void test_free_null(void)
{
    my_free(NULL); /* must not crash */
    check("free(NULL) is a no-op", 1);
}

/* Exhaust the heap and confirm NULL is returned. */
static void test_heap_exhaustion(void)
{
    /* Drain the arena with small allocations. */
    while (my_malloc(ALIGNMENT))
        ;
    void *p = my_malloc(ALIGNMENT);
    check("malloc returns NULL when heap is full", p == NULL);
    /* Note: no free here — arena is intentionally exhausted. */
}

/* ── Entry point ──────────────────────────────────────────────────────────── */

int main(void)
{
    printf("dumalloc test suite\n");
    printf("-------------------\n");

    test_malloc_zero();
    test_malloc_basic();
    test_alignment();
    test_write_read();
    test_multiple_allocs();
    test_reuse_after_free();
    test_free_null();
    test_heap_exhaustion();

    printf("-------------------\n");
    printf("  %d passed, %d failed\n", passed, failed);

    return failed == 0 ? 0 : 1;
}
